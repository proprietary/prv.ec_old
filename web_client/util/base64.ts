/**
 * Encodes a byte array to a base64-encoded string.
 * @param {Uint8Array} bytes - byte array
 * @returns {string} url-safe base64-encoded string
 */
export function fromByteArray(bytes: Uint8Array): string {
	let s = '';
	for (let i = 0; i < bytes.byteLength; ++i) {
		s += String.fromCharCode(bytes[i]);
	}
	let b = window.btoa(s);
	const urlSafeStr = b.replace('+', '-').replace('/', '_');
	return urlSafeStr;
}

export function toByteArray(base64EncodedString: string): Uint8Array {
	if (isURLSafe(base64EncodedString)) {
		base64EncodedString = fromURLSafeBase64(base64EncodedString);
	}
	const decoded = window.atob(base64EncodedString);
	const r = new Array(decoded.length);
	for (let i = 0; i < decoded.length; ++i) {
		r[i] = decoded.charCodeAt(i);
	}
	return new Uint8Array(r);
}

export function toURLSafeBase64(s: string): string {
	return s.replace('+', '-').replace('/', '_');
}

export function fromURLSafeBase64(s: string): string {
	return s.replace('-', '+').replace('_', '/');
}

function isURLSafe(s: string): boolean {
	return s.indexOf('-') !== -1 || s.indexOf('_') !== -1;
}