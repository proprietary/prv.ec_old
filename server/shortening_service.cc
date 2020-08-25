#include "server/shortening_service.h"
#include "idl/generated/private_url_generated.h"
#include "idl/generated/shortening_service_generated.h"
#include "idl/generated/url_index_generated.h"
#include "server/db.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace {

///
/// Fast, non-cryptographically secure random number generator
/// https://en.wikipedia.org/wiki/Xorshift
/// https://www.jstatsoft.org/article/view/v008i14
///
class XORShift {
private:
	uint64_t state_;
	static std::unique_ptr<XORShift> instance_;
public:
	explicit XORShift() {
		// seed with real randomness from system call
		std::random_device rd("/dev/urandom");
		std::uniform_int_distribution<uint64_t> d(0);
		state_ = d(rd);
		instance_.reset(this);
	}

	static auto instance() -> std::unique_ptr<XORShift> {
		if (!instance_) {
			instance_.reset(new XORShift{});
		}
		return instance_;
	}

	///
	/// Get next pseudo-random number.
	///
	auto rand() noexcept -> uint64_t {
		uint64_t x = state_;
		x ^= x >> 12; // a
		x ^= x << 25; // b
		x ^= x >> 27; // c
		state_ = x;
		return x * 0x2545F4914F6CDD1DLL;
	}
};

} // namespace

namespace ec_prv {
namespace shortening_service {

ServiceHandle::ServiceHandle(std::shared_ptr<::ec_prv::db::KVStore> store) : store_{store} {}

auto ServiceHandle::handle_shortening_request(std::vector<uint8_t>& inbuf)
    -> flatbuffers::DetachedBuffer {
	bool ok = false;
	ok = ::ec_prv::ShorteningRequest::Verify(
	    ::flatbuffers::Verifier(inbuf.data(), inbuf.size()));
	if (!ok) {
		return;
	}
	auto* req = ::flatbuffers::GetRoot<::ec_prv::ShorteningRequest>(inbuf.data());
	switch (req->version()) {
	case 1: {
		::flatbuffers::FlatBufferBuilder fbb;
		// Create payload
		std::span<uint8_t> private_url_serialized;
		{
			::ec_prv::PrivateURLBuilder private_url_builder;
			private_url_builder.add_version(1);
			private_url_builder.add_iv(req->iv());
			private_url_builder.add_salt(req->salt());
			private_url_builder.add_blinded_url(req->blinded_url());
			// TODO
			private_url_builder.add_expiry(/* req->expiry() */ 1000);
			::ec_prv::FinishPrivateURLBuffer(fbb, private_url_builder.Finish());
			private_url_serialized =
			    std::span<uint8_t>(fbb.GetBufferPointer(), fbb.GetSize());
		}

		// Create key index
		{
			// find new index not taken
			std::string buf;
			while (true) {
				::ec_prv::URLIndexBuilder url_index_builder;
				url_index_builder.add_version(1);
				auto rand_index = XORShift::instance()->rand();
				url_index_builder.add_id(rand_index);
				fbb.Clear();
				auto url_index = url_index_builder.Finish();
				::ec_prv::FinishURLIndexBuffer(fbb, url_index);
				fbb.Finished();
				std::span<uint8_t> url_index_bytes(fbb.GetBufferPointer(),
								   fbb.GetSize());
				store_->get(buf, url_index_bytes);
				// key doesn't exist already in database
				if (buf.length() == 0) {
					auto ok =
					    store_->put(url_index_bytes, private_url_serialized);
					auto resp = ::ec_prv::CreateShorteningResponse(
					    fbb, 1, ok, 0, url_index);
					return fbb.Release();
				}
			}
		}
		return {};
	}
	default:
		return {};
	}
}

auto ServiceHandle::handle_lookup_request(std::vector<uint8_t>& src)
    -> ::flatbuffers::DetachedBuffer {
	bool ok =
	    ::ec_prv::LookupRequest::Verify(::flatbuffers::Verifier{inbuf.data(), inbuf.size()});
	if (!ok) {
		return;
	}
	auto* req = ::flatbuffers::GetRoot<::ec_prv::LookupRequest>(src.data());
	switch (req->version()) {
	case 1: {
		auto* url_index = req->lookup_key();
		if (url_index == nullptr) {
			// TODO return error message as flatbuffer
			return {};
		}
		if (url_index->version() != 1) {
			// TODO Return error message
			return {};
		}
		std::string buf;
		::flatbuffers::FlatBufferBuilder fbb;
		fbb.Finish(::ec_prv::URLIndex::Pack(fbb, url_index));
		store_->get(buf, std::span<uint8_t>(fbb.GetBufferPointer(), fbb.GetSize()));
		if (buf.size() == 0) {
			// TODO return error message
			return {};
		}
		fbb.Clear();
		ok = ::ec_prv::VerifyPrivateURLBuffer(
		    ::flatbuffers::Verifier{buf.data(), buf.size()});
		if (!ok) {
			// TODO return error message
			return {};
		}
		auto* pu = ::ec_prv::GetPrivateURL(buf.data());
		// Construct response
		fbb.Clear();
		auto resp = ::ec_prv::CreateLookupResponse(fbb, 1, true, *pu);
		return resp.Release();
	}
	default:
		return;
	}
}

} // namespace shortening_service
} // namespace ec_prv
