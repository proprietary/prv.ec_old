#include "web.h"
#include "b64.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <uWebSockets/HttpContext.h>
#include <uWebSockets/HttpParser.h>
#include <uWebSockets/HttpResponse.h>
#include <utility>
#include <vector>

using namespace std::literals;

namespace {

static const char BASE_64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_=";

auto parse_auth_header(std::string_view authorization_header,
		       std::string_view strip_prefix = "Basic "sv) -> std::string {
	using namespace std::literals;

	if (authorization_header.size() < strip_prefix.size()) {
		return {};
	}
	auto const* a = authorization_header.begin() + strip_prefix.size();
	std::string_view token{a, authorization_header.end()};
	auto decoded = ::ec_prv::b64::dec(token);
	return std::string(reinterpret_cast<char const*>(decoded.data()), decoded.size());
}

auto parse_http_basic_auth_header(std::string_view authorization_header)
    -> std::pair<std::string, std::string> {
	auto token = parse_auth_header(authorization_header, "Basic "sv);
	if (token.length() == 0) {
		return {};
	}
	auto idx = token.find(':');
	if (idx == std::string::npos) {
		return {};
	}
	auto user = token.substr(0, idx);
	if (idx + 1 > token.size()) {
		return {};
	}
	auto pass = token.substr(idx + 1);
	return std::make_pair(user, pass);
}

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

auto pack_bytes_u32le(std::span<uint8_t> bytes) -> uint32_t {
	if (bytes.size() < 4) {
		return 0;
	}
	return (bytes[0] & 0xff) | (bytes[1] << 0x8) | (bytes[2] << 0x10) | (bytes[3] << 0x18);
}

} // namespace

namespace ec_prv {
namespace web {

Server::Server(int const port)
    : port_{port}, store_{}, svc_{&store_}, rpc_user_{::getenv("EC_PRV_RPC_USER")},
      rpc_pass_{::getenv("EC_PRV_RPC_PASS")} {
	if (rpc_user_.length() == 0) {
		throw std::runtime_error{"EC_PRV_RPC_USER environment variable not set"};
	}
	if (rpc_pass_.length() == 0) {
		throw std::runtime_error{"EC_PRV_RPC_PASS environment variable not set"};
	}
}

void Server::run() {
	using namespace std::literals;
	uWS::App()
	    .get("/",
		 [](auto* res, auto* req) {
			 res->end("<!doctype html><html><body><h1>prv.ec</h1></body></html>");
		 })
	    .post("/shortening_request",
		  [this](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
	// clang-format off
#ifndef DEBUG
			  res->writeHeader("Access-Control-Allow-Origin", "https://prv.ec");
#endif
			  // clang-format on

			  // Request a new shortened URL
			  accept_rpc<::ec_prv::fbs::ShorteningRequest, false>(res, req);
		  })
	    .post("/lookup_request",
		  [this](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
	// clang-format off
#ifndef DEBUG
			  res->writeHeader("Access-Control-Allow-Origin", "https://prv.ec");
#endif
			  //clang-format on

			  accept_rpc<::ec_prv::fbs::LookupRequest, false>(res, req);
		  })
	    .post("/trusted_shortening_request",
		[this](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
			// Authenticated endpoint for trusted clients performing all crypto
			// server-side.
			res->writeHeader("Access-Control-Allow-Origin", "*");
			auto authorization = req->getHeader("authorization");
			auto [user, pass] = parse_http_basic_auth_header(authorization);
			if (user != this->rpc_user_ && pass != this->rpc_pass_) {
				res->writeStatus("403 Forbidden");
				res->writeHeader("Content-Type", "text/plain");
				res->end();
				return;
			}
			this->accept_rpc<::ec_prv::fbs::TrustedShorteningRequest, false>(res, req);
		})
	    .post(
		"/trusted_lookup_request",
		[this](auto* res, auto* req) {
			res->writeHeader("Access-Control-Allow-Origin", "*");
			auto authorization = req->getHeader("authorization");
			auto [user, pass] = parse_http_basic_auth_header(authorization);
			if (user != "user" && pass != "dummy") {
				res->writeStatus("403 Forbidden");
				res->writeHeader("Content-Type", "text/plain");
				res->end();
				return;
			}
			this->accept_rpc<::ec_prv::fbs::TrustedLookupRequest, false>(res, req);
		})
		.get("/*", [this](auto* res, auto* req) -> void {
			auto url = req->getUrl();
			auto identifier = parse_shorturl(url);
			auto identifier_as_bytes = b64::dec(identifier);
			if (identifier_as_bytes.size() > 4) {
				res->cork([res]() -> void {
					res->writeStatus("302 Found");
					res->writeHeader("location", "https://prv.ec");
					res->end();
				});
				return;
			}
			uint32_t identifier_parsed = pack_bytes_u32le(identifier_as_bytes);
			if (identifier_parsed == 0) {
				res->cork([res]() -> void {
					res->writeStatus("302 Found");
					res->writeHeader("location", "https://prv.ec");
					res->end();
				});
				return;
			}
			::flatbuffers::FlatBufferBuilder ui_fbb;
			auto uioffset = ::ec_prv::fbs::CreateURLIndex(ui_fbb, 1, identifier_parsed);
			ui_fbb.Finish(uioffset);
			// lookup in rocksdb
			std::string o;
			this->store_.get(o, std::span<uint8_t>{ui_fbb.GetBufferPointer(), ui_fbb.GetSize()});
			if (o.length() == 0) {
				res->cork([res]() -> void {
					res->writeStatus("302 Found");
					res->writeHeader("location", "https://prv.ec");
					res->end();
				});
				return;
			}
			::flatbuffers::FlatBufferBuilder resp_fbb;
			::ec_prv::fbs::LookupResponseBuilder lrb{resp_fbb};
			lrb.add_version(1);
			lrb.add_error(false);
			lrb.add_data(resp_fbb.CreateVector(reinterpret_cast<uint8_t const*>(o.data()), o.size()));
			resp_fbb.Finish(lrb.Finish());
			auto const encoded = ::ec_prv::b64::enc(std::span{reinterpret_cast<uint8_t*>(o.data()), o.size()});
			res->cork([res, &encoded]() -> void {
				res->writeStatus("200 OK");
				res->writeHeader("content-type", "text/html");
				res->write(R"(<!doctype html><html><body><script>var a=")");
				res->write(encoded);
				// TODO(zds): write template abstraction
				res->write(R"("; window.addEventListener('DOMContentLoaded', function() { window.location.replace(decryptLookupResponse(a)); });</script></body></html>)");
				res->end();
			});
		})
	    .listen(8000,
		    [](auto* token) {
			    if (token) {
				    std::printf("Listening on port %d\n", 8000);
			    }
		    })
	    .run();
}


} // namespace web
} // namespace ec_prv
