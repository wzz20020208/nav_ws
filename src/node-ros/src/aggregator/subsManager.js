const aggregatorTopics = [
	{
		type: 'themis_state_msgs/msg/THJointState',
		topic: '/th_joint_state',
		name: 'joint_state',
	}, //20hz
	{
		type: 'themis_state_msgs/msg/FootPrintArray',
		topic: '/foot_print',
		name: 'foot_print',
	}, //20hz
	{
		type: 'themis_state_msgs/msg/BearState',
		topic: '/bear_state',
		name: 'bear_state',
	}, //20hz
	{
		type: 'themis_state_msgs/msg/ThreadState',
		topic: '/thread_state',
		name: 'thread_state',
	}, //20hz
	{
		type: 'themis_state_msgs/msg/BaseState',
		topic: '/base_state',
		name: 'base_state',
	}, //20hz
	{
		type: 'themis_state_msgs/msg/BatteryState',
		topic: '/themis_battery',
		name: 'themis_battery',
	}, //20hz
	{
		type: 'themis_state_msgs/msg/BearTemperature',
		topic: '/bear_temperature',
		name: 'bear_temperature',
	}, //20hz
	{ type: 'std_msgs/msg/String', topic: '/walk_state', name: 'walk_state' }, //20hz
	{ type: 'std_msgs/msg/String', topic: '/under_mode', name: 'under_mode' }, //20hz
];

const recognizeTopics = [
	{
		type: 'geometry_msgs/msg/PointStamped',
		topic: '/marker_point',
		name: 'marker_point',
	},
	{
		type: 'themis_state_msgs/msg/DetectedObjectArray',
		topic: '/detected_objects_with_size',
		name: 'detected_objects',
	},
	{
		type: 'geometry_msgs/msg/PoseArray',
		topic: '/direction_points_2d',
		name: 'direction_points_2d',
	},
	{
		type: 'themis_detection_msgs/msg/DetectedOBBObjectArray',
		topic: '/remembered_direction_objects_map',
		name: 'objects_3d',
	},
	{
		type: 'geometry_msgs/msg/PoseArray',
		topic: '/hand_final_straight_trajectory_world',
		name: 'handTrajectory',
	},

	{
		type: 'geometry_msgs/msg/PointStamped',
		topic: '/interaction_point_2d_pixel',
		name: 'interaction_point_2d_pixel',
	},
];

const mapTopics = [
	{
		type: 'nav_msgs/msg/OccupancyGrid',
		topic: '/local_costmap/costmap',
		name: 'costmap',
	},
	{
		type: 'nav_msgs/msg/Path',
		topic: '/plan',
		name: 'local_path',
	},
	{
		type: 'geometry_msgs/msg/PoseArray',
		topic: '/plan_2d_zed2i',
		name: 'local_path_2d_zed2i',
	},
	{
		type: 'geometry_msgs/msg/PoseArray',
		topic: '/plan_2d_zedxm',
		name: 'local_path_2d_zedxm',
	},
	{
		type: 'visualization_msgs/msg/MarkerArray',
		topic: '/plane/convex_polygons',
		name: 'convex_polygons',
	},
	{ type: 'std_msgs/msg/Float32', topic: '/robot_yaw', name: 'robot_yaw' },
	{
		type: 'geometry_msgs/msg/PoseStamped',
		topic: '/robot_pose',
		name: 'robot_pose',
	},
	// {
	// 	type: "grid_map_msgs/msg/GridMap",
	// 	topic: "/mold/elevation_map",
	// 	name: "elevation_map",
	// },
	{
		type: 'nav_msgs/msg/OccupancyGrid',
		topic: '/rtab_map',
		name: 'rtab_map',
	},
];

const videoTopics = [
	{
		type: 'sensor_msgs/msg/CompressedImage',
		topic: '/zed2i/zed2i/rgb/color/rect/image/compressed',
		name: 'zed2i_video',
	},
	{
		type: 'sensor_msgs/msg/CompressedImage',
		topic: '/zedxm/zedxm/rgb/color/rect/image/compressed',
		name: 'zedxm_video',
	},
];

const watchDogTopics = [
	{
		type: 'themis_state_msgs/msg/ThreadState',
		topic: '/thread_state',
		name: 'thread_state',
	},
	{
		type: 'nav_msgs/msg/OccupancyGrid',
		topic: '/local_costmap/costmap',
		name: 'costmap',
	},
	{
		type: 'std_msgs/msg/Header',
		topic: '/watchdog/zed2i_status',
		name: 'zed2i',
	},
	{
		type: 'std_msgs/msg/Header',
		topic: '/watchdog/zedxm_status',
		name: 'zedxm',
	},
	{
		type: 'geometry_msgs/msg/PointStamped',
		topic: '/marker_point',
		name: 'marker_point',
	},
	{
		type: 'themis_state_msgs/msg/DetectedObjectArray',
		topic: '/detected_objects_with_size',
		name: 'detected_objects',
	},
	{
		type: 'geometry_msgs/msg/PoseArray',
		topic: '/direction_points_2d',
		name: 'direction_points_2d',
	},
];

const logTopics = [
	{
		type: 'geometry_msgs/msg/PoseStamped',
		topic: '/target_kpt_base',
		name: 'target_kpt',
	},
	{
		type: 'std_msgs/msg/Int32',
		topic: '/trajectory_ready_count',
		name: 'trajectory_ready_count',
	},
	{ type: 'std_msgs/msg/String', topic: '/manipulation_traj_info', name: 'manipu_traj_info' },
];

const getQosSetting = (rclnodejs, depth = 1, isvideo) => {
	// 1. 创建基础 QoS (1 代表 Keep Last 策略)
	const qos = new rclnodejs.QoS(1, depth);

	// 2. 直接用数字赋值，绕过那些 undefined 的常量名
	// 数字 1 代表 RELIABLE (可靠传输)
	// 数字 2 代表 BEST_EFFORT (尽力而为，用于视频)
	qos.reliability = isvideo ? 2 : 1;

	// 3. 剩下的检查保留
	if (qos.lifespan === undefined) qos.lifespan = 0;
	if (qos.deadline === undefined) qos.deadline = 0;
	if (qos.liveliness === undefined) qos.liveliness = 1; // 1 = AUTOMATIC

	return qos;
};

import {
	convertJointOrBearStateMsg,
	convertFootPrintMsg,
	convertDetectedObjects,
	convertBearTempState,
	convertBatteryState,
} from '../server/msgUtil.js';

const createTopicSubscriptions = (node, callback, allTopics, rclnodejs) => {
	for (let i = 0; i < allTopics.length; i++) {
		let topic = allTopics[i];
		let depth = 1;
		const qos = getQosSetting(rclnodejs, depth, false);
		node.createSubscription(topic.type, topic.topic, { qos: qos }, (msg) => {
			try {
				if (callback && msg) {
					let convertMsg;
					switch (topic.name) {
						case 'joint_state':
						case 'bear_state':
							convertMsg = convertJointOrBearStateMsg(msg);
							break;
						case 'battery_state':
						case 'bear_temperature':
							convertMsg = convertBearTempState(msg);
							break;
						case 'detected_objects':
							convertMsg = convertDetectedObjects(msg);
							break;
						case 'foot_print':
							convertMsg = convertFootPrintMsg(msg);
							break;
						case 'themis_battery':
							convertMsg = convertBatteryState(msg);
							break;
						default:
							convertMsg = msg;
					}
					callback(convertMsg, topic.name);
				}
			} catch (error) {
				console.error(`Error in ${topic.name} subscription:, ${error}`);
			}
		});
	}
};

const getFreshTopicData = (topics, dataStore, timeout) => {
	const now = Date.now();
	const payload = {};

	for (const topicName of topics.map((e) => e.name)) {
		const record = dataStore[topicName];
		if (record && now - record.last_update_time < timeout) {
			payload[topicName] = record;
		} else {
			payload[topicName] = {};
		}
	}

	return payload;
};

export {
	createTopicSubscriptions,
	getFreshTopicData,
	getQosSetting,
	aggregatorTopics,
	videoTopics,
	mapTopics,
	recognizeTopics,
	watchDogTopics,
	logTopics,
};
