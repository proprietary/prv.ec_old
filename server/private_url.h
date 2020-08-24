#ifndef __INCLUDE_EC_PRV_SERVER_PRIVATE_URL_H
#define __INCLUDE_EC_PRV_SERVER_PRIVATE_URL_H
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

namespace ec_prv {
namespace private_url {

class PrivateURL {
private:
	static constexpr uint32_t PBKDF2_ROUNDS = 1'000'000;
	static constexpr size_t PASS_BYTES = 8;
	static constexpr size_t SALT_BYTES = 8;
	static constexpr size_t IV_BYTES = 12;
	// PBKDF2 MAC function is SHA-256, which produces 32 byte keys
	static constexpr size_t KEY_LEN = 32;
	std::vector<uint8_t> salt_;
	std::vector<uint8_t> iv_;
	std::vector<uint8_t> blinded_url_;
	static auto encrypt(std::vector<uint8_t>& key, std::vector<uint8_t>& iv,
			    std::string const& plaintext) -> std::vector<uint8_t>;

public:
	explicit PrivateURL(std::vector<uint8_t>&& salt, std::vector<uint8_t>&& iv,
			    std::vector<uint8_t>&& blinded_url);

	///
	/// Generate cryptographic primitives completely server-side. This
	/// is not the intended use of this software, which is to do as
	/// much cryptography as possible on the client. This serves as a
	/// fallback for noscript user agents.
	///
	static auto generate(std::string const& plaintext_url)
	    -> std::tuple<PrivateURL, std::string>;

	///
	/// Checks that the cryptographic parameters are valid.
	///
	auto valid() const -> bool;
};

} // namespace private_url
} // namespace ec_prv
#endif // __INCLUDE_EC_PRV_SERVER_PRIVATE_URL_H
