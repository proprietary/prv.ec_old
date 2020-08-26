#include "server/web.h"
#include "b64/b64.h"
#include "idl/all_generated_flatbuffers.h"
#include <algorithm>
#include <cstdio>
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

// valid base64 encoding characters with [62] and [63] as '-' and '_' to be url-safe
static const char BASE_64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_=";

// auto parse_shorturl(std::string_view url) -> std::string_view {
// 	auto it = url.begin();
// 	if (*(it++) != '/') {
// 		// unexpected
// 		assert(false);
// 		return {};
// 	}
// 	auto shorturl_start = it;
// 	while (it != url.end()) {
// 		// validate that all characters exist in base64 alphabet
// 		auto i = sizeof(BASE_64_CHARS);
// 		while (i > 0) {
// 			if (BASE_64_CHARS[i] == *it) {
// 				break;
// 			}
// 			i--;
// 		}
// 		if (i == 0) {
// 			// fail
// 			// character not found in base64 alphabet
// 			return {};
// 		}
// 		it++;
// 	}
// 	auto shorturl_end = it;
// 	return std::string_view(shorturl_start, shorturl_end);
// }

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

Server::Server(int const port) : port_{port}, store_{}, svc_{&store_} {}

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
			  res->onAborted([]() { uWS::Loop::get()->defer([]() {}); });
			  std::vector<uint8_t> buf;
			  res->onData([this, &buf, res](std::string_view chunk, bool is_end) {
				  std::copy(chunk.begin(), chunk.end(), std::back_inserter(buf));
				  if (is_end) {
					  res->cork([this, &buf, res]() -> void {
						  ::flatbuffers::Verifier v{buf.data(), buf.size()};
						  if (!::ec_prv::fbs::VerifyShorteningRequestBuffer(
							  v)) {
							  res->end();
							  return;
						  }
						  res->writeStatus("200 OK");
						  auto fb = ::ec_prv::fbs::UnPackShorteningRequest(
						      buf.data());
						  auto shortening_response =
						      this->svc_.handle_shortening_request(
							  std::move(fb));
						  res->end(std::string_view{
						      reinterpret_cast<char const*>(
							  shortening_response.data()),
						      shortening_response.size()});
					  });
				  }
			  });
		  })
	    .post(
		"/lookup_request",
		[this](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
	// clang-format off
#ifndef DEBUG
			  res->writeHeader("Access-Control-Allow-Origin", "https://prv.ec");
#endif
			  //clang-format on

			  std::vector<uint8_t> buf;
			  res->onData([this, &buf, res](std::string_view chunk, bool is_end) {
				  std::copy(chunk.begin(), chunk.end(), std::back_inserter(buf));
				  if (is_end) {
					  res->cork([this, &buf, res]() -> void {
						  ::flatbuffers::Verifier v{buf.data(), buf.size()};
						  if (!::ec_prv::fbs::VerifyLookupRequestBuffer(v)) {
							  res->end();
							  return;
						  }
						  res->writeStatus("200 OK");
						  auto fb = ::ec_prv::fbs::UnPackLookupRequest(buf.data());
						  auto lookup_response = this->svc_.handle_lookup_request(std::move(fb));
						  res->end(std::string_view{reinterpret_cast<char const*>(lookup_response.data()), lookup_response.size()});
					  });
				  }
			  });
		  })
	    .post("/trusted_shortening_request",
		[this](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
			// Authenticated endpoint for trusted clients performing all crypto
			// server-side, returning a 302 Redirect Authenticated with HTTP Basic
			// Auth
			res->writeHeader("Access-Control-Allow-Origin", "*");
			auto authorization = req->getHeader("authorization");
			auto [user, pass] = parse_http_basic_auth_header(authorization);
			if (user != "user" && pass != "dummy") {
				res->writeStatus("403 Forbidden");
				res->writeHeader("Content-Type", "text/plain");
				res->end();
				return;
			}
			std::vector<uint8_t> buf;
			res->onData([this, res, &buf](std::string_view chunk, bool is_end) -> void {
				std::copy(chunk.begin(), chunk.end(),
					  std::back_inserter(buf));
				if (is_end) {
					::flatbuffers::Verifier v{buf.data(), buf.size()};
					if (!::ec_prv::fbs::VerifyTrustedShorteningRequestBuffer(
						v)) {
						res->end();
						return;
					}
					::flatbuffers::FlatBufferBuilder fbb;
					auto fb = ::ec_prv::fbs::UnPackTrustedShorteningRequest(
					    buf.data());
					this->svc_.handle_trusted_shortening_request(fbb,
										     std::move(fb));
					res->write(std::string_view{reinterpret_cast<char const*>(fbb.GetBufferPointer()), fbb.GetSize()});
				}
			});
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
			std::vector<uint8_t> buf;
			res->onData([this, res, &buf](std::string_view chunk, bool is_end) -> void {
				std::copy(chunk.begin(), chunk.end(),
					  std::back_inserter(buf));
				if (is_end) {
					::flatbuffers::FlatBufferBuilder fbb;
					::flatbuffers::Verifier v{buf.data(), buf.size()};
					if (!::ec_prv::fbs::VerifyTrustedLookupRequestBuffer(v)) {
						res->end();
						return;
					}
					auto fb =
					    ::ec_prv::fbs::UnPackTrustedLookupRequest(buf.data());
					this->svc_.handle_trusted_lookup_request(fbb,
										 std::move(fb));
					res->write(std::string_view{
					    reinterpret_cast<char const*>(fbb.GetBufferPointer()),
					    fbb.GetSize()});
				}
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
