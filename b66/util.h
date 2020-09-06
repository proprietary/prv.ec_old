#ifndef _INCLUDE_EC_PRV_B66_UTIL_H
#define _INCLUDE_EC_PRV_B66_UTIL_H

#include <cstdint>
#include <vector>
#include <span>

namespace ec_prv::b66::util {

auto unpack(uint32_t n) -> std::vector<uint8_t>;

auto unpack(std::vector<uint8_t>& dst, uint32_t n) -> void;

auto pack(std::span<uint8_t> src) -> uint32_t;

auto pack(uint32_t& dst, std::span<uint8_t> src) -> void;

} // namespace ec_prv::b66::util
#endif // _INCLUDE_EC_PRV_B66_UTIL_H

