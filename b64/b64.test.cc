#include <gtest/gtest.h>
#include <b64.h>

namespace {

auto test_encode(std::string input) -> std::string {
	std::vector<unsigned char> ba {input.begin(), input.end()};
	auto res = ec_prv::b64::enc(ba);
	return res;
}

TEST(Base64, Encode) {
	EXPECT_STREQ("YXNsa2RmajthbHNrZGpmO2Fs", test_encode("aslkdfj;alskdjf;al").c_str());
	EXPECT_STREQ("", test_encode("").c_str());
	EXPECT_STREQ("YQ==", test_encode("a").c_str());
	EXPECT_STREQ("MjkzNzQxODkyNzM0MDkxMjgzNzI5Mzg0NzIzMDk4cmhpamZxb3dpdTRoZm9pcXd1ZmpocGZqMzM5ODIzMDk4cjFoMDQ5dDgweXB0OThmZw==", test_encode("293741892734091283729384723098rhijfqowiu4hfoiqwufjhpfj339823098r1h049t80ypt98fg").c_str());
}

} // namespace

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
