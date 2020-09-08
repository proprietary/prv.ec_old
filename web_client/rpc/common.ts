export type RPCMethodName = "lookup_request" | "shortening_request" | "lookup_request_web";

export async function post(name: RPCMethodName, message: ArrayBuffer): Promise<ArrayBuffer|undefined> {
	const r = await window.fetch('/accept', {
		method: 'POST',
		headers: {
			'x-rpc-method': name,
		},
		body: new Uint8Array(message),
	});
	console.log(message);
	if (r.status == 200) {
		const resp = await r.arrayBuffer();
		return resp;
	}
}
