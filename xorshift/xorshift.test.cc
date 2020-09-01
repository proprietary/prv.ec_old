#include "xorshift/xorshift.h"
#include <gtest/gtest.h>

namespace {

TEST(XORShift, XORShiftU32SmokeTest) {
	using ::ec_prv::xorshift::XORShiftU32;
	XORShiftU32 a;
	uint32_t prev = 0;
	for (int i = 0; i < 20; ++i) {
		auto res = a.rand();
		ASSERT_GT(res, 0);
		ASSERT_NE(res, prev);
		prev = res;
		// std::cout << res << std::endl;
	}
}

TEST(XORShift, XORShiftU64SmokeTest) {
	using ::ec_prv::xorshift::XORShiftU64;
	XORShiftU64 a;
	uint64_t prev = 0;
	for (int i = 0; i < 20; ++i) {
		auto res = a.rand();
		ASSERT_GT(res, 0);
		ASSERT_NE(res, prev);
		prev = res;
		// std::cout << res << std::endl;
	}
}


} // namespace

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
