#ifndef _INCLUDE_EC_PRV_SHORTENING_SERVICE_H
#define _INCLUDE_EC_PRV_SHORTENING_SERVICE_H
#include "server/db.h"
#include <cstdint>
#include <flatbuffers/flatbuffers.h>
#include <memory>
#include <string>
#include <vector>

namespace ec_prv {
namespace shortening_service {

class ServiceHandle {
private:
	std::shared_ptr<::ec_prv::db::KVStore> store_;

public:
	explicit ServiceHandle(std::shared_ptr<::ec_prv::db::KVStore> store);

	auto handle_shortening_request(std::vector<uint8_t>& src) -> ::flatbuffers::DetachedBuffer;

	auto handle_lookup_request(std::vector<uint8_t>& dst, std::vector<uint8_t>& src)
	    -> ::flatbuffers::DetachedBuffer;
};

} // namespace shortening_service
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_SHORTENING_SERVICE_H
