#include <gtest/gtest.h>
#include "url_index/url_index.h"

namespace {

TEST(URLIndex, URLIndexTest) {
	using ::ec_prv::url_index::URLIndex;
	auto a = URLIndex::random();
	auto b = a.as_bytes();
	auto c = URLIndex::from_bytes(b);
	ASSERT_EQ(a.as_integer(), c.as_integer());
}

} // namespace

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

