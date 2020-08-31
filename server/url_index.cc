#include "server/url_index.h"
#include "server/xorshift.h"
#include <cassert>
#include <stdexcept>

namespace ec_prv {
namespace url_index {

xorshift::XORShiftU32 URLIndex::xorshift_handle_ = xorshift::XORShiftU32{};

URLIndex::URLIndex(URLIndexAccess access_type) : access_type_{access_type} {
	switch (access_type_) {
	case URLIndexAccess::PUBLIC:
		n_ = generate(PUBLIC_MASK);
		break;

	case URLIndexAccess::PRIVILEGED:
		n_ = generate(PRIVILEGED_MASK);
		break;
	}
}

URLIndex::URLIndex(uint32_t n) : n_{n} {
	if ((n_ & PUBLIC_MASK) > 0) {
		access_type_ = URLIndexAccess::PUBLIC;
	} else if ((n_ & PRIVILEGED_MASK) > 0) {
		access_type_ = URLIndexAccess::PRIVILEGED;
	} else {
		assert(false);
	}
}

auto URLIndex::generate(uint32_t mask) -> uint32_t {
	uint32_t result = 0;
	do {
		result = rand();
	} while ((result & mask) == 0);
	return result;
}

auto URLIndex::is_privileged() -> bool {
	assert(((n_ & PRIVILEGED_MASK) > 0 && access_type_ == URLIndexAccess::PRIVILEGED) ||
	       ((n_ & PUBLIC_MASK) > 0 && access_type_ == URLIndexAccess::PUBLIC));
	return (n_ & PRIVILEGED_MASK) > 0 && access_type_ == URLIndexAccess::PRIVILEGED;
}

auto URLIndex::as_integer() -> uint32_t { return n_; }

auto URLIndex::as_bytes() -> std::vector<uint8_t> {
	std::vector<uint8_t> out(4, 0);
	out[0] = n_ & 0xff;
	out[1] = (n_ >> 8) & 0xff;
	out[2] = (n_ >> 16) & 0xff;
	out[3] = (n_ >> 24) & 0xff;
	return out;
}

auto URLIndex::from_bytes(std::span<uint8_t> src) -> URLIndex {
	auto const sz = src.size();
	if (sz == 0)
		throw std::runtime_error{"binary URL index is empty or too small"};
	uint32_t n = 0;
	n |= src[0];
	if (sz > 1) {
		n |= (src[1] << 8);
	}
	if (sz > 2) {
		n |= (src[2] << 16);
	}
	if (sz > 3) {
		n |= (src[3] << 24);
	}
	return URLIndex{n};
}

} // namespace url_index
} // namespace ec_prv
