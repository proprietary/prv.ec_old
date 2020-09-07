#include "cli_client/shortening_client.h"
#include "cli_client/http_requests.h"

#include "b66/b66.h"
#include "idl/all_generated_flatbuffers.h"
#include "private_url/private_url.h"
#include "url_index/url_index.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <flatbuffers/flatbuffers.h>
#include <iostream>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using namespace std::string_view_literals;

using namespace std::chrono_literals;

constexpr auto MAX_AGE = 8759h;

auto default_expiry() -> uint64_t {
	auto n = std::chrono::system_clock::now() + MAX_AGE;
	return std::chrono::duration_cast<std::chrono::seconds>(n.time_since_epoch()).count();
}

auto current_unix_timestamp() -> uint64_t {
	auto const n = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::seconds>(n.time_since_epoch()).count();
}

} // namespace

namespace ec_prv {
namespace shortening_client {

auto parse_shortened_url(std::string_view full_url, std::string_view upstream_server)
    -> std::pair<std::string, std::string> {
	auto* it = full_url.begin();
	for (auto* us = upstream_server.begin();
	     it != full_url.end() && us != upstream_server.end(); ++it, ++us) {
		if (*us != *it) {
			return {};
		}
	}
	if (it == full_url.end()) {
		return {};
	}
	if (it++; it != full_url.end() && *it != '/') {
		return {};
	}
	if (++it == full_url.end()) {
		return {};
	}
	auto identifier_start = it;
	auto pass_start = std::find(identifier_start, full_url.end(), '#');
	if (pass_start == full_url.end()) {
		return {};
	}
	std::string identifier;
	identifier.assign(full_url.begin(), pass_start);
	if (++pass_start == full_url.end()) {
		return {};
	}
	std::string pass;
	pass.assign(pass_start, full_url.end());
	return {std::move(identifier), std::move(pass)};
}

namespace v1 {

auto shorten_request(std::vector<uint8_t>& dst, std::vector<uint8_t>& pass_dst,
		     std::string const& url_plaintext) -> bool {
	using ::ec_prv::private_url::PrivateURL;

	auto p = PrivateURL::generate(url_plaintext);
	if (!p) {
		return false;
	}
	auto const& [private_url, pass] = p.value();
	pass_dst.assign(pass.begin(), pass.end());

	::flatbuffers::FlatBufferBuilder fbb;
	auto iv = fbb.CreateVector(private_url.iv().data(), private_url.iv().size());
	auto salt = fbb.CreateVector(private_url.salt().data(), private_url.salt().size());
	auto blinded_url =
	    fbb.CreateVector(private_url.blinded_url().data(), private_url.blinded_url().size());
	::ec_prv::fbs::ShorteningRequestBuilder srb{fbb};
	srb.add_iv(iv);
	srb.add_salt(salt);
	srb.add_blinded_url(blinded_url);
	srb.add_version(1);
	srb.add_pbkdf2_iters(2'000'000);
	srb.add_expiry(default_expiry());
	auto sr = srb.Finish();
	fbb.Finish(sr);
	dst.reserve(fbb.GetSize());
	dst.assign(fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
	return true;
}

auto read_shortening_response(std::span<uint8_t> src, std::span<uint8_t> pass,
			      std::string_view upstream_server) -> std::string {
	using ::ec_prv::fbs::ShorteningResponse;
	using ::ec_prv::url_index::URLIndex;
	using ::flatbuffers::FlatBufferBuilder;
	using ::flatbuffers::Verifier;

	Verifier v{src.data(), src.size()};
	if (!::ec_prv::fbs::VerifyShorteningResponseBuffer(v)) {
		std::cout << "shortening response flatbuffer did not verify\n";
		return {};
	}

	auto const* srp = ::ec_prv::fbs::GetShorteningResponse(src.data());
	if (srp->error()) {
		std::cout << "encountered error\n";
		return {};
	}
	if (srp->version() != 1) {
		std::cout << "read_shortening_response(): version is not 1\n";
		return {};
	}

	// create fully formatted shortened URL
	auto ui = URLIndex::from_integer(srp->lookup_key());
	auto uib = ui.as_bytes();
	std::string pass_encoded;
	std::string url_index_encoded;
	::ec_prv::b66::enc(url_index_encoded, std::span<uint8_t>{uib});
	::ec_prv::b66::enc(pass_encoded, pass);
	std::ostringstream ss;
	ss << upstream_server << '/' << url_index_encoded << '#' << pass_encoded;
	return ss.str();
}

auto lookup_request(std::vector<uint8_t>& dst, ::ec_prv::url_index::URLIndex url_index) -> bool {
	using ::ec_prv::fbs::LookupRequest;
	using ::ec_prv::fbs::LookupRequestBuilder;
	using ::flatbuffers::FlatBufferBuilder;

	FlatBufferBuilder fbb;
	LookupRequestBuilder lrb{fbb};
	lrb.add_version(1);
	lrb.add_lookup_key(url_index.as_integer());
	auto lro = lrb.Finish();
	fbb.Finish(lro);

	// write buffer
	dst.resize(fbb.GetSize());
	memcpy(dst.data(), fbb.GetBufferPointer(), fbb.GetSize());
	return true;
}

auto read_lookup_response(std::string& dst, std::span<uint8_t> src, std::vector<uint8_t> pass)
    -> bool {
	using ::ec_prv::private_url::PrivateURL;
	using ::flatbuffers::Verifier;

	Verifier v{src.data(), src.size()};
	if (!::ec_prv::fbs::VerifyLookupResponseBuffer(v)) {
		return false;
	}

	auto lookup_response = std::make_unique<::ec_prv::fbs::LookupResponseT>();
	::ec_prv::fbs::GetLookupResponse(src.data())->UnPackTo(lookup_response.get());
	if (lookup_response->error) {
		return false;
	}
	if (lookup_response->version != 1) {
		return false;
	}
	auto private_url = std::make_unique<::ec_prv::fbs::PrivateURLT>();
	::flatbuffers::GetRoot<::ec_prv::fbs::PrivateURL>(lookup_response->data.data())
	    ->UnPackTo(private_url.get());
	if (private_url->version != 1) {
		return false;
	}
	if (private_url->expiry > current_unix_timestamp()) {
		return false;
	}
	PrivateURL pu{std::move(private_url->salt), std::move(private_url->iv),
		      std::move(private_url->blinded_url)};
	auto url_plaintext = pu.get_plaintext(std::move(pass));
	dst = std::move(url_plaintext);
	return true;
}

auto ClientV1::shorten(std::string const& url_plaintext) -> std::string {
	using ::ec_prv::cli_client::request;

	std::vector<uint8_t> srbuf;
	std::vector<uint8_t> generated_pass;
	auto status = shorten_request(srbuf, generated_pass, url_plaintext);
	if (!status) {
		return {};
	}

	std::ostringstream ss;
	ss << upstream_server << "/shortening_request";
	auto endpoint = ss.str();
	std::vector<uint8_t> sink;
	status = request(sink, endpoint, srbuf);
	if (!status) {
		return {};
	}
	auto shortened_url = read_shortening_response(sink, generated_pass, upstream_server);
	return shortened_url;
}

auto ClientV1::lookup(std::string const& base_identifier, std::string_view pass) -> std::string {
	using ::ec_prv::b66::dec;
	using ::ec_prv::cli_client::request;
	using ::ec_prv::url_index::URLIndex;

	// deserialize url index
	std::vector<uint8_t> url_index_bytes;
	dec(url_index_bytes, base_identifier);
	auto ui = URLIndex::from_bytes(url_index_bytes);

	// create request flatbuffer
	std::vector<uint8_t> req;
	auto status = lookup_request(req, ui);
	if (!status) {
		return {};
	}
	// post it
	std::ostringstream ss;
	ss << upstream_server << "/lookup_request";
	auto endpoint = ss.str();
	std::vector<uint8_t> response_bytes;
	status = request(response_bytes, std::move(endpoint), req);
	if (!status) {
		return {};
	}
	// read response and decrypt
	std::vector<uint8_t> pass_bytes;
	dec(pass_bytes, pass);
	std::string plaintext_url;
	status = read_lookup_response(plaintext_url, response_bytes, pass_bytes);
	if (!status) {
		return {};
	}
	return plaintext_url;
};

} // namespace v1
} // namespace shortening_client
} // namespace ec_prv
