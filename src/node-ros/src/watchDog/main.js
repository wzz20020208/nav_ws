import { syncOnlineDevices } from '../tools/scanner/networkScanner.js';
import { createTopicSubscriptions, watchDogTopics } from '../aggregator/subsManager.js';
import { checkServiceStatus } from '../server/bashUtil.js';
import rclnodejs from 'rclnodejs';
import init from '../aggregator/init.js';

let watchDogDataStore = {
	manipulation: {
		estimation: { status: -1, updateTime: Date.now(), desc: 'Estimation' },
		control: { status: -1, updateTime: Date.now(), desc: 'Control' },
		locomotion: { status: -1, updateTime: Date.now(), desc: 'Locomotion' },
		command: { status: -1, updateTime: Date.now(), desc: 'Command' },
	},
	hardware: {
		bear_head: { status: -1, updateTime: Date.now(), desc: 'Robot Head' },
		bear_left_arm: { status: -1, updateTime: Date.now(), desc: 'Robot Left Arm' },
		bear_right_arm: { status: -1, updateTime: Date.now(), desc: 'Robot Right Arm' },
		bear_left_leg: { status: -1, updateTime: Date.now(), desc: 'Robot Left Leg' },
		bear_right_leg: { status: -1, updateTime: Date.now(), desc: 'Robot Rigth Leg' },
		zed2i: { status: -1, updateTime: Date.now(), desc: 'Head Camera' },
		zedxm: { status: -1, updateTime: Date.now(), desc: 'Front Camera' },
		head: { status: -1, ip: '', temperature: null, updateTime: Date.now(), desc: 'Head PC' },
		main: { status: -1, ip: '', temperature: null, updateTime: Date.now(), desc: 'Main PC' },
		body: { status: -1, ip: '', temperature: null, updateTime: Date.now(), desc: 'Body PC' },
	},
	navigation: {
		costmap: {
			status: -1,
			updateTime: Date.now(),
			desc: 'Costmap',
		},
	},
	interaction: {
		targetPose: { status: -1, updateTime: Date.now(), desc: 'Target Pose' },
		direction2D: { status: -1, updateTime: Date.now(), desc: 'Direction Point' },
		detectBox: { status: -1, updateTime: Date.now(), desc: 'Detection Box' },
		segementMap: { status: -1, desc: 'Segement Map' },
	},
};

const STALE_THRESHOLD_MS = 5000;
const PUBLISH_INTERVAL_MS = 1000;
let watchDogInterval = null;
let node = null;

const syncSegementMap = (serviceName) => {
	const runCheck = async () => {
		try {
			const isActive = await checkServiceStatus(serviceName);
			watchDogDataStore.interaction.segementMap.status = isActive ? 1 : -1;
		} catch (err) {
			console.log(err);
		} finally {
			setTimeout(runCheck, 2000);
		}
	};

	runCheck();
};

const syncWatchDogFromSubs = (topicData, topicName) => {
	const now = Date.now();

	switch (topicName) {
		case 'thread_state':
			for (const key in watchDogDataStore.manipulation) {
				if (topicData[key] !== undefined) {
					watchDogDataStore.manipulation[key].status = topicData[key];
					watchDogDataStore.manipulation[key].updateTime = now;
				}
			}
			for (const key in watchDogDataStore.hardware) {
				if (topicData[key] !== undefined) {
					watchDogDataStore.hardware[key].status = topicData[key];
					watchDogDataStore.hardware[key].updateTime = now;
				}
			}
			break;

		case 'costmap':
			watchDogDataStore.navigation.costmap.status = 1;
			watchDogDataStore.navigation.costmap.updateTime = now;
			break;

		case 'zed2i':
			watchDogDataStore.hardware.zed2i.status = 1;
			watchDogDataStore.hardware.zed2i.updateTime = now;
			break;

		case 'zedxm':
			watchDogDataStore.hardware.zedxm.status = 1;
			watchDogDataStore.hardware.zedxm.updateTime = now;
			break;

		case 'marker_point':
			watchDogDataStore.interaction.targetPose.status = 1;
			watchDogDataStore.interaction.targetPose.updateTime = now;
			break;

		case 'detected_objects':
			watchDogDataStore.interaction.detectBox.status = 1;
			watchDogDataStore.interaction.detectBox.updateTime = now;
			break;

		case 'direction_points_2d':
			watchDogDataStore.interaction.direction2D.status = 1;
			watchDogDataStore.interaction.direction2D.updateTime = now;
			break;
	}
};

const checkDataFreshness = () => {
	const now = Date.now();

	const validate = (obj) => {
		for (const key in obj) {
			const item = obj[key];
			const noCheckKeys = ['head', 'main', 'body', 'segementMap'];
			if (item && !noCheckKeys.includes(key) && typeof item === 'object' && 'updateTime' in item) {
				if (now - item.updateTime > STALE_THRESHOLD_MS) {
					item.status = -1;
				}
			} else if (typeof item === 'object') {
				validate(item);
			}
		}
	};

	validate(watchDogDataStore);
};

// Network Scanner logic
syncOnlineDevices(
	(scannedResults) => {
		const devices = scannedResults.robotDevices;
		const now = Date.now();
		for (const key in devices) {
			const scanned = devices[key];
			const hardware = watchDogDataStore.hardware[key];
			if (!scanned?.ip_address || !hardware) continue;

			if ('temperature' in scanned) {
				hardware.temperature = scanned.temperature;
			}

			if (scanned.status === 'online') {
				hardware.status = 1;
				hardware.ip = scanned.ip_address;
				hardware.updateTime = now;
			} else if (scanned.status === 'offline') {
				hardware.status = -1;
				hardware.ip = scanned.ip_address;
			}
		}
	},
	(err) => console.warn('[WATCHDOG]Scanner Error:', err)
);

syncSegementMap('segmentmap');

const sendWatchDog = () => {
	checkDataFreshness();

	// [运行模式判断] process.send 存在说明是 fork 出来的子进程，不存在说明是手动直接运行
	if (process.send) {
		// [子进程模式]：如果父进程还连着，就正常发消息
		if (process.connected) {
			try {
				process.send({ type: 'WATCHDOG_DATA', data: watchDogDataStore });
			} catch (err) {
				// 发送失败通常意味着 IPC 管道已坏，自杀以防变成孤儿进程
				process.exit(0);
			}
		} else {
			// [孤儿保护]：父进程已断开但子进程还在跑，自杀以防节点重名冲突
			process.exit(0);
		}
	} else {
		// [本地调试模式]：手动 node 运行此文件时，直接在控制台打印结果
		console.log('WatchDog Data:', watchDogDataStore);
	}
};

async function initWatchDog() {
	try {
		const nodename = 'watchDog';
		await init(nodename);
		node = new rclnodejs.Node(nodename);

		createTopicSubscriptions(node, syncWatchDogFromSubs, watchDogTopics, rclnodejs);

		node.spin();
		watchDogInterval = setInterval(sendWatchDog, PUBLISH_INTERVAL_MS);
	} catch (err) {
		console.error('[WATCHDOG] Failed to initialized:', err);
	}
}

process.on('disconnect', () => {
	console.log('[WATCHDOG] Exited by main node');
	process.exit(0);
});

process.on('uncaughtException', (err) => {
	console.error('[WATCHDOG] Crashed:', err);
	process.exit(1);
});

process.on('message', async (msg) => {
	if (msg === 'shutdown') {
		console.log('[WATCHDOG] Received shutdown from main node');
		clearInterval(watchDogInterval);
		if (node) {
			try {
				node.destroy();
			} catch (e) {}
		}
		await rclnodejs.shutdown();
		process.exit(0);
	}
});

process.on('SIGINT', () => {});

initWatchDog();
