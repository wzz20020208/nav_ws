import { spawn, exec, execSync } from 'child_process';
import dotenv from 'dotenv';
import path from 'path';
import fs from 'fs';
dotenv.config({ path: path.join(import.meta.dirname, '..', '..', '.env') });

export const NUC_ADDRESS = process.env.NUC_ADDRESS || '192.168.0.11';
export const BODY_ADDRESS = process.env.BODY_ADDRESS || '192.168.0.13';
export const JETSON_ADDRESS = process.env.JETSON_ADDRESS || '192.168.0.12';
export const NUC_HOMEDIR = process.env.NUC_HOMEDIR || '/home/themis';
export const BODY_HOMEDIR = process.env.BODY_HOMEDIR || '/home/nvidia';
export const HEAD_HOMEDIR = process.env.HEAD_HOMEDIR || '/home/nvidia';

export const NUC_CONSTANT = {
	username: 'themis',
	password: 'themis',
	address: NUC_ADDRESS,
	terminateCmd: NUC_HOMEDIR + '/THEMIS/THEMIS/Play/terminate.sh',
	bootupCmd: NUC_HOMEDIR + '/THEMIS/THEMIS/Play/bootup_gamepad.sh',
	terminateChairCmd: NUC_HOMEDIR + '/THEMIS/THEMIS/Play/terminate_chair.sh',
	graspInMotionCmd_on: NUC_HOMEDIR + '/THEMIS/THEMIS/Play/Manipulation/enable_tracking.sh',
	graspInMotionCmd_off: NUC_HOMEDIR + '/THEMIS/THEMIS/Play/Manipulation/disable_tracking.sh',
};

export const JETSON_CONSTANT = {
	username: 'nvidia',
	password: 'nvidia',
	address: JETSON_ADDRESS,
	runObjDetectionCmd: HEAD_HOMEDIR + '/THEMIS/THEMIS/ros2_detection/src/restart_detection.sh',
};

export const BODY_CONSTANT = {
	username: 'nvidia',
	password: 'nvidia',
	address: BODY_ADDRESS,
};

export const DETECTION_CONSTANT = {
	YAML_DIR: HEAD_HOMEDIR + '/THEMIS/THEMIS/ros2_detection/src/interaction.yaml',
	MODEL_DIR: HEAD_HOMEDIR + '/THEMIS/THEMIS/ros2_detection/weights/',
	POINTS_DIR: NUC_HOMEDIR + '/THEMIS/THEMIS/Play/Navigation/idcoordinates.yaml',
};

export const LOCAL_CONSTANT = {
	username: 'themis',
	password: 'themis',
	address: 'localhost',
};

export const checkService = (serviceName) => {
	return new Promise((resolve) => {
		exec(`systemctl is-active ${serviceName}`, { timeout: 2000 }, (error, stdout) => {
			console.log(stdout);
			const status = (stdout || '').trim();
			resolve(status === 'active');
		});
	});
};

export const checkServiceStatus = (serviceName) => {
	const checkServiceCmd = `systemctl is-active ${serviceName}`;
	const fullCmd = `sshpass -p '${BODY_CONSTANT.password}' ssh -o StrictHostKeyChecking=no ${BODY_CONSTANT.username}@${BODY_CONSTANT.address} '${checkServiceCmd}'`;
	return new Promise((res) => {
		exec(fullCmd, (err, stdout, stderr) => {
			const status = (stdout || '').trim();
			res(!err && status === 'active');
		});
	});
};

export const runTeriminateCmd = (password, address, username, cmd, onData) => {
	return new Promise((resolve, reject) => {
		const p = spawn(
			'sshpass',
			[
				'-p',
				password,
				'ssh',
				'-tt',
				'-o',
				'StrictHostKeyChecking=no',
				`${username}@${address}`,
				cmd,
			],
			{
				stdio: ['pipe', 'pipe', 'inherit'],
			}
		);

		p.stdout.on('data', (data) => {
			const text = data.toString();
			process.stdout.write(text);
			if (onData) onData(text);

			if (/Press (ENTER|Enter|START)/i.test(text)) {
				p.stdin.write('\n');
			}
		});

		p.on('exit', (code) => {
			console.log(`[terminate - Process Exit] Code: ${code}`);
			resolve(code);
		});

		p.on('error', (err) => {
			console.error('Failed to start process:', err);
			reject(err);
		});
	});
};

export const runBootupInBackground = (password, address, username, cmd, onLog) => {
	const p = spawn(
		'sshpass',
		[
			'-p',
			password,
			'ssh',
			'-tt',
			'-o',
			'StrictHostKeyChecking=no',
			`${username}@${address}`,
			cmd,
		],
		{
			stdio: ['pipe', 'pipe', 'inherit'],
		}
	);

	p.stdout.on('data', (data) => {
		const text = data.toString();
		process.stdout.write(`[bootup bash Log]: ${text}`);
		if (onLog) onLog(text);
	});

	p.on('exit', (code) => {
		console.log(`[bootup - Process Exit] Code: ${code}`);
	});

	return p;
};

export const runRecoveryLimitScript = (script, id, cwd) => {
	return new Promise((resolve, reject) => {
		const p = spawn('python3', ['-m', script, '--id', id], {
			cwd: cwd,
			stdio: ['ignore', 'pipe', 'inherit'],
		});

		let resultData = '';

		p.stdout.on('data', (data) => {
			resultData += data.toString();
		});

		p.on('close', (code) => {
			if (code === 0) {
				resolve('pass ' + resultData);
			} else {
				resolve(code);
			}
		});

		p.on('error', (err) => {
			console.log('wrong...');
			reject(err);
		});
	});
};

export const runRemoteScript = (password, address, username, scriptPath) => {
	const fullCmd = `sshpass -p '${password}' ssh -o StrictHostKeyChecking=no ${username}@${address} 'chmod +x ${scriptPath} && ${scriptPath}'`;

	return new Promise((resolve, reject) => {
		exec(fullCmd, (err, stdout, stderr) => {
			if (err) {
				console.error(`[RemoteScript Error]: ${stderr || err.message}`);
				reject(err);
				return;
			}
			console.log(`[RemoteScript Success]: ${stdout}`);
			resolve(stdout);
		});
	});
};

export const restartService = (password, address, username, serviceCmd) => {
	const fullCmd = `sshpass -p '${password}' ssh -o StrictHostKeyChecking=no ${username}@${address} 'echo "${password}" | sudo -S ${serviceCmd}'`;
	return new Promise((resolve, reject) => {
		exec(fullCmd, (err, stdout, stderr) => {
			if (err) {
				reject(new Error(stderr.trim() || err.message));
				return;
			}
			resolve(stdout);
		});
	});
};

export const restartLocalService = (password, serviceCmd) => {
	const fullCmd = `echo "${password}" | sudo -S ${serviceCmd} &`;
	return new Promise((resolve, reject) => {
		exec(fullCmd, (err, stdout, stderr) => {
			if (err) {
				reject(new Error(err.message));
				return;
			}
			resolve(stdout);
		});
	});
};

/** Jetson screen session detection / window detect: Ctrl+C then restart OBB node */
export const restartDetectionDetectScreen = (password, address, username) => {
	const ssh = `sshpass -p '${password}' ssh -o StrictHostKeyChecking=no ${username}@${address}`;
	const detectRunCmd = 'ros2 run object_detection object_obb_detection0601';

	return new Promise((resolve, reject) => {
		exec(
			`${ssh} "screen -S detection -p detect -X stuff $(printf \\\"\\\\003\\\")"`,
			(stopErr) => {
				if (stopErr) {
					reject(stopErr);
					return;
				}
				delay(1500).then(() => {
					exec(
						`${ssh} "screen -S detection -p detect -X stuff '${detectRunCmd}\\n'"`,
						(startErr) => {
							if (startErr) reject(startErr);
							else resolve();
						}
					);
				});
			}
		);
	});
};

/**
 * @param {string} password - 远程密码
 * @param {string} address - 远程IP
 * @param {string} username - 远程用户名
 * @param {string} scriptPath - 脚本在远程机器上的路径
 * @param {string} stepName - 要执行的具体步骤 (例如: clean_threads, start_map, save_map)
 * @param {function} callback - 进度回传
 */

export const run_map_script = (password, address, username, stepName, scriptPath) => {
	const slice_start_string = '[STATUS]';
	const pid_start_string = '[SYSTEM] SCRIPT_PID';
	let bash_pid = null;
	let buffer = '';
	let stderr_msg = '';

	return new Promise((resolve, reject) => {
		const cmd = `bash ${scriptPath} ${stepName}`;
		let settled = false;

		const rejectOnce = (err) => {
			if (settled) return;
			settled = true;
			reject(err);
			p.kill();
		};
		const p = spawn(
			'sshpass',
			[
				'-p',
				password,
				'ssh',
				'-o',
				'StrictHostKeyChecking=no',
				`${username}@${address}`,
				cmd,
			],
			{
				stdio: ['pipe', 'pipe', 'pipe'],
			}
		);

		p.stdout.on('data', (data) => {
			const rawOutput = data.toString();
			console.log(`[BASH LOG - ${stepName}]: ${rawOutput}`);

			buffer += rawOutput;
			const lines = buffer.split('\n');
			buffer = lines.pop(); // 保留不完整的一行

			for (const line of lines) {
				const message = line.trim();
				if (!message) continue;

				if (message.startsWith(pid_start_string)) {
					const pid = message.split(':')[1].trim();
					bash_pid = pid;
					console.log(`[MAPPING] bash_pid captured: ${bash_pid}`);
					continue;
				}

				if (message.startsWith(slice_start_string)) {
					try {
						const sliced_prefix_message = message.slice(slice_start_string.length);
						const bracketIndex = sliced_prefix_message.indexOf('}');
						const sliced_message = sliced_prefix_message
							.slice(0, bracketIndex + 1)
							.trim();
						const status = JSON.parse(sliced_message);
						resolve(status);
					} catch (e) {
						console.log('[MAPPING] error: ', e);
						rejectOnce(e);
					}
				}
			}
		});
		p.on('exit', (code) => {
			console.log(`[run_map_script - Process Exit] Code: ${code} \n`);
		});

		p.on('close', (code) => {
			console.log(`[run_map_script - Process Close] Code: ${code} \n`);
			if (code !== 0) {
				const errorDetail = stderr_msg.trim() || `Exit Code ${code}`;
				rejectOnce(new Error(`Remote script failed: ${errorDetail}`));
			} else {
				if (!settled) {
					settled = true;
					resolve(true);
				}
			}
		});

		p.on('error', (err) => {
			console.error('Failed to start process:', err);
			rejectOnce(err);
		});

		return p;
	});
};

export const getMapData = (bytes) => {
	let offset = 0;

	// 工具函数：读取下一个非注释、非空白的 Token
	const readNextToken = () => {
		let token = '';
		while (offset < bytes.length) {
			const char = bytes[offset];

			// 1. 处理空白字符 (空格, \n, \r, \t)
			if (char <= 32) {
				if (token.length > 0) {
					offset++;
					break;
				}
				offset++;
				continue;
			}

			// 2. 处理注释行 (以 # 开头)
			if (char === 35) {
				// 35 是 '#' 的 ASCII
				while (offset < bytes.length && bytes[offset] !== 10) {
					offset++; // 跳过这一行直到换行符
				}
				continue;
			}

			// 3. 拼接有效字符
			token += String.fromCharCode(char);
			offset++;
		}
		return token;
	};

	const magicNumber = readNextToken(); // 应该是 "P5"
	if (magicNumber !== 'P5') {
		console.error('不是有效的二进制 PGM (P5) 格式');
		return null;
	}

	const width = parseInt(readNextToken());
	const height = parseInt(readNextToken());
	const maxVal = readNextToken(); // 跳过最大值 (通常是 255)

	// 数据部分紧跟在最后一个 Token 后的单各空白符之后
	// 此时 offset 已经指向了数据开始的位置（或数据前的空白）

	const mapData = bytes.slice(offset);
	console.log(width, height);
	return {
		width: Number(width),
		height: Number(height),
		mapData,
	};
};

export class PCManager {
	constructor(nuc, head, body) {
		this.nuc = nuc;
		this.head = head;
		this.body = body;
		// this.active_ips = [nuc.address, head.address, body.address];
	}

	async isReallyOffline(ip) {
		const checkOffline = () => {
			try {
				// 发送 3 个包，每个包等 1 sec, 只要 3 个包里有一个通了，这个命令就返回成功
				execSync(`ping -c 3 -W 3 ${ip}`, { stdio: 'ignore' });
				return false; //online
			} catch (e) {
				return true; //offline
			}
		};
		//send again after 1 sec
		if (checkOffline()) {
			await new Promise((r) => setTimeout(r, 1000));
			return checkOffline();
		}

		return false;
	}

	async shutDown(pc) {
		const shutdownCmd = `echo ${pc.password} | sudo -S shutdown -h now`;

		return new Promise((resolve, reject) => {
			const p = spawn(
				'sshpass',
				[
					'-p',
					pc.password,
					'ssh',
					'-o',
					'StrictHostKeyChecking=no',
					`${pc.username}@${pc.address}`,
					shutdownCmd,
				],
				{
					stdio: ['pipe', 'pipe', 'pipe'],
				}
			);

			p.on('error', (err) => {
				console.log(`[GAMEPAD > shutdown] process error ${err}`);
				reject(err);
			});

			p.on('exit', (code) => {
				if (code === 0 || code === 255) {
					resolve(code);
				} else {
					reject(new Error(`Exit code ${code}`));
				}
			});

			setTimeout(() => {
				if (p.exitCode === null) {
					console.log('timeout,force to kill the process');
					p.kill('SIGKILL');
					resolve('timeout');
				}
			}, 10000);
		});
	}

	async shutdownAll() {
		this.shutDown(this.head);
		this.shutDown(this.body);

		let activeIps = [this.head.address, this.body.address];

		while (activeIps.length > 0) {
			for (let i = activeIps.length - 1; i >= 0; i--) {
				if (await this.isReallyOffline(activeIps[i])) {
					console.log(`[GAMEPAD SERVER > shutdown] - PC ${activeIps[i]} down`);
					activeIps.splice(i, 1);
				}
			}
			if (activeIps.length > 0) await new Promise((r) => setTimeout(r, 2000));
		}
		console.log(`[GAMEPAD SERVER > shutdown] - all remote pcs down. shutting down local..`);
		execSync(`echo ${this.nuc.password} | sudo -S shutdown -h now`);
	}
}

export const updateSettingsConf = (key, value) => {
	try {
		const filePath = NUC_HOMEDIR + '/THEMIS/THEMIS/Beta/settings.conf';
		//const filePath = '/home/themis/themis-jianqi/gamepad_server/src/gamepad-updater/cmd/server_bash/settings.conf';

		const command = `sed -i 's|^${key}=.*|${key}="${value}"|' ${filePath}`;
		execSync(command);
		return true;
	} catch (error) {
		console.error(`Failed update settings.conf: ${error.message}`);
		return false;
	}
};

export const delay = (ms) => {
	return new Promise((resolve) => setTimeout(resolve, ms));
};

export const parseSettingConf = (filePath) => {
	try {
		// 1. 同步读取，速度最快
		const content = fs.readFileSync(filePath, 'utf8');
		const config = {};

		// 2. 按行分割
		const lines = content.split(/\r?\n/);

		for (let i = 0; i < lines.length; i++) {
			let line = lines[i].trim();

			// --- 过滤逻辑 ---
			// 3. 跳过空行 或 以 # 开头的注释行
			if (!line || line.startsWith('#')) {
				continue;
			}

			// 4. 定位第一个等号
			const eqIndex = line.indexOf('=');
			if (eqIndex === -1) continue;

			// 5. 提取 Key (去掉前后空格)
			const key = line.substring(0, eqIndex).trim();

			// 6. 提取 Value 并处理行尾注释
			let value = line.substring(eqIndex + 1).trim();

			// 如果值是用引号包围的，提取引号内的内容（这样可以防止引号后的 # 被误判为注释）
			if (value.startsWith('"')) {
				// 寻找第二个引号
				const lastQuoteIndex = value.lastIndexOf('"');
				if (lastQuoteIndex > 0) {
					value = value.substring(1, lastQuoteIndex);
				}
			} else {
				// 如果没有引号，则直接去掉 # 及其后面的内容
				const hashIndex = value.indexOf('#');
				if (hashIndex !== -1) {
					value = value.substring(0, hashIndex).trim();
				}
			}

			config[key] = value;
		}

		return config;
	} catch (err) {
		console.error('Failed on loading settings conf :', err);
		return {};
	}
};
