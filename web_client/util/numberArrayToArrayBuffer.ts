export function numberArrayToByteArray(arr: number[]): Uint8Array {
	const buf = new Uint8Array(arr.length);
	return Uint8Array.from(arr);
}