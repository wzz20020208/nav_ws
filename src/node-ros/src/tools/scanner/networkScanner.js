import { exec } from 'child_process';
import NetworkScanner from 'network-scanner-js';
import {
	NUC_CONSTANT,
	BODY_CONSTANT,
	JETSON_CONSTANT,
} from '../../server/bashUtil.js';

const ROBOT_PCS = {
	main: NUC_CONSTANT,
	body: BODY_CONSTANT,
	head: JETSON_CONSTANT,
};

const ROBOT_IPS = Object.values(ROBOT_PCS).map((pc) => pc.address);

const THERMAL_CMD = 'cat /sys/class/thermal/thermal_zone*/temp 2>/dev/null';

const netScan = new NetworkScanner();
const SCAN_INTERVAL_MS = 3000;
const SSH_TIMEOUT_MS = 8000;

const emptyDevice = (pc) => ({
	host: pc.address,
	ip_address: pc.address,
	status: 'offline',
	res_avg: '',
	packet_loss: '',
	log: '',
	times: [],
	temperature: null,
});

const robotDevices = Object.fromEntries(
	Object.entries(ROBOT_PCS).map(([role, pc]) => [role, emptyDevice(pc)])
);

const parseMaxTemperature = (stdout) => {
	const raw = (stdout || '')
		.trim()
		.split(/\s+/)
		.map((v) => parseInt(v, 10))
		.filter((n) => !Number.isNaN(n));
	if (raw.length === 0) return null;
	const celsius = raw.map((v) => (v > 1000 ? v / 1000 : v));
	return Math.round(Math.max(...celsius) * 10) / 10;
};

const runShell = (command, timeoutMs) =>
	new Promise((resolve) => {
		exec(command, { timeout: timeoutMs }, (err, stdout) => {
			if (err) {
				resolve(null);
				return;
			}
			resolve(stdout);
		});
	});

const readMaxTemperature = async (pc) => {
	const ip = pc.address;
	let stdout;
	if (ip === ROBOT_PCS.main.address) {
		stdout = await runShell(THERMAL_CMD, 5000);
	} else {
		const remoteCmd = THERMAL_CMD.replace(/'/g, "'\\''");
		const sshCmd = `sshpass -p '${pc.password}' ssh -o StrictHostKeyChecking=no -o ConnectTimeout=3 ${pc.username}@${ip} '${remoteCmd}'`;
		stdout = await runShell(sshCmd, SSH_TIMEOUT_MS);
	}
	return parseMaxTemperature(stdout);
};

const toPingRecord = (pc) => ({
	host: pc.host,
	ip_address: pc.ip_address,
	status: pc.status,
	res_avg: pc.res_avg,
	packet_loss: pc.packet_loss,
	log: pc.log ?? '',
	times: pc.times || [],
});

const clusterPingAll = () =>
	new Promise((resolve, reject) => {
		netScan.clusterPing(ROBOT_IPS, (nodes) => {
			if (!Array.isArray(nodes)) {
				reject(new Error('Invalid clusterPing result'));
				return;
			}
			resolve(nodes.filter((pc) => pc && !(pc instanceof Error)));
		});
	});

const emitScannedResults = (onData) => {
	if (!onData) return;
	onData({
		allDevices: Object.values(robotDevices),
		robotDevices: { ...robotDevices },
	});
};

const applyPingResults = (pingResults) => {
	for (const [role, pc] of Object.entries(ROBOT_PCS)) {
		const hit = pingResults.find(
			(p) => (p.ip_address || p.host) === pc.address
		);
		robotDevices[role] = {
			...(hit ? toPingRecord(hit) : emptyDevice(pc)),
			temperature: robotDevices[role].temperature,
		};
	}
};

export const syncOnlineDevices = (onData, onError) => {
	let pingInProgress = false;
	let tempInProgress = false;

	const runPing = async () => {
		if (pingInProgress) return;
		pingInProgress = true;
		try {
			const pingResults = await clusterPingAll();
			applyPingResults(pingResults);
			emitScannedResults(onData);
		} catch (error) {
			if (onError) onError(error.message);
		} finally {
			pingInProgress = false;
			setTimeout(runPing, SCAN_INTERVAL_MS);
		}
	};

	const runTemp = async () => {
		if (tempInProgress) return;
		tempInProgress = true;
		try {
			await Promise.all(
				Object.entries(ROBOT_PCS).map(async ([role, pc]) => {
					const temperature = await readMaxTemperature(pc);
					robotDevices[role] = {
						...robotDevices[role],
						temperature,
					};
				})
			);
			emitScannedResults(onData);
		} catch (error) {
			if (onError) onError(error.message);
		} finally {
			tempInProgress = false;
			setTimeout(runTemp, SCAN_INTERVAL_MS);
		}
	};

	runPing();
	runTemp();
};
