// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';
/**
 * @constructor
 */
export namespace ec_prv.fbs{
export class TrustedShorteningResponse {
  bb: flatbuffers.ByteBuffer|null = null;

  bb_pos:number = 0;
/**
 * @param number i
 * @param flatbuffers.ByteBuffer bb
 * @returns TrustedShorteningResponse
 */
__init(i:number, bb:flatbuffers.ByteBuffer):TrustedShorteningResponse {
  this.bb_pos = i;
  this.bb = bb;
  return this;
};

/**
 * @param flatbuffers.ByteBuffer bb
 * @param TrustedShorteningResponse= obj
 * @returns TrustedShorteningResponse
 */
static getRootAsTrustedShorteningResponse(bb:flatbuffers.ByteBuffer, obj?:TrustedShorteningResponse):TrustedShorteningResponse {
  return (obj || new TrustedShorteningResponse()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
};

/**
 * @param flatbuffers.ByteBuffer bb
 * @param TrustedShorteningResponse= obj
 * @returns TrustedShorteningResponse
 */
static getSizePrefixedRootAsTrustedShorteningResponse(bb:flatbuffers.ByteBuffer, obj?:TrustedShorteningResponse):TrustedShorteningResponse {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new TrustedShorteningResponse()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
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
pass(index: number):number|null {
  var offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.readUint8(this.bb!.__vector(this.bb_pos + offset) + index) : 0;
};

/**
 * @returns number
 */
passLength():number {
  var offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.__vector_len(this.bb_pos + offset) : 0;
};

/**
 * @returns Uint8Array
 */
passArray():Uint8Array|null {
  var offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? new Uint8Array(this.bb!.bytes().buffer, this.bb!.bytes().byteOffset + this.bb!.__vector(this.bb_pos + offset), this.bb!.__vector_len(this.bb_pos + offset)) : null;
};

/**
 * @param number index
 * @returns number
 */
lookupKey(index: number):number|null {
  var offset = this.bb!.__offset(this.bb_pos, 10);
  return offset ? this.bb!.readUint8(this.bb!.__vector(this.bb_pos + offset) + index) : 0;
};

/**
 * @returns number
 */
lookupKeyLength():number {
  var offset = this.bb!.__offset(this.bb_pos, 10);
  return offset ? this.bb!.__vector_len(this.bb_pos + offset) : 0;
};

/**
 * @returns Uint8Array
 */
lookupKeyArray():Uint8Array|null {
  var offset = this.bb!.__offset(this.bb_pos, 10);
  return offset ? new Uint8Array(this.bb!.bytes().buffer, this.bb!.bytes().byteOffset + this.bb!.__vector(this.bb_pos + offset), this.bb!.__vector_len(this.bb_pos + offset)) : null;
};

/**
 * @param flatbuffers.Builder builder
 */
static startTrustedShorteningResponse(builder:flatbuffers.Builder) {
  builder.startObject(4);
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
 * @param flatbuffers.Offset passOffset
 */
static addPass(builder:flatbuffers.Builder, passOffset:flatbuffers.Offset) {
  builder.addFieldOffset(2, passOffset, 0);
};

/**
 * @param flatbuffers.Builder builder
 * @param Array.<number> data
 * @returns flatbuffers.Offset
 */
static createPassVector(builder:flatbuffers.Builder, data:number[]|Uint8Array):flatbuffers.Offset {
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
static startPassVector(builder:flatbuffers.Builder, numElems:number) {
  builder.startVector(1, numElems, 1);
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset lookupKeyOffset
 */
static addLookupKey(builder:flatbuffers.Builder, lookupKeyOffset:flatbuffers.Offset) {
  builder.addFieldOffset(3, lookupKeyOffset, 0);
};

/**
 * @param flatbuffers.Builder builder
 * @param Array.<number> data
 * @returns flatbuffers.Offset
 */
static createLookupKeyVector(builder:flatbuffers.Builder, data:number[]|Uint8Array):flatbuffers.Offset {
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
static startLookupKeyVector(builder:flatbuffers.Builder, numElems:number) {
  builder.startVector(1, numElems, 1);
};

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
static endTrustedShorteningResponse(builder:flatbuffers.Builder):flatbuffers.Offset {
  var offset = builder.endObject();
  return offset;
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset offset
 */
static finishTrustedShorteningResponseBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset);
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset offset
 */
static finishSizePrefixedTrustedShorteningResponseBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset, undefined, true);
};

static createTrustedShorteningResponse(builder:flatbuffers.Builder, version:number, error:boolean, passOffset:flatbuffers.Offset, lookupKeyOffset:flatbuffers.Offset):flatbuffers.Offset {
  TrustedShorteningResponse.startTrustedShorteningResponse(builder);
  TrustedShorteningResponse.addVersion(builder, version);
  TrustedShorteningResponse.addError(builder, error);
  TrustedShorteningResponse.addPass(builder, passOffset);
  TrustedShorteningResponse.addLookupKey(builder, lookupKeyOffset);
  return TrustedShorteningResponse.endTrustedShorteningResponse(builder);
}

/**
 * @returns TrustedShorteningResponseT
 */
unpack(): TrustedShorteningResponseT {
  return new TrustedShorteningResponseT(
    this.version(),
    this.error(),
    this.bb!.createScalarList(this.pass.bind(this), this.passLength()),
    this.bb!.createScalarList(this.lookupKey.bind(this), this.lookupKeyLength())
  );
};

/**
 * @param TrustedShorteningResponseT _o
 */
unpackTo(_o: TrustedShorteningResponseT): void {
  _o.version = this.version();
  _o.error = this.error();
  _o.pass = this.bb!.createScalarList(this.pass.bind(this), this.passLength());
  _o.lookupKey = this.bb!.createScalarList(this.lookupKey.bind(this), this.lookupKeyLength());
};
}

export class TrustedShorteningResponseT {
/**
 * @constructor
 * @param number version
 * @param boolean error
 * @param (number)[] pass
 * @param (number)[] lookupKey
 */
constructor(
  public version: number = 1,
  public error: boolean = false,
  public pass: (number)[] = [],
  public lookupKey: (number)[] = []
){};

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
pack(builder:flatbuffers.Builder): flatbuffers.Offset {
  const pass = ec_prv.fbs.TrustedShorteningResponse.createPassVector(builder, this.pass);
  const lookupKey = ec_prv.fbs.TrustedShorteningResponse.createLookupKeyVector(builder, this.lookupKey);

  return ec_prv.fbs.TrustedShorteningResponse.createTrustedShorteningResponse(builder,
    this.version,
    this.error,
    pass,
    lookupKey
  );
};
}
}