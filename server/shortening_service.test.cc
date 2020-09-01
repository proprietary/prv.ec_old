#include "b64/b64.h"
#include "idl/all_generated_flatbuffers.h"
#include "private_url/private_url.h"
#include "server/db.h"
#include "server/shortening_service.h"
#include <chrono>
#include <filesystem>
#include <flatbuffers/flatbuffers.h>
#include <gtest/gtest.h>
#include <memory>

namespace {

using ::flatbuffers::FlatBufferBuilder;

class ShorteningServiceTest : public ::testing::Test {
protected:
	std::unique_ptr<ec_prv::db::KVStore> kvstore_;
	std::unique_ptr<ec_prv::shortening_service::ServiceHandle> svc_;
	// Linux-only
	static constexpr std::string_view datadir_ = "/dev/shm/shortening_service_test";

	void SetUp() override {
		kvstore_ = std::make_unique<ec_prv::db::KVStore>(datadir_);
		svc_ = std::make_unique<ec_prv::shortening_service::ServiceHandle>(kvstore_.get());
	}

	void TearDown() override { std::filesystem::remove_all(datadir_); }
};

TEST_F(ShorteningServiceTest, TestShorteningRequest) {
	char const* original_url = "https://en.wikipedia.org/wiki/Main_Page";
	auto pu = ec_prv::private_url::PrivateURL::generate(original_url);
	ASSERT_TRUE(pu.has_value());
	auto const& [private_url, pass] = pu.value();
	ASSERT_GT(pass.length(), 0);
	ASSERT_TRUE(private_url.valid());
	FlatBufferBuilder req_fbb;
	{
		auto iv = req_fbb.CreateVector(private_url.iv().data(), private_url.iv().size());
		auto salt =
		    req_fbb.CreateVector(private_url.salt().data(), private_url.salt().size());
		auto blinded_url = req_fbb.CreateVector(private_url.blinded_url().data(),
							private_url.blinded_url().size());
		// expire in 30 days
		auto exp = std::chrono::system_clock::now() + std::chrono::days(30);
		ec_prv::fbs::ShorteningRequestBuilder srb{req_fbb};
		srb.add_iv(iv);
		srb.add_salt(salt);
		srb.add_blinded_url(blinded_url);
		srb.add_expiry(
		    std::chrono::duration_cast<std::chrono::seconds>(exp.time_since_epoch())
			.count());
		srb.add_pbkdf2_iters(2'000'000);
		srb.add_version(1);
		auto sr = srb.Finish();
		req_fbb.Finish(sr);
	}
	auto srt = ec_prv::fbs::UnPackShorteningRequest(req_fbb.GetBufferPointer());
	auto res = svc_->handle(std::move(srt));
	{
		::flatbuffers::Verifier v{res.data(), res.size()};
		ASSERT_TRUE(ec_prv::fbs::VerifyShorteningRequestBuffer(v));
	}
	FlatBufferBuilder resp_fbb;
	auto resp = ec_prv::fbs::GetShorteningResponse(res.data());
	ASSERT_EQ(1, resp->version());
	ASSERT_FALSE(resp->error());
	ASSERT_GT(resp->lookup_key(), 0);
	{
		rocksdb::PinnableSlice private_url_from_db;
		auto ui = ec_prv::url_index::URLIndex::from_integer(resp->lookup_key());
		ASSERT_TRUE(!ui.is_privileged());
		ASSERT_EQ(ec_prv::url_index::URLIndexAccess::PUBLIC, ui.access_type());
		auto s = kvstore_->get(private_url_from_db, ui);
		ASSERT_TRUE(s.ok());
		ASSERT_GT(private_url_from_db.size(), 0);
		::flatbuffers::Verifier v{
		    reinterpret_cast<uint8_t const*>(private_url_from_db.data()),
		    private_url_from_db.size()};
		ASSERT_TRUE(ec_prv::fbs::VerifyPrivateURLBuffer(v));
		auto const* pu = ec_prv::fbs::GetPrivateURL(private_url_from_db.data());
		ASSERT_EQ(1, pu->version());
		ASSERT_EQ(2'000'000, pu->pbkdf2_iters());
		// decrypt with generated password
		{
			auto const* private_url_obj = pu->UnPack();
			// make copies
			auto salt = private_url_obj->salt;
			auto iv = private_url_obj->iv;
			auto blinded_url = private_url_obj->blinded_url;
			auto pass_encoded = ec_prv::b64::dec(pass);
			::ec_prv::private_url::PrivateURL private_url_retrieved{
			    std::move(salt), std::move(iv), std::move(blinded_url)};
			auto url_retrieved =
			    private_url_retrieved.get_plaintext(std::move(pass_encoded));
			ASSERT_STREQ(url_retrieved.c_str(), original_url);
		}
	}
}

} // namespace

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
