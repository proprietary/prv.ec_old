export type RPCMethodName = "lookup_request" | "shortening_request";

export async function post(name: RPCMethodName, message: ArrayBuffer): Promise<ArrayBuffer|undefined> {
	const r = await window.fetch('/accept', {
		method: 'POST',
		headers: {
			'Content-Type': 'application/octet-stream',
			'X-RPC-Method': name,
		},
		body: message,
	});
	if (r.status == 200) {
		const resp = await r.arrayBuffer();
		return resp;
	}
}
