#include <b64.h>
#include <cassert>
#include <memory>
#include <openssl/base64.h>
#include <stdexcept>
#include <string_view>

namespace {

// ASCII table (not url-safe)
//
const unsigned char pr2six[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64,
    64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 64, 0,  1,	2,  3,	4,
    5, 6, 7,	8,  9,	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64,
    64, 64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
    46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64};

// ASCII table
static const unsigned char urlsafe_table[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 64, 0,  1,	2,  3,	4,  5,	6,
    7,	8,  9,	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 63,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64};

const char BASIS_64_URLSAFE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

const char BASIS_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

int encode_len(int len) { return ((len + 2) / 3 * 4) + 1; }

} // namespace

namespace ec_prv {
namespace b64 {

auto enc_secure(std::vector<uint8_t>& inbuf) -> std::string {
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

auto dec_secure(std::string_view s) -> std::vector<uint8_t> {
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

auto enc_urlsafe(std::vector<uint8_t>& inbuf) -> std::string {
	return enc_urlsafe(std::span<uint8_t>{inbuf});
}

auto enc_urlsafe(std::span<uint8_t> inbuf) -> std::string {
	int32_t i = 0;
	int32_t len = inbuf.size();
	std::string out(encode_len(inbuf.size()), '\0');
	auto* p = const_cast<char*>(out.data());
	auto* o = inbuf.data();
	for (i = 0LL; i < len - 2; i += 3) {
		*p++ = BASIS_64_URLSAFE[(o[i] >> 2) & 0x3F];
		*p++ = BASIS_64_URLSAFE[((o[i] & 0x3) << 4) | (static_cast<int32_t>(o[i + 1] & 0xF0) >> 4)];
		*p++ = BASIS_64_URLSAFE[((o[i + 1] & 0xF) << 2) |
				(static_cast<int32_t>(o[i + 2] & 0xC0) >> 6)];
		*p++ = BASIS_64_URLSAFE[o[i + 2] & 0x3F];
	}
	if (i < len) {
		*p++ = BASIS_64_URLSAFE[(o[i] >> 2) & 0x3F];
		if (i == (len - 1)) {
			*p++ = BASIS_64_URLSAFE[((o[i] & 0x3) << 4)];
			*p++ = '=';
		} else {
			*p++ = BASIS_64_URLSAFE[((o[i] & 0x3) << 4) |
					(static_cast<int32_t>(o[i + 1] & 0xF0) >> 4)];
			*p++ = BASIS_64_URLSAFE[((o[i + 1] & 0xF) << 2)];
		}
		*p++ = '=';
	}
	return out;
}

auto enc(std::span<uint8_t> inbuf) -> std::string {
	int32_t i = 0;
	int32_t len = inbuf.size();
	std::string out(encode_len(inbuf.size()), '\0');
	auto* p = const_cast<char*>(out.data());
	auto* o = inbuf.data();
	for (i = 0LL; i < len - 2; i += 3) {
		*p++ = BASIS_64[(o[i] >> 2) & 0x3F];
		*p++ = BASIS_64[((o[i] & 0x3) << 4) | (static_cast<int32_t>(o[i + 1] & 0xF0) >> 4)];
		*p++ = BASIS_64[((o[i + 1] & 0xF) << 2) |
				(static_cast<int32_t>(o[i + 2] & 0xC0) >> 6)];
		*p++ = BASIS_64[o[i + 2] & 0x3F];
	}
	if (i < len) {
		*p++ = BASIS_64[(o[i] >> 2) & 0x3F];
		if (i == (len - 1)) {
			*p++ = BASIS_64[((o[i] & 0x3) << 4)];
			*p++ = '=';
		} else {
			*p++ = BASIS_64[((o[i] & 0x3) << 4) |
					(static_cast<int32_t>(o[i + 1] & 0xF0) >> 4)];
			*p++ = BASIS_64[((o[i + 1] & 0xF) << 2)];
		}
		*p++ = '=';
	}
	return out;
}

auto dec_urlsafe(std::string const& s) -> std::vector<uint8_t> { return dec_urlsafe(std::string_view{s}); }

auto dec_urlsafe(std::string_view s) -> std::vector<uint8_t> {
	// calculate decoded length
	auto* bufin = reinterpret_cast<uint8_t*>(const_cast<char*>(s.data()));
	int nprbytes = 0;
	while (urlsafe_table[*(bufin++)] <= 63) {
		nprbytes++;
	}
	auto nbytesdecoded = ((nprbytes + 3) / 4) * 3;
	// create output buffer
	std::vector<uint8_t> out(nbytesdecoded, 0);
	auto* bufout = out.data();
	bufin = reinterpret_cast<uint8_t*>(const_cast<char*>(s.data()));

	// decode

	while (nprbytes > 4) {
		*(bufout++) = static_cast<unsigned char>(urlsafe_table[*bufin] << 2 |
							 urlsafe_table[bufin[1]] >> 4);
		*(bufout++) = static_cast<unsigned char>(urlsafe_table[bufin[1]] << 4 |
							 urlsafe_table[bufin[2]] >> 2);
		*(bufout++) = static_cast<unsigned char>(urlsafe_table[bufin[2]] << 6 |
							 urlsafe_table[bufin[3]]);
		bufin += 4;
		nprbytes -= 4;
	}

	/* Note: (nprbytes == 1) would be an error, so just ingore that case */
	if (nprbytes > 1) {
		*(bufout++) = static_cast<unsigned char>(urlsafe_table[*bufin] << 2 |
							 urlsafe_table[bufin[1]] >> 4);
	}
	if (nprbytes > 2) {
		*(bufout++) = static_cast<unsigned char>(urlsafe_table[bufin[1]] << 4 |
							 urlsafe_table[bufin[2]] >> 2);
	}
	if (nprbytes > 3) {
		*(bufout++) = static_cast<unsigned char>(urlsafe_table[bufin[2]] << 6 |
							 urlsafe_table[bufin[3]]);
	}

	return out;
}

auto dec(std::string_view s) -> std::vector<uint8_t> {
	// calculate decoded length
	auto* bufin = reinterpret_cast<uint8_t*>(const_cast<char*>(s.data()));
	int nprbytes = 0;
	while (pr2six[*(bufin++)] <= 63) {
		nprbytes++;
	}
	auto nbytesdecoded = ((nprbytes + 3) / 4) * 3;
	// create output buffer
	std::vector<uint8_t> out(nbytesdecoded, 0);
	auto* bufout = out.data();
	bufin = reinterpret_cast<uint8_t*>(const_cast<char*>(s.data()));

	// decode

	while (nprbytes > 4) {
		*(bufout++) = static_cast<unsigned char>(pr2six[*bufin] << 2 |
							 pr2six[bufin[1]] >> 4);
		*(bufout++) = static_cast<unsigned char>(pr2six[bufin[1]] << 4 |
							 pr2six[bufin[2]] >> 2);
		*(bufout++) = static_cast<unsigned char>(pr2six[bufin[2]] << 6 |
							 pr2six[bufin[3]]);
		bufin += 4;
		nprbytes -= 4;
	}

	/* Note: (nprbytes == 1) would be an error, so just ingore that case */
	if (nprbytes > 1) {
		*(bufout++) = static_cast<unsigned char>(pr2six[*bufin] << 2 |
							 pr2six[bufin[1]] >> 4);
	}
	if (nprbytes > 2) {
		*(bufout++) = static_cast<unsigned char>(pr2six[bufin[1]] << 4 |
							 pr2six[bufin[2]] >> 2);
	}
	if (nprbytes > 3) {
		*(bufout++) = static_cast<unsigned char>(pr2six[bufin[2]] << 6 |
							 pr2six[bufin[3]]);
	}

	return out;
}


///
// Convert from non-url-safe base64 to url-safe base64
/// See: https://en.wikipedia.org/wiki/Base64#Variants_summary_table
///
void to_urlsafe(std::string& s) {
	for (auto idx = 0ULL; idx < s.size(); ++idx) {
		switch (s[idx]) {
		case '+':
			s[idx] = '-';
			break;
		case '/':
		case ',':
			s[idx] = '_';
			break;
		default:
			continue;
		}
	}
}

///
/// Preprocess from url-safe base64 to standard RFC4864 format
/// See: https://en.wikipedia.org/wiki/Base64#Variants_summary_table
///
// void from_urlsafe(std::string& s) {
// 	for (auto idx = 0ULL; idx < s.size(); ++idx) {
// 		switch (s[idx]) {
// 		case '-':
// 			s[idx] = '+';
// 			break;
// 		case '_':
// 			s[idx] = '/';
// 			break;
// 		default:
// 			continue;
// 		}
// 	}
// }

///
/// Detects if base64-encoded string is in the "url-safe" format.
///
bool is_urlsafe(std::string_view s) {
	for (auto ch : s) {
		switch (ch) {
		case '-':
			return true;
		case '_':
			return true;
		default:
			continue;
		}
	}
	return false;
}

} // namespace b64
} // namespace ec_prv
