import { spawn } from "child_process";
import path from "path";
import { fileURLToPath } from "url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));

let pyProcess = null;
let isShutdowning = false;
export const startPythonProjector = () => {
	if (pyProcess) {
		console.warn("Python 投影脚本已经在运行中。");
		return pyProcess;
	}
	isShutdowning = false; // 启动时重置标志位

	const scriptPath = path.resolve(__dirname, "./path_projector.py");

	// 启动子进程
	pyProcess = spawn("python3", ["-u", scriptPath], {
		stdio: "pipe",
		shell: false,
	});

	// pyProcess.stdout.on("data", (data) => {
	// 	const message = data.toString().trim();
	// 	console.log(`path projector data: ${message}`);
	// });

	pyProcess.stderr.on("data", (data) => {
		console.error(`[Python Error]: ${data.toString()}`);
	});

	pyProcess.on("error", (err) => {
		console.error("无法启动 Python 脚本:", err);
	});

	pyProcess.on("close", (code) => {
		console.log(`Python 投影脚本已退出， exit code:${code}`);
		pyProcess = null;

		if (code !== 0 && !isShutdowning) {
			console.log("python脚本挂了....");
			if (!isShutdowning) startPythonProjector();
		}
	});

	return pyProcess;
};

export const stopPythonProjector = () => {
	if (pyProcess) {
		isShutdowning = true;
		pyProcess.kill("SIGINT");
		const pid = pyProcess.pid;
		setTimeout(() => {
			try {
				process.kill(pid, "SIGKILL");
			} catch (e) {}
		}, 500);
		pyProcess = null;
	}
};
