#include "b64/b64.h"
#include "private_url/private_url.h"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

TEST(PrivateURL, EncDecBasicClientTest) {
	std::string original_url{"https://en.wikipedia.org/wiki/Main_Page"};
	auto pu = ::ec_prv::private_url::PrivateURL::generate(original_url);
	ASSERT_TRUE(pu.has_value());
	auto private_url_handle =
	    std::move(std::get<::ec_prv::private_url::PrivateURL>(pu.value()));
	auto const pass_b64encoded = std::move(std::get<std::string>(pu.value()));
	auto pass_raw_bytes = ::ec_prv::b64::dec(pass_b64encoded);
	auto decrypted_url = private_url_handle.get_plaintext(std::move(pass_raw_bytes));
	ASSERT_TRUE(decrypted_url.length() > 0);
	std::cout << "decrypted_url: " << decrypted_url << std::endl
		  << "original_url: " << original_url << std::endl;
	ASSERT_STREQ(original_url.c_str(), decrypted_url.c_str());
}

} // namespace

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
