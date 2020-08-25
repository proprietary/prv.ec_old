#ifndef _INCLUDE_EC_PRV_SHORTENING_SERVICE_H
#define _INCLUDE_EC_PRV_SHORTENING_SERVICE_H
#include "server/db.h"
#include <cstdint>
#include <flatbuffers/flatbuffers.h>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

namespace ec_prv {
namespace shortening_service {

using namespace std::chrono_literals;

class ServiceHandle {
private:
	std::shared_ptr<::ec_prv::db::KVStore> store_;

	///
	/// Maximum time allowed to retain links.
	///
	static constexpr auto MAX_AGE = 8760h;

	///
	/// Ensures a user-generated expiry is less than the maximuma age.
	///
	static bool check_expiry(uint64_t input_expiry);
public:
	explicit ServiceHandle(std::shared_ptr<::ec_prv::db::KVStore> store);

	auto handle_shortening_request(std::span<uint8_t> src) -> ::flatbuffers::DetachedBuffer;

	auto handle_lookup_request(std::span<uint8_t> src)
	    -> ::flatbuffers::DetachedBuffer;
};

} // namespace shortening_service
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_SHORTENING_SERVICE_H
