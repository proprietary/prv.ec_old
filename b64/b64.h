#ifndef __INCLUDE_EC_PRV_B64_B64_H
#define __INCLUDE_EC_PRV_B64_B64_H
#include <string>
#include <string_view>
#include <vector>
#include <span>

namespace ec_prv {
namespace b64 {

///
/// Encode raw bytes to a base64-encoded string with resistance to side-channel attacks (slow).
///
auto enc_nonurlsafe(std::vector<uint8_t>&) -> std::string;

///
/// Decode a base64-encoded string to bytes with resistance to side-channel attacks (slow).
///
auto dec_nonurlsafe(std::string_view) -> std::vector<uint8_t>;

///
/// Encode raw bytes to a base64-encoded string.
///
auto enc(std::vector<uint8_t>&) -> std::string;

///
/// Encode raw bytes to a base64-encoded string.
///
auto enc(std::span<uint8_t>) -> std::string;

///
/// Decode a base64-encoded string to bytes.
///
auto dec(std::string_view) -> std::vector<uint8_t>;

///
/// Decode a base64-encoded string to bytes.
///
auto dec(std::string const&) -> std::vector<uint8_t>;

///
/// Converts a non-url-safe base64-encoded string to a url-safe one.
///
void to_urlsafe(std::string& s);

///
/// Detects if the input string is url-safe.
///
bool is_urlsafe(std::string_view s);

} // namespace b64
} // namespace ec_prv
#endif // __INCLUDE_EC_PRV_B64_B64_H
