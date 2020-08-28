const flatbuffers = require('flatbuffers');
const urli = require('./fbs/url_index_generated.js');

(function () {
	window.addEventListener("DOMContentLoaded", () => {
		console.log("hello typescript");
		const fbb = new flatbuffers.flatbuffers.Builder();
		const u = new urli.ec_prv.fbs.URLIndex();
		const a = urli.ec_prv.fbs.createURLIndex(fbb, 1, 4);
		fbb.finish(a);
		const b = fbb.asUint8Array();
		console.log(b);
	});
})();
