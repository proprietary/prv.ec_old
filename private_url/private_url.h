#ifndef __INCLUDE_EC_PRV_PRIVATE_URL_PRIVATE_URL_H
#define __INCLUDE_EC_PRV_PRIVATE_URL_PRIVATE_URL_H
#include "private_url/crypto_params.h"
#include <cstdint>
#include <optional>
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
	CryptoParams crypto_params_ = crypto_params_v1;

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
	/// Generates cryptographic primitives completely server-side. This
	/// is not the intended use of this software, which is to do as
	/// much cryptography as possible on the client. This serves as a
	/// fallback for noscript user agents.
	///
	[[nodiscard]] static auto generate(std::string const& plaintext_url) noexcept
	    -> std::optional<std::tuple<PrivateURL, std::string>>;

	///
	/// Gets underlying URL using the secret pass, which is a byte
	/// array. Base66-encoded strings need to be converted to byte
	/// arrays first.
	///
	[[nodiscard]] auto get_plaintext(std::vector<uint8_t>&& pass) noexcept -> std::string;

	///
	/// Gets the underlying, original URL using the secret password,
	/// which is here a base-66 encoded string.
	///
	[[nodiscard]] auto get_plaintext(std::string_view pass) noexcept -> std::string;

	///
	/// Checks that the cryptographic parameters are valid.
	///
	[[nodiscard]] auto valid() const -> bool;

	[[nodiscard]] auto salt() const -> std::vector<uint8_t> const&;

	[[nodiscard]] auto iv() const -> std::vector<uint8_t> const&;

	[[nodiscard]] auto blinded_url() const -> std::vector<uint8_t> const&;

	///
	/// Change the number of PBKDF2 rounds.
	///
	void mutate_pbkdf2_rounds(size_t pbkdf2_rounds);
};

} // namespace private_url
} // namespace ec_prv
#endif // __INCLUDE_EC_PRV_PRIVATE_URL_PRIVATE_URL_H
