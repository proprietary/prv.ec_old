#ifndef _INCLUDE_EC_PRV_CLI_CLIENT_HTTP_REQUESTS_H
#define _INCLUDE_EC_PRV_CLI_CLIENT_HTTP_REQUESTS_H

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <span>

namespace ec_prv::cli_client {

auto request(std::vector<uint8_t>& dst, std::string endpoint, std::span<uint8_t> postdata) -> bool;

} // namespace ec_prv::cli_client

#endif // _INCLUDE_EC_PRV_CLI_CLIENT_HTTP_REQUESTS_H
