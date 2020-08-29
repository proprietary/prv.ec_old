#include <cassert>
#include <cstring>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr char BASIS_58[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

constexpr int8_t BASE_58_DECODE_MAP[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 0,  1,	2,  3,	4,  5,	6,  7,	8,  -1, -1, -1, -1, -1, -1, -1, 9,  10, 11, 12, 13, 14, 15,
    16, -1, 17, 18, 19, 20, 21, -1, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1,
    -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

} // namespace

namespace ec_prv {
namespace b58 {

auto enc(std::span<uint8_t> src) -> std::string {
	static_assert(sizeof(BASIS_58) / sizeof(BASIS_58[0]),
		      "base58 encode map should have 58 elements");
	// little-endian

	auto max_size = src.size() * 138 / 100 + 1;
	if (max_size < 1)
		return {};
	size_t length = 0;
	std::vector<uint8_t> nums(max_size, 0);
	for (auto ch = src.rbegin(); ch != src.rend(); ++ch) {
		auto carry = static_cast<uint32_t>(*ch);
		decltype(nums.size()) i = 0;
		for (auto it = nums.rbegin(); it != nums.rend() && (carry != 0 || i < length);
		     ++it, ++i) {
			carry += static_cast<uint32_t>(nums[i] * 256);
			nums[i] = static_cast<uint8_t>(carry % 58);
			carry /= 58;
		}
		assert(carry == 0);
		length = i;
	}
	std::string s(length, '\0');
	size_t i = 0;
	assert(s.size() <= nums.size());
	auto nums_start = nums.begin();
	while (nums_start != nums.end() && i < s.size()) {
		s[i++] = BASIS_58[static_cast<uint8_t>(*(nums_start++))];
	}
	return s;
}

auto dec(std::string_view src) -> std::vector<uint8_t> {
	static_assert(sizeof(BASE_58_DECODE_MAP) / sizeof(BASE_58_DECODE_MAP[0]) == 256,
		      "base58 decode map should have 256 elements");
	// little-endian

	// max decoded length: ceil[log 58 / log 256]
	auto max_size = src.size() * 733 / 1000 + 1;
	std::vector<uint8_t> out(max_size, 0);
	size_t length = 0;
	for (auto ch = src.rbegin(); ch != src.rend(); ++ch) {
		int32_t carry = BASE_58_DECODE_MAP[static_cast<uint16_t>(*ch)];
		if (carry == -1) {
			// invalid character
			return {};
		}
		decltype(out.size()) i = 0;
		for (auto out_it = out.begin(); out_it != out.end() && (carry != 0 || i < length);
		     ++out_it, ++i) {
			carry += *out_it * 58;
			*out_it = carry % 256;
			carry /= 256;
		}
		assert(0 == carry);
		length = i;
	}
	out.resize(length);
	return out;
}

} // namespace b58
} // namespace ec_prv
