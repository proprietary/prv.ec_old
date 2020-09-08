#include "server/shortening_service.h"

#include "flatbuffers/flatbuffers.h"
#include "idl/all_generated_flatbuffers.h"
#include "private_url/private_url.h"
#include "server/db.h"
#include "url_index/url_index.h"
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
auto find_new_url_index_v1(::ec_prv::db::KVStore& kvstore) -> ec_prv::url_index::URLIndex {
	auto k = ec_prv::url_index::URLIndex::random();
	rocksdb::PinnableSlice v;
	while (true) {
		auto s = kvstore.get(v, k);
		if (s.IsNotFound()) {
			// found available url index
			break;
		} else if (!s.ok()) {
			throw ec_prv::db::RocksDBError{"RocksDB error"};
		}
		v.Reset();
		k = ec_prv::url_index::URLIndex::random();
	}
	return k;
}

} // namespace

namespace ec_prv {
namespace shortening_service {

ServiceHandle::ServiceHandle(::ec_prv::db::KVStore* store) : store_{store} {}

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
		auto url_index = find_new_url_index_v1(*store_);
		auto ok = store_->put(url_index, fbspan(private_url_fbb));
		assert(ok == true); // TODO: handle error better
		::flatbuffers::FlatBufferBuilder resp_fbb;
		auto url_index_b66 = resp_fbb.CreateString(url_index.as_base_66_string());
		::ec_prv::fbs::ShorteningResponseBuilder srb{resp_fbb};
		srb.add_version(1);
		srb.add_error(!ok);
		srb.add_lookup_key(url_index.as_integer());
		srb.add_lookup_key_encoded(url_index_b66);
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
		auto lookup_key = url_index::URLIndex::from_integer(req->lookup_key);
		if (lookup_key.access_type() != url_index::URLIndexAccess::PUBLIC) {
			break;
		}
		rocksdb::PinnableSlice rocksdb_result_buf;
		auto s = store_->get(rocksdb_result_buf, lookup_key);
		if (!s.ok()) {
			break;
		}
		if (rocksdb_result_buf.empty()) {
			break;
		}
		// Construct success response
		::flatbuffers::FlatBufferBuilder fbb;
		auto puv =
		    fbb.CreateVector(reinterpret_cast<uint8_t const*>(rocksdb_result_buf.data()),
				     rocksdb_result_buf.size());
		::ec_prv::fbs::LookupResponseBuilder lrb{fbb};
		lrb.add_version(1);
		lrb.add_error(false);
		lrb.add_data(puv);
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

auto ServiceHandle::handle(std::unique_ptr<::ec_prv::fbs::LookupRequestWebT> req)
    -> ::flatbuffers::DetachedBuffer {
	using ::flatbuffers::FlatBufferBuilder;
	using url_index::URLIndex;
	using url_index::URLIndexAccess;

	FlatBufferBuilder fbb;
	auto lookup_key = URLIndex::from_base_66_string(req->lookup_key);
	if (lookup_key.access_type() != URLIndexAccess::PUBLIC) {
		auto resp = fbs::CreateLookupResponse(fbb, 1, true);
		fbb.Finish(resp);
		return fbb.Release();
	}
	rocksdb::PinnableSlice rocksdb_result_buf;
	auto status = store_->get(rocksdb_result_buf, lookup_key);
	if (!status.ok() || rocksdb_result_buf.empty()) {
		auto resp = fbs::CreateLookupResponse(fbb, 1, true);
		fbb.Finish(resp);
		return fbb.Release();
	}
	auto puv = fbb.CreateVector(reinterpret_cast<uint8_t const*>(rocksdb_result_buf.data()),
				    rocksdb_result_buf.size());
	fbs::LookupResponseBuilder b{fbb};
	b.add_version(1);
	b.add_error(false);
	b.add_data(puv);
	auto resp = b.Finish();
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
		auto url_index = find_new_url_index_v1(*store_);
		// put into rocksdb
		store_->put(url_index, fbspan(private_url_fbb));
		// construct response
		auto const& pass = std::get<std::string>(pu.value());
		FlatBufferBuilder resp_fbb;
		auto pass_vec = resp_fbb.CreateVector(reinterpret_cast<uint8_t const*>(pass.data()),
						      pass.size());
		TrustedShorteningResponseBuilder tsrb{resp_fbb};
		tsrb.add_version(1);
		tsrb.add_error(false);
		tsrb.add_pass(pass_vec);
		tsrb.add_lookup_key(url_index.as_integer());
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
		auto url_index = url_index::URLIndex::from_integer(req->lookup_key);
		rocksdb::PinnableSlice private_url_raw;
		auto s = store_->get(private_url_raw, url_index);
		if (!s.ok()) {
			break;
		}
		if (private_url_raw.empty()) {
			break;
		}
		auto pu = std::make_unique<PrivateURLT>();
		GetPrivateURL(private_url_raw.data())->UnPackTo(pu.get());
		if (pu->version != 1) {
			break;
		}
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
