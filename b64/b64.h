#ifndef __INCLUDE_EC_PRV_B64_B64_H
#define __INCLUDE_EC_PRV_B64_B64_H
#include <string>
#include <vector>

namespace ec_prv {
namespace b64 {

///
/// Encode raw bytes to a base64-encoded string.
///
auto enc(std::vector<uint8_t>&) -> std::string;

///
/// Decode a base64-encoded string to bytes.
///
auto dec(std::string const&) -> std::vector<uint8_t>;

} // namespace b64
} // namespace ec_prv
#endif // __INCLUDE_EC_PRV_B64_B64_H
