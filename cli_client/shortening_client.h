#ifndef _INCLUDE_EC_PRV_CLI_CLIENT_SHORTENING_CLIENT_H
#define _INCLUDE_EC_PRV_CLI_CLIENT_SHORTENING_CLIENT_H

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <memory>

namespace ec_prv {
namespace shortening_client {

struct client_v1_t {
	char const* upstream_server;
	int32_t pbkdf2_rounds;

	auto shorten_v1(std::string_view url_plaintext) -> std::string;

	auto lookup_v1(std::string_view base_identifier, std::string_view pass) -> std::string;
};


} // namespace shortening_client
} // namespace ec_prv

// TODO(zds): make C interface
// extern "C" {
// 	void shorten_v1(client_v1_t*, char const* url_plaintext);

// 	void lookup_v1(client_v1_t*, char* dst, char const* base_identifier, char const* pass);
// }


#endif // _INCLUDE_EC_PRV_CLI_CLIENT_SHORTENING_CLIENT_H
