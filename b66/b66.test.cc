#include "b66/b66.h"
#include <gtest/gtest.h>
#include <ios>
#include <iostream>
#include <cstdio>

namespace {

TEST(TestB66, SmokeTest) {
	using ::ec_prv::b66::enc;
	using ::ec_prv::b66::dec;
	std::string o;
	std::array<uint8_t, 4> a {0x1, 0x33, 0x44, 0xf9};
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
		EXPECT_EQ(a[i], w[i]);
	}
}

} // namespace

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
