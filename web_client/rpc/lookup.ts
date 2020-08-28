import * as flatbuffers from 'flatbuffers';
import * as lookup_request from '../fbs/lookup_request_generated';
import * as lookup_response from '../fbs/lookup_response_generated';
import * as url_index from '../fbs/url_index_generated';
const {URLIndex} = url_index.ec_prv.fbs;
const {LookupRequest, LookupRequestT} = lookup_request.ec_prv.fbs;
const {LookupResponse, LookupResponseT} = lookup_response.ec_prv.fbs;
import { post } from './common';


export async function lookupRequestV1(urlIndex: url_index.ec_prv.fbs.URLIndexT): Promise<null|lookup_response.ec_prv.fbs.LookupResponseT> {
	const urlIndexFbb = new flatbuffers.Builder();
	const ui = urlIndex.pack(urlIndexFbb);
	urlIndexFbb.finish(ui);
	const reqFbb = new flatbuffers.Builder();
	const lkv = LookupRequest.createLookupKeyVector(reqFbb, urlIndexFbb.dataBuffer().bytes());
	LookupRequest.startLookupRequest(reqFbb);
	LookupRequest.addVersion(reqFbb, 1);
	LookupRequest.addLookupKey(reqFbb, lkv);
	const lr = LookupRequest.endLookupRequest(reqFbb);
	reqFbb.finish(lr);
	const resp = await post('lookup_request', reqFbb.dataBuffer().bytes());
	if (resp == null) {
		return null;
	}
	const lookupResponse = LookupResponse.getRootAsLookupResponse(new flatbuffers.ByteBuffer(new Uint8Array(resp)));
	return lookupResponse.unpack();
}
