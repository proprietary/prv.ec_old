import { shorteningRequestV1, lookupRequestV1 } from '../rpc';
import {
	encryptV1,
	decryptV1,
	deriveKeyV1,
	DeriveKeyV1Params,
	IV_BYTES_V1,
	PASS_BYTES_V1,
	SALT_BYTES_V1,
} from '../crypto';
import { fromByteArray, toByteArray, fromURLSafeBase64 } from '../util';
import * as url_index from '../fbs/url_index_generated';
import * as private_url from '../fbs/private_url_generated';
import * as flatbuffers from 'flatbuffers';

export class ShortenerClient {
	private iv_: Uint8Array = new Uint8Array(IV_BYTES_V1);
	private pass_: Uint8Array = new Uint8Array(PASS_BYTES_V1);
	private salt_: Uint8Array = new Uint8Array(SALT_BYTES_V1);
	private expiry_: Date = defaultExpiry();

	public constructor() {
		window.crypto.getRandomValues(this.iv_);
		window.crypto.getRandomValues(this.pass_);
		window.crypto.getRandomValues(this.salt_);
	}

	public async shorten(url: string) {
		const key = await deriveKeyV1({
			salt: this.salt_,
			iv: this.iv_,
			pass: this.pass_,
		});
		const m = await encryptV1({
			key,
			iv: this.iv_,
			url,
		});
		const identifier = await shorteningRequestV1(
			new Uint8Array(m),
			this.iv_,
			this.salt_,
			this.expiry_,
		);
		const passAsStr = fromByteArray(this.pass_);
		return `https://prv.ec/${identifier}#${passAsStr}`;
	}

	public set expiry(d: Date) {
		this.expiry_ = d;
	}

	public get expiry() {
		return this.expiry_;
	}
}

export class LookupClient {
	private identifier_: Uint8Array; // 32-bit little-endian unsigned integer
	private pass_: Uint8Array;

	public constructor(identifier: string, pass: string) {
		// decode identifier string to bytes
		this.identifier_ = toByteArray(identifier);
		if (this.identifier_.length != 4) {
			throw new Error();
		}
		this.pass_ = toByteArray(pass);
	}

	private buildUrlIndex() {
		// little-endian byte array to u32
		let n: number = 0;
		n = this.identifier_[0] | (this.identifier_[1] << 8) | (this.identifier_[2] << 16) | (this.identifier_[3] << 24);
		const fbb = new flatbuffers.Builder();
		const ui = url_index.ec_prv.fbs.URLIndex.createURLIndex(fbb, 1, n);
		fbb.finish(ui);
		return url_index.ec_prv.fbs.URLIndex.getRootAsURLIndex(fbb.dataBuffer()).unpack();
	}

	public async lookup(): Promise<string> {
		const ui = this.buildUrlIndex();
		let r = await lookupRequestV1(ui);
		if (r == null) {
			return '';
		}
		let pu = private_url.ec_prv.fbs.PrivateURL.getRootAsPrivateURL(
			new flatbuffers.ByteBuffer(new Uint8Array(r.data)),
		).unpack();
		const key = await deriveKeyV1({
			salt: new Uint8Array(pu.salt),
			iv: new Uint8Array(pu.iv),
			pass: this.pass_,
		});
		const p = await decryptV1({
			key,
			iv: new Uint8Array(pu.iv),
			ciphertext: new Uint8Array(pu.blindedUrl),
		});
		return new TextDecoder('utf-8').decode(p);
	}
}

function defaultExpiry(): Date {
	return new Date(
		Math.floor(new Date().getTime() / 1000) + 60 * 60 * 24 * 365,
	);
}
