#ifndef _INCLUDE_EC_PRV_B66_B66_H
#define _INCLUDE_EC_PRV_B66_B66_H

#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <cstdint>
#include <iterator>

namespace ec_prv {

namespace b66 {

auto enc(std::string& dst, std::span<uint8_t> src) -> void;

auto dec(std::vector<uint8_t>& dst, std::string_view src) -> void;

auto unpack(uint32_t n) -> std::vector<uint8_t>;

auto pack(std::vector<uint8_t> const& src) -> uint32_t;

// auto strip_leading_zeros(std::vector<uint8_t>& dst) -> void;

} // namespace b66;

} // namespace ec_prv

#endif // _INCLUDE_EC_PRV_B66_B66_H

