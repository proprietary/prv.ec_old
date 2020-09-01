#ifndef _INCLUDE_EC_PRV_SERVER_URL_INDEX_H
#define _INCLUDE_EC_PRV_SERVER_URL_INDEX_H
#include "server/xorshift.h"
#include <cstdint>
#include <span>
#include <string>
#include <array>
#include <vector>
#include <optional>

///
/// URL shortening records are indexed as unsigned 32-bit integers
///

namespace ec_prv {
namespace url_index {

enum class URLIndexAccess {
	PUBLIC = 1,
	PRIVILEGED,
};

class URLIndex {
private:
	uint32_t n_;
	URLIndexAccess access_type_;
	static constexpr uint32_t PRIVILEGED_MASK = 0x0000ffff;
	static constexpr uint32_t PUBLIC_MASK = 0xffff0000;
	static xorshift::XORShiftU32 xorshift_handle_;
	static auto generate(uint32_t mask = PUBLIC_MASK) -> uint32_t;
	explicit URLIndex(uint32_t n);

public:
	static auto random(URLIndexAccess access_type = URLIndexAccess::PUBLIC) -> URLIndex;

	auto as_integer() -> uint32_t;

	///
	/// Checks if the index is in the lower, reserved range not
	/// intentended for public consumption. A URL index using fewer
	/// bits has a shorter URL and may be reserved for different uses.
	///
	auto is_privileged() -> bool;

	auto as_bytes() -> std::array<uint8_t, 4>;

	static auto from_bytes(std::span<uint8_t>) -> URLIndex;

	static auto from_integer(uint32_t src) -> URLIndex;

	auto access_type() -> URLIndexAccess;
};

} // namespace url_index
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_SERVER_URL_INDEX_H
