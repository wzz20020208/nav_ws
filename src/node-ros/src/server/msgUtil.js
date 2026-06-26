const convertTypedArrayForPack = (data) => {
	return data?.buffer instanceof ArrayBuffer ? Array.from(data) : data;
};

export const convertJointOrBearStateMsg = (jointState) => {
	for (const jointName in jointState) {
		const joint = jointState[jointName];

		for (const field in joint) {
			jointState[jointName][field] = convertTypedArrayForPack(joint[field]);
		}
	}

	return jointState;
};

export const convertBearTempState = (data) => {
	for (const field in data) {
		data[field] = convertTypedArrayForPack(data[field]);
	}
	return data;
};

export const convertBatteryState = (data) => {
	const result = {};

	for (const field in data) {
		const value = data[field];

		if (ArrayBuffer.isView(value)) {
			result[field] = Array.from(value);
		} else {
			result[field] = value;
		}
	}

	return result;
};

export const convertFootPrintMsg = (footPrint) => {
	footPrint.current_foot_print?.forEach((item) => {
		item.feet = item.feet?.map(convertTypedArrayForPack);
	});

	footPrint.goal_foot_print?.forEach((item) => {
		item.feet = item.feet?.map(convertTypedArrayForPack);
	});

	return footPrint;
};

export const convertDetectedObjects = (detected_objects) => {
	detected_objects.objects?.forEach((obj) => {
		obj.bbox = convertTypedArrayForPack(obj.bbox);
	});
	return detected_objects;
};

export const buildLocalCostMap = (localCostMap) => {
	let json = {};
	json.type = 'local_cost_map';
	json.data = {
		costmap_data: Array.from(localCostMap.data) || [],
		width: localCostMap.info.width,
		height: localCostMap.info.height,
		resolution: localCostMap.info.resolution,
		origin_x: localCostMap.info.origin.position.x,
		origin_y: localCostMap.info.origin.position.y,
	};

	return json;
};

export const buildPathData = (pathMsg) => {
	const json = {
		type: 'planned_path',
		data: {
			poses: pathMsg.poses.map((poseStamped) => {
				const pos = poseStamped.pose.position;
				const ori = poseStamped.pose.orientation;

				const theta = Math.atan2(
					2.0 * (ori.w * ori.z + ori.x * ori.y),
					1.0 - 2.0 * (ori.y * ori.y + ori.z * ori.z)
				);

				return {
					x: pos.x,
					y: pos.y,
					theta: theta,
				};
			}),
		},
	};
	return json;
};

export const buildPathData2d = (pathMsg, camera) => {
	const type = camera.includes('zed2i') ? 'planned_path_2d_zed2i' : 'planned_path_2d_zedxm';
	const json = {
		type: type,
		data: pathMsg.poses.map((e) => {
			return {
				x: e.position.x,
				y: e.position.y,
			};
		}),
	};
	return json;
};

export const buildDetectedObjects = (detected_objects) => {
	return {
		type: 'detected_objects',
		data: detected_objects.objects,
	};
};

export const buildMarkerPoint = (msg, topicName) => {
	const msgType = topicName === 'marker_point' ? 'marker_point' : 'interaction_point';
	return {
		type: msgType,
		data: msg.point,
	};
};

export const buildDirectionPoint2D = (points) => {
	return {
		type: 'direction_points_2d',
		data: points.poses,
	};
};

export const buildRobotYawData = (data) => {
	return {
		type: 'robot_yaw',
		data: data.data,
	};
};

export const buildConvexPlaneData = (data) => {
	let list = [];
	if (Array.isArray(data.markers) && data.markers.length > 0) {
		list = data.markers.map((e) => e.points);
	}
	return {
		type: 'convex_polygons',
		data: list,
	};
};

export const build3dObjectsData = (data) => {
	let list = [];
	if (Array.isArray(data.objects) && data.objects.length > 0) {
		list = data.objects.map((e) => {
			return {
				category: e.category,
				obb_points: e.obb_points,
				bbox: e.bbox,
			};
		});
	}
	return {
		type: 'objects_3d',
		data: list,
	};
};

export const buildElevationMapData = (data) => {
	return {
		type: 'elevation_map',
		data: {
			layout: data.data[0].layout,
			data: convertTypedArrayForPack(data.data[0].data),
		},
	};
};

export const buildRobotPoseData = (data) => {
	return {
		type: 'robot_pose',
		data: data.pose,
	};
};

export const buildRtMapData = (msg) => {
	return {
		type: 'rt_map',
		data: {
			mapData: msg.data,
			info: msg.info,
		},
	};
};

export const handTrajectoryData = (msg) => {
	return {
		type: 'handTrajectory',
		data: {
			data: msg.poses,
		},
	};
};
