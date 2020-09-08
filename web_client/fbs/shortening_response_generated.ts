// automatically generated by the FlatBuffers compiler, do not modify

import {flatbuffers} from '../vendor/flatbuffers/flatbuffers';
/**
 * @constructor
 */
export namespace ec_prv.fbs{
export class ShorteningResponse {
  bb: flatbuffers.ByteBuffer|null = null;

  bb_pos:number = 0;
/**
 * @param number i
 * @param flatbuffers.ByteBuffer bb
 * @returns ShorteningResponse
 */
__init(i:number, bb:flatbuffers.ByteBuffer):ShorteningResponse {
  this.bb_pos = i;
  this.bb = bb;
  return this;
};

/**
 * @param flatbuffers.ByteBuffer bb
 * @param ShorteningResponse= obj
 * @returns ShorteningResponse
 */
static getRootAsShorteningResponse(bb:flatbuffers.ByteBuffer, obj?:ShorteningResponse):ShorteningResponse {
  return (obj || new ShorteningResponse()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
};

/**
 * @param flatbuffers.ByteBuffer bb
 * @param ShorteningResponse= obj
 * @returns ShorteningResponse
 */
static getSizePrefixedRootAsShorteningResponse(bb:flatbuffers.ByteBuffer, obj?:ShorteningResponse):ShorteningResponse {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new ShorteningResponse()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
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
 * @returns number
 */
lookupKey():number {
  var offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.readUint32(this.bb_pos + offset) : 0;
};

/**
 * @param flatbuffers.Encoding= optionalEncoding
 * @returns string|Uint8Array|null
 */
lookupKeyEncoded():string|null
lookupKeyEncoded(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
lookupKeyEncoded(optionalEncoding?:any):string|Uint8Array|null {
  var offset = this.bb!.__offset(this.bb_pos, 10);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
};

/**
 * @param flatbuffers.Builder builder
 */
static startShorteningResponse(builder:flatbuffers.Builder) {
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
 * @param number lookupKey
 */
static addLookupKey(builder:flatbuffers.Builder, lookupKey:number) {
  builder.addFieldInt32(2, lookupKey, 0);
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset lookupKeyEncodedOffset
 */
static addLookupKeyEncoded(builder:flatbuffers.Builder, lookupKeyEncodedOffset:flatbuffers.Offset) {
  builder.addFieldOffset(3, lookupKeyEncodedOffset, 0);
};

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
static endShorteningResponse(builder:flatbuffers.Builder):flatbuffers.Offset {
  var offset = builder.endObject();
  return offset;
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset offset
 */
static finishShorteningResponseBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset);
};

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset offset
 */
static finishSizePrefixedShorteningResponseBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset, undefined, true);
};

static createShorteningResponse(builder:flatbuffers.Builder, version:number, error:boolean, lookupKey:number, lookupKeyEncodedOffset:flatbuffers.Offset):flatbuffers.Offset {
  ShorteningResponse.startShorteningResponse(builder);
  ShorteningResponse.addVersion(builder, version);
  ShorteningResponse.addError(builder, error);
  ShorteningResponse.addLookupKey(builder, lookupKey);
  ShorteningResponse.addLookupKeyEncoded(builder, lookupKeyEncodedOffset);
  return ShorteningResponse.endShorteningResponse(builder);
}

/**
 * @returns ShorteningResponseT
 */
unpack(): ShorteningResponseT {
  return new ShorteningResponseT(
    this.version(),
    this.error(),
    this.lookupKey(),
    this.lookupKeyEncoded()
  );
};

/**
 * @param ShorteningResponseT _o
 */
unpackTo(_o: ShorteningResponseT): void {
  _o.version = this.version();
  _o.error = this.error();
  _o.lookupKey = this.lookupKey();
  _o.lookupKeyEncoded = this.lookupKeyEncoded();
};
}

export class ShorteningResponseT {
/**
 * @constructor
 * @param number version
 * @param boolean error
 * @param number lookupKey
 * @param string|Uint8Array|null lookupKeyEncoded
 */
constructor(
  public version: number = 1,
  public error: boolean = false,
  public lookupKey: number = 0,
  public lookupKeyEncoded: string|Uint8Array|null = null
){};

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
pack(builder:flatbuffers.Builder): flatbuffers.Offset {
  const lookupKeyEncoded = (this.lookupKeyEncoded !== null ? builder.createString(this.lookupKeyEncoded!) : 0);

  return ec_prv.fbs.ShorteningResponse.createShorteningResponse(builder,
    this.version,
    this.error,
    this.lookupKey,
    lookupKeyEncoded
  );
};
}
}
