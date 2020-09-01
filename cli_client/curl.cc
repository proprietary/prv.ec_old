#include <stdio.h>
#include <curl/curl.h>
#include "cli_client/cacert.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

 
static CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm)
{
  CURLcode rv = CURLE_ABORTED_BY_CALLBACK;
 
  BIO *cbio = BIO_new_mem_buf(::ec_prv::cli_client::CACERT_PEM, sizeof(::ec_prv::cli_client::CACERT_PEM));
  X509_STORE  *cts = SSL_CTX_get_cert_store((SSL_CTX *)sslctx);
  STACK_OF(X509_INFO) *inf;
  (void)curl;
  (void)parm;
 
  if(!cts || !cbio) {
    return rv;
  }
 
  inf = PEM_X509_INFO_read_bio(cbio, NULL, NULL, NULL);
 
  if(!inf) {
    BIO_free(cbio);
    return rv;
  }
 
  for(auto i = 0UL; i < sk_X509_INFO_num(inf); i++) {
    X509_INFO *itmp = sk_X509_INFO_value(inf, i);
    if(itmp->x509) {
      X509_STORE_add_cert(cts, itmp->x509);
    }
    if(itmp->crl) {
      X509_STORE_add_crl(cts, itmp->crl);
    }
  }
 
  sk_X509_INFO_pop_free(inf, X509_INFO_free);
  BIO_free(cbio);
 
  rv = CURLE_OK;
  return rv;
}


int main(void)
{
  CURL *curl;
  CURLcode res;
 
  curl_global_init(CURL_GLOBAL_DEFAULT);
 
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/");
 
#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
 
#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

	curl_easy_setopt(curl, CURLOPT_CAPATH, nullptr);
	curl_easy_setopt(curl, CURLOPT_CAINFO, nullptr);
	curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
 
  curl_global_cleanup();
 
  return 0;
}
