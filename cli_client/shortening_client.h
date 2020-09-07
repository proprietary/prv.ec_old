#ifndef _INCLUDE_EC_PRV_CLI_CLIENT_SHORTENING_CLIENT_H
#define _INCLUDE_EC_PRV_CLI_CLIENT_SHORTENING_CLIENT_H

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ec_prv {
namespace shortening_client {

using namespace std::string_view_literals;

///
/// Split apart a shortened URL to the identifier part and the "password" part.
/// @returns (identifier, pass)
///
auto parse_shortened_url(std::string_view full_url,
			 std::string_view upstream_server = "https://prv.ec"sv)
    -> std::pair<std::string, std::string>;

namespace v1 {

struct ClientV1 {
	char const* upstream_server;
	int32_t pbkdf2_rounds;

	auto shorten(std::string const& url_plaintext) -> std::string;

	auto lookup(std::string const& base_identifier, std::string_view pass) -> std::string;
};

} // namespace v1

} // namespace shortening_client
} // namespace ec_prv

// TODO(zds): make C interface
// extern "C" {
// 	void shorten_v1(client_v1_t*, char const* url_plaintext);

// 	void lookup_v1(client_v1_t*, char* dst, char const* base_identifier, char const* pass);
// }

#endif // _INCLUDE_EC_PRV_CLI_CLIENT_SHORTENING_CLIENT_H
