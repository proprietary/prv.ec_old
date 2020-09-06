#ifndef _INCLUDE_EC_PRV_B66_MARSHAL_INT_H
#define _INCLUDE_EC_PRV_B66_MARHSAL_INT_H

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ec_prv {
namespace b66 {

///
/// Encode a u32 to a base66-encoded string. Takes references to
/// intermediate buffers for a zero-copy optimization. `buf` should
/// have 4 bytes.
///
auto marshal(std::string& dst, std::vector<uint8_t>& buf, uint32_t n) -> void;

///
/// Decode a base66-encoded string to a u32. Takes references to
/// intermediate buffers for a zero-copy implementation. `buf` should
/// have 4 bytes.
///
auto unmarshal(uint32_t& dst, std::vector<uint8_t>& buf, std::string_view src) -> void;

///
/// Convenience function to encode a u32 to a base66-encoded
/// string. Possibly less performant than the zero-copy overload.
///
auto marshal(uint32_t n) -> std::string;

///
/// Convenience function to decode a base66-encoded string to a
/// u32. Possibly less performant than the zero-copy overload.
///
auto unmarshal(std::string_view src) -> uint32_t;

} // namespace b66
} // namespace ec_prv

#endif // _INCLUDE_EC_PRV_B66_MARSHAL_INT_H
