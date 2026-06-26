import { execSync } from 'child_process';
import path from 'path';
import fs from 'fs';

import dotenv from 'dotenv';
dotenv.config({ path: path.join(import.meta.dirname, '..', '..', '..', '.env') });

const HOME_DIR = process.env.HOME_DIR || '/home/themis/';
const logsDir = path.join(HOME_DIR, 'themis-logs');
const compressDir = path.join(HOME_DIR, 'themis-logs-archived');

if (!fs.existsSync(compressDir)) {
	fs.mkdirSync(compressDir, { recursive: true });
	console.log(`Log directory created: ${compressDir}`);
}

const zipFile = path.join(compressDir, `backup_log.zip`);
const encFile = `${zipFile}.enc`;

const password = process.env.BACKUP_PASS;

if (!password) {
	console.error('❌ BACKUP_PASS is not set. Please configure it in systemd or .env file');
	process.exit(1);
}

try {
	fs.mkdirSync(compressDir, { recursive: true });

	console.log(`📦 Compressing logs...`);
	execSync(`cd "${path.dirname(logsDir)}" && zip -9 -r "${zipFile}" "${path.basename(logsDir)}"`, {
		stdio: 'inherit',
	});

	console.log(`🔐 Encrypting with OpenSSL...`);
	execSync(`openssl enc -aes-256-cbc -pbkdf2 -salt -pass pass:${password} -in "${zipFile}" -out "${encFile}"`, {
		stdio: 'inherit',
	});

	console.log(`✅ Encrypted file created: ${encFile}`);

	fs.rmSync(zipFile);
	console.log(`🧹 Deleted unencrypted file: ${zipFile}, process complete`);
	process.exit(0);
} catch (err) {
	console.error('❌ Backup failed:', err.message);
	process.exit(1);
}

//unzip:

/*
	openssl enc -d -aes-256-cbc -pbkdf2 -salt -pass pass:"romela111111" -in backup_log.zip.enc -out backup_logs_decrypted.zip
*/

/*
[Service]
ExecStart=/usr/bin/node /opt/app/secure_backup.js
WorkingDirectory=/opt/app
Environment="BACKUP_PASS=YourServerStrongPassword!"
User=ubuntu
*/
