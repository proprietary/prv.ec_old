#include "b66/b66.h"
#include <gtest/gtest.h>
#include <ios>
#include <iostream>
#include <random>
#include <cstdio>
#include <limits>

namespace {

TEST(TestB66, SmokeTest) {
	using ::ec_prv::b66::enc;
	using ::ec_prv::b66::dec;
	std::string o;
	std::vector<uint8_t> a {0x04, 0x08, 0x99, 0xff, 0xff, 0xea, 0xf9, 0x22, 0x91, 0x11, 0x42, 0xf8, 0xff};
	enc(o, std::span{a});
	std::cout << o << std::endl;
	std::vector<uint8_t> w;
	dec(w, o);
	for (auto ch : w) {
		// std::cout << std::hex << ch;
		printf("%02x ", ch);
	}
	std::cout << std::endl;
	for (auto i = 0ULL; i < a.size(); ++i) {
		ASSERT_EQ(a[i], w[i]);
	}
}

TEST(TestB66, EncodeDecode) {
	using ::ec_prv::b66::enc;
	using ::ec_prv::b66::dec;
	for (int j = 0; j < 5; ++j) {
		for (int i = 2; i < 0xff; ++i) {
			std::random_device rd{};
			std::uniform_int_distribution<uint8_t> d{0, static_cast<uint8_t>(0xff)};
			std::vector<uint8_t> byte_array(i, 0);
			std::transform(byte_array.begin(), byte_array.end(), byte_array.begin(),
				       [&d, &rd](auto x) { return d(rd); });
			// ::ec_prv::b66::strip_leading_zeros(byte_array);
			// for (auto i = 0UL; i < byte_array.size(); ++i) {
			// 	printf("%02x", byte_array[i]);
			// }
			// puts("");
			std::string encoded_result;
			enc(encoded_result, std::span{byte_array});
			// std::cout << "encoded result: " << encoded_result << std::endl;
			std::vector<uint8_t> decoded_result;
			dec(decoded_result, encoded_result);
			// for (auto c : decoded_result) {
			// 	printf("%02x", static_cast<uint8_t>(c));
			// }
			// puts("");
			ASSERT_GT(decoded_result.size(), 0);
			auto ba = byte_array.begin();
			auto dr = decoded_result.begin();
			while (ba != byte_array.end() && dr != decoded_result.end()) {
				ASSERT_EQ(*(dr++), *(ba++));
			}
			std::string encoded_again;
			enc(encoded_again, decoded_result);
			ASSERT_STREQ(encoded_result.c_str(), encoded_again.c_str());
		}
	}	
}

auto unpack(uint32_t n) -> std::vector<uint8_t> {
	std::vector<uint8_t> out(4, 0);
	out[0] = n & 0xff;
	out[1] = (n >> 8) & 0xff;
	out[2] = (n >> 16) & 0xff;
	out[3] = (n >> 24) & 0xff;
	return out;
}

auto pack(std::vector<uint8_t> const& src) -> uint32_t {
	uint32_t out = 0;
	if (src.size() > 0) {
		out |= src[0];
	}
	if (src.size() > 1) {
		out |= src[1] << 8;
	}
	if (src.size() > 2) {
		out |= src[2] << 16;
	}
	if (src.size() > 3) {
		out |= src[3] << 24;
	}
	return out;
}

TEST(TestB66, Integers) {
	using ::ec_prv::b66::enc;
	using ::ec_prv::b66::dec;
	std::random_device rd{};
	std::uniform_int_distribution<uint32_t> dist{0};
	for (uint32_t j = 1; j < 10; j++) {
		auto i = dist(rd);
		std::cout << "\ni = " << i << std::endl;
		auto a = unpack(i);
		// ec_prv::b66::strip_leading_zeros(a);
		// std::cout << "unpacked integer: ";
		// for (auto c : a) {
		// 	printf("%02x", c);
		// }
		// puts("");
		std::string encoded_result;
		enc(encoded_result, a);
		// std::cout << "encoded result: " << encoded_result<< std::endl;
		std::vector<uint8_t> decoded_result;
		dec(decoded_result, encoded_result);
		ASSERT_GT(decoded_result.size(), 0);
		// std::cout << "decoded result: ";
		// for (auto c : decoded_result) {
		// 	printf("%02x", c);
		// }
		// puts("");
		auto it1 = a.begin();
		auto it2 = decoded_result.begin();
		for (; it1 != a.end() && it2 != decoded_result.end(); ++it1, ++it2) {
			ASSERT_EQ(*it1, *it2);
		}
		// ec_prv::b66::strip_leading_zeros(decoded_result);
		auto recovered_integer = pack(decoded_result);
		ASSERT_EQ(i, recovered_integer);
	}
}

} // namespace

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
