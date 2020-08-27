#include "server/web.h"
#include "b64/b64.h"
#include "idl/all_generated_flatbuffers.h"
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
