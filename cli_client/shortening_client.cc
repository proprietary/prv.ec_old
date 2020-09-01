n // #include "cli_client/shortening_client.h"
#include "cli_client/shortening_client.h"

#include "b64/b64.h"
#include "idl/all_generated_flatbuffers.h"
#include "private_url/private_url.h"
#include "url_index/url_index.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <flatbuffers/flatbuffers.h>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace {

	using namespace std::string_view_literals;

	auto default_expiry()->uint64_t {
		auto n = std::chrono::system_clock::now() + std::chrono::years(1);
		return std::chrono::duration_cast<std::chrono::seconds>(n.time_since_epoch())
		    .count();
	}

	auto current_unix_timestamp()->uint64_t {
		auto const n = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::seconds>(n.time_since_epoch())
		    .count();
	}

	auto parse_shortened_url(std::string_view full_url,
				 std::string_view upstream_server = "https://prv.ec"sv)
	    ->std::tuple<std::string, std::string> {
		auto* it = full_url.begin();
		for (auto* us = upstream_server.begin();
		     it != full_url.end() && us != upstream_server.end(); ++it, ++us) {
			if (*us != *it) {
				return {};
			}
		}
		if ((it - full_url.begin()) < upstream_server.length()) {
			return {};
		}
		if (it++; it != full_url.end() && *it != '/') {
			return {};
		}
		std::string identifier;
		identifier.reserve(full_url.end() - it);
		for (; it != full_url.end(); ++it) {
			if (*it == '#') {
				break;
			}
			identifier += *it;
		}
		std::string pass;
		pass.assign(it, full_url.end());
		return {identifier, pass};
	}

} // namespace

namespace ec_prv {
namespace shortening_client {

namespace v1 {

auto shorten_request(std::string const& url_plaintext)
    -> std::unique_ptr<::ec_prv::fbs::ShorteningRequestT> {
	using ::ec_prv::private_url::PrivateURL;

	auto p = PrivateURL::generate(url_plaintext);
	if (!p) {
		return nullptr;
	}
	auto const& [private_url, pass] = p.value();
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
	auto sruptr = std::make_unique<::ec_prv::fbs::ShorteningRequestT>();
	::flatbuffers::GetRoot<::ec_prv::fbs::ShorteningRequest>(fbb.GetBufferPointer())
	    ->UnPackTo(sruptr.get());
	return sruptr;
}

auto read_shortening_response(std::span<uint8_t> src, std::span<uint8_t> pass,
			      std::string_view upstream_server) -> std::string {
	using ::ec_prv::fbs::ShorteningResponse;
	using ::ec_prv::url_index::URLIndex;
	using ::flatbuffers::FlatBufferBuilder;
	using ::flatbuffers::Verifier;

	Verifier v{src.data(), src.size()};
	if (!::ec_prv::fbs::VerifyShorteningResponseBuffer(v)) {
		return {};
	}

	auto const* srp = ::ec_prv::fbs::GetShorteningResponse(src.data());
	if (srp->error()) {
		return {};
	}
	if (srp->version() != 1) {
		return {};
	}

	// create fully formatted shortened URL
	auto ui = URLIndex::from_integer(srp->lookup_key());
	auto uib = ui.as_bytes();
	auto encoded_index = ::ec_prv::b64::enc(std::span<uint8_t>{uib});
	auto encoded_pass = ::ec_prv::b64::enc(pass);
	std::ostringstream ss;
	ss << upstream_server << '/' << encoded_index << '#' << encoded_pass;
	return ss.str();
}

auto lookup_request(std::vector<uint8_t>& dst, ::ec_prv::url_index::URLIndex url_index) -> void {
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
}

auto read_lookup_request(std::string& dst, std::span<uint8_t> src, std::vector<uint8_t> pass)
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

} // namespace v1
} // namespace shortening_client
} // namespace ec_prv
