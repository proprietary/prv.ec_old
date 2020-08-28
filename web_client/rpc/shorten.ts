import * as shortening_request from '../fbs/shortening_request_generated';
import * as shortening_response from '../fbs/shortening_response_generated';
import * as private_url from '../fbs/private_url_generated';
import * as url_index from '../fbs/url_index_generated';
const { URLIndex } = url_index.ec_prv.fbs;
const { ShorteningRequestT, ShorteningRequest } = shortening_request.ec_prv.fbs;
const {
	ShorteningResponseT,
	ShorteningResponse,
} = shortening_response.ec_prv.fbs;
const { PrivateURL, PrivateURLT } = private_url.ec_prv.fbs;
import * as flatbuffers from 'flatbuffers';
import { post } from './common';
import { toLong } from '../util';

/**
 * Makes a request to shorten a URL and returns the generated identifier representing the shortened URL.
 * @param {Uint8Array} blindedUrl - the ciphertext of the URL
 * @param {Uint8Array} iv - initialization vector used to encrypt blindedUrl
 * @param {Uint8Array} salt - salt used to derive the key used to encrypt blindedUrl
 * @returns {Promise<string>} Promise object represents the identifier associated with the successfully generated record; for example, the returned string would be "IDENTIFIER" in "https://prv.ec/IDENTIFIER#p4ss". If the request fails, an empty string is returned.
 */
export async function shorteningRequestV1(
	blindedUrl: Uint8Array,
	iv: Uint8Array,
	salt: Uint8Array,
	expiry: Date,
): Promise<string> {
	const fbb = new flatbuffers.Builder();
	ShorteningRequest.startShorteningRequest(fbb);
	const expiryAsSeconds = toLong(Math.floor(expiry.getTime()/1000));
	const saltVec = ShorteningRequest.createSaltVector(fbb, salt);
	const ivVec = ShorteningRequest.createIvVector(fbb, iv);
	const blindedUrlVector = ShorteningRequest.createBlindedUrlVector(
		fbb,
		blindedUrl,
	);
	ShorteningRequest.addExpiry(fbb, expiryAsSeconds);
	ShorteningRequest.addVersion(fbb, 1);
	ShorteningRequest.addIv(fbb, ivVec);
	ShorteningRequest.addSalt(fbb, saltVec);
	ShorteningRequest.addBlindedUrl(fbb, blindedUrlVector);
	const sr = ShorteningRequest.endShorteningRequest(fbb);
	fbb.finish(sr);
	const resp = await post('shortening_request', fbb.dataBuffer().bytes());
	if (resp == null) {
		return '';
	}
	const ret = ShorteningResponse.getRootAsShorteningResponse(
		new flatbuffers.ByteBuffer(new Uint8Array(resp)),
	);
	if (ret.error()) {
		return '';
	}	
	const lookupKeyBuf = Uint8Array.from(ret.unpack().lookupKey);
	const urlIndex = URLIndex.getRootAsURLIndex(
		new flatbuffers.ByteBuffer(lookupKeyBuf),
	);
	return urlIndex.id.toString();
}
