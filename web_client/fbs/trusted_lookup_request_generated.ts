// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';
/**
 * @constructor
 */
export namespace ec_prv.fbs{
export class TrustedLookupRequest {
  bb: flatbuffers.ByteBuffer|null = null;

  bb_pos:number = 0;
/**
 * @param number i
 * @param flatbuffers.ByteBuffer bb
 * @returns TrustedLookupRequest
 */
__init(i:number, bb:flatbuffers.ByteBuffer):TrustedLookupRequest {
  this.bb_pos = i;
  this.bb = bb;
  return this;
};

/**
 * @param flatbuffers.ByteBuffer bb
 * @param TrustedLookupRequest= obj
 * @returns TrustedLookupRequest
 */
static getRootAsTrustedLookupRequest(bb:flatbuffers.ByteBuffer, obj?:TrustedLookupRequest):TrustedLookupRequest {
  return (obj || new TrustedLookupRequest()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
};

/**
 * @param flatbuffers.ByteBuffer bb
 * @param TrustedLookupRequest= obj
 * @returns TrustedLookupRequest
 */
static getSizePrefixedRootAsTrustedLookupRequest(bb:flatbuffers.ByteBuffer, obj?:TrustedLookupRequest):TrustedLookupRequest {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new TrustedLookupRequest()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
};

/**
 * @returns number
 */
version():number {
  var offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readUint8(this.bb_pos + offset) : 1;
};

/**
 * @returns number
 */
lookupKey():number {
  var offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.readUint32(this.bb_pos + offset) : 0;
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
 * @param flatbuffers.Builder builder
 */
static startTrustedLookupRequest(builder:flatbuffers.Builder) {
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
 * @param number lookupKey
 */
static addLookupKey(builder:flatbuffers.Builder, lookupKey:number) {
  builder.addFieldInt32(1, lookupKey, 0);
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
 * @returns flatbuffers.Offset
 */
static endTrustedLookupRequest(builder:flatbuffers.Builder):flatbuffers.Offset {
  var offset = builder.endObject();
  return offset;
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset offset
 */
static finishTrustedLookupRequestBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset);
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset offset
 */
static finishSizePrefixedTrustedLookupRequestBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset, undefined, true);
};

static createTrustedLookupRequest(builder:flatbuffers.Builder, version:number, lookupKey:number, passOffset:flatbuffers.Offset):flatbuffers.Offset {
  TrustedLookupRequest.startTrustedLookupRequest(builder);
  TrustedLookupRequest.addVersion(builder, version);
  TrustedLookupRequest.addLookupKey(builder, lookupKey);
  TrustedLookupRequest.addPass(builder, passOffset);
  return TrustedLookupRequest.endTrustedLookupRequest(builder);
}

/**
 * @returns TrustedLookupRequestT
 */
unpack(): TrustedLookupRequestT {
  return new TrustedLookupRequestT(
    this.version(),
    this.lookupKey(),
    this.bb!.createScalarList(this.pass.bind(this), this.passLength())
  );
};

/**
 * @param TrustedLookupRequestT _o
 */
unpackTo(_o: TrustedLookupRequestT): void {
  _o.version = this.version();
  _o.lookupKey = this.lookupKey();
  _o.pass = this.bb!.createScalarList(this.pass.bind(this), this.passLength());
};
}

export class TrustedLookupRequestT {
/**
 * @constructor
 * @param number version
 * @param number lookupKey
 * @param (number)[] pass
 */
constructor(
  public version: number = 1,
  public lookupKey: number = 0,
  public pass: (number)[] = []
){};

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
pack(builder:flatbuffers.Builder): flatbuffers.Offset {
  const pass = ec_prv.fbs.TrustedLookupRequest.createPassVector(builder, this.pass);

  return ec_prv.fbs.TrustedLookupRequest.createTrustedLookupRequest(builder,
    this.version,
    this.lookupKey,
    pass
  );
};
}
}
