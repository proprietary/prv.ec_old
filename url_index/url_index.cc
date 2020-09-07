#include "url_index/url_index.h"
#include "b66/marshal_int.h"
#include "xorshift/xorshift.h"
#include <cassert>
#include <stdexcept>

namespace ec_prv {
namespace url_index {

xorshift::XORShiftU32 URLIndex::xorshift_handle_ = xorshift::XORShiftU32{};

auto URLIndex::random(URLIndexAccess access_type) -> URLIndex {
	uint32_t n = 0;
	switch (access_type) {
	case URLIndexAccess::PUBLIC:
		n = generate(PUBLIC_MASK);
		break;

	case URLIndexAccess::PRIVILEGED:
		n = generate(PRIVILEGED_MASK);
		break;
	}
	auto generated = URLIndex{n};
	assert(generated.access_type() == access_type);
	return generated;
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

auto URLIndex::is_privileged() const noexcept -> bool {
	assert(((n_ & PRIVILEGED_MASK) > 0 && access_type_ == URLIndexAccess::PRIVILEGED) ||
	       ((n_ & PUBLIC_MASK) > 0 && access_type_ == URLIndexAccess::PUBLIC));
	return (n_ & PRIVILEGED_MASK) > 0 && access_type_ == URLIndexAccess::PRIVILEGED;
}

auto URLIndex::access_type() const noexcept -> URLIndexAccess { return access_type_; }

auto URLIndex::as_integer() -> uint32_t { return n_; }

auto URLIndex::as_bytes() -> std::array<uint8_t, 4> {
	std::array<uint8_t, 4> out;
	out.fill(0);
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

auto URLIndex::from_integer(uint32_t src) noexcept -> URLIndex {
	URLIndex o{src};
	return o;
}

auto URLIndex::as_base_66_string() const noexcept -> std::string {
	using ::ec_prv::b66::marshal;
	return marshal(n_);
}

auto URLIndex::from_base_66_string(std::string_view src) noexcept -> URLIndex {
	using ::ec_prv::b66::unmarshal;
	uint32_t m = unmarshal(src);
	return from_integer(m);
}

} // namespace url_index
} // namespace ec_prv
