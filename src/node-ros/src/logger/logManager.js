import { highRateLogger, normalRateLogger, watchDogLogger } from "./logger.js";

// --- Aggregator High Rate (1Hz) ---
let highRateData = {};
let isHighRateEnabled = false;

setInterval(() => {
	// 只有当开启记录，且数据不为空时才写
	if (isHighRateEnabled && highRateData && Object.keys(highRateData).length > 0) {
		highRateLogger.highRate(highRateData, "High Rate Log");
	}
}, 1000);

export const updateHighRateLog = (newData, enable) => {
	highRateData = { ...highRateData, ...newData };
	isHighRateEnabled = enable;
};

// --- Aggregator Normal Rate (60s) ---
let normalRateData = {};
let isNormalRateEnabled = false;

setInterval(() => {
	if (isNormalRateEnabled && normalRateData && Object.keys(normalRateData).length > 0) {
		normalRateLogger.normalRate(normalRateData, "Normal Rate Log");
	}
}, 60000);

export const updateNormalRateLog = (newData, enable) => {
	normalRateData = { ...normalRateData, ...newData };
	isNormalRateEnabled = enable;
};

// --- WatchDog Logic ---
export const logWatchDog = (data) => {
	watchDogLogger.info(data, "WatchDog Event");
};
