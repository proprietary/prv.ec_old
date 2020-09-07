#include "cli_client/http_requests.h"
#include "cli_client/cacert.h"
#include <curl/curl.h>
#include <functional>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace {

auto sslctx_function(CURL* curl, void* sslctx, void* parm) -> CURLcode {
	CURLcode rv = CURLE_ABORTED_BY_CALLBACK;

	BIO* cbio = BIO_new_mem_buf(::ec_prv::cli_client::CACERT_PEM,
								sizeof(::ec_prv::cli_client::CACERT_PEM)/sizeof(::ec_prv::cli_client::CACERT_PEM[0]));
	X509_STORE* cts = SSL_CTX_get_cert_store((SSL_CTX*)sslctx);
	STACK_OF(X509_INFO) * inf;
	(void)curl;
	(void)parm;

	if (!cts || !cbio) {
		return rv;
	}

	inf = PEM_X509_INFO_read_bio(cbio, NULL, NULL, NULL);

	if (!inf) {
		BIO_free(cbio);
		return rv;
	}

	for (auto i = 0UL; i < sk_X509_INFO_num(inf); i++) {
		X509_INFO* itmp = sk_X509_INFO_value(inf, i);
		if (itmp->x509) {
			X509_STORE_add_cert(cts, itmp->x509);
		}
		if (itmp->crl) {
			X509_STORE_add_crl(cts, itmp->crl);
		}
	}

	sk_X509_INFO_pop_free(inf, X509_INFO_free);
	BIO_free(cbio);

	rv = CURLE_OK;
	return rv;
}

class cURLHeaders {
private:
	std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> headers_;
public:
	explicit cURLHeaders(char const* first_header) : headers_{curl_slist_append(nullptr, first_header), &curl_slist_free_all} {
	}

	void add(char const* header) noexcept {
		headers_.reset(curl_slist_append(headers_.release(), header));
	}

	[[nodiscard]] auto get() const noexcept -> curl_slist* {
		return headers_.get();
	}
};

struct cURLDeleter {
	void operator()(CURL* curl) noexcept {
		curl_easy_cleanup(curl);
	}
};

} // namespace

namespace ec_prv::cli_client {

auto request(std::vector<uint8_t>& dst, std::string endpoint, std::span<uint8_t> postdata) -> bool {
	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl (curl_easy_init(), curl_easy_cleanup);
	// don't use system CA bundle
	curl_easy_setopt(curl.get(), CURLOPT_CAPATH, nullptr);
	curl_easy_setopt(curl.get(), CURLOPT_CAINFO, nullptr);
	// use in-memory CA bundle stored as program text
	curl_easy_setopt(curl.get(), CURLOPT_SSL_CTX_FUNCTION, sslctx_function);
	// verify server
	curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 1L);

	curl_easy_setopt(curl.get(), CURLOPT_URL, endpoint.c_str());
	curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);

	struct write_cursor_t {
		std::span<uint8_t>* postdata;
		size_t bytes_left;
	};

	auto read_callback = +[](void* dest, size_t size, size_t nitems, void* userdata) -> size_t {
		auto* src = static_cast<write_cursor_t*>(userdata);
		auto realsize = size * nitems;
		auto n = std::max(0UL, std::min(src->bytes_left, realsize));
		auto offset = src->postdata->size() - src->bytes_left;
		memcpy(dest, src->postdata->data() + offset, n);
		src->bytes_left -= n;
		return n;
	};
	
	curl_easy_setopt(curl.get(), CURLOPT_READFUNCTION, read_callback);
	write_cursor_t write_cursor {.postdata = &postdata, .bytes_left = postdata.size()};
	curl_easy_setopt(curl.get(), CURLOPT_READDATA, &write_cursor);
	curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, write_cursor.bytes_left);

	// for debugging:
	// curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);

	// set headers
	cURLHeaders hdrs{"Content-Type: application/octet-stream"};
	curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, hdrs.get());

	auto write_callback = +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
		auto realsize = size * nmemb;
		auto* outbuf = static_cast<std::vector<uint8_t>*>(userdata);
		outbuf->insert(outbuf->end(), reinterpret_cast<uint8_t*>(ptr), reinterpret_cast<uint8_t*>(ptr + realsize));
		return realsize;
	};

	curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &dst);
	dst.clear();

	auto res = curl_easy_perform(curl.get());
	if (res != CURLE_OK) {
		fprintf(stderr, "cURL request in request() failed: %s\n", curl_easy_strerror(res));
		return false;
	}
	return true;
}


} // namespace ec_prv::cli_client
