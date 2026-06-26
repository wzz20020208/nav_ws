import http from 'http';
import path from 'path';
import fs from 'fs';
import yaml from 'js-yaml';
import { WebSocketServer } from 'ws';
import { URL } from 'url';
import { sendGamepadToLCM } from './gamepadToLCM.js';
import {
	NUC_CONSTANT,
	BODY_CONSTANT,
	NUC_HOMEDIR,
	HEAD_HOMEDIR,
	BODY_HOMEDIR,
	LOCAL_CONSTANT,
	DETECTION_CONSTANT,
	JETSON_CONSTANT,
	runTeriminateCmd,
	runRemoteScript,
	runBootupInBackground,
	runRecoveryLimitScript,
	run_map_script,
	restartService,
	restartLocalService,
	getMapData,
	PCManager,
	updateSettingsConf,
	delay,
	parseSettingConf,
	restartDetectionDetectScreen,
} from './bashUtil.js';
import { exec, execSync } from 'child_process';
import { checkServiceStatus } from './bashUtil.js';
import { sendReply, sendSegmentRply, sendResponse } from '../server/serverUtil.js';

/* -- single master control -- */
import { getDeviceInfo } from '../server/serverUtil.js';
import { dbManager } from '../sqlite/db.js';
// const MASTER_CONTROL_LEVEL = 7;
// const heartbeatMap = new Map();

/* -- websocket server -- */
const server = http.createServer();
const PORT = process.env.PORT || 9002;
const wssRealtime = new WebSocketServer({ noServer: true });
const PGM_MAP_DIR = NUC_HOMEDIR + '/THEMIS/THEMIS/Beta/assets/PGM';

const pcManager = new PCManager(NUC_CONSTANT, JETSON_CONSTANT, BODY_CONSTANT);

const UPDATE_SETTING_CONF_MAP = {
	update_manip_velocity: { key: 'MANIPULATION_VELOCITY' },
	update_placeSideOffset: { key: 'PLACE_SIDE_OFFSET' },
	update_placeYawOffsetDeg: { key: 'PLACE_YAW_OFFSET_DEG' },
	update_manip_offset: {
		key: 'MANIPULATION_OFFSET',
		format: (v) => `${v.x} ${v.y} ${v.z}`,
	},
	update_aPointOffset: {
		key: 'A_POINT_OFFSET',
		format: (v) => `${v.x} ${v.y}`,
	},
	update_bPointOffset: {
		key: 'B_POINT_OFFSET',
		format: (v) => `${v.x} ${v.y}`,
	},
};

//  setInterval(() => {
// 	const now = Date.now();
// 	const timeout = 6000;
// 	let hasChanged = false;
// 	heartbeatMap.forEach((lastHeartbeat, deviceId) => {
// 		if (now - lastHeartbeat > timeout) {
// 			dbManager.markDeviceDisconnected(deviceId);
// 			heartbeatMap.delete(deviceId);
// 			hasChanged = true;
// 			console.log(`Device ${deviceId} marked as disconnected due to heartbeat timeout ${timeout}.`);
// 		}
// 	});

// 	if (hasChanged) {
// 		broadcast(wssRealtime, {
// 			type: "online_devices",
// 			data: dbManager.getAllOnlineDevices(),
// 		});
// 	}
// }, 5000);

//publish click_coordinates
let onVideoClickCallback = null;
const setVideoClickHandler = (fn) => {
	onVideoClickCallback = fn;
};

let onNavClickCallback = null;
const setNavClickHandler = (fn) => {
	onNavClickCallback = fn;
};

let onUngraspTargetCallback = null;
const setUngraspTargetCallback = (fn) => {
	onUngraspTargetCallback = fn;
};

let onSendFixpointOffsetCallback = null;
const setFixpointOffset = (fn) => {
	onSendFixpointOffsetCallback = fn;
};

let onPgmClickCallback = null;
const setPgmClickHandler = (fn) => {
	onPgmClickCallback = fn;
};

//websocket server...

wssRealtime.on('connection', function connection(ws, req) {
	ws.isAlive = true;
	ws.deviceId = null;
	console.log('Found Client');

	ws.on('pong', () => {
		ws.isAlive = true;
	});

	const interval = setInterval(() => {
		if (!ws.isAlive) {
			ws.terminate();
			return;
		}
		ws.isAlive = false;
		ws.ping();
	}, 3000);

	ws.on('message', async (rawMessage) => {
		try {
			let payload = rawMessage;
			if (!payload) {
				console.error('[GAMEPAD_SERVER > realtime] Error: Received empty payload!');
				return;
			}
			let j;
			try {
				j = JSON.parse(payload);
			} catch (e) {
				console.error('[GAMEPAD_SERVER > realtime] Error: Failed to parse JSON', e.message);
				return;
			}
			if (!j.data) j.data = {};

			if (!j.type) {
				console.error('[GAMEPAD_SERVER > realtime] Missing type in message!');
				return;
			}
			const type = j.type;
			const settingConf_rq = UPDATE_SETTING_CONF_MAP[type];

			/*@@@@@ single master control logic */

			/*
				1.Everytime got req with type "identify_id" from new device,
					first thing needs to check if this device info is online,
				2.if its online, no need to do anything
				3.if not online, check if master exists,
					if the master was there then save as guest else save as master
					if master not there, save it as master
			*/

			/* a disconnect request will update disconnectAt time with device id where disconnectAt is null */
			/* server send the connected online devices infomation to all the client */

			// =================================================
			// 1.heart beat && identify id
			// =================================================

			if (type === 'identify_id') {
				if (!j.deviceId) {
					console.log(
						'[GAMEPAD_SERVER > realtime] Missing deviceId in identify_id message!'
					);
					return;
				}

				ws.deviceId = j.deviceId;

				const header = req.headers['user-agent'];
				const info = getDeviceInfo(header, req);

				const device = {
					type: info.device || 'unknow',
					browser: info.browser || 'unknow',
					os: info.os || 'unknow',
					ip: info.ip || 'unknow',
					deviceId: j.deviceId,
					connectAt: new Date().toISOString() || null,
					disconnectAt: null,
					controlLevel: 0,
				};

				ws.deviceId = j.deviceId;
				dbManager.saveDevice(device);
				console.log(
					`[GAMEPAD_SERVER] Device connected: ${j.deviceId} ${device.ip} ${device.type} ${device.os}`
				);

				// try {
				// 	//check is online
				// 	const isOnline = dbManager.isOnline(j.deviceId);
				// 	if (!isOnline) {
				// 		const isMasterOnline = dbManager.isMasterOnline();
				// 		if (!isMasterOnline) {
				// 			device.controlLevel = MASTER_CONTROL_LEVEL; //master
				// 		} else {
				// 			device.controlLevel = 0; //guest
				// 		}
				// 		//save new device and tell everyone
				// 		dbManager.saveDevice(device);
				// 		broadcast(wssRealtime, {
				// 			type: 'online_devices',
				// 			data: dbManager.getAllOnlineDevices(),
				// 		});
				// 		console.log(`Device save with ip ${device.ip} as ${!isMasterOnline ? 'MASTER' : 'GUEST'}`);
				// 	} else {
				// 		console.log(`Device ${device.type} with ip ${device.ip} is already online.`);
				// 	}
				// } catch (err) {
				// 	console.log(err);
				// 	return;
				// }
			}

			if (type === 'get_all_devices') {
				try {
					const devices = dbManager.getAllDevices();
					sendResponse(ws, 'get_all_devices_rp', {
						success: true,
						devices: devices,
					});
				} catch (err) {
					sendResponse(ws, 'get_all_devices_rp', {
						success: false,
						devices: [],
					});
				}
			}

			// =================================================
			// 2. GAMEPAD 逻辑 (复刻 handle_gamepad_lcm)
			// =================================================
			if (type === 'gamepad') {
				if (!j.data || !j.data.button || !j.data.axis || !j.data.combo) {
					console.error('Invalid gamepad message!');
					return;
				}
				const button = j.data.button;
				const axis = j.data.axis;
				const combo = j.data.combo;
				const gamepadMsg = {
					// Buttons
					A: button[0],
					B: button[1],
					Y: button[2],
					X: button[3],
					LS: button[4],
					RS: button[5],
					LS2: button[6],
					RS2: button[7],
					BK: button[8],
					ST: button[9],
					LZ: button[10],
					RZ: button[11],
					U: button[12],
					D: button[13],
					L: button[14],
					R: button[15],

					// Axes
					LX: axis[0],
					LY: axis[1],
					RX: axis[2],
					RY: axis[3],
					L2: axis[4],
					R2: axis[5],

					// Combos
					STAND: combo[0],
					WALK: combo[1],
					NAVIGATION: combo[2],
					MANIPULATION: combo[3],
					APF: combo[4],
				};

				console.log(`[GAMEPAD_SERVER > gamepad] ${JSON.stringify(gamepadMsg)}`);

				// const logEntry = `[${new Date().toISOString()}] ${JSON.stringify(gamepadMsg)}\n`;
				// fs.appendFile("gamepad_logs.txt", logEntry, (err) => {
				// 	if (err) console.error("Error writing gamepad log:", err);
				// });
				sendGamepadToLCM(gamepadMsg);

				return;
			}

			// =================================================
			// 3. operate:video
			// =================================================

			if (
				(j.type === 'operate:video' ||
					j.type === 'operate:ungrasp_target' ||
					j.type === 'operate:click_for_nav') &&
				j.data &&
				j.data.position
			) {
				const pos = j.data.position;
				console.log(j);
				if (Array.isArray(pos) && pos.length === 2) {
					let x, y;
					// 复刻 C++ std::stod 逻辑，处理字符串或数字
					const rawX = pos[0];
					const rawY = pos[1];

					x = typeof rawX === 'string' ? parseFloat(rawX) : rawX;
					y = typeof rawY === 'string' ? parseFloat(rawY) : rawY;

					if (!isNaN(x) && !isNaN(y)) {
						if (j.type === 'operate:video') {
							onVideoClickCallback(x, y);
							console.log(
								`[GAMEPAD_SERVER > ${j.type}] Triggered ROS callback: x=${x}, y=${y} on Click Video`
							);
						} else if (j.type === 'operate:ungrasp_target') {
							onUngraspTargetCallback(x, y);
							console.log(
								`[GAMEPAD_SERVER > ${j.type}] Triggered ROS callback: x=${x}, y=${y} on Click Ungrasp Target`
							);
						} else if (j.type === 'operate:click_for_nav') {
							onNavClickCallback(x, y);
							onVideoClickCallback(x, y);
							console.log(
								`[GAMEPAD_SERVER > ${j.type}] Triggered ROS callback: x=${x}, y=${y} on move to the target`
							);
						} else {
							console.warn(
								`[GAMEPAD_SERVER > ${j.type}] Received click, but ROS not able to publish click_coordinates.`
							);
						}
					} else {
						console.error(`[GAMEPAD_SERVER > ${j.type}] Invalid coordinates (NaN)`);
					}
				}
				return;
			}

			if (j.type === 'operate:pgmMap' && j.data && j.data.position) {
				const pos = j.data.position;
				if (Array.isArray(pos) && pos.length === 2) {
					const x = pos[0];
					const y = pos[1];

					if (!isNaN(x) && !isNaN(y)) {
						if (onPgmClickCallback) {
							onPgmClickCallback(x, y);
							console.log(
								`[GAMEPAD_SERVER > operate:PgmMap] Triggered ROS callback: x=${x}, y=${y}`
							);
						} else {
							console.warn(
								'[GAMEPAD_SERVER > operate:PgmMap] Received click, but ROS not able to publish pgm_click_coordinates.'
							);
						}
					} else {
						console.error(
							'[GAMEPAD_SERVER > operate:PgmMap] Invalid coordinates (NaN)'
						);
					}
				}
				return;
			}

			// =================================================
			// 3.5 restart video node service
			// =================================================
			if (type === 'restart_node_video') {
				const restart = j.data.restarted;
				if (restart === undefined) {
					sendResponse(ws, 'restart_node_video_rp', {
						success: false,
					});
					console.error(
						'[GAMEPAD_SERVER > restart_node_video] Failed to restart node_video.service: data.restart is undefined'
					);
					return;
				}

				const serviceCmd = 'systemctl restart node_video.service';
				try {
					await restartLocalService(NUC_CONSTANT.password, serviceCmd);
					sendResponse(ws, 'restart_node_video_rp', {
						success: true,
					});
					console.log(`[GAMEPAD_SERVER > restart_node_video] restarted successfully`);
				} catch (err) {
					console.error(
						`[GAMEPAD_SERVER > restart_node_video] Exec Error: ${err.message}`
					);

					sendResponse(ws, 'restart_node_video_rp', {
						success: false,
					});
				}
				return;
			}

			// =================================================
			// 3.6 restart aggregator service
			// =================================================
			if (type === 'restart_aggregator') {
				if (j.data.restarted === undefined) {
					sendResponse(ws, 'restart_aggregator_rp', { success: false });
					console.error('[GAMEPAD_SERVER] Failed: data.restarted is undefined');
					return;
				}

				// 2. 先发送响应给前端
				sendResponse(ws, 'restart_aggregator_rp', {
					success: true,
					status: 'initiating',
				});
				console.log(`[GAMEPAD_SERVER] Sent success response, triggering restart...`);

				setTimeout(async () => {
					const serviceCmd = 'systemctl restart aggregator_api.service';
					try {
						await restartLocalService(NUC_CONSTANT.password, serviceCmd);
						console.log(`[GAMEPAD_SERVER] Restart command executed successfully.`);
					} catch (err) {
						// 注意：此时进程可能还没挂，如果进到这里，说明 sudo 命令本身报错了（如密码错）
						console.error(`[GAMEPAD_SERVER] Actual Exec Error: ${err.message}`);
						// 可选：如果没重启成功，可以再推一个错误消息告知前端
						sendResponse(ws, 'restart_aggregator_rp', {
							success: false,
							error: 'Command failed',
						});
					}
				}, 200);

				return;
			}

			// =================================================
			// 4. setup
			// =================================================
			if (type === 'setup') {
				if (!j.data || !j.data.type) {
					console.error('Missing data.type in setup message!');
					return;
				}

				const subType = j.data.type;
				const hdl = ws;

				try {
					if (subType === 'bootup') {
						//  terminate-> startup
						// 1. Terminate

						console.log('[GAMEPAD_SERVER > bootup] Running bootup terminate bash..');
						sendReply(hdl, 'terminate', 'terminate', '1');

						await runTeriminateCmd(
							NUC_CONSTANT.password,
							NUC_CONSTANT.address,
							NUC_CONSTANT.username,
							NUC_CONSTANT.terminateCmd
						);
						sendReply(hdl, 'startup', 'bootup', '1');

						// 2. Bootup

						console.log(
							'[GAMEPAD_SERVER > bootup] Running bootup bash in background..'
						);
						runBootupInBackground(
							NUC_CONSTANT.password,
							NUC_CONSTANT.address,
							NUC_CONSTANT.username,
							NUC_CONSTANT.bootupCmd
						);
					} else if (
						[
							'run_bear',
							'initialize',
							'run_sense',
							'low_level',
							'high_level',
							'top_level',
						].includes(subType)
					) {
						//  resume_process
						const resumeCmd = `sshpass -p ${NUC_CONSTANT.password} ssh -o StrictHostKeyChecking=no ${NUC_CONSTANT.username}@${NUC_CONSTANT.address} "pkill -18 -f bootup"`;

						exec(resumeCmd, (err) => {
							if (err && err.code !== 1) {
								console.error('[GAMEPAD_SERVER > resume process] error', err);
							}
						});

						setTimeout(() => {
							sendReply(hdl, 'startup', subType, '1');
						}, 1000);
					} else if (subType === 'terminateChair') {
						//send msg before running terminate chair bash
						sendReply(hdl, 'terminate_chair', 'terminate', '1');

						runTeriminateCmd(
							NUC_CONSTANT.password,
							NUC_CONSTANT.address,
							NUC_CONSTANT.username,
							NUC_CONSTANT.terminateChairCmd
						);
					} else {
						console.error(
							`[GAMEPAD_SERVER >  bootup steps] Unknown setup command type: ${subType}`
						);
					}
				} catch (err) {
					console.error(
						'[GAMEPAD_SERVER > bootup steps] Error processing setup command:',
						err
					);
				}
			}

			// =================================================
			// 4.1. shutdown
			// =================================================
			if (type === 'shutdown') {
				const shutdown_pc = j.data.pc;
				if (!shutdown_pc) return;

				if (shutdown_pc == 'all') {
					await pcManager.shutdownAll();
				}
				return;
			}
			// =================================================
			// 5. set bear position
			// =================================================
			if (type === 'set_bear_position') {
				if (!j.data || !j.data.data) {
					console.error(
						'[GAMEPAD_SERVER > set_bear_position] Missing data in set bear position message!'
					);
					return;
				}

				const code = j.data.data;

				const remoteCmd = `cd ${NUC_HOMEDIR}/THEMIS/THEMIS/; python3 -m Util.bear_set_posi_lite --id ${code}`;

				const fullCmd = `sshpass -p '${NUC_CONSTANT.password}' ssh -o StrictHostKeyChecking=no ${NUC_CONSTANT.username}@${NUC_CONSTANT.address} '${remoteCmd}'`;

				exec(fullCmd, (error, stdout, stderr) => {
					let output = stdout || '';
					let isSuccess = false;

					if (error) {
						console.error(
							`[GAMEPAD_SERVER > set_bear_position] Error: ${error.message}`
						);
						if (stderr) output += stderr;
					}

					if (output.includes('True') || output.includes('succeeded')) {
						isSuccess = true;
					}

					sendResponse(ws, 'set_bear_position_rp', {
						success: isSuccess,
						bear_id: code,
					});
					console.log(
						`[GAMEPAD_SERVER > set_bear_position] Reply sent. Output length: ${output.length}`
					);
				});

				return;
			}

			// =================================================
			// 6. calibrate bear
			// =================================================
			if (type === 'calibrate_bear') {
				if (!j.data || !j.data.bear_id) {
					console.error(
						'[GAMEPAD_SERVER > calibrate_bear] Missing bear_id in calibrate_bear message!'
					);
					return;
				}

				const bearId = j.data.bear_id;

				// 目标文件: /home/themis/THEMIS/THEMIS/Util/bear_callibration_lite.py

				const remoteCmd = `cd ${NUC_HOMEDIR}/THEMIS/THEMIS/; python3 -m Util.bear_calibration_lite --id ${bearId}`;

				const fullCmd = `sshpass -p '${NUC_CONSTANT.password}' ssh -o StrictHostKeyChecking=no ${NUC_CONSTANT.username}@${NUC_CONSTANT.address} '${remoteCmd}'`;

				console.log(`[GAMEPAD_SERVER > calibrate_bear] calibration on bear_id ${bearId}`);
				exec(fullCmd, (error, stdout, stderr) => {
					let output = stdout || '';
					let isSuccess = false;

					if (error) {
						console.error(`[GAMEPAD_SERVER > calibrate_bear] Error: ${error.message}`);
						if (stderr) output += stderr;
					}

					if (output.includes('True') || output.includes('succeeded')) {
						isSuccess = true;
					}

					sendResponse(ws, 'calibrate_bear_rp', {
						success: isSuccess,
						bear_id: bearId,
					});
					console.log(`[GAMEPAD_SERVER > calibrate_bear] Reply sent for ID ${bearId}.`);
				});

				return;
			}

			// =================================================
			// 7. segmentMap
			// =================================================

			if (type === 'segmentMap') {
				const open = j.data.open;
				if (open === undefined) {
					console.error(
						'[GAMEPAD_SERVER > segmentMap] Missing open action value in segmentmap'
					);
					return;
				}

				const startServiceCmd = 'systemctl restart segmentmap.service';
				const shutDownServiceCmd = 'systemctl stop segmentmap.service';

				const password = BODY_CONSTANT.password;
				const actionCmd = open ? startServiceCmd : shutDownServiceCmd;

				const fullCmd = `sshpass -p '${password}' ssh -o StrictHostKeyChecking=no ${BODY_CONSTANT.username}@${BODY_CONSTANT.address} 'echo "${password}" | sudo -S ${actionCmd}'`;

				const isServiceOpen = await checkServiceStatus('segmentmap');

				if (isServiceOpen === open) {
					sendSegmentRply(ws, open);
				} else {
					exec(fullCmd, async (err, stdout, stderr) => {
						if (err)
							console.error(`[GAMEPAD_SERVER > segmentMap] Exec Error: ${stderr}`);

						const finalStatus = await checkServiceStatus('segmentmap');
						sendSegmentRply(ws, finalStatus);
						console.log(
							`[GAMEPAD_SERVER > segmentMap] Action finished. Requested: ${open}, Actual: ${finalStatus}`
						);
					});
				}
			}

			// =================================================
			// 8. restart target pose
			// =================================================

			if (type === 'restart_targetPose') {
				const restart = j.data.restart;
				if (restart === undefined) {
					sendResponse(ws, 'restart_targetPose_rp', {
						success: false,
					});
					console.error(
						'[GAMEPAD_SERVER > restart_targetPose] Failed to restart target pose: data.restart is undefined'
					);
					return;
				}
				const { username, address, password } = JETSON_CONSTANT;

				const restartServiceCmd = 'systemctl restart start_click_to_map.service';

				try {
					await restartService(password, address, username, restartServiceCmd);
					sendResponse(ws, 'restart_targetPose_rp', {
						success: true,
					});
					console.log(`[GAMEPAD_SERVER > ${type}] targetpose restart successfully`);
				} catch (err) {
					console.error(`[GAMEPAD_SERVER > ${type}] Exec Error: ${err.message}`);

					sendResponse(ws, 'restart_targetPose_rp', {
						success: false,
						message: err.message,
					});
				}
				return;
			}

			// =================================================
			// 9. restart direction point 2d
			// =================================================

			if (type === 'restart_objectDetection') {
				const restart = j.data.restart;
				if (restart === undefined) {
					sendResponse(ws, 'restart_objectDetection_rp', {
						success: false,
					});
					console.error(
						`[GAMEPAD_SERVER > ${type}] Failed to restart object detection: data.restart is undefined`
					);
					return;
				}
				const { username, address, password } = JETSON_CONSTANT;
				const restartServiceCmd = 'systemctl restart run-object-detection.service';

				try {
					await restartService(password, address, username, restartServiceCmd);
					sendResponse(ws, 'restart_objectDetection_rp', {
						success: true,
					});
					console.log(`[GAMEPAD_SERVER > ${type}] Restart object detection successfully`);
				} catch (err) {
					console.error(`[GAMEPAD_SERVER > ${type}] Exec Error: ${err.message}`);

					sendResponse(ws, 'restart_objectDetection_rp', {
						success: false,
						message: err.message,
					});
				}

				return;
			}

			// =================================================
			// 10. restart zed2i || zedxm
			// =================================================

			if (type === 'restart_zed2i' || type === 'restart_zedxm') {
				let username, address, password, camera;

				if (type.includes('zed2i')) {
					camera = 'zed2i';
					username = JETSON_CONSTANT.username;
					address = JETSON_CONSTANT.address;
					password = JETSON_CONSTANT.password;
				} else {
					camera = 'zedxm';
					username = BODY_CONSTANT.username;
					address = BODY_CONSTANT.address;
					password = BODY_CONSTANT.password;
				}

				console.log(
					`[GAMEPAD_SERVER > restart_zed2i || restart_zedxm] Restart ${camera}....`
				);

				const rsp_name = `restart_${camera}_rp`;
				const restart = j.data.restart;

				if (restart === undefined || !camera || !username || !address || !password) {
					sendResponse(ws, rsp_name, {
						success: false,
					});
					console.error(
						`[GAMEPAD_SERVER > restart_zed2i || restart_zedxm] Failed to restart ${camera}: data.restart is undefined`
					);
					return;
				}

				const restartStartupCmd = 'systemctl restart startup.service';

				try {
					await restartService(password, address, username, restartStartupCmd);

					sendResponse(ws, rsp_name, { success: true });
					console.log(`[GAMEPAD_SERVER > ${camera.toUpperCase()}] restarted`);
				} catch (err) {
					console.error(
						`[GAMEPAD_SERVER > ${camera.toUpperCase()}] restart Error: ${err.message}`
					);

					sendResponse(ws, rsp_name, {
						success: false,
						message: err.message,
					});
				}
			}

			// =================================================
			// 11. IMU calibration
			// =================================================

			if (type === 'calibrate_imu') {
				const calibrate = j.data.calibrate;
				if (calibrate !== true) {
					sendResponse(ws, 'calibrate_imu_rp', {
						success: false,
						message: 'Invalid data',
					});

					console.error(
						'[GAMEPAD_SERVER > calibrate_imu] Failed to calibrate_imu: data.restart is undefined'
					);
					return;
				}

				const { username, address, password } = NUC_CONSTANT;

				// 目标文件: /home/themis/THEMIS/THEMIS/Util/imu_calibration_lite.py

				const remoteCmd = `cd ${NUC_HOMEDIR}/THEMIS/THEMIS/; python3 -m Util.imu_calibration_lite`;
				const fullCmd = `sshpass -p '${password}' ssh -o StrictHostKeyChecking=no ${username}@${address} '${remoteCmd}'`;

				console.log(`[GAMEPAD_SERVER > calibrate_imu] calibration on IMU`);
				exec(fullCmd, (error, stdout, stderr) => {
					let output = stdout || '';
					let isSuccess = false;

					if (error) {
						console.error(`[GAMEPAD_SERVER > calibrate_imu] Error: ${error.message}`);

						sendResponse(ws, 'calibrate_imu_rp', {
							success: false,
							message: stderr || error.message,
						});
						return;
					}

					if (output.includes('True') || output.includes('succeeded')) {
						isSuccess = true;
					}
					sendResponse(ws, 'calibrate_imu_rp', {
						success: isSuccess,
						message: isSuccess ? 'Calibration Finished' : 'Calibration Failed',
					});
				});
			}

			// =================================================
			// 12. restart costmap
			// =================================================

			if (type === 'restart_costmap') {
				const restart = j.data.restart;
				if (restart === undefined) {
					sendResponse(ws, 'restart_costmap_rp', { success: false });
					console.error(`[GAMEPAD_SERVER > ${type}] Failed: data.restart is undefined`);
					return;
				}

				const { username, address, password } = BODY_CONSTANT;
				const restartStartupCmd = 'systemctl restart startup.service';
				try {
					await restartService(password, address, username, restartStartupCmd);

					sendResponse(ws, 'restart_costmap_rp', { success: true });
					console.log(`[GAMEPAD_SERVER > ${type}] restarted`);
				} catch (err) {
					console.error(`[GAMEPAD_SERVER > ${type}] restart Error: ${err.message}`);

					sendResponse(ws, 'restart_costmap_rp', {
						success: false,
						message: err.message,
					});
				}
			}

			// =================================================
			// 13. update_direction2d
			// =================================================

			if (type === 'update_fixPointOffset') {
				const { zOffset, angle } = j.data;
				if (typeof zOffset !== 'number' || typeof angle !== 'number') {
					console.log('[GAMEPAD_SERVER > update_fixPointOffset] failed, invalid value');
					sendResponse(ws, 'update_fixPointOffset_rp', {
						success: false,
					});
					return;
				}

				if (onSendFixpointOffsetCallback) {
					const success = await onSendFixpointOffsetCallback(zOffset, angle);
					console.log(
						`[GAMEPAD_SERVER > update_fixPointOffset] Triggered ROS callback: zOffset=${zOffset}, angle=${angle}`
					);
					sendResponse(ws, 'update_fixPointOffset_rp', {
						success: success,
					});
					return;
				} else {
					console.warn(
						'[GAMEPAD_SERVER > update_fixPointOffset] Received data, but ROS not able to publish'
					);
					sendResponse(ws, 'update_fixPointOffset_rp', {
						success: false,
					});
					return;
				}
			}

			// =================================================
			// 15. grasp in motion
			// =================================================

			if (type === 'grasp_in_motion') {
				const { username, address, password, graspInMotionCmd_on, graspInMotionCmd_off } =
					NUC_CONSTANT;

				const enable = j.data.enable;
				const targetScript = enable ? graspInMotionCmd_on : graspInMotionCmd_off;
				const action = enable ? 'ON' : 'OFF';
				if (targetScript) {
					try {
						console.log(
							`[GAMEPAD_SERVER > grasp_in_motion] Excute Grasp in motion ${action}`
						);

						await runRemoteScript(password, address, username, targetScript);

						sendResponse(ws, 'grasp_in_motion_rp', {
							success: true,
							isTurnOn: enable,
						});

						console.log(
							`[GAMEPAD_SERVER > grasp_in_motion] Turn excuting grasp in motion ${action} successfully!`
						);
					} catch (err) {
						console.error(
							`[GAMEPAD_SERVER > grasp_in_motion] Excuting Grasp in motion error:`,
							err
						);
						sendResponse(ws, 'grasp_in_motion_rp', {
							success: false,
							isTurnOn: enable,
						});
					}
				}
			}

			// =================================================
			// 16. start create rt map
			// =================================================

			if (type === 'start_create_rt_map') {
				const step = j.data.step;
				if (!step) {
					console.error(
						'[GAMEPAD_SERVER > start_create_rt_map] Missing step value in start_create_rt_map'
					);
					sendResponse(ws, 'start_create_rt_map_rp', {
						success: false,
						message: 'Missing step value in start_create_rt_map',
					});
					return;
				}

				const { username, address, password } = BODY_CONSTANT;
				const scriptPath = BODY_HOMEDIR + '/map/mapping.sh';

				// const local = {
				// 	password: "themis",
				// 	address: "localhost",
				// 	username: "themis",
				// };
				// const { password, address, username } = local;
				// const scriptPath = path.join(import.meta.dirname, "..", "..", "bash/test/test1.sh");

				try {
					const status = await run_map_script(
						password,
						address,
						username,
						step,
						scriptPath
					);
					sendResponse(ws, 'start_create_rt_map_rp', {
						success: true,
						message: status,
					});
				} catch (err) {
					//collect all the rejectOnce details and send response to client
					sendResponse(ws, 'start_create_rt_map_rp', {
						success: false,
						message: err,
					});
				}

				return;
			}

			// =================================================
			// 18. loading available maps
			// =================================================

			if (type === 'load_availble_maps') {
				try {
					const folders = fs
						.readdirSync(PGM_MAP_DIR, { withFileTypes: true })
						.filter((item) => item.isDirectory())
						.map((item) => item.name);

					sendResponse(ws, 'load_availble_maps_rp', {
						success: true,
						data: folders,
					});
				} catch (err) {
					console.log(err);
					sendResponse(ws, 'load_availble_maps_rp', {
						success: false,
						msg: err,
						data: [],
					});
				}
			}

			// =================================================
			// 20. update pgm maps
			// =================================================
			if (type === 'get_map_data') {
				try {
					const mapDir = path.join(PGM_MAP_DIR, j.data.mapFolderName);
					const yamlFile_dir = path.join(mapDir, 'map.yaml');
					const mapFile_dir = path.join(mapDir, 'map.pgm');
					const yamlConfig = yaml.load(fs.readFileSync(yamlFile_dir, 'utf8'));
					const mapBytes = fs.readFileSync(mapFile_dir);
					const { width, height, mapData } = getMapData(mapBytes);
					const {
						resolution,
						origin: [originX, originY],
					} = yamlConfig;

					sendResponse(ws, 'get_map_data_rp', {
						success: true,
						data: {
							width,
							height,
							resolution,
							originX,
							originY,
							mapData,
						},
					});
				} catch (err) {
					console.log(err);
					sendResponse(ws, 'get_map_data_rp', {
						success: false,
						data: {},
					});
				}
			}

			// =================================================
			// 21. recovery bear limit
			// =================================================
			if (type == 'recovery_bear_limit') {
				if (!j.data || !j.data.bear_id) {
					console.error(
						'[GAMEPAD_SERVER > recovery_bear_limit] Missing bear_id in bear recovery message!'
					);
					sendResponse(ws, 'recovery_bear_limit_rp', {
						success: false,
						msg: 'invalid bear id',
						data: {},
					});
					return;
				}

				//const cwd = "/home/themis/themis-jianqi/AOS-repos/AOS.dev/Main PC/THEMIS/THEMIS";
				const cwd = NUC_HOMEDIR + '/THEMIS/THEMIS/';
				const script = 'Util.bear_limit_recovery';
				const bear_id = j.data.bear_id;

				try {
					const result = await runRecoveryLimitScript(script, bear_id, cwd);
					const errMsg = `failed to recovery bear ${j.data.bear_id}, script error`;
					if (result.includes('pass')) {
						const resultArr = result.trimEnd().split(' ');
						resultArr.shift();

						const obj = {
							connected: resultArr[0],
							status: resultArr[1],
							deg: resultArr[2],
						};
						sendResponse(ws, 'recovery_bear_limit_rp', {
							success: true,
							errMsg: '',
							data: obj,
						});
					} else {
						sendResponse(ws, 'recovery_bear_limit_rp', {
							success: false,
							errMsg: errMsg,
							data: {},
						});
					}
				} catch (err) {
					const errMsg =
						`failed to recovery bear ${j.data.bear_id} ` + err.toString() || '';
					sendResponse(ws, 'recovery_bear_limit_rp', {
						success: false,
						errMsg: errMsg,
						data: {},
					});
				}
			}

			if (settingConf_rq) {
				let rawValue = j.data.value;
				const finalValue = settingConf_rq.format
					? settingConf_rq.format(rawValue)
					: rawValue;
				const updated = updateSettingsConf(settingConf_rq.key, finalValue);

				sendResponse(ws, `${type}_rp`, {
					success: !!updated,
				});

				return;
			}

			// =================================================
			// Navigation Point Settings.......
			// =================================================
			if (type == 'load_manip_setting') {
				const filePath = NUC_HOMEDIR + '/THEMIS/THEMIS/Beta/settings.conf';
				// const filePath =
				// 	'/home/themis/themis-jianqi/gamepad_server/src/gamepad-updater/cmd/server_bash/settings.conf';
				const result = parseSettingConf(filePath);

				if (result) {
					sendResponse(ws, 'load_manip_setting_rp', { success: true, data: result });
				} else {
					sendResponse(ws, 'load_manip_setting_rp', { success: false, data: {} });
				}
			}

			if (type === 'get_detectionType_and_modelList') {
				const { username, address, password } = JETSON_CONSTANT;
				const modelDir = DETECTION_CONSTANT.MODEL_DIR;
				const interactYamlDir = DETECTION_CONSTANT.YAML_DIR;

				// const { username, address, password } = LOCAL_CONSTANT;
				// const modelDir = '/home/themis/themis-jianqi/debug/global';
				// const interactYamlDir =
				// 	'/home/themis/themis-jianqi/gamepad_server/src/Test/interaction.yaml';

				try {
					//1. get model list
					const getModeDir_cmd = `sshpass -p "${password}" ssh -o StrictHostKeyChecking=no ${username}@${address} "find ${modelDir} -maxdepth 1 -mindepth 1"`;
					const output = execSync(getModeDir_cmd).toString();
					const modelList = output.split('\n').filter((name) => name.trim() !== '');

					//2. get detection types, allowed classes and current model from yaml
					const getDetectType_cmd = `sshpass -p "${password}" ssh -o StrictHostKeyChecking=no ${username}@${address} "cat ${interactYamlDir}"`;
					const fileContent = execSync(getDetectType_cmd).toString();
					let data = yaml.load(fileContent);
					const detectionTypes = data.detection.allclasses;
					const modelInUse = data.detection.model_path;
					const detectionTypesInUse = data.detection.allowed_classes;
					sendResponse(ws, `${type}_rp`, {
						sccuess: true,
						data: { modelList, detectionTypes, modelInUse, detectionTypesInUse },
					});
				} catch (err) {
					sendResponse(ws, `${type}_rp`, { sccuess: false });
				}
			}

			if (type === 'update_detection_model') {
				const { username, address, password } = JETSON_CONSTANT;
				const interactYamlDir = DETECTION_CONSTANT.YAML_DIR;

				// const { username, address, password } = LOCAL_CONSTANT;
				// const interactYamlDir =
				// 	'/home/themis/themis-jianqi/gamepad_server/src/Test/interaction.yaml';

				const new_model_path = j.data.value;
				const sedExpr = `s#model_path:.*#model_path: '${new_model_path}'#`;
				const fullCommand = `sshpass -p "${password}" ssh -o StrictHostKeyChecking=no ${username}@${address} "sed -i \\"${sedExpr}\\" ${interactYamlDir}"`;
				try {
					execSync(fullCommand);
					await restartDetectionDetectScreen(password, address, username);
					sendResponse(ws, `${type}_rp`, { success: true });
				} catch (err) {
					console.error('[update_detection_model] restart detection:', err);
					sendResponse(ws, `${type}_rp`, { success: false });
				}
			}

			if (type === 'update_detection_types') {
				const { username, address, password } = JETSON_CONSTANT;
				const interactYamlDir = DETECTION_CONSTANT.YAML_DIR;

				// const { username, address, password } = LOCAL_CONSTANT;
				// const interactYamlDir =
				// 	'/home/themis/themis-jianqi/gamepad_server/src/Test/interaction.yaml';

				const updatedTypes = j.data.value;

				const arrayStr =
					'[' + updatedTypes.map((item) => `'\\''${item}'\\''`).join(', ') + ']';
				const sedPattern = `s@^\\(\\s*allowed_classes:\\).*@\\1 ${arrayStr}@`;
				const fullCommand = `sshpass -p "${password}" ssh -o StrictHostKeyChecking=no ${username}@${address} "sed -i '${sedPattern}' ${interactYamlDir}"`;
				try {
					execSync(fullCommand);

					sendResponse(ws, `${type}_rp`, { success: true });
				} catch (err) {
					console.error('Error:', err);
					sendResponse(ws, `${type}_rp`, { success: false });
				}
			}

			if (type === 'get_point_ids') {
				const interactYamlDir = DETECTION_CONSTANT.POINTS_DIR;
				// const interactYamlDir =
				// 	'/home/themis/themis-jianqi/gamepad_server/src/gamepad-updater/cmd/idcoordinates.yaml';

				const fileContent = fs.readFileSync(interactYamlDir, 'utf8');

				let data = yaml.load(fileContent);

				try {
					const ids = Object.keys(data).map((e) => Number(e.replace('-', '')));
					sendResponse(ws, `${type}_rp`, { success: true, data: ids });
				} catch (err) {
					console.error('Error:', err);
					sendResponse(ws, `${type}_rp`, { success: false, data: [] });
				}
			}

			if (type === 'update_start_point') {
				const { id, yaw, pointType } = j.data;

				if (!id || !yaw || !pointType) {
					sendResponse(ws, `${type}_rp`, { success: false });
					return;
				}

				// const interactYamlDir =
				// 	'/home/themis/themis-jianqi/gamepad_server/src/gamepad-updater/cmd/idcoordinates.yaml';

				const interactYamlDir = DETECTION_CONSTANT.POINTS_DIR;

				//1.find ids from yaml
				const fileContent = fs.readFileSync(interactYamlDir, 'utf8');
				let list = yaml.load(fileContent);

				try {
					const { x, y } = list['-' + id];
					if (!x || !y) {
						sendResponse(ws, `${type}_rp`, { success: false });
						return;
					}
					const coordinateValues = `${x} ${y} ${yaw}`;
					let settingsKey;
					let idKey;
					switch (pointType) {
						case 'A':
							settingsKey = 'A_POINT';
							idKey = 'A_POINT_ID';
							break;
						case 'B':
							settingsKey = 'B_POINT';
							idKey = 'B_POINT_ID';
							break;
						case 'C':
							settingsKey = 'C_POINT';
							idKey = 'C_POINT_ID';
							break;
					}

					updateSettingsConf(settingsKey, coordinateValues);
					updateSettingsConf(idKey, id);
					sendResponse(ws, `${type}_rp`, { success: true });
				} catch (err) {
					console.error('Error:', err);
					sendResponse(ws, `${type}_rp`, { success: false });
				}
			}

			if (type === 'generate_id_coordinate') {
				const yamlPath = DETECTION_CONSTANT.POINTS_DIR;
				const cmd = `source /opt/ros/humble/setup.bash && python3 ${NUC_HOMEDIR}/THEMIS/THEMIS/Play/Navigation/idextract.py`;
				exec(cmd, { shell: '/bin/bash' }, (err, stdout, stderr) => {
					console.log('[generate_id_coordinate] exec python3 script running...');

					if (stdout) console.log('[generate_id_coordinate]', stdout);
					if (stderr) console.warn('[generate_id_coordinate]', stderr);
					if (err) {
						console.error('[generate_id_coordinate] failed:', err);
						sendResponse(ws, `${type}_rp`, {
							success: false,
						});
					}
					console.log('finished');
					delay(2000).then(() => {
						sendResponse(ws, `${type}_rp`, {
							success: fs.existsSync(yamlPath),
						});
					});
				});
			}

			if (type == 'play_navigation5') {
				const session = 'themis';
				const window = 'auto_navigation';
				const isStart = j.data.value; // true 为启动，false 为停止

				// 1. 定义发送 Ctrl+C 的指令 (使用 printf 兼容性最好)
				const stopCmd = `screen -S ${session} -p ${window} -X stuff $(printf "\\003")`;

				// 2. 执行停止 (Ctrl+C)
				exec(stopCmd, (err) => {
					if (err) {
						return sendResponse(ws, `${type}_rp`, { success: false });
					}

					// 如果只是停止 (false)，现在就可以回信了
					if (!isStart) {
						return sendResponse(ws, `${type}_rp`, { success: true });
					}

					// 3. 如果是启动 (true)，延迟 1 秒等待旧程序退出，再发新命令
					setTimeout(() => {
						const startCmdString = `cd ${NUC_HOMEDIR}/THEMIS/THEMIS && python3 -m Play.Navigation.auto_navigation6`;
						const runCmd = `screen -S ${session} -p ${window} -X stuff "${startCmdString}\\n"`;

						exec(runCmd, (runErr) => {
							sendResponse(ws, `${type}_rp`, {
								success: !runErr,
							});
						});
					}, 1500);
				});
			}
		} catch (err) {
			console.error('[GAMEPAD_SERVER > realtime] Unexpected error in message handler:', err);
		}
	});
	ws.on('close', () => {
		clearInterval(interval);
		console.log('[GAMEPAD_SERVER > realtime] Client disconnected');
		dbManager.markDeviceDisconnected(ws.deviceId);
		// if (ws.deviceId && heartbeatMap.has(ws.deviceId)) {
		// 	heartbeatMap.delete(ws.deviceId);
		// }
		// if (ws.deviceId) {
		// 	dbManager.markDeviceDisconnected(ws.deviceId);
		// 	broadcast(wssRealtime, {
		// 		type: "online_devices",
		// 		data: dbManager.getAllOnlineDevices(),
		// 	});
		// }
	});
});

server.on('upgrade', function upgrade(request, socket, head) {
	const parsedUrl = new URL(request.url, 'http://base');
	const pathname = parsedUrl.pathname;

	if (pathname === '/realtime') {
		wssRealtime.handleUpgrade(request, socket, head, function done(ws) {
			wssRealtime.emit('connection', ws, request);
		});
	} else {
		socket.destroy();
	}
});

const startHttpServer = () => {
	return new Promise((resolve) => {
		server.listen(PORT, () => {
			console.log(`[WEBSOCKET] Websocket server started on port ${PORT}`);
			resolve();
		});
	});
};

const closeServer = () => {
	console.log('[WEBSOCKET] SHUTING DOWN WebSocket SERVER...');

	wssRealtime.clients.forEach((client) => {
		client.terminate();
	});
	wssRealtime.close();

	return new Promise((resolve) => {
		server.close(() => {
			console.log(`[WEBSOCKET] HTTP PORT ${PORT} RELEASED`);
			resolve();
		});
		server.closeAllConnections();
	});
};

export {
	wssRealtime,
	setVideoClickHandler,
	setUngraspTargetCallback,
	setNavClickHandler,
	setFixpointOffset,
	setPgmClickHandler,
	closeServer,
	startHttpServer,
};
