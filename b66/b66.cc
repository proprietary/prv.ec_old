#include "b66/b66.h"
#include <algorithm>
#include <cassert>
#include <iterator>

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
	// count leading zeros and skip them
	size_t leading_zeros = 0;
	for (auto c : src) {
		if (c != 0x00) {
			break;
		}
		leading_zeros++;
	}
	auto start = src.begin();
	std::advance(start, leading_zeros);
	// src.size * ceil[log 256 / log 66] + 1
	// 1 extra for null terminator
	auto max_len = src.size() * 1366 / 1000 + 1;
	dst.assign(max_len, '\0');
	size_t length = 0;
	for (auto b = start; b != src.end(); ++b) {
		int32_t carry = static_cast<uint8_t>(*b);
		auto it = dst.rbegin();
		size_t i = 0;
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
	assert(max_len >= length);
	// handle leading zeros
	dst.erase(dst.begin(), dst.begin() + max_len - length);
	dst.insert(dst.begin(), leading_zeros, 0x00);
	// map to characters
	for (auto it = dst.begin(); it != dst.end(); ++it) {
		*it = BASIS_66[static_cast<uint8_t>(*it)];
	}
}

auto dec(std::vector<uint8_t>& dst, std::string_view src) -> void {
	// leading zeros
	size_t leading_zeros = 0;
	for (auto c : src) {
		if (c != BASIS_66[0]) {
			break;
		}
		leading_zeros++;
	}
	if (leading_zeros >= src.length()) {
		return;
	}
	auto start = src.begin();
	std::advance(start, leading_zeros);
	// src.size * ceil[log 66 / log 256]
	auto max_len = src.size() * 756 / 1000 + 1;
	dst.assign(max_len, 0);
	size_t length = 0;
	for (auto ch = start; ch != src.end(); ++ch) {
		int32_t n = BASE_66_LOOKUP_TABLE[static_cast<uint8_t>(*ch)];
		assert(n != 66);
		if (n == 66) {
			return;
		}
		int32_t carry = n;
		auto it = dst.rbegin();
		size_t i = 0;
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
	assert(max_len >= length);
	dst.erase(dst.begin(), dst.begin() + max_len - length);
	dst.insert(dst.begin(), leading_zeros, 0x00);
}

} // namespace b66

} // namespace ec_prv
