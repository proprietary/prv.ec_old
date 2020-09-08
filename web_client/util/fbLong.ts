import { flatbuffers } from '../vendor/flatbuffers/flatbuffers';

export function toLong(n: number): flatbuffers.Long {
	let binaryString = n.toString(2);
	binaryString = binaryString.padStart(64, '0'); 
	const high = window.parseInt(binaryString.substr(0, 32), 2);
	const low = window.parseInt(binaryString.substr(32), 2);
	return flatbuffers.createLong(low, high);
}