#include <cassert>
#include <cstring>
#include <iterator>
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
	// count leading zeros
	size_t leading_zeros = 0;
	for (auto b : src) {
		if (b != 0) {
			break;
		} else {
			leading_zeros++;
		}
	}
	auto max_size = src.size() * 138 / 100 + 1;
	if (max_size < 1)
		return {};
	size_t length = 0;
	// compute which indicies of BASIS_58 should comprise the output string
	std::vector<uint8_t> nums(max_size, 0);
	// skip leading zeros; will be added later in post-processing
	for (size_t j = leading_zeros; j < src.size(); ++j) {
		auto const n = src[j];
		auto carry = static_cast<uint32_t>(n);
		decltype(nums.size()) i = 0;
		for (auto it = nums.rbegin(); it != nums.rend() && (carry != 0 || i < length);
		     ++it, ++i) {
			carry += static_cast<uint32_t>(*it * 256);
			*it = static_cast<uint8_t>(carry % 58);
			carry /= 58;
		}
		assert(carry == 0);
		length = i;
	}
	assert(max_size >= (length + leading_zeros));
	// initialize to all "zero" (which in base-58 is 'A') values so
	// that leading zeros are taken care of automatically
	std::string output_string(length + leading_zeros, BASIS_58[0]);
	// map the computed indices of BASIS_58 to the actual characters,
	// creating the output string
	{
		auto nums_start = nums.begin();
		// skip unused cells
		std::advance(nums_start, max_size - length);
		size_t i = leading_zeros;
		while (nums_start != nums.end() && i < output_string.size()) {
			output_string[i++] = BASIS_58[static_cast<uint8_t>(*(nums_start++))];
		}
	}
	return output_string;
}

auto dec(std::string_view src) -> std::vector<uint8_t> {
	static_assert(sizeof(BASE_58_DECODE_MAP) / sizeof(BASE_58_DECODE_MAP[0]) == 256,
		      "base58 decode map should have 256 elements");
	// count leading zeros
	size_t leading_zeros = 0;
	for (auto c : src) {
		if (c != BASIS_58[0]) {
			break;
		} else {
			leading_zeros++;
		}
	}
	// skip the leading zeros; will be added later
	auto start = src.begin();
	std::advance(start, leading_zeros);
	// max decoded length: ceil[log 58 / log 256]
	auto max_size = src.size() * 733 / 1000 + 1;
	std::vector<uint8_t> buf(max_size, 0);
	size_t length = 0;
	for (auto ch = start; ch != src.end(); ++ch) {
		int32_t carry = BASE_58_DECODE_MAP[static_cast<uint16_t>(*ch)];
		if (carry == -1) {
			// invalid character
			return {};
		}
		decltype(buf.size()) i = 0;
		for (auto it = buf.rbegin(); it != buf.rend() && (carry != 0 || i < length);
		     ++it, ++i) {
			carry += *it * 58;
			*it = carry % 256;
			carry /= 256;
		}
		assert(0 == carry);
		length = i;
	}
	// prepare output
	std::vector<uint8_t> out(length + leading_zeros, 0);
	auto buf_start = buf.begin();
	std::advance(buf_start, max_size - length);
	auto out_first_nonzero = out.begin();
	std::advance(out_first_nonzero, leading_zeros);
	std::copy(buf_start, buf.end(), out_first_nonzero);
	return out;
}

} // namespace b58
} // namespace ec_prv
