#include <b64.h>
#include <cassert>
#include <memory>
#include <openssl/base64.h>
#include <stdexcept>

namespace ec_prv {
namespace b64 {

auto enc(std::vector<uint8_t>& inbuf) -> std::string {
	size_t out_len = 0;
	auto err = EVP_EncodedLength(&out_len, inbuf.size());
	assert(err > 0);
	if (out_len == 0) {
		return {};
	}
	std::string out(out_len, '\0');
	auto bytes_written = EVP_EncodeBlock(
	    reinterpret_cast<uint8_t*>(const_cast<char*>(out.data())), inbuf.data(), inbuf.size());
	assert(bytes_written <= out_len);
	return out;
}

auto dec(std::string const& s) -> std::vector<uint8_t> {
	size_t max_len = 0;
	auto err = EVP_DecodedLength(&max_len, s.length());
	assert(err > 0);
	if (max_len == 0) {
		return {};
	}
	std::vector<uint8_t> out(max_len, 0);
	size_t out_len = 0;
	err = EVP_DecodeBase64(out.data(), &out_len, max_len,
			       reinterpret_cast<uint8_t const*>(s.data()), s.length());
	assert(err > 0);
	return out;
}

} // namespace b64
} // namespace ec_prv
