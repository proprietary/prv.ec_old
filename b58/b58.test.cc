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
			for (auto i = 0UL; i < byte_array.size(); ++i) {
				printf("%02x", byte_array[i]);
			}
			puts("");
			auto encoded_result = ec_prv::b58::enc(std::span<uint8_t>{byte_array});
			std::cout << "encoded result: " << encoded_result << std::endl;
			auto decoded_result = ec_prv::b58::dec(encoded_result);
			for (auto c : decoded_result) {
				printf("%02x", static_cast<uint8_t>(c));
			}
			puts("\n");
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
	char const* expected = "6tkHfVswPjTQmj";
	auto actual = ec_prv::b58::enc(input);
	puts(actual.c_str());
	ASSERT_STREQ(expected, actual.c_str());
}

} // namespace

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
