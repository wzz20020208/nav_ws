import Database from 'better-sqlite3';
import { NUC_HOMEDIR } from '../server/bashUtil.js';
import path from 'path';
const MASTER_CONTROL_LEVEL = 7;

const database_dir = path.join(NUC_HOMEDIR + '/THEMIS/THEMIS/Beta/assets/database', 'connected_devices.db');

const db = new Database(database_dir);
db.pragma('journal_mode = WAL');

db.prepare(
	`
CREATE TABLE IF NOT EXISTS connected_devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    deviceId TEXT,
    type TEXT,
    browser TEXT,
    os TEXT,
    ip TEXT,
    connectAt TEXT NOT NULL,
    disconnectAt TEXT,
    controlLevel NUMBER DEFAULT 0
)`
).run();

const checkOnlineStmt = db.prepare(`
    SELECT COUNT(*) as count FROM connected_devices WHERE deviceId = @id AND disconnectAt IS NULL
`);

const checkMasterStmt = db.prepare(`
    SELECT COUNT(*) as count FROM connected_devices WHERE controlLevel = @MASTER_CONTROL_LEVEL AND disconnectAt IS NULL
`);

const getAllOnlineDevicesStmt = db.prepare(`
    SELECT * FROM connected_devices WHERE disconnectAt IS NULL
`);

const markDeviceDisconnected = db.prepare(`
    UPDATE connected_devices SET disconnectAt = @disconnectAt WHERE deviceId = @deviceId
`);

const getAllDevicesStmt = db.prepare(`
    SELECT * FROM connected_devices
`);

export const dbManager = {
	saveDevice(device) {
		const sql = `INSERT OR REPLACE INTO connected_devices (deviceId, controlLevel, type, browser, os, ip, connectAt, disconnectAt)
                     VALUES (@deviceId, @controlLevel,@type, @browser, @os, @ip, @connectAt, @disconnectAt)`;
		const stmt = db.prepare(sql);
		stmt.run(device);
	},

	getMasterId() {
		const sql = `SELECT deviceId FROM connected_devices WHERE controlLevel = 0`;
		return db.prepare(sql).get();
	},

	isOnline(deviceId) {
		const row = checkOnlineStmt.get({ id: deviceId });
		return row.count > 0;
	},

	isMasterOnline() {
		const row = checkMasterStmt.get({ MASTER_CONTROL_LEVEL });
		return row.count > 0;
	},
	getAllOnlineDevices() {
		return getAllOnlineDevicesStmt.all();
	},

	getAllDevices() {
		return getAllDevicesStmt.all();
	},

	markDeviceDisconnected(deviceId) {
		const disconnectAt = new Date().toISOString();
		markDeviceDisconnected.run({ deviceId, disconnectAt });
	},
};

// console.log(db.prepare("delete FROM connected_devices").run());

console.log(new Date(), 'Connected Devices DataBase initialized');
