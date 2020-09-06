#include "b66/b66.h"
#include <algorithm>
#include <cassert>

namespace {

///
/// RFC 3986:
/// https://tools.ietf.org/html/rfc3986
///
/// 2.3.  Unreserved Characters
///
///    Characters that are allowed in a URI but do not have a reserved
///    purpose are called unreserved.  These include uppercase and lowercase
///    letters, decimal digits, hyphen, period, underscore, and tilde.
///
///       unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
///

const char BASIS_66[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

///
/// You can generate this with `generate_ascii_table.py`.
///
/// % python3 -m b66.generate_ascii_table
///
const unsigned char BASE_66_LOOKUP_TABLE[256] = {
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 62, 64, 66,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 66, 66, 66, 66, 66, 66, 66, 0,  1,	2,  3,	4,  5,	6,
    7,	8,  9,	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 66, 66, 66, 66, 63,
    66, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 66, 66, 66, 65, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66};

} // namespace

namespace ec_prv {

namespace b66 {

auto enc(std::string& dst, std::span<uint8_t> src) -> void {
	// src.size * ceil[log 256 / log 66] + 1
	// 1 extra for null terminator
	auto max_len = src.size() * 1366 / 1000 + 1;
	dst.assign(max_len, '\0');
	int length = 0;
	for (auto b = src.begin(); b != src.end(); ++b) {
		int32_t carry = static_cast<uint8_t>(*b);
		auto it = dst.rbegin();
		int i = 0;
		while ((carry > 0 || i < length) && it != dst.rend()) {
			carry += static_cast<uint8_t>(*it) * 256;
			*it = static_cast<uint8_t>(carry % 66);
			carry /= 66;
			it++;
			i++;
		}
		assert(carry == 0);
		length = i;
	}
	// skip "trailing" (insignificant) zeros (occurs at start because big-endian)
	auto it = dst.begin();
	for (; it != dst.end() && *it == '\0'; ++it);
	if (it != dst.begin() && it != dst.end()) {
		dst.erase(dst.begin(), it);
	}
	// map to characters
	for (auto it = dst.begin(); it != dst.end(); ++it) {
		*it = BASIS_66[static_cast<uint8_t>(*it)];
	}
}

auto dec(std::vector<uint8_t>& dst, std::string_view src) -> void {
	// src.size * ceil[log 66 / log 256]
	auto max_len = src.size() * 756 / 1000 + 1;
	dst.assign(max_len, 0);
	int length = 0;
	for (auto ch = src.begin(); ch != src.end(); ++ch) {
		int32_t n = BASE_66_LOOKUP_TABLE[static_cast<uint8_t>(*ch)];
		assert(n != 66);
		if (n == 66) {
			return;
		}
		int32_t carry = n;
		auto it = dst.rbegin();
		int i = 0;
		while ((carry > 0 || i < length) && it != dst.rend()) {
			carry += static_cast<int32_t>(*it) * 66;
			*it = carry % 256;
			carry /= 256;
			it++;
			i++;
		}
		assert(carry == 0);
		length = i;
	}
	// strip "trailing" (insignficant) zeros (occurs at start because big endian)
	strip_leading_zeros(dst);
}

auto strip_leading_zeros(std::vector<uint8_t>& dst) -> void {
	auto first_nonzero = dst.begin();
	while (first_nonzero != dst.end() && *first_nonzero == 0) {
		++first_nonzero;
	}
	if (first_nonzero != dst.begin() && first_nonzero != dst.end()) {
		dst.erase(dst.begin(), first_nonzero);
	}
}

} // namespace b66

} // namespace ec_prv
