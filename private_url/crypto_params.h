#ifndef _INCLUDE_EC_PRV_PRIVATE_URL_CRYPTO_PARAMS_H
#define _INCLUDE_EC_PRV_PRIVATE_URL_CRYPTO_PARAMS_H
#include <cstdint>
#include <string>

namespace ec_prv {
namespace private_url {

struct CryptoParams {
	// quoth the PBKDF2 standard: "For especiall	y critical keys, or for
	// very powerful systems or systems where user-perceived
	// performance is not critical, an iteration count of 10,000,000
	// may be appropriate."
	// https://tools.ietf.org/html/rfc8018#section-4
	size_t pbkdf2_rounds;
	// yes, seriously, just 8^3 bits of entropy; that's why `pbdkf2_rounds` is so high
	size_t pass_bytes;
	// PBKDF2 MAC function is SHA-256, which produces 32 byte keys
	size_t key_len;
	// default IV size for AES-GCM is 96 bits
	size_t iv_bytes;
	// PBKDF2 standard recommends at lea	st 8 bytes for the salt
	// https://tools.ietf.org/html/rfc8018#section-4
	size_t salt_bytes;
	// additional data for AES-GCM; should pretty much never be changed
	std::string_view aad_data = "www.prv.ec";
	// default tag size for AES-GCM is 128-bits
	size_t tag_bytes;
	// EVP_CIPHER_block_size(EVP_aes_256_gcm());
	static constexpr size_t AES_GCM_BLOCK_SIZE = 1;
};

static constexpr CryptoParams crypto_params_v1{
    .pbkdf2_rounds = 10'000'000,
    .pass_bytes = 3,
    .key_len = 32,
    .iv_bytes = 12,
    .salt_bytes = 32,
    .aad_data = "www.prv.ec",
    .tag_bytes = 16,
};

static constexpr size_t MIN_ACCEPTABLE_PBKDF2_ITERS = 2'000'000;

} // namespace private_url
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_PRIVATE_URL_CRYPTO_PARAMS_H
