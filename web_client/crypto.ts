export async function deriveKeyV1(
	options: DeriveKeyV1Params,
): Promise<CryptoKey> {
	if (options.pbkdf2Rounds == null) {
		options.pbkdf2Rounds = DEFAULT_PBKDF2_ROUNDS_V1;
	}
	const passKey = await window.crypto.subtle.importKey(
		'raw',
		options.pass,
		'PBKDF2',
		false,
		['deriveKey'],
	);
	const k = await window.crypto.subtle.deriveKey(
		{
			name: 'PBKDF2',
			salt: options.salt,
			iterations: options.pbkdf2Rounds,
			hash: 'SHA-256',
		},
		passKey,
		{ name: 'AES-GCM', length: KEY_BYTES_V1 * 8 },
		true,
		['encrypt', 'decrypt'],
	);
	return k;
}

export interface DeriveKeyV1Params {
	pbkdf2Rounds?: number;
	salt: Uint8Array;
	iv: Uint8Array;
	pass: Uint8Array;
}

export async function encryptV1(
	options: EncryptV1Params,
): Promise<ArrayBuffer> {
	if (options.iv.byteLength != IV_BYTES_V1) {
		throw new Error();
	}
	const encoded = new TextEncoder().encode(options.url);
	let ciphertext = await window.crypto.subtle.encrypt(
		{
			name: 'AES-GCM',
			iv: options.iv,
			additionalData: AAD_DATA_V1,
			tagLength: TAG_BYTES_V1 * 8,
		},
		options.key,
		encoded,
	);
	return ciphertext;
}

export interface EncryptV1Params {
	key: CryptoKey;
	iv: Uint8Array;
	url: string;
}

export async function decryptV1(
	options: DecryptV1Params,
): Promise<ArrayBuffer> {
	if (options.iv.byteLength != IV_BYTES_V1) {
		throw new Error();
	}
	const r = await crypto.subtle.decrypt(
		{
			name: 'AES-GCM',
			iv: options.iv,
			additionalData: AAD_DATA_V1,
			tagLength: TAG_BYTES_V1 * 8,
		},
		options.key,
		options.ciphertext,
	);
	return r;
}

export interface DecryptV1Params {
	key: CryptoKey;
	iv: Uint8Array;
	ciphertext: Uint8Array;
}

export const DEFAULT_PBKDF2_ROUNDS_V1 = 2_000_000;
export const AAD_DATA_V1 = new TextEncoder().encode('www.prv.ec');
export const SALT_BYTES_V1 = 16;
export const IV_BYTES_V1 = 12;
export const PASS_BYTES_V1 = 3;
export const KEY_BYTES_V1 = 32;
export const TAG_BYTES_V1 = 16;
