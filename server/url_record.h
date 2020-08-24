#ifndef __INCLUDE_EC_PRV_URL_RECORD_H
#define __INCLUDE_EC_PRV_URL_RECORD_H
#include <vector>
#include <string>
#include <string_view>
#include <span>

namespace ec_prv {
namespace url_record {

struct URLRecord {
	uint8_t version_ = 1;
	uint64_t expiry_;
	std::string_view url_;
};

struct PrivateURLRecord {
	uint8_t version_ = 1;
	uint64_t expiry_;
	std::span<uint8_t> iv_;
	std::span<uint8_t> blinded_url_;
	static PrivateURLRecord from();
};


} // namespace url_record
} // namespace ec_prv


#endif // __INCLUDE_EC_PRV_URL_RECORD_H
