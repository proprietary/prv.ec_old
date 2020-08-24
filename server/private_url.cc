#include <b64/b64.h>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <private_url.h>
#include <utility>

namespace {

void handle_openssl_error() {
	auto ec = ERR_get_error();
	// ERR_error_string lets you put the error string in static space
	// instead of allocating.
	char const* err_string = ERR_error_string(ec, nullptr);
	std::fputs(err_string, stderr);
	throw std::runtime_error{err_string};
}

void fill_random(std::vector<uint8_t>& dst) {
	auto err = RAND_bytes(dst.get(), dst.size());
	if (!err) {
		handle_openssl_error();
	}
}

} // namespace

namespace ec_prv {
namespace private_url {

auto PrivateURL::encrypt(std::vector<uint8_t>& key, std::vector<uint8_t>& iv,
			 std::string const& plaintext) -> std::vector<uint8_t> {
	assert(key.size() == KEY_LEN);
	assert(iv.size() == IV_BYTES);
	std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx{EVP_CIPHER_CTX_new()};
	if (!ctx) {
		handle_openssl_error();
	}
	std::vector<uint8_t> out(plaintext.size() + KEY_LEN - 1, 0);
	auto err = EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, key.get(), iv.get());
	if (err != 1) {
		handle_openssl_error();
	}
	int len = 0, ciphertext_len = 0;
	err = EVP_EncryptUpdate(ctx.get(), out.data(), &len, plaintext.data(), plaintext.size());
	if (err != 1) {
		handle_openssl_error();
	}
	ciphertext_len += len;
	err = EVP_EncryptFinal_ex(ctx.get(), out.data() + len, &len);
	if (err != 1) {
		handle_openssl_error();
	}
	ciphertext_len += len;
	assert(out.size() <= ciphertext_len);
	return out;
}

auto PrivateURL::generate(std::string const& plaintext_url) -> std::tuple<PrivateURL, std::string> {
	auto pass = std::vector<uint8_t>(PASS_BYTES);
	auto salt = std::vector<uint8_t>(SALT_BYTES);
	auto iv = std::vector<uint8_t>(IV_BYTES);
	fill_random(pass);
	fill_random(salt);
	fill_random(iv);
	// derive secret key
	auto key = std::vector<uint8_t>(KEY_LEN);
	err = PKCS5_PBKDF2_HMAC(reinterpret_cast<char*>(pass.get()), pass.size(), salt.get(),
				salt.size(), PBKDF2, EVP_sha256(), key.size());
	if (!err) {
		handle_openssl_error();
	}
	// this is the secret key to be returned to the client and never stored on the server
	auto secret_key = ::ec_prv::b64::enc(key);
	// encrypt plaintext URL
	auto ciphertext = PrivateURL::encrypt(key, iv, plaintext_url);
	return std::make_tuple<>(PrivateURL{std::move(salt), std::move(iv), std::move(ciphertext)},
				 secret_key);
}

} // namespace private_url
} // namespace ec_prv
