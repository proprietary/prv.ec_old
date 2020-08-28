import * as flatbuffers from 'flatbuffers';
import {ec_prv} from './fbs/url_index_generated';


(function() {
	console.log('hello typescript');
	const fbb = new flatbuffers.Builder(1024);
	const l = flatbuffers.createLong(4, 4);
	ec_prv.fbs.URLIndex.startURLIndex(fbb);
	ec_prv.fbs.URLIndex.addId(fbb, l);
	ec_prv.fbs.URLIndex.addVersion(fbb, 1);
	const ui = ec_prv.fbs.URLIndex.endURLIndex(fbb);
	fbb.finish(ui);
	
	const buf = fbb.asUint8Array();
	console.info(buf);
})();