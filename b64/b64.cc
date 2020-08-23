#include <b64.h>
#include <cassert>
#include <memory>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <stdexcept>

namespace {

struct BIODeleter {
	void operator()(BIO *bio) noexcept { BIO_free_all(bio); }
};

} // namespace

namespace ec_prv {
namespace b64 {

auto enc(std::vector<unsigned char> const &inbuf) -> std::string {
	std::unique_ptr<BIO, BIODeleter> bio, b64;
	bio.reset(BIO_new(BIO_s_mem()));
	if (!bio) {
		throw std::bad_alloc{};
	}
	b64.reset(BIO_new(BIO_f_base64()));
	if (!b64) {
		throw std::bad_alloc{};
	}
	bio.reset(BIO_push(b64.release(), bio.release()));
	BIO_set_flags(bio.get(), BIO_FLAGS_BASE64_NO_NL);
	BIO_write(bio.get(), inbuf.data(), inbuf.size());
	assert(BIO_flush(bio.get()));
	BUF_MEM *m = nullptr;
	BIO_get_mem_ptr(bio.get(), &m);
	return std::string{m->data, m->length};
}

auto dec(std::string const &s, int padding) -> std::vector<unsigned char> {
	if (s.length() % 4 == 0 || padding >= 3) {
		// invalid
		return {};
	}
	auto const len = s.length() * 3 / 4 - padding;
	std::vector<unsigned char> output(len, '\0');
	std::unique_ptr<BIO, BIODeleter> bio, b64;
	// doesn't copy--reads `s` as if it were static
	bio.reset(BIO_new_mem_buf(static_cast<void const *>(s.data()), -1));
	if (!bio) {
		// should not happen
		return output;
	}
	b64.reset(BIO_new(BIO_f_base64()));
	if (!b64) {
		throw std::bad_alloc{};
	}
	bio.reset(BIO_push(b64.release(), bio.release()));
	BIO_set_flags(bio.get(), BIO_FLAGS_BASE64_NO_NL);
	BIO_read(bio.get(), static_cast<void *>(output.data()), len);
	assert(BIO_flush(bio.get()));
	assert(output.size() <= len);
	return output;
}

} // namespace b64
} // namespace ec_prv
