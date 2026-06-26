import { UAParser } from "ua-parser-js";
import requestIp from "request-ip";
import msgpack from "@msgpack/msgpack";
import { WebSocket } from 'ws';

const extractIPv4 = (ip) => {
	const ipv4Pattern = /\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}\b/;
	const match = ip.match(ipv4Pattern);
	return match ? match[0] : ip;
};

export const getDeviceInfo = (reqHeader, req) => {
	const parser = new UAParser(reqHeader);
	let ip = requestIp.getClientIp(req) || "Unknown";

	ip = ip === "::1" ? "127.0.0.1" : extractIPv4(ip);
	return {
		browser: parser.getBrowser().name,
		device: parser.getDevice().type,
		os: parser.getOS().name,
		ip,
	};
};

//setup type
export const sendReply = (ws, type, dataType, state) => {
	if (ws.readyState === WebSocket.OPEN) {
		const reply = {
			type: type,
			data: {
				type: dataType,
				state: state,
			},
		};
		const binaryPayload = msgpack.encode(reply);
		ws.send(binaryPayload);
		console.log(`[THREAD_STATE] type: ${type}, name: ${dataType}, state: ${state}`);
	}
};

export const sendSegmentRply = (ws, isOpen) => {
	if (ws.readyState === WebSocket.OPEN) {
		const reply = { type: "segmentMap_rp", data: { isOpen } };
		ws.send(msgpack.encode(reply));
	}
};

export const sendResponse = (ws, type, data) => {
	if (ws && ws.readyState === WebSocket.OPEN) {
		try {
			const reply = { type, data };
			ws.send(msgpack.encode(reply));
			console.log(`[REPLY] Sent ${type}: ${JSON.stringify(data)} \n`);
		} catch (err) {
			console.error(`[REPLY ERROR] Failed to encode/send ${type}: ${err} \n`);
		}
	} else {
		console.warn(`[REPLY WARN] Connection closed. Dropping message: ${type} \n`);
	}
};

export const broadcast = (wss, data, isVideo) => {
	let payload = isVideo ? data : msgpack.encode(data);
	wss.clients.forEach((client) => {
		if (client.readyState === WebSocket.OPEN) {
			if (client.bufferAmount > 1024 * 1024) {
				return;
			}
			client.send(payload);
		}
	});
};
