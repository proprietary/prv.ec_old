#include "b64/b64.h"
#include <gtest/gtest.h>
#include <random>

namespace {

auto test_encode(std::string input) -> std::string {
	std::vector<uint8_t> ba{input.begin(), input.end()};
	auto res = ec_prv::b64::enc(ba);
	return res;
}

auto test_decode(std::string input) -> std::string {
	auto res = ec_prv::b64::dec(input);
	std::string s{res.begin(), res.end()};
	return s;
}

TEST(Base64, Encode) {
	EXPECT_STREQ("YXNsa2RmajthbHNrZGpmO2Fs", test_encode("aslkdfj;alskdjf;al").c_str());
	EXPECT_STREQ("", test_encode("").c_str());
	EXPECT_STREQ("YQ==", test_encode("a").c_str());
	EXPECT_STREQ(
	    "MjkzNzQxODkyNzM0MDkxMjgzNzI5Mzg0NzIzMDk4cmhpamZxb3dpdTRoZm9pcXd1ZmpocGZqMzM5ODIzMDk4cj"
	    "FoMDQ5dDgweXB0OThmZw==",
	    test_encode(
		"293741892734091283729384723098rhijfqowiu4hfoiqwufjhpfj339823098r1h049t80ypt98fg")
		.c_str());
}

TEST(Base64, Decode) {
	EXPECT_STREQ("A programming system called LISP (for LISt Processor) has been developed for "
		     "the IBM 704 computer by the Artificial Intelligence group at M.I.T.",
		     test_decode("QSBwcm9ncmFtbWluZyBzeXN0ZW0gY2FsbGVkIExJU1AgKGZvciBMSVN0IFByb2Nlc"
				 "3NvcikgaGFzIGJlZW4gZGV2ZWxvcGVkIGZvciB0aGUgSUJNIDcwNCBjb21wdXRlci"
				 "BieSB0aGUgQXJ0aWZpY2lhbCBJbnRlbGxpZ2VuY2UgZ3JvdXAgYXQgTS5JLlQu")
			 .c_str());
	EXPECT_STREQ("21", test_decode("MjE=").c_str());
	EXPECT_STREQ("", test_decode("").c_str());
}

// test encoding/decoding of random strings
TEST(Base64, Integration) {
	static constexpr char alphabet[] = {
	    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', '0', '1', '2', '3', '4',
	    '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y'};
	std::random_device rd{};
	std::uniform_int_distribution dist{0UL, std::size(alphabet)};
	int rounds = 50;
	while (rounds-- > 0) {
		constexpr auto len = 100;
		std::string s(len + 1, '\0');
		for (int i = 0; i < len; ++i) {
			auto idx = dist(rd);
			s[i] = alphabet[idx];
		}
		std::vector<uint8_t> buf{s.begin(), s.end()};
		{
			auto encoded = ::ec_prv::b64::enc_secure(buf);
			auto decoded = ::ec_prv::b64::dec(encoded);
			std::string_view decoded_as_string{
			    reinterpret_cast<char const*>(decoded.data()), decoded.size()};
			ASSERT_STREQ(s.c_str(), std::string{decoded_as_string}.c_str());
		}
		{
			auto encoded = ::ec_prv::b64::enc(buf);
			auto decoded = ::ec_prv::b64::dec(encoded);
			std::string_view decoded_as_string{
			    reinterpret_cast<char const*>(decoded.data()), decoded.size()};
			ASSERT_STREQ(s.c_str(), std::string{decoded_as_string}.c_str());
		}
	}
}

} // namespace

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
