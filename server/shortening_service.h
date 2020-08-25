#ifndef _INCLUDE_EC_PRV_SHORTENING_SERVICE_H
#define _INCLUDE_EC_PRV_SHORTENING_SERVICE_H
#include "server/db.h"
#include "server/xorshift.h"
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
	std::shared_ptr<::ec_prv::db::KVStore> store_;

	std::shared_ptr<::ec_prv::xorshift::XORShift> rand_source_;

	///
	/// Maximum time allowed to retain links.
	///
	static constexpr auto MAX_AGE = 8760h;

	///
	/// Ensures a user-generated expiry is less than the maximuma age.
	///
	static auto check_expiry(uint64_t input_expiry) -> bool;

public:
	explicit ServiceHandle(std::shared_ptr<::ec_prv::db::KVStore> store,
			       std::shared_ptr<::ec_prv::xorshift::XORShift> xorshift =
				   std::make_shared<::ec_prv::xorshift::XORShift>());

	auto handle_shortening_request(std::span<uint8_t> src) -> ::flatbuffers::DetachedBuffer;

	auto handle_lookup_request(std::span<uint8_t> src) -> ::flatbuffers::DetachedBuffer;
};

} // namespace shortening_service
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_SHORTENING_SERVICE_H
