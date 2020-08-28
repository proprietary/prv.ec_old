#include "server/shortening_service.h"

#include "idl/all_generated_flatbuffers.h"
#include "server/db.h"
#include "server/private_url.h"
#include "server/xorshift.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace {

std::span<uint8_t> fbspan(::flatbuffers::FlatBufferBuilder& fbb) {
	return std::span<uint8_t>{fbb.GetBufferPointer(), fbb.GetSize()};
}

///
/// Generate new URL indices we find one not yet taken.
///
auto find_new_url_index_v1(::ec_prv::xorshift::XORShiftU32& rand_source,
			   ::ec_prv::db::KVStore& kvstore) -> ::flatbuffers::DetachedBuffer {
	// find new index not taken
	auto available_key = kvstore.find_new_key([&rand_source]() -> ::flatbuffers::DetachedBuffer {
			::flatbuffers::FlatBufferBuilder url_index_fbb;
			::ec_prv::fbs::URLIndexBuilder uib{url_index_fbb};
			uib.add_version(1);
			uib.add_id(rand_source.rand());
			url_index_fbb.Finish(uib.Finish());
			return url_index_fbb.Release();
		});
	return available_key;
}

} // namespace

namespace ec_prv {
namespace shortening_service {

ServiceHandle::ServiceHandle(::ec_prv::db::KVStore* store,
			     std::shared_ptr<xorshift::XORShiftU32> xorshift)
    : store_{store}, rand_source_{xorshift} {}

auto ServiceHandle::handle(std::unique_ptr<::ec_prv::fbs::ShorteningRequestT> req)
    -> ::flatbuffers::DetachedBuffer {
	switch (req->version) {
	case 1: {
		// validate
		{
			bool ok =
			    !req->salt.empty() && !req->iv.empty() && !req->blinded_url.empty();
			if (!ok) {
				break;
			}
			if (!check_expiry(req->expiry)) {
				break;
			}
			// TODO: factor out crypto parameters to separate module
			if (req->pbkdf2_iters != 2'000'000) {
				break;
			}
		}

		// Create payload
		::flatbuffers::FlatBufferBuilder private_url_fbb;
		{
			auto iv = private_url_fbb.CreateVector(req->iv.data(), req->iv.size());
			auto salt =
			    private_url_fbb.CreateVector(req->salt.data(), req->salt.size());
			auto blinded_url = private_url_fbb.CreateVector(req->blinded_url.data(),
									req->blinded_url.size());
			fbs::PrivateURLBuilder pb{private_url_fbb};
			pb.add_pbkdf2_iters(req->pbkdf2_iters);
			pb.add_iv(iv);
			pb.add_salt(salt);
			pb.add_version(1);
			pb.add_expiry(req->expiry);
			pb.add_blinded_url(blinded_url);
			private_url_fbb.Finish(pb.Finish());
		}

		// Create unique identifier
		auto url_index = find_new_url_index_v1(*rand_source_, *store_);
		std::span<uint8_t> url_index_bytes{url_index.data(), url_index.size()};
		auto ok = store_->put(url_index_bytes, fbspan(private_url_fbb));
		assert(ok == true); // TODO: handle error better
		::flatbuffers::FlatBufferBuilder resp_fbb;
		auto uiv = resp_fbb.CreateVector<uint8_t>(
		    url_index.data(),
		    url_index.size());
		::ec_prv::fbs::ShorteningResponseBuilder srb{resp_fbb};
		srb.add_version(1);
		srb.add_error(!ok);
		srb.add_lookup_key(uiv);
		resp_fbb.Finish(srb.Finish());
		return resp_fbb.Release();
	}
	default:
		break;
	}
	// return error
	::flatbuffers::FlatBufferBuilder resp_fbb;
	::ec_prv::fbs::ShorteningResponseBuilder srb{resp_fbb};
	srb.add_version(1);
	srb.add_error(true);
	auto sr = srb.Finish();
	resp_fbb.Finish(sr);
	return resp_fbb.Release();
}

auto ServiceHandle::handle(std::unique_ptr<::ec_prv::fbs::LookupRequestT> req)
    -> ::flatbuffers::DetachedBuffer {
	switch (req->version) {
	case 1: {
		::flatbuffers::Verifier v{req->lookup_key.data(), req->lookup_key.size()};
		if (!v.VerifyBuffer<::ec_prv::fbs::URLIndex>()) {
			break;
		}
		auto const* url_index =
		    ::flatbuffers::GetRoot<::ec_prv::fbs::URLIndex>(req->lookup_key.data());
		if (url_index == nullptr) {
			break;
		}
		if (url_index->version() != 1) {
			break;
		}
		if (url_index->id() <= 0) {
			break;
		}
		std::string rocksdb_result_buf;
		store_->get(rocksdb_result_buf, std::span<uint8_t>{req->lookup_key});
		if (rocksdb_result_buf.length() == 0) {
			break;
		}
		// Construct success response
		::flatbuffers::FlatBufferBuilder fbb;
		::ec_prv::fbs::LookupResponseBuilder lrb{fbb};
		lrb.add_version(1);
		lrb.add_error(false);
		lrb.add_data(
		    fbb.CreateVector(reinterpret_cast<uint8_t const*>(rocksdb_result_buf.data()),
				     rocksdb_result_buf.size()));
		fbb.Finish(lrb.Finish());
		return fbb.Release();
	}
	default:
		break;
	}
	// failure case
	::flatbuffers::FlatBufferBuilder fbb;
	auto resp = ::ec_prv::fbs::CreateLookupResponse(fbb, 1, true);
	fbb.Finish(resp);
	return fbb.Release();
}

auto ServiceHandle::handle(std::unique_ptr<::ec_prv::fbs::TrustedShorteningRequestT> req)
    -> ::flatbuffers::DetachedBuffer {
	using namespace ::ec_prv::fbs;
	using namespace ::ec_prv::private_url;
	using ::flatbuffers::FlatBufferBuilder;
	switch (req->version) {
	case 1: {
		auto pu = ::ec_prv::private_url::PrivateURL::generate(req->url);
		if (!pu) {
			break;
		}
		auto const& private_url = std::get<::ec_prv::private_url::PrivateURL>(pu.value());
		// create payload to place into rocksdb
		FlatBufferBuilder private_url_fbb;
		auto salt = private_url_fbb.CreateVector(private_url.salt());
		auto iv = private_url_fbb.CreateVector(private_url.iv());
		auto blinded_url = private_url_fbb.CreateVector(private_url.blinded_url());
		PrivateURLBuilder pub{private_url_fbb};
		pub.add_version(1);
		pub.add_salt(salt);
		pub.add_iv(iv);
		pub.add_blinded_url(blinded_url);
		pub.add_pbkdf2_iters(2'000'000); // TODO(zds): factor out crypto params
		private_url_fbb.Finish(pub.Finish());
		// generate new key for rocksdb
		auto url_index = find_new_url_index_v1(*rand_source_, *store_);
		std::span<uint8_t> url_index_bytes{url_index.data(), url_index.size()};
		// put into rocksdb
		store_->put(url_index_bytes, fbspan(private_url_fbb));
		// construct response
		auto const& pass = std::get<std::string>(pu.value());
		FlatBufferBuilder resp_fbb;
		auto uiv = resp_fbb.CreateVector(url_index_bytes.data(), url_index_bytes.size());
		TrustedShorteningResponseBuilder tsrb{resp_fbb};
		tsrb.add_version(1);
		tsrb.add_error(false);
		auto pass_vec = resp_fbb.CreateVector(reinterpret_cast<uint8_t const*>(pass.data()),
						      pass.size());
		tsrb.add_pass(pass_vec);
		tsrb.add_lookup_key(uiv);
		resp_fbb.Finish(tsrb.Finish());
		return resp_fbb.Release();
	}
	default:
		break;
	}
	// failure case
	::flatbuffers::FlatBufferBuilder resp_fbb;
	::ec_prv::fbs::TrustedShorteningResponseBuilder tsrb{resp_fbb};
	tsrb.add_version(1);
	tsrb.add_error(true);
	resp_fbb.Finish(tsrb.Finish());
	return resp_fbb.Release();
}

auto ServiceHandle::handle(std::unique_ptr<::ec_prv::fbs::TrustedLookupRequestT> req)
    -> ::flatbuffers::DetachedBuffer {
	using namespace ::ec_prv::fbs;
	using ::flatbuffers::FlatBufferBuilder;

	switch (req->version) {
	case 1: {
		// search for the key in database
		::flatbuffers::Verifier v{req->lookup_key.data(), req->lookup_key.size()};
		if (!v.VerifyBuffer<URLIndex>()) {
			break;
		}
		std::string private_url_raw;
		store_->get(private_url_raw, std::span<uint8_t>{req->lookup_key});
		if (private_url_raw.length() == 0) {
			break;
		}
		auto* pu = GetPrivateURL(private_url_raw.data())->UnPack();
		// TODO(zds): factor out crypto params
		if (pu->pbkdf2_iters < 2'000'000) {
			break;
		}
		// perform decryption
		private_url::PrivateURL p{std::move(pu->salt), std::move(pu->iv),
					  std::move(pu->blinded_url)};
		auto decrypted_url = p.get_plaintext(std::move(req->pass));
		// build response message
		FlatBufferBuilder resp_fbb;
		auto dus = resp_fbb.CreateString(decrypted_url);
		auto resp_fb_builder = TrustedLookupResponseBuilder(resp_fbb);
		resp_fb_builder.add_version(1);
		resp_fb_builder.add_error(false);
		resp_fb_builder.add_url(dus);
		auto resp = resp_fb_builder.Finish();
		resp_fbb.Finish(resp);
		return resp_fbb.Release();
	}
	default:
		break;
	}
	// 	error case
	FlatBufferBuilder resp_fbb;
	auto tlrb = CreateTrustedLookupResponse(resp_fbb, 1, true);
	resp_fbb.Finish(tlrb);
	return resp_fbb.Release();
}

auto ServiceHandle::check_expiry(uint64_t input_expiry) -> bool {
	auto const now = std::chrono::system_clock::now();
	auto const epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
	auto const max_expected_expiry =
	    std::chrono::duration_cast<std::chrono::seconds>(MAX_AGE) + epoch;
	return max_expected_expiry > std::chrono::seconds{input_expiry};
}

} // namespace shortening_service
} // namespace ec_prv
