#include "b66/b66.h"
#include "b66/util.h"

namespace ec_prv {
namespace b66 {

auto marshal(std::string& dst, std::vector<uint8_t>& buf, uint32_t n) -> void {
	util::unpack(buf, n);
	enc(dst, buf);
}

auto unmarshal(uint32_t& dst, std::vector<uint8_t>& buf, std::string_view src) -> void {
	dec(buf, src);
	util::pack(dst, buf);
}

auto marshal(uint32_t src) -> std::string {
	std::string output;
	std::vector<uint8_t> buf(4, 0);
	marshal(output, buf, src);
	return output;
}

auto unmarshal(std::string_view src) -> uint32_t {
	std::vector<uint8_t> buf(4, 0);
	uint32_t output = 0;
	unmarshal(output, buf, src);
	return output;
}

} // namespace b66
} // namespace ec_prv
