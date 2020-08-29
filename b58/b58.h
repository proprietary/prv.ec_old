#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ec_prv {
namespace b58 {

///
/// Encode byte array as a base-58 string; little-endian byte order.
/// Note: Since `src` is assumed to be little-endian, the rightmost
/// trailing 0s don't matter. If you have an array like {0x1, 0x55,
/// 0x0, 0x0} it is effectively equal to {0x1, 0x55}.
///
auto enc(std::span<uint8_t> src) -> std::string;

///
/// Decode a base-58 encoded string string as a byte array.
///
auto dec(std::string_view src) -> std::vector<uint8_t>;

} // namespace b58
} // namespace ec_prv
