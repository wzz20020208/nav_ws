import fs from 'fs';
import { readdir } from 'fs/promises';
import path from 'path';
import dotenv from 'dotenv';
dotenv.config({ path: path.join(import.meta.dirname, '..', '..', '..', '.env') });
const LOGS_PATH = process.env.HOME_DIR || '/home/themis' + '/THEMIS/themis-logs';
const keepDays = 3;
const dateRegex = /\d{4}-\d{2}-\d{2}/;

const getLogDatesForCleaning = (files) => {
	const allDates = files
		.map((f) => {
			const match = f.match(dateRegex);
			return match ? match[0] : null;
		})
		.filter(Boolean);

	const distinctDates = [...new Set(allDates)].sort();

	if (distinctDates.length <= keepDays) {
		return [];
	}

	return distinctDates.slice(0, distinctDates.length - keepDays);
};

const cleanOldLogs = (datesToClean, files) => {
	const cleanSet = new Set(datesToClean);

	files.forEach((name) => {
		const match = name.match(dateRegex);
		const fileDate = match ? match[0] : null;

		if (fileDate && cleanSet.has(fileDate)) {
			try {
				const fullPath = path.join(LOGS_PATH, name);
				fs.rmSync(fullPath, { recursive: true, force: true });
				console.log(`Successfully deleted: ${name}`);
			} catch (err) {
				console.error(`Failed to delete ${name}:`, err.message);
			}
		}
	});
};

const main = async () => {
	try {
		if (!fs.existsSync(LOGS_PATH)) {
			process.exit(0);
		}

		const files = await readdir(LOGS_PATH);

		if (files.length > 0) {
			const datesToClean = getLogDatesForCleaning(files);
			if (datesToClean.length > 0) {
				cleanOldLogs(datesToClean, files);
			}
		}
		process.exit(0);
	} catch (error) {
		console.error('Cleaning task failed:', error);
		process.exit(1);
	}
};

main();
