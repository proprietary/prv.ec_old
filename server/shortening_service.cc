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
auto find_new_url_index_v1(::ec_prv::xorshift::XORShift& rand_source,
			   ::ec_prv::db::KVStore const& kvstore) -> ::flatbuffers::DetachedBuffer {
	// find new index not taken
	std::string buf;
	::flatbuffers::FlatBufferBuilder url_index_fbb;
	while (true) {
		::ec_prv::fbs::URLIndexBuilder url_index_builder(url_index_fbb);
		url_index_builder.add_version(1);
		auto rand_index = rand_source->rand();
		url_index_builder.add_id(rand_index);
		auto url_index = url_index_builder.Finish();
		::ec_prv::fbs::FinishURLIndexBuffer(url_index_fbb, url_index);
		auto url_index_bytes = fbspan(url_index_fbb);
		kvstore.get(buf, url_index_bytes);
		// generated an random index that doesn't exist already--good!
		if (buf.length() == 0) {
			return url_index_fbb.Release();
		}
		url_index_fbb.Clear();
	}
}

} // namespace

namespace ec_prv {
namespace shortening_service {

ServiceHandle::ServiceHandle(::ec_prv::db::KVStore* store,
			     std::shared_ptr<xorshift::XORShift> xorshift)
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
		}

		// Create payload
		::flatbuffers::FlatBufferBuilder private_url_fbb;
		{
			auto iv = private_url_fbb.CreateVector(req->iv.data(), req->iv.size());
			auto salt =
			    private_url_fbb.CreateVector(req->salt.data(), req->salt.size());
			auto blinded_url = private_url_fbb.CreateVector(req->blinded_url.data(),
									req->blinded_url.size());
			auto p = fbs::CreatePrivateURL(private_url_fbb, 1, req->expiry, salt, iv,
						       blinded_url);
			private_url_fbb.Finish(p);
		}

		// Create unique identifier
		auto url_index = find_new_url_index_v1(*rand_source_, *store_);
		std::span<uint8_t> url_index_bytes{url_index.data(), url_index.size()};
		auto ok = store_->put(url_index_bytes, fbspan(private_url_fbb));
		assert(ok == true); // TODO: handle error better
		::flatbuffers::FlatBufferBuilder resp_fbb;
		auto resp = ::ec_prv::fbs::CreateShorteningResponse(resp_fbb, 1, ok, url_index);
		resp_fbb.Finish(resp);
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
		auto url_index = std::move(req->lookup_key);
		if (!url_index) {
			break;
		}
		if (url_index->version != 1) {
			break;
		}
		if (url_index->id <= 0) {
			break;
		}
		std::string rocksdb_result_buf;
		{
			::flatbuffers::FlatBufferBuilder url_index_fbb;
			url_index_fbb.Finish(
			    ::ec_prv::fbs::URLIndex::Pack(url_index_fbb, url_index.get()));
			store_->get(rocksdb_result_buf, fbspan(url_index_fbb));
			// TODO(zds): if failure... resp = ::ec_prv::fbs::CreateLookupResponse(fbb,
			// 1, false);
		}
		auto* pu = ::ec_prv::fbs::GetPrivateURL(rocksdb_result_buf.data());
		// Construct success response
		::flatbuffers::FlatBufferBuilder fbb;
		auto resp = ::ec_prv::fbs::CreateLookupResponse(
		    fbb, 1, true, ::ec_prv::fbs::PrivateURL::Pack(fbb, pu->UnPack()));
		fbb.Finish(resp);
		return fbb.Release();
	}
	default:
		break;
	}
	// failure case
	::flatbuffers::FlatBufferBuilder fbb;
	auto resp = ::ec_prv::fbs::CreateLookupResponse(fbb, 1, false);
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
		auto pubuf = CreatePrivateURL(private_url_fbb, 1, 0, salt, iv, blinded_url);
		private_url_fbb.Finish(pubuf);
		// generate new key for rocksdb
		auto url_index = find_new_url_index_v1(*rand_source_, *store_);
		std::span<uint8_t> url_index_bytes{url_index.data(), url_index.size()};
		// put into rocksdb
		store_->put(url_index_bytes, fbspan(private_url_fbb));
		// construct response
		auto const& pass = std::get<std::string>(pu.value());
		FlatBufferBuilder resp_fbb;
		auto tsrb = TrustedShorteningResponseBuilder(resp_fbb);
		tsrb.add_version(1);
		auto pass_vec = resp_fbb.CreateVector(reinterpret_cast<uint8_t const*>(pass.data()),
						      pass.size());
		// make another copy of the new url index recently generated
		auto const* url_index_struct = url_index.GetRoot<URLIndex>()->UnPack();
		// parses out the id from the url index structure
		auto resp_lookup_key = CreateURLIndex(resp_fbb, 1, url_index_struct->id);
		tsrb.add_lookup_key(resp_lookup_key);
		tsrb.add_pass(pass_vec);
		auto tsr = tsrb.Finish();
		resp_fbb.Finish(tsr);
		return resp_fbb.Release();
	}
	default:
		break;
	}
	// failure case
	::flatbuffers::FlatBufferBuilder resp_fbb;
	::ec_prv::fbs::TrustedShorteningResponseBuilder tsrb(resp_fbb);
	tsrb.add_version(1);
	tsrb.add_error(true);
	auto tsr = tsrb.Finish();
	resp_fbb.Finish(tsr);
	return resp_fbb.Release();
}

auto ServiceHandle::handle(std::unique_ptr<::ec_prv::fbs::TrustedLookupRequestT> req)
    -> ::flatbuffers::DetachedBuffer {
	using namespace ::ec_prv::fbs;
	using ::flatbuffers::FlatBufferBuilder;

	switch (req->version) {
	case 1: {
		// search for the key in database
		FlatBufferBuilder url_index_fbb;
		url_index_fbb.Finish(URLIndex::Pack(url_index_fbb, req->lookup_key.get()));
		std::string private_url_raw;
		store_->get(private_url_raw, fbspan(url_index_fbb));
		FlatBufferBuilder private_url_fbb;
		auto* pu = GetPrivateURL(private_url_raw.data())->UnPack();
		// perform decryption
		private_url::PrivateURL p{std::move(pu->salt), std::move(pu->iv),
					  std::move(pu->blinded_url)};
		auto decrypted_url = p.get_plaintext(std::move(req->pass));
		// build response message
		FlatBufferBuilder resp_fbb;
		auto resp_fb_builder = TrustedLookupResponseBuilder(resp_fbb);
		resp_fb_builder.add_version(1);
		resp_fb_builder.add_url(resp_fbb.CreateString(decrypted_url));
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
