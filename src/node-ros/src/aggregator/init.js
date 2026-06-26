import rclnodejs from "rclnodejs";

let initialized = false;

async function init(nodename) {
	if (!initialized) {
		console.log(`[${nodename}] Starting rclnodejs.init() ...`);
		try {
			await rclnodejs.init();
			initialized = true;
			console.log(`[${nodename}] rclnodejs initialized successfully`);
		} catch (err) {
			console.error(`[${nodename}] rclnodejs.init() failed:`, err);
			throw err;
		}
	}
	return rclnodejs;
}

export default init;
