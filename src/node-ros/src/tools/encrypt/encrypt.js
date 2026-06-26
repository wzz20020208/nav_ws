import fs from "fs";
import path from "path";
import { globSync } from "glob";
import JavaScriptObfuscator from "javascript-obfuscator";

const SRC = "src";
const DIST = "dist";

fs.rmSync(DIST, { recursive: true, force: true });

const files = globSync(`${SRC}/**/*.js`, {
	ignore: ["**/node_modules/**"],
});

for (const file of files) {
	const code = fs.readFileSync(file, "utf8");

	const out = JavaScriptObfuscator.obfuscate(code, {
		compact: true,
		controlFlowFlattening: true,
		stringArray: true,
		stringArrayEncoding: ["base64"],
		renameGlobals: false,
	});

	const target = file.replace(/^src/, DIST);
	fs.mkdirSync(path.dirname(target), { recursive: true });
	fs.writeFileSync(target, out.getObfuscatedCode());
}

console.log("done");
