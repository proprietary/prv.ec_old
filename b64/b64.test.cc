#include <b64.h>
#include <gtest/gtest.h>

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

} // namespace

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
