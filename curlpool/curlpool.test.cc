#include "curlpool/curlpool.h"
#include <gtest/gtest.h>
#include <cassert>

namespace {

TEST(cURLPoolTest, SmokeTest) {
	using ec_prv::curlpool::cURLPool;

	cURLPool a{};
	for (auto i = 0; i < 100; i++) {
		auto* curl = curl_easy_init();
		ASSERT_NE(curl, nullptr);
		curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		a.add_handle(curl);
	}
	a.run();
}

} // namespace

int main(int argc, char** argv) {
	auto ec = curl_global_init(CURL_GLOBAL_ALL);
	if (ec != CURLE_OK) {
		fputs(curl_easy_strerror(ec), stderr);
		return EXIT_FAILURE;
	}
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
