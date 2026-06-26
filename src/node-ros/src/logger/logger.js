import pino from "pino";
import path from "path";
import fs from "fs";
const __dirname = import.meta.dirname;

const getLogDirectoryName = () => {
	const now = new Date();
	const year = now.getFullYear();
	const month = String(now.getMonth() + 1).padStart(2, "0");
	const day = String(now.getDate()).padStart(2, "0");
	const hours = String(now.getHours()).padStart(2, "0");
	const minutes = String(now.getMinutes()).padStart(2, "0");
	const seconds = String(now.getSeconds()).padStart(2, "0");
	return `log_${year}-${month}-${day}_${hours}-${minutes}-${seconds}`;
};

const createTimestampedLogDirectory = () => {
	const logDirectoryName = getLogDirectoryName();
	const baseLogDirectory = path.join(
		__dirname,
		"..",
		"..",
		"..",
		"..",
		"..",
		"..",
		"themis-logs"
	);
	const timestampedLogDirectory = path.join(baseLogDirectory, logDirectoryName);

	if (!fs.existsSync(timestampedLogDirectory)) {
		fs.mkdirSync(timestampedLogDirectory, { recursive: true });
		console.log(`Log directory created: ${timestampedLogDirectory}`);
	}
	return timestampedLogDirectory;
};

const newLogDirectory = createTimestampedLogDirectory();

// Create a logger instance for each custom level
const highRateLogger = pino({
	customLevels: { highRate: 45 },
	useOnlyCustomLevels: true,
	level: "highRate",
	base: null,
	transport: {
		target: "pino/file",
		options: {
			destination: path.join(newLogDirectory, "highRate.log"),
		},
	},
});

const normalRateLogger = pino({
	customLevels: { normalRate: 30 },
	useOnlyCustomLevels: true,
	level: "normalRate",
	base: null,
	transport: {
		target: "pino/file",
		options: {
			destination: path.join(newLogDirectory, "normalRate.log"),
		},
	},
});

const watchDogLogger = pino({
	level: "info",
	transport: {
		target: "pino/file",
		options: { destination: path.join(newLogDirectory, "watchdog.log") },
	},
});

export { highRateLogger, normalRateLogger, watchDogLogger };
