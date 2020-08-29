#include "b58/b58.h"
#include <algorithm>
#include <cstdio>
#include <gtest/gtest.h>
#include <ios>
#include <iostream>
#include <limits>
#include <ostream>
#include <random>

namespace {

TEST(Base58, EncodeDecode) {
	for (int j = 0; j < 5; ++j) {
		for (int i = 2; i < 0xff; ++i) {
			std::random_device rd{};
			std::uniform_int_distribution<uint8_t> d{0, static_cast<uint8_t>(0xff)};
			std::vector<uint8_t> byte_array(i, 0);
			std::transform(byte_array.begin(), byte_array.end(), byte_array.begin(),
				       [&d, &rd](auto x) { return d(rd); });
			// for (auto i = 0UL; i < byte_array.size(); ++i) {
			// 	printf("%02x", byte_array[i]);
			// }
			// puts("");
			auto encoded_result = ec_prv::b58::enc(std::span<uint8_t>{byte_array});
			// std::cout << "encoded result: " << encoded_result << std::endl;
			auto decoded_result = ec_prv::b58::dec(encoded_result);
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
			auto encoded_again = ec_prv::b58::enc(decoded_result);
			ASSERT_STREQ(encoded_result.c_str(), encoded_again.c_str());
		}
	}
}

TEST(Base58, Encode1) {
	uint8_t input[] = {0x68, 0xe0, 0x80, 0x92, 0xa8, 0xbb, 0x43, 0xe4, 0xff, 0x4e};
	char const* expected = "ZwJGnHXu8LaRS5";
	auto actual = ec_prv::b58::enc(input);
	ASSERT_STREQ(expected, actual.c_str());
}

TEST(Base58, Encode2) {
	uint8_t input[] = {0xc2, 0x37, 0xb1, 0xd4, 0x8e, 0xc9, 0x9b, 0xea, 0x3e, 0xb4, 0x78, 0x23,
			   0x96, 0xb6, 0x7b, 0x22, 0x0d, 0xc5, 0x8d, 0x9b, 0x78, 0xd4, 0xc9, 0xcf,
			   0x4a, 0xa9, 0x4c, 0xac, 0xf1, 0xbd, 0x87, 0x7f, 0xe3, 0x27, 0x97, 0x6e,
			   0xe7, 0x30, 0x30, 0x95, 0x04, 0x56, 0x22, 0x98, 0xb4, 0x18, 0x24, 0xdd,
			   0x60, 0x88, 0x0c, 0x7a, 0x91, 0xbd, 0x33, 0xd7, 0x00, 0x44, 0x5f, 0x9a,
			   0x0a, 0x5a, 0x53, 0x2c, 0x81, 0x31, 0xd8, 0x72, 0xaf, 0x88, 0xbd, 0x02,
			   0x85, 0x38, 0x20, 0x65, 0x46, 0x9e, 0xc3, 0x98, 0x01, 0x65, 0x43, 0xe1,
			   0x8a, 0x5c, 0xbd, 0xad, 0xaf, 0x06, 0xec, 0xe8, 0x60, 0x3b, 0x23, 0xd2,
			   0xd5, 0x01, 0x18, 0xd4, 0x8e, 0xd2, 0xc2, 0x49, 0x91, 0xf5};
	char const* expected =
	    "3qusZhYwT9h9f8g2ymnPVTW1z7DcE5PqLTrDWG2FXeDRjbdwAtedBoo6xZUKc1q4bjzBMCSuXynJkjF5jQYboA"
	    "zAP6DDWVFXHyoH9kuzzoptVDpSynUJ7jASXmcNg6hWF1z4aaT9WccG4UeyM";
	auto actual = ec_prv::b58::enc(input);
	ASSERT_STREQ(expected, actual.c_str());
}

TEST(Base58, Decode) {
	uint8_t expected[] = {0x68, 0xe0, 0x80, 0x92, 0xa8, 0xbb, 0x43, 0xe4, 0xff, 0x4e};
	char const* input = "ZwJGnHXu8LaRS5";
	auto actual = ec_prv::b58::dec(input);
	ASSERT_EQ(actual.size(), sizeof(expected) / sizeof(expected[0]));
	for (auto i = 0UL; i < actual.size(); ++i) {
		ASSERT_EQ(expected[i], actual[i]);
	}
}

} // namespace

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
