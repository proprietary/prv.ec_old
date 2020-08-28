'use strict';

// Uint8Array -> string
function b64enc(u8Array) {
	return window.btoa(String.fromCharCode.apply(null, u8Array));
}

// string -> Uint8Array
function b64dec(s) {
	const raw = window.atob(s);
	const arr = [];
	for (let i = 0; i < raw.length; ++i) {
		const cc = raw.charCodeAt(i);
		arr.push(cc);
	}
	return new Uint8Array(arr);
}

export class URLKey {
	constructor(privkey, iv) {
		this.privkey = privkey;
		this.iv = iv;
	}

	static derive() {
		const key = window.crypto.getRandomValues(new Uint8Array(32));
		const iv = window.crypto.getRandomValues(new Uint8Array(32));
		return new URLKey(key, iv);
	}

	decode(s) {
		const [ privkeyEncoded, ivEncoded ] = s.split('!');
		if (privkeyEncoded == null || ivEncoded == null) {
			throw new Error();
		}
		const privkeyDecoded = b64dec(privkeyEncoded);
		const ivDecoded = b64dec(ivEncoded);
		return new URLKey(privkeyDecoded, ivDecoded);
	}

	static encode() {
		const privkeyEncoded = b64enc(this.privkey);
		const ivEncoded = b64enc(this.iv);
		return privkeyEncoded + '!' + ivEncoded;
	}
}


function getKeyMaterial(password) {
	return window.crypto.subtle.importKey(
		"raw",
		new TextEncoder().encode(password),
		"PBKDF2",
		false,
		["deriveBits", "deriveKey"]
	);
}

function randomKey() {
	window.crypto.getRandomValues();
}

async function deriveKeyFromPassword(password) {
	const keyMaterial = await getKeyMaterial(password);
	return window.crypto.subtle.deriveKey({
		name: 'PBKDF2',
		salt: new Uint8Array([1,2,3]),
		hash: 'SHA-256',
		iterations: 10_000,
	}, keyMaterial, {
		name: 'AES-GCM',
		length: 256,
	}, true, ["encrypt", "decrypt"])
}
