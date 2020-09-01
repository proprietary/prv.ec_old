#include <gtest/gtest.h>
#include "url_index/url_index.h"

namespace {

TEST(URLIndex, URLIndexTest) {
	ec_prv::url_index::URLIndex a{};
	auto b = a.as_bytes();
	auto c = ec_prv::url_index::URLIndex::from_bytes(b);
	ASSERT_EQ(a.as_integer(), c.as_integer());
}

} // namespace

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

