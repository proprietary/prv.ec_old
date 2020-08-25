#include "server/shortening_service.h"

// ORDER MATTERS:
//
// Unfortunately the build rules for flatc don't
// #include the generated headers of the *.fbs files that were
// included in the .fbs schema
//
/*	._________________.    	  .____________________.
 * 	|				  |		  |		   			   |
 *  | private_url.fbs |	   	  |	  url_index.fbs	   |
 *	|				  |		  |		   		   	   |
 *	._________________.	   	  .____________________.
 *				^  					   ^
 *				|	   	   	   	   	   |
 *				|  	   	   	   	   	   |
 *			.______________________________.
 *			|  	   	   	   	   	   	   	   |
 *			|  shortening service  	   	   |
 *			|  ├── lookup_request.fbs      |
 * 	   	   	|  ├── lookup_response.fbs     |
 *		   	|  ├── shortening_request.fbs  |
 *			|  └── shortening_response.fbs |
 *			|      						   |
 * 		   	.______________________________.
 *
 *
*/
#include "idl/private_url_generated.h"
#include "idl/url_index_generated.h"
#include "idl/lookup_request_generated.h"
#include "idl/lookup_response_generated.h"
#include "idl/shortening_request_generated.h"
#include "idl/shortening_response_generated.h"
// /ORDER MATTERS

#include "server/db.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <chrono>
#include <limits>

namespace {

///
/// Fast, non-cryptographically secure random number generator
/// https://en.wikipedia.org/wiki/Xorshift
/// https://www.jstatsoft.org/article/view/v008i14
///
class XORShift {
private:
	uint64_t state_ = 0;
	static std::unique_ptr<XORShift> instance_;
public:
	explicit XORShift() {
		// seed with real randomness from system call
		std::random_device rd("/dev/urandom");
		std::uniform_int_distribution<unsigned long> d(0);
		state_ = d(rd);
		instance_.reset(this);
	}

	static auto instance() -> std::unique_ptr<XORShift>&& {
		if (!instance_) {
			instance_.reset(new XORShift{});
		}
		return std::move(instance_);
	}

	///
	/// Get next pseudo-random number.
	///
	auto rand() noexcept -> uint64_t {
		return 1;
		// uint64_t x = state_;
		// x ^= x >> 12; // a
		// x ^= x << 25; // b
		// x ^= x >> 27; // c
		// state_ = x;
		// return x * 0x2545F4914F6CDD1DULL;
	}
};

std::span<uint8_t> fbspan(::flatbuffers::FlatBufferBuilder& fbb) {
	return std::span<uint8_t>(fbb.GetBufferPointer(), fbb.GetSize());
}

} // namespace

namespace ec_prv {
namespace shortening_service {

ServiceHandle::ServiceHandle(std::shared_ptr<::ec_prv::db::KVStore> store) : store_{store} {}

auto ServiceHandle::handle_shortening_request(std::span<uint8_t> inbuf)
    -> flatbuffers::DetachedBuffer {
	bool ok = false;
	::flatbuffers::Verifier v(inbuf.data(), inbuf.size());
	ok = ::ec_prv::fbs::VerifyShorteningRequestBuffer(v);
	if (!ok) {
		::flatbuffers::FlatBufferBuilder fbb;
		::ec_prv::fbs::ShorteningResponseBuilder srb{fbb};
		srb.add_error(true);
		srb.add_version(1);
		auto sr = srb.Finish();
		fbb.Finish(sr);
		return fbb.Release();
	}
	auto* req = flatbuffers::GetRoot<::ec_prv::fbs::ShorteningRequest>(inbuf.data());
	switch (req->version()) {
	case 1: {
		// validate
		{
			ok = req->salt()->size() > 0 && req->iv()->size() > 0 && req->blinded_url()->size() > 0;
			if (!ok) {
				break;
			}
			if (!check_expiry(req->expiry())) {
				break;
			}
		}
		// Create payload
		::flatbuffers::FlatBufferBuilder private_url_fbb;
		{
			auto iv = private_url_fbb.CreateVector(req->iv()->data(), req->iv()->size());
			auto salt = private_url_fbb.CreateVector(req->salt()->data(), req->salt()->size());
			auto blinded_url = private_url_fbb.CreateVector(req->blinded_url()->data(), req->blinded_url()->size());
			auto p = fbs::CreatePrivateURL(private_url_fbb, 1, req->expiry(), salt, iv, blinded_url);
			private_url_fbb.Finish(p);
		}
		std::span<uint8_t> private_url_serialized(private_url_fbb.GetBufferPointer(), private_url_fbb.GetSize());

		// Create key index
		{
			// find new index not taken
			std::string buf;
			while (true) {
				::flatbuffers::FlatBufferBuilder url_index_fbb;
				::ec_prv::fbs::URLIndexBuilder url_index_builder(url_index_fbb);
				url_index_builder.add_version(1);
				// auto rand_index = XORShift::instance()->rand();
				uint64_t rand_index = 4; // !! TODO
				url_index_builder.add_id(rand_index);
				auto url_index = url_index_builder.Finish();
				::ec_prv::fbs::FinishURLIndexBuffer(url_index_fbb, url_index);
				std::span<uint8_t> url_index_bytes(url_index_fbb.GetBufferPointer(),
								   url_index_fbb.GetSize());
				store_->get(buf, url_index_bytes);
				// generated an random index that doesn't exist already--good!
				if (buf.length() == 0) {
					::flatbuffers::FlatBufferBuilder err_fbb;
					auto ok =
					    store_->put(url_index_bytes, private_url_serialized);
					auto resp = ::ec_prv::fbs::CreateShorteningResponse(err_fbb, 1, ok, url_index);
					err_fbb.Finish(resp);
					return err_fbb.Release();
				}
			}
		}
		break;
	}
	default:
		break;
	}
	// return error
	::flatbuffers::FlatBufferBuilder fbb;
	::ec_prv::fbs::ShorteningResponseBuilder srb{fbb};
	srb.add_version(1);
	srb.add_error(true);
	auto sr = srb.Finish();
	fbb.Finish(sr);
	return fbb.Release();
}

auto ServiceHandle::handle_lookup_request(std::span<uint8_t> src)
    -> ::flatbuffers::DetachedBuffer {
	::flatbuffers::Verifier v{src.data(), src.size()};
	if (!::ec_prv::fbs::VerifyLookupRequestBuffer(v)) {
		::flatbuffers::FlatBufferBuilder fbb;
		auto resp = ::ec_prv::fbs::CreateLookupResponse(fbb, 1, true);
		fbb.Finish(resp);
		return fbb.Release();
	}
	auto* req = ::flatbuffers::GetRoot<::ec_prv::fbs::LookupRequest>(src.data());
	switch (req->version()) {
	case 1: {
		auto* url_index = req->lookup_key();
		if (url_index == nullptr) {
			break;
		}
		if (url_index->version() != 1) {
			break;
		}
		std::string rocksdb_result_buf;
		{
			::flatbuffers::FlatBufferBuilder url_index_fbb;
			url_index_fbb.Finish(::ec_prv::fbs::URLIndex::Pack(url_index_fbb, url_index->UnPack()));
			store_->get(rocksdb_result_buf, fbspan(url_index_fbb));
			// TODO(zds): if failure... resp = ::ec_prv::fbs::CreateLookupResponse(fbb, 1, false);
		}
		auto* pu = ::ec_prv::fbs::GetPrivateURL(rocksdb_result_buf.data());
		// Construct success response
		::flatbuffers::FlatBufferBuilder fbb;
		auto resp = ::ec_prv::fbs::CreateLookupResponse(fbb, 1, true, ::ec_prv::fbs::PrivateURL::Pack(fbb, pu->UnPack()));
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

bool ServiceHandle::check_expiry(uint64_t input_expiry) {
	auto const now = std::chrono::system_clock::now();
	auto const epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
	auto const max_expected_expiry = std::chrono::duration_cast<std::chrono::seconds>(MAX_AGE) + epoch;
	return max_expected_expiry > std::chrono::seconds{input_expiry};
}

} // namespace shortening_service
} // namespace ec_prv

