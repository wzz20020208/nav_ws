// Change this line
import { normalRateLogger } from "./logger.js"; // Use object destructuring

let currentData = {};
let isLoggingEnabled = false;

const normalRateLogInterval = 60000; // 60000ms = 60秒，即每分钟1次

setInterval(() => {
	if (isLoggingEnabled && currentData) {
		console.log("Saving normal rate log");
		normalRateLogger.normalRate(currentData, "Data logged once per minute.");
	}
}, normalRateLogInterval);

const updateNormalRateLog = (newData, enable) => {
	currentData = { ...currentData, ...newData };
	isLoggingEnabled = enable;
};
export { updateNormalRateLog };
