import { flatbuffers } from '../vendor/flatbuffers/flatbuffers';
import * as lookup_request from '../fbs/lookup_request_generated';
import * as lookup_response from '../fbs/lookup_response_generated';
import * as lookup_request_web from '../fbs/lookup_request_web_generated';
const {LookupRequest, LookupRequestT} = lookup_request.ec_prv.fbs;
const {LookupRequestWeb} = lookup_request_web.ec_prv.fbs;
const {LookupResponse, LookupResponseT} = lookup_response.ec_prv.fbs;
import { post } from './common';

export async function lookupRequestV1(urlIndex: lookup_request.ec_prv.fbs.LookupRequestT): Promise<null|lookup_response.ec_prv.fbs.LookupResponseT> {
	throw new Error("unimplemented");
}

export async function lookupRequestWeb(urlIndex: string): Promise<null|lookup_response.ec_prv.fbs.LookupResponseT> {
	const fbb = new flatbuffers.Builder();
	const lk = fbb.createString(urlIndex);
	LookupRequestWeb.startLookupRequestWeb(fbb);
	LookupRequestWeb.addLookupKey(fbb, lk);
	const lrw = LookupRequestWeb.endLookupRequestWeb(fbb);
	fbb.finish(lrw);
	const fb = fbb.asUint8Array();
	const resp = await post('lookup_request_web', fb);
	if (resp == null) {
		return null;
	}
	const lookupResponse = LookupResponse.getRootAsLookupResponse(new flatbuffers.ByteBuffer(new Uint8Array(resp)));
	return lookupResponse.unpack();
}
