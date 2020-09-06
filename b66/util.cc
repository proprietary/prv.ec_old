#include "b66/util.h"

namespace ec_prv::b66::util {

auto unpack(uint32_t n) -> std::vector<uint8_t> {
	std::vector<uint8_t> o(4, 0);
	unpack(o, n);
	return o;
}

auto unpack(std::vector<uint8_t>& dst, uint32_t n) -> void {
	dst.resize(4);
	dst[0] = n & 0xff;
	dst[1] = (n >> 8) & 0xff;
	dst[2] = (n >> 16) & 0xff;
	dst[3] = (n >> 24) & 0xff;
}

auto pack(std::span<uint8_t> src) -> uint32_t {
	uint32_t o = 0;
	pack(o, src);
	return o;
}

auto pack(uint32_t& dst, std::span<uint8_t> src) -> void {
	dst = 0;
	if (src.size() > 0) {
		dst |= src[0];
	}
	if (src.size() > 1) {
		dst |= src[1] << 8;
	}
	if (src.size() > 2) {
		dst |= src[2] << 16;
	}
	if (src.size() > 3) {
		dst |= src[3] << 24;
	}
}

} // namespace ec_prv::b66::util
