#ifndef _INCLUDE_EC_PRV_SERVER_SHORTENING_SERVICE_H
#define _INCLUDE_EC_PRV_SERVER_SHORTENING_SERVICE_H
#include "idl/all_generated_flatbuffers.h"
#include "server/db.h"
#include <chrono>
#include <cstdint>
#include <flatbuffers/flatbuffers.h>
#include <memory>
#include <string>
#include <vector>

namespace ec_prv {
namespace shortening_service {

using namespace std::chrono_literals;

class ServiceHandle {
private:
	::ec_prv::db::KVStore* const store_;

	///
	/// Maximum time allowed to retain links.
	///
	static constexpr auto MAX_AGE = 8760h;

	///
	/// Ensures a user-generated expiry is less than the maximuma age.
	///
	static auto check_expiry(uint64_t input_expiry) -> bool;

public:
	explicit ServiceHandle(::ec_prv::db::KVStore* store);

	auto handle(std::unique_ptr<::ec_prv::fbs::ShorteningRequestT> req)
	    -> ::flatbuffers::DetachedBuffer;

	auto handle(std::unique_ptr<::ec_prv::fbs::LookupRequestT> req)
	    -> ::flatbuffers::DetachedBuffer;

	auto handle(std::unique_ptr<::ec_prv::fbs::TrustedShorteningRequestT> req) -> ::flatbuffers::DetachedBuffer;

	auto handle(std::unique_ptr<::ec_prv::fbs::TrustedLookupRequestT> req) -> ::flatbuffers::DetachedBuffer;
};

} // namespace shortening_service
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_SERVER_SHORTENING_SERVICE_H
