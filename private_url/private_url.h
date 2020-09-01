#ifndef __INCLUDE_EC_PRV_PRIVATE_URL_PRIVATE_URL_H
#define __INCLUDE_EC_PRV_PRIVATE_URL_PRIVATE_URL_H
#include <optional>
#include <stdint.h>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace ec_prv {
namespace private_url {

class PrivateURL {
private:
	std::vector<uint8_t> salt_;
	std::vector<uint8_t> iv_;
	std::vector<uint8_t> blinded_url_;

	//
	// crypto parameters
	//

	// quoth the PBKDF2 standard: "For especially critical keys, or for
	// very powerful systems or systems where user-perceived
	// performance is not critical, an iteration count of 10,000,000
	// may be appropriate."
	// https://tools.ietf.org/html/rfc8018#section-4
	static constexpr uint32_t PBKDF2_ROUNDS = 2'000'000; // yes, seriously

	// yes, seriously, just 24 bits of entropy; that's why PBDKF2_ROUNDS is high
	static constexpr size_t PASS_BYTES = 3;

	// PBKDF2 standard recommends at least 8 bytes for the salt
	// https://tools.ietf.org/html/rfc8018#section-4
	static constexpr size_t SALT_BYTES = 32;

	// PBKDF2 MAC function is SHA-256, which produces 32 byte keys
	static constexpr size_t KEY_LEN = 32;

	// default IV size for AES-GCM is 96 bits
	static constexpr size_t IV_BYTES = 12;

	static constexpr std::string_view AAD_DATA = "www.prv.ec";

	// default tag size for AES-GCM is 128-bits
	static constexpr size_t TAG_BYTES = 16;

	static constexpr size_t AES_GCM_BLOCK_SIZE = 1; // EVP_CIPHER_block_size(EVP_aes_256_gcm());

	///
	/// Encrypt plaintext with AES-256-GCM. Crypto parameters are
	/// static members of PrivateURL class and must be the same as
	/// those on the client.
	///
	static auto encrypt(std::vector<uint8_t>&& key, std::vector<uint8_t>& iv,
			    std::string const& plaintext) noexcept
	    -> std::optional<std::vector<uint8_t>>;

	///
	/// Derive key with PBKDF2-HMAC-SHA256. Crypto parameters are
	/// static members of PrivateURL class and must be the same as
	/// those on the client.
	///
	static auto derive_secret_key(std::vector<uint8_t>&& pass,
				      std::vector<uint8_t>& salt) noexcept
	    -> std::optional<std::vector<uint8_t>>;

	///
	/// Decrypt ciphertext encrypted with AES-256-GCM.
	///
	static auto decrypt(std::vector<uint8_t>&& key, std::vector<uint8_t>& ciphertext,
			    std::vector<uint8_t>& iv) noexcept
	    -> std::optional<std::vector<uint8_t>>;

	///
	/// Perform key derivation and decryption in one step as a shortcut.
	///
	static auto decrypt_using_pass(std::vector<uint8_t>&& pass, std::vector<uint8_t>& salt,
				       std::vector<uint8_t>& ciphertext,
				       std::vector<uint8_t>& iv) noexcept
	    -> std::optional<std::vector<uint8_t>>;

public:
	explicit PrivateURL(std::vector<uint8_t>&& salt, std::vector<uint8_t>&& iv,
			    std::vector<uint8_t>&& blinded_url);

	///
	/// Generate cryptographic primitives completely server-side. This
	/// is not the intended use of this software, which is to do as
	/// much cryptography as possible on the client. This serves as a
	/// fallback for noscript user agents.
	///
	static auto generate(std::string const& plaintext_url) noexcept
	    -> std::optional<std::tuple<PrivateURL, std::string>>;

	///
	/// Get underlying URL using the secret pass, which is a byte
	/// array. Base64-encoded strings need to be converted to byte
	/// arrays first.
	///
	auto get_plaintext(std::vector<uint8_t>&& pass) noexcept -> std::string;

	///
	/// Checks that the cryptographic parameters are valid.
	///
	auto valid() const -> bool;

	auto salt() const -> std::vector<uint8_t> const&;

	auto iv() const -> std::vector<uint8_t> const&;

	auto blinded_url() const -> std::vector<uint8_t> const&;
};

} // namespace private_url
} // namespace ec_prv
#endif // __INCLUDE_EC_PRV_PRIVATE_URL_PRIVATE_URL_H
