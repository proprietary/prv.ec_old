// automatically generated by the FlatBuffers compiler, do not modify

import {flatbuffers} from '../vendor/flatbuffers/flatbuffers';
/**
 * @constructor
 */
export namespace ec_prv.fbs{
export class LookupResponse {
  bb: flatbuffers.ByteBuffer|null = null;

  bb_pos:number = 0;
/**
 * @param number i
 * @param flatbuffers.ByteBuffer bb
 * @returns LookupResponse
 */
__init(i:number, bb:flatbuffers.ByteBuffer):LookupResponse {
  this.bb_pos = i;
  this.bb = bb;
  return this;
};

/**
 * @param flatbuffers.ByteBuffer bb
 * @param LookupResponse= obj
 * @returns LookupResponse
 */
static getRootAsLookupResponse(bb:flatbuffers.ByteBuffer, obj?:LookupResponse):LookupResponse {
  return (obj || new LookupResponse()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
};

/**
 * @param flatbuffers.ByteBuffer bb
 * @param LookupResponse= obj
 * @returns LookupResponse
 */
static getSizePrefixedRootAsLookupResponse(bb:flatbuffers.ByteBuffer, obj?:LookupResponse):LookupResponse {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new LookupResponse()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
};

/**
 * @returns number
 */
version():number {
  var offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readUint8(this.bb_pos + offset) : 1;
};

/**
 * @returns boolean
 */
error():boolean {
  var offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
};

/**
 * @param number index
 * @returns number
 */
data(index: number):number|null {
  var offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.readUint8(this.bb!.__vector(this.bb_pos + offset) + index) : 0;
};

/**
 * @returns number
 */
dataLength():number {
  var offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.__vector_len(this.bb_pos + offset) : 0;
};

/**
 * @returns Uint8Array
 */
dataArray():Uint8Array|null {
  var offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? new Uint8Array(this.bb!.bytes().buffer, this.bb!.bytes().byteOffset + this.bb!.__vector(this.bb_pos + offset), this.bb!.__vector_len(this.bb_pos + offset)) : null;
};

/**
 * @param flatbuffers.Builder builder
 */
static startLookupResponse(builder:flatbuffers.Builder) {
  builder.startObject(3);
};

/**
 * @param flatbuffers.Builder builder
 * @param number version
 */
static addVersion(builder:flatbuffers.Builder, version:number) {
  builder.addFieldInt8(0, version, 1);
};

/**
 * @param flatbuffers.Builder builder
 * @param boolean error
 */
static addError(builder:flatbuffers.Builder, error:boolean) {
  builder.addFieldInt8(1, +error, +false);
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset dataOffset
 */
static addData(builder:flatbuffers.Builder, dataOffset:flatbuffers.Offset) {
  builder.addFieldOffset(2, dataOffset, 0);
};

/**
 * @param flatbuffers.Builder builder
 * @param Array.<number> data
 * @returns flatbuffers.Offset
 */
static createDataVector(builder:flatbuffers.Builder, data:number[]|Uint8Array):flatbuffers.Offset {
  builder.startVector(1, data.length, 1);
  for (var i = data.length - 1; i >= 0; i--) {
    builder.addInt8(data[i]);
  }
  return builder.endVector();
};

/**
 * @param flatbuffers.Builder builder
 * @param number numElems
 */
static startDataVector(builder:flatbuffers.Builder, numElems:number) {
  builder.startVector(1, numElems, 1);
};

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
static endLookupResponse(builder:flatbuffers.Builder):flatbuffers.Offset {
  var offset = builder.endObject();
  return offset;
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset offset
 */
static finishLookupResponseBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset);
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset offset
 */
static finishSizePrefixedLookupResponseBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset, undefined, true);
};

static createLookupResponse(builder:flatbuffers.Builder, version:number, error:boolean, dataOffset:flatbuffers.Offset):flatbuffers.Offset {
  LookupResponse.startLookupResponse(builder);
  LookupResponse.addVersion(builder, version);
  LookupResponse.addError(builder, error);
  LookupResponse.addData(builder, dataOffset);
  return LookupResponse.endLookupResponse(builder);
}

/**
 * @returns LookupResponseT
 */
unpack(): LookupResponseT {
  return new LookupResponseT(
    this.version(),
    this.error(),
    this.bb!.createScalarList(this.data.bind(this), this.dataLength())
  );
};

/**
 * @param LookupResponseT _o
 */
unpackTo(_o: LookupResponseT): void {
  _o.version = this.version();
  _o.error = this.error();
  _o.data = this.bb!.createScalarList(this.data.bind(this), this.dataLength());
};
}

export class LookupResponseT {
/**
 * @constructor
 * @param number version
 * @param boolean error
 * @param (number)[] data
 */
constructor(
  public version: number = 1,
  public error: boolean = false,
  public data: (number)[] = []
){};

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
pack(builder:flatbuffers.Builder): flatbuffers.Offset {
  const data = ec_prv.fbs.LookupResponse.createDataVector(builder, this.data);

  return ec_prv.fbs.LookupResponse.createLookupResponse(builder,
    this.version,
    this.error,
    data
  );
};
}
}
