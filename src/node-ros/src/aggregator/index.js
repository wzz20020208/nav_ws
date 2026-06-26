import path from 'path';
import init from './init.js';
import rclnodejs from 'rclnodejs';
import { fork } from 'child_process';
import { broadcast } from '../server/serverUtil.js';

import {
	buildLocalCostMap,
	buildPathData,
	buildPathData2d,
	buildDetectedObjects,
	buildMarkerPoint,
	buildDirectionPoint2D,
	buildRobotYawData,
	buildConvexPlaneData,
	build3dObjectsData,
	buildElevationMapData,
	buildRobotPoseData,
	buildRtMapData,
	handTrajectoryData,
} from '../server/msgUtil.js';

import { stopPythonProjector, startPythonProjector } from '../tools/path/path_projector.js';

import { updateHighRateLog, updateNormalRateLog, logWatchDog } from '../logger/logManager.js';
import {
	createTopicSubscriptions,
	aggregatorTopics,
	recognizeTopics,
	mapTopics,
	getFreshTopicData,
	getQosSetting,
	logTopics,
} from './subsManager.js';

import {
	wssRealtime,
	setVideoClickHandler,
	setUngraspTargetCallback,
	setNavClickHandler,
	closeServer,
	setFixpointOffset,
	setPgmClickHandler,
	startHttpServer,
} from '../server/server1.js';

// import VideoSub from '../video/VideoSub.js'; // video Node

const aggregatorPeriod = 50n * 1_000_000n; // 20Hz

//const aggregatorPeriod = 100n * 1_000_000n; // 10Hz

const FRESHDATA_TIMEOUT = 2000;

//callback subs data/send data -> save to aggregatorstore/send
let robotAggdataStore = {};

const callback = (msg, topicName) => {
	let sendData;
	switch (topicName) {
		case 'costmap':
			sendData = buildLocalCostMap(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'local_path':
			sendData = buildPathData(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'local_path_2d_zed2i':
		case 'local_path_2d_zedxm':
			sendData = buildPathData2d(msg, topicName);
			broadcast(wssRealtime, sendData);
			break;
		case 'detected_objects':
			sendData = buildDetectedObjects(msg, topicName);
			broadcast(wssRealtime, sendData);
			break;
		case 'marker_point':
		case 'interaction_point_2d_pixel':
			sendData = buildMarkerPoint(msg, topicName);
			broadcast(wssRealtime, sendData);
			break;
		case 'direction_points_2d':
			sendData = buildDirectionPoint2D(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'robot_yaw':
			sendData = buildRobotYawData(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'convex_polygons':
			sendData = buildConvexPlaneData(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'objects_3d':
			sendData = build3dObjectsData(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'robot_pose':
			sendData = buildRobotPoseData(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'elevation_map':
			sendData = buildElevationMapData(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'rtab_map':
			sendData = buildRtMapData(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'handTrajectory':
			sendData = handTrajectoryData(msg);
			broadcast(wssRealtime, sendData);
			break;
		case 'target_kpt':
			sendData = {
				type: 'target_kpt',
				data: msg.pose,
			};
			broadcast(wssRealtime, sendData);
			break;
		case 'trajectory_ready_count':
			sendData = { type: 'trajectory_ready_count', data: msg.data };
			broadcast(wssRealtime, sendData);
			break;
		case 'manipu_traj_info':
			sendData = { type: 'manipu_traj_info', data: msg.data };
			broadcast(wssRealtime, sendData);
			break;
		default:
			sendData = msg;
	}

	//exclude costmap data from data store
	if (topicName !== 'costmap') {
		robotAggdataStore[topicName] = {
			data: sendData,
			last_update_time: Date.now(),
		};
	}
};
let watchDogFork;

async function main() {
	const realtimeNodeName = 'rcljs_aggregator';
	try {
		await new Promise((resolve) => setTimeout(resolve, 2000));

		// 2. 初始化 rclnodejs 环境
		const rcl = await init(realtimeNodeName);
		const realtimeNode = new rcl.Node(realtimeNodeName);

		realtimeNode.spin();

		// 3. 启动 Watchdog
		watchDogFork = fork(path.join(import.meta.dirname, '..', 'watchDog/main.js'));
		watchDogFork.on('message', (message) => {
			if (message.type === 'WATCHDOG_DATA') {
				broadcast(wssRealtime, message);
				logWatchDog(message);
			}
		});

		// startPythonProjector();

		await new Promise((resolve) => setTimeout(resolve, 2000));
		createTopicSubscriptions(
			realtimeNode,
			callback,
			[...aggregatorTopics, ...recognizeTopics, ...mapTopics, ...logTopics],
			rclnodejs
		);

		// 7. 设置 10Hz 定时器任务
		realtimeNode.createTimer(aggregatorPeriod, () => {
			const robotAggPayload = getFreshTopicData(aggregatorTopics, robotAggdataStore, FRESHDATA_TIMEOUT);

			const threadState = robotAggPayload.thread_state.data || {};
			const isHighRateMode = threadState.command === 1;

			const sendData = {
				type: 'robot_aggregator',
				data: robotAggPayload,
			};
			broadcast(wssRealtime, sendData);

			const nonAggregatorTopics = [...recognizeTopics, ...mapTopics];
			const restPayload = getFreshTopicData(nonAggregatorTopics, robotAggdataStore, FRESHDATA_TIMEOUT);

			const fullLogData = {
				...robotAggPayload,
				...restPayload,
			};

			if (isHighRateMode) {
				updateHighRateLog(fullLogData, true);
				updateNormalRateLog(null, false);
			} else {
				updateHighRateLog(null, false);
				updateNormalRateLog(fullLogData, true);
			}
		});

		// 8. 创建所有的 Publisher 和 Handler

		// Click Coordinates
		const clickPublisher = realtimeNode.createPublisher('geometry_msgs/msg/Point', 'click_coordinates');
		setVideoClickHandler((x, y) => {
			const pointMsg = { x: x, y: y, z: 0.0 };
			clickPublisher.publish(pointMsg);
			console.log(`[ROS-Topic] Published /click_coordinates: x=${x}, y=${y}`);
		});

		// Place Coordinates
		const ungraspTargetPublisher = realtimeNode.createPublisher('geometry_msgs/msg/Point', 'place_coordinates');
		setUngraspTargetCallback((x, y) => {
			const pointMsg = { x: x, y: y, z: 0.0 };
			ungraspTargetPublisher.publish(pointMsg);
			console.log(`[ROS-Topic] Published /place_coordinates: x=${x}, y=${y}`);
		});

		// Click for Nav
		const clickToNavPublisher = realtimeNode.createPublisher('geometry_msgs/msg/Point', 'click_for_nav');
		setNavClickHandler((x, y) => {
			const pointMsg = { x: x, y: y, z: 0.0 };
			clickToNavPublisher.publish(pointMsg);
			console.log(`[ROS-Topic] Published /click_for_nav: x=${x}, y=${y}`);
		});

		// FixPoint Offsets
		const fixPointOffsetPublisher = realtimeNode.createPublisher('geometry_msgs/msg/Point', 'fixPoint_offsets');
		setFixpointOffset((zOffset, angle) => {
			try {
				const msg = {
					x: parseFloat(angle),
					y: 0.0,
					z: parseFloat(zOffset),
				};
				fixPointOffsetPublisher.publish(msg);
				console.log(`[ROS-Topic] Published fixPoint_offsets: zOffset=${zOffset}, angle=${angle}`);
				return true;
			} catch (error) {
				console.error(`[ROS-Topic] fixPoint_offsets: Failed to publish:`, error);
				return false;
			}
		});

		// PGM Click
		const pgmClickPublisher = realtimeNode.createPublisher('geometry_msgs/msg/Point', 'pgm_click_from_screen');
		setPgmClickHandler((x, y) => {
			try {
				const msg = { x: x, y: y, z: 0.0 };
				pgmClickPublisher.publish(msg);
				console.log(`[ROS-Topic] Published pgm_click_coordinates: x=${x}, y=${y}`);
				return true;
			} catch (error) {
				console.error(`[ROS-Topic] pgm_click_coordinates: Failed to publish:`, error);
				return false;
			}
		});

		// --- 启动完成确认 ---
		console.log(`${realtimeNodeName} ros2 node running`);

		//5s 后启动server
		await startHttpServer();
	} catch (err) {
		console.error(`Failed to start node list ${realtimeNodeName}:`, err);
		process.exit(1);
	}
}

async function shutdown() {
	console.log('Shutting down ...');
	try {
		await closeServer().catch(() => {});

		if (watchDogFork && watchDogFork.connected) {
			watchDogFork.send('shutdown');
			await new Promise((resolve) => setTimeout(resolve, 500));
			watchDogFork.kill('SIGKILL');
		}

		if (rclnodejs) {
			rclnodejs.shutdown();
		}
	} catch (err) {
		console.error('Error during graceful shutdown:', err);
	}
}

process.on('SIGINT', async () => {
	await shutdown();
	process.exit(0);
});

process.on('SIGTERM', async () => {
	await shutdown();
	process.exit(0);
});

process.on('uncaughtException', async (err) => {
	console.error('UNKNOWN ERROR!', err);
	await shutdown();
	process.exit(1);
});

main();
