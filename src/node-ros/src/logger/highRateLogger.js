import { highRateLogger } from "./logger.js"; // Import the specific logger

let currentData = {};
let isLoggingEnabled = false;

const highRateLogInterval = 1000;

setInterval(() => {
	if (isLoggingEnabled && currentData) {
		console.log("Saving high rate log");
		highRateLogger.highRate(currentData, "Data logged at 1Hz (60 times per minute).");
	}
}, highRateLogInterval);

const updateHighRateLog = (newData, enable) => {
	currentData = { ...currentData, ...newData };
	isLoggingEnabled = enable;
};
export { updateHighRateLog };
