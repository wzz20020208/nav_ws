// videoNode.js
import { wssZed2i, wssZedxm } from '../server/server1.js';
import { broadcast } from '../server/serverUtil.js';
export default class VideoSub {
	constructor(rcl, nodeName, topics, getQosSetting) {
		this.node = new rcl.Node(nodeName);
		this.LATENCY_THRESHOLD = 500;
		this.topics = topics;
		this.rcl = rcl;
		this.getQosSetting = getQosSetting;
	}

	async start() {
		console.log(`[VideoSub] Waiting 2s before subscribing to video...`);
		await new Promise((resolve) => setTimeout(resolve, 2000));
		this.setupSubscriptions();
		this.node.spin();
		console.log(`[VideoSub] Video subscriptions active.`);
	}

	buildImagePacket(msg) {
		const rawData = msg.data;
		const dataLen = rawData.length;
		const msgMs = Number(msg.header.stamp.sec) * 1000 + Math.floor(msg.header.stamp.nanosec / 1000000);
		const nowMs = Date.now();
		if (nowMs - msgMs > this.LATENCY_THRESHOLD) {
			//console.log(`video dropped - ${nowMs - msgMs} diff ms `);
			return null;
		}
		const packet = Buffer.allocUnsafe(20 + dataLen);
		packet.writeUInt32LE(dataLen, 0);
		packet.writeBigUInt64LE(BigInt(msgMs), 4);
		packet.writeUInt32LE(1280, 12);
		packet.writeUInt32LE(720, 16);
		packet.set(rawData, 20);

		return packet;
	}

	setupSubscriptions() {
		this.topics.forEach((topic) => {
			const qos = this.getQosSetting(this.rcl, 1, true);

			this.node.createSubscription(topic.type, topic.topic, { qos }, (msg) => {
				const sendData = this.buildImagePacket(msg);
				//console.log(sendData);
				if (sendData && topic.name === 'zed2i_video') {
					broadcast(wssZed2i, sendData, true);
				} else if (sendData && topic.name === 'zedxm_video') {
					broadcast(wssZedxm, sendData, true);
				}
			});
		});
	}

	spin() {
		this.node.spin();
	}
}
