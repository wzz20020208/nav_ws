import dgram from "dgram";

// === 1. 初始化 (对应 C++ lcm_("udpm://...")) ===
const LCM_TARGET_IP = "239.255.76.67";
const LCM_TARGET_PORT = 7667;
const udpSocket = dgram.createSocket({ type: "udp4", reuseAddr: true });

// 开启组播功能 (对应 C++ ?ttl=1)
udpSocket.bind(() => {
	udpSocket.setBroadcast(true);
	udpSocket.setMulticastTTL(1);
	console.log("[GAMEPAD] LCM UDP Initialized");
});

// === 2. 核心广播函数 ===
function sendGamepadToLCM(jsData) {
	// A. 准备头部信息 (这是 LCM 协议规定的信封，必须有)
	const channelName = "GAMEPAD_DATA";
	const channelBuf = Buffer.from(channelName);

	// 计算总长度 = 头部魔法数字(4) + 序列号(4) + 频道名长度(N) + 结束符(1) + 哈希(8) + 数据长度(20 + 6*4)
	// 数据长度 = 20个int8(20字节) + 6个float(24字节) = 44字节
	const packetSize = 4 + 4 + channelBuf.length + 1 + 8 + 45;

	const buf = Buffer.alloc(packetSize);
	let offset = 0;

	// --- LCM 协议头 (固定的，照抄就行) ---
	buf.writeUInt32BE(0x4c433032, offset);
	offset += 4; // Magic "LC02"
	buf.writeUInt32BE(0, offset);
	offset += 4; // SeqNo
	channelBuf.copy(buf, offset);
	offset += channelBuf.length;
	buf.writeUInt8(0, offset);
	offset += 1; // 字符串结尾

	// --- 结构体指纹 (Hash) ---
	// 这个数字代表了 "struct GamepadData" 的唯一身份证
	// 如果这个不对，C++ 接收端会认为数据格式不匹配而丢弃
	// 这是根据你提供的 C++ 代码算出来的固定值
	buf.writeBigUInt64BE(0xf54acc2e09a92660n, offset);
	offset += 8;

	// --- 开始填充 Struct 数据 (这里就是你问的 struct 用在哪) ---
	// 必须严格遵守 struct 定义的顺序！

	// 20个 int8
	buf.writeInt8(jsData.A || 0, offset++);
	buf.writeInt8(jsData.B || 0, offset++);
	buf.writeInt8(jsData.Y || 0, offset++);
	buf.writeInt8(jsData.X || 0, offset++);
	buf.writeInt8(jsData.LS || 0, offset++);
	buf.writeInt8(jsData.RS || 0, offset++);
	buf.writeInt8(jsData.LS2 || 0, offset++);
	buf.writeInt8(jsData.RS2 || 0, offset++);
	buf.writeInt8(jsData.BK || 0, offset++);
	buf.writeInt8(jsData.ST || 0, offset++);
	buf.writeInt8(jsData.LZ || 0, offset++);
	buf.writeInt8(jsData.RZ || 0, offset++);
	buf.writeInt8(jsData.U || 0, offset++);
	buf.writeInt8(jsData.D || 0, offset++);
	buf.writeInt8(jsData.L || 0, offset++);
	buf.writeInt8(jsData.R || 0, offset++);
	buf.writeInt8(jsData.STAND || 0, offset++);
	buf.writeInt8(jsData.WALK || 0, offset++);
	buf.writeInt8(jsData.NAVIGATION || 0, offset++);
	buf.writeInt8(jsData.MANIPULATION || 0, offset++);
	buf.writeInt8(jsData.APF || 0, offset++);

	// 6个 float (注意 writeFloatBE，占用4字节)
	buf.writeFloatBE(jsData.LX || 0.0, offset);
	offset += 4;
	buf.writeFloatBE(jsData.LY || 0.0, offset);
	offset += 4;
	buf.writeFloatBE(jsData.RX || 0.0, offset);
	offset += 4;
	buf.writeFloatBE(jsData.RY || 0.0, offset);
	offset += 4;
	buf.writeFloatBE(jsData.L2 || 0.0, offset);
	offset += 4;
	buf.writeFloatBE(jsData.R2 || 0.0, offset);
	offset += 4;

	// === 3. 发送 (对应 C++ lcm_.publish) ===
	udpSocket.send(buf, 0, buf.length, LCM_TARGET_PORT, LCM_TARGET_IP, (err) => {
		if (err) console.error("LCM Broadcast Error:", err);
	});
}

// 导出这个函数供 WebSocket 接收处调用
export { sendGamepadToLCM };
