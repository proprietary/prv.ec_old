#include "private_url/private_url.h"
#include "b64/b64.h"
#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <limits>
#include <memory>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <type_traits>
#include <utility>

namespace {

// void handle_openssl_error() {
// 	auto ec = ERR_get_error();
// 	// ERR_error_string lets you put the error string in static space
// 	// instead of allocating.
// 	char const* err_string = ERR_error_string(ec, nullptr);
// 	std::fputs(err_string, stderr);
// 	throw std::runtime_error{err_string};
// }

void handle_openssl_error_nothrow() noexcept {
	auto ec = ERR_get_error();
	// // ERR_error_string lets you put the error string in static space
	// instead of allocating.
	// char const* err_string = ERR_error_string(ec, nullptr);
	constexpr auto bufsz = 256UL;
	char* buf = new char[bufsz];
	char const* err_string = ERR_error_string(ec, buf);
	std::fputs(err_string, stderr);
	delete[] buf;
	// TODO: assert(false); (?)
}

void fill_random(std::vector<uint8_t>& v) noexcept {
	auto err = RAND_bytes(v.data(), v.size());
	if (err != 1) {
		handle_openssl_error_nothrow();
		assert(false);
	}
}

} // namespace

namespace ec_prv {
namespace private_url {

PrivateURL::PrivateURL(std::vector<uint8_t>&& salt, std::vector<uint8_t>&& iv,
		       std::vector<uint8_t>&& blinded_url)
    : salt_{std::move(salt)}, iv_{std::move(iv)}, blinded_url_{std::move(blinded_url)},
      crypto_params_{crypto_params_v1} {
	assert(crypto_params_.AES_GCM_BLOCK_SIZE == EVP_CIPHER_block_size(EVP_aes_256_gcm()));
}

auto PrivateURL::valid() const -> bool {
	return !blinded_url_.empty() && iv_.size() == crypto_params_.iv_bytes &&
	       salt_.size() == crypto_params_.salt_bytes;
}

auto PrivateURL::encrypt(std::vector<uint8_t>&& key, std::vector<uint8_t>& iv,
			 std::string const& plaintext) noexcept
    -> std::optional<std::vector<uint8_t>> {
	assert(key.size() == crypto_params_v1.key_len);
	assert(iv.size() == crypto_params_v1.iv_bytes);
	std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx{EVP_CIPHER_CTX_new(),
									    EVP_CIPHER_CTX_free};
	if (!ctx) {
		handle_openssl_error_nothrow();
		return {};
	}
	// No need to set IV length because it is the default for AES-GCM: 96-bits
	static_assert(crypto_params_v1.iv_bytes == 12);
	// No need to set tag length because it is already configured to the default for AES-GCM:
	// 128-bits
	static_assert(crypto_params_v1.tag_bytes == 16);
	auto const out_sz = plaintext.size() + CryptoParams::AES_GCM_BLOCK_SIZE - 1;
	std::vector<uint8_t> out(out_sz, 0);
	auto err = EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, key.data(), iv.data());
	if (err != 1) {
		handle_openssl_error_nothrow();
		return {};
	}
	int len = 0; // outvar for written length; changes after every EVP_* call that uses it
		     // insert AAD data (well-known plaintext)
	err = EVP_EncryptUpdate(ctx.get(), nullptr, &len,
				reinterpret_cast<uint8_t const*>(crypto_params_v1.aad_data.data()),
				crypto_params_v1.aad_data.size());
	if (err != 1) {
		handle_openssl_error_nothrow();
		return {};
	}
	assert(len == crypto_params_v1.aad_data.size());
	err =
	    EVP_EncryptUpdate(ctx.get(), out.data(), &len,
			      reinterpret_cast<uint8_t const*>(plaintext.data()), plaintext.size());
	if (err != 1) {
		handle_openssl_error_nothrow();
		return {};
	}
	int ciphertext_len = len;
	err = EVP_EncryptFinal_ex(ctx.get(), out.data() + ciphertext_len, &len);
	if (err != 1) {
		handle_openssl_error_nothrow();
		return {};
	}
	ciphertext_len += len;
	if (out.size() >= std::numeric_limits<decltype(ciphertext_len)>::max()) {
		assert(false);
		return {};
	}
	assert(static_cast<decltype(ciphertext_len)>(out.size()) >= ciphertext_len);
	out.resize(ciphertext_len);
	return out;
}

auto PrivateURL::derive_secret_key(std::vector<uint8_t>&& pass, std::vector<uint8_t>& salt) noexcept
    -> std::optional<std::vector<uint8_t>> {
	assert(pass.size() == crypto_params_v1.pass_bytes);
	assert(salt.size() == crypto_params_v1.salt_bytes);
	std::vector<uint8_t> out(crypto_params_v1.key_len);
	auto err = PKCS5_PBKDF2_HMAC(reinterpret_cast<char*>(pass.data()), pass.size(), salt.data(),
				     salt.size(), crypto_params_v1.pbkdf2_rounds, EVP_sha256(),
				     out.size(), out.data());
	if (err == 0) {
		handle_openssl_error_nothrow();
		return {};
	}
	return out;
}

auto PrivateURL::decrypt(std::vector<uint8_t>&& key, std::vector<uint8_t>& ciphertext,
			 std::vector<uint8_t>& iv) noexcept -> std::optional<std::vector<uint8_t>> {
	assert(static_cast<decltype(crypto_params_v1.key_len)>(key.size()) ==
	       crypto_params_v1.key_len);
	assert(iv.size() == crypto_params_v1.iv_bytes);

	std::vector<uint8_t> out(ciphertext.size() + CryptoParams::AES_GCM_BLOCK_SIZE, 0);
	std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx{EVP_CIPHER_CTX_new(),
									    EVP_CIPHER_CTX_free};
	if (!ctx) {
		handle_openssl_error_nothrow();
	}
	auto err = EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, key.data(), iv.data());
	if (err != 1) {
		handle_openssl_error_nothrow();
		return {};
	}
	// No need to set IV length because it is already configured to the default for AES-GCM: 96
	// bits
	static_assert(crypto_params_v1.iv_bytes == 12);
	// No need to set tag length because it is already configured to the default for AES-GCM:
	// 128 bits
	static_assert(crypto_params_v1.tag_bytes == 16);
	// outvar for written length; changes after every EVP_* call that uses it
	int len = 0;
	// provide AAD data
	err = EVP_DecryptUpdate(ctx.get(), nullptr, &len,
				reinterpret_cast<uint8_t const*>(crypto_params_v1.aad_data.data()),
				crypto_params_v1.aad_data.size());
	if (err != 1) {
		handle_openssl_error_nothrow();
		return {};
	}
	assert(len == crypto_params_v1.aad_data.size());
	int plaintext_len = 0;
	// process block cipher
	{
		constexpr auto chunk_size = 2000UL;
		for (auto ciphertext_bytes_left = ciphertext.size(); ciphertext_bytes_left > 0ULL;
		     ciphertext_bytes_left -= len) {
			auto const bite = std::min(chunk_size, ciphertext_bytes_left);
			if (plaintext_len + bite > out.size()) {
				out.resize(plaintext_len + bite);
			}
			err = EVP_DecryptUpdate(ctx.get(), out.data() + plaintext_len, &len,
						ciphertext.data(), bite);
			if (err != 1) {
				handle_openssl_error_nothrow();
				return {};
			}
			if (len == 0) {
				assert(false);
				return {};
			}
			plaintext_len += len;
		}
	}
	if (plaintext_len + crypto_params_v1.key_len > out.size()) {
		out.resize(plaintext_len + crypto_params_v1.key_len);
	}
	(void)EVP_DecryptFinal_ex(ctx.get(), out.data() + plaintext_len, &len);
	plaintext_len += len;
	if (out.size() >= std::numeric_limits<decltype(plaintext_len)>::max()) {
		assert(false);
		return {};
	}
	assert(static_cast<decltype(plaintext_len)>(out.size()) >= plaintext_len);
	return out;
}

auto PrivateURL::decrypt_using_pass(std::vector<uint8_t>&& pass, std::vector<uint8_t>& salt,
				    std::vector<uint8_t>& ciphertext,
				    std::vector<uint8_t>& iv) noexcept
    -> std::optional<std::vector<uint8_t>> {
	assert(pass.size() == crypto_params_v1.pass_bytes);
	assert(salt.size() == crypto_params_v1.salt_bytes);
	assert(iv.size() == crypto_params_v1.iv_bytes);
	auto secret_key = PrivateURL::derive_secret_key(std::move(pass), salt);
	if (!secret_key) {
		return {};
	}
	auto plaintext = PrivateURL::decrypt(std::move(secret_key.value()), ciphertext, iv);
	if (!plaintext) {
		return {};
	}
	return std::move(plaintext.value());
}

auto PrivateURL::get_plaintext(std::vector<uint8_t>&& pass) noexcept -> std::string {
	auto plaintext_bytes =
	    PrivateURL::decrypt_using_pass(std::move(pass), salt_, blinded_url_, iv_);
	if (!plaintext_bytes) {
		return {};
	}
	return std::string(plaintext_bytes.value().data(),
			   plaintext_bytes.value().data() + plaintext_bytes.value().size());
}

auto PrivateURL::get_plaintext(std::string_view pass) noexcept -> std::string {
	auto pass_bytes = ::ec_prv::b64::dec(pass);
	return get_plaintext(std::move(pass_bytes));
}

auto PrivateURL::generate(std::string const& plaintext_url) noexcept
    -> std::optional<std::tuple<PrivateURL, std::string>> {
	auto pass = std::vector<uint8_t>(crypto_params_v1.pass_bytes, 0);
	auto salt = std::vector<uint8_t>(crypto_params_v1.salt_bytes, 0);
	auto iv = std::vector<uint8_t>(crypto_params_v1.iv_bytes, 0);
	::fill_random(pass);
	::fill_random(salt);
	::fill_random(iv);
	// derive secret key
	auto key = derive_secret_key(std::move(pass), salt);
	if (!key) {
		return {};
	}
	// this is the secret key to be returned to the client and never stored on the server
	auto b64_pass = ::ec_prv::b64::enc_urlsafe(pass);
	// encrypt plaintext URL
	auto ciphertext = PrivateURL::encrypt(std::move(key.value()), iv, plaintext_url);
	return std::make_tuple<PrivateURL, std::string>(
	    PrivateURL(std::move(salt), std::move(iv), std::move(ciphertext.value())),
	    std::move(b64_pass));
}

auto PrivateURL::iv() const -> std::vector<uint8_t> const& { return iv_; }

auto PrivateURL::salt() const -> std::vector<uint8_t> const& { return salt_; }

auto PrivateURL::blinded_url() const -> std::vector<uint8_t> const& { return blinded_url_; }

void PrivateURL::mutate_pbkdf2_rounds(size_t pbkdf2_rounds) {
	crypto_params_.pbkdf2_rounds = pbkdf2_rounds;
}

} // namespace private_url
} // namespace ec_prv
