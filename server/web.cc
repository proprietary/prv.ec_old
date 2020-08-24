#include "server/web.h"
#include "uWebSockets/HttpContext.h"
#include "uWebSockets/HttpParser.h"
#include "uWebSockets/HttpResponse.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace ec_prv {
namespace web {

void rpc_dispatch(std::string_view inbuf) {
	// TODO
	return;
}

// void handle_lookup_request(std::string_view inbuf) {
// 	// uint8_t const* const buf = reinterpret_cast<uint8_t const*>(inbuf.data());
// 	// auto const sz = inbuf.size();
// 	// bool ok = ::ec_prv::VerifyURLRecordBuffer(::flatbuffers::Verifier(buf, sz));
// 	// if (!ok) {
// 	// 	// bad input
// 	// 	return;
// 	// }
// 	// auto* ur = ::ec_prv::GetURLRecord(buf);
// 	// switch (ur->version()) {
// 	// case 1: {
// 	// }
// 	// default:
// 	// 	// bad input
// 	// 	return;
// 	// }
// }

} // namespace web
} // namespace ec_prv

namespace {

// valid base64 encoding characters with [62] and [63] as '-' and '_' to be url-safe
static const char BASE_64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_=";

auto parse_shorturl(std::string_view url) -> std::string_view {
	auto it = url.begin();
	if (*(it++) != '/') {
		// unexpected
		assert(false);
		return {};
	}
	auto shorturl_start = it;
	while (it != url.end()) {
		// validate that all characters exist in base64 alphabet
		auto i = sizeof(BASE_64_CHARS);
		while (i > 0) {
			if (BASE_64_CHARS[i] == *it) {
				break;
			}
			i--;
		}
		if (i == 0) {
			// fail
			// character not found in base64 alphabet
			return {};
		}
		it++;
	}
	auto shorturl_end = it;
	return std::string_view(shorturl_start, shorturl_end);
}

} // namespace

int main(int argc, char** argv) {
	using namespace std::literals;
	uWS::App()
	    .get("/",
		 [](auto* res, auto* req) {
			 res->end("<!doctype html><html><body><h1>prv.ec</h1></body></html>");
		 })
	    .post("/shortening_request",
		  [](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
			  // Request a new shortened URL
			  res->onAborted([]() { uWS::Loop::get()->defer([]() {}); });
			  res->onData([res](std::string_view chunk, bool is_end) {
				  if (is_end) {
					  res->cork([res, chunk]() {
						  // ::ec_prv::web::rpc_dispatch(std::span<uint8_t>(reinterpret_cast<uint8_t*>(const_cast<char*>(chunk.data())),
						  // chunk.size()));
						  res->write("something ahead");
						  res->end(chunk);
					  });
				  } else {
					  res->write(chunk);
				  }
			  });
		  })
	    .post("/lookup_request",
		  [](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
			  // TODO: lookup rpc
			  res->end("TODO");
		  })
	    .post("/trusted_shortening_request",
		  [](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
			  // Authenticated endpoint for trusted clients performing all crypto
			  // server-side, returning a 302 Redirect Authenticated with HTTP Basic
			  // Auth
			  // TODO: HTTP basic auth
			  res->end("TODO");
		  })
	    .get("/*",
		 [](auto* res, uWS::HttpRequest* req) {
			 // Route shortened URLs
			 // TODO
			 auto url = req->getUrl();
			 std::cout << url << std::endl;
			 auto lookup_key = parse_shorturl(url);
			 std::cout << lookup_key << std::endl;
			 res->end(lookup_key);
		 })
	    .listen(8000,
		    [](auto* token) {
			    if (token) {
				    std::printf("Listening on port %d\n", 8000);
			    }
		    })
	    .run();
	return 0;
}
