#ifndef _INCLUDE_EC_PRV_SERVER_WEB_H
#define _INCLUDE_EC_PRV_SERVER_WEB_H
#include "server/db.h"
#include "idl/all_generated_flatbuffers.h"
#include "shortening_service.h"
#include <cstdint>
#include <flatbuffers/flatbuffers.h>
#include <string>
#include <string_view>
#include <uWebSockets/App.h>
#include <vector>

namespace ec_prv {
namespace web {

class Server {
private:
	int const port_;
	db::KVStore store_;
	shortening_service::ServiceHandle svc_;

	/// HTTP Basic Auth credentials for "trusted" endpoints
	std::string const rpc_user_;
	std::string const rpc_pass_;

	template <typename RequestMessageT, bool SSL = false>
	void accept_rpc(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req) {
		std::vector<uint8_t> buf;
		res->onData([this, &buf, res](std::string_view chunk, bool is_end) {
			std::copy(chunk.begin(), chunk.end(), std::back_inserter(buf));
			if (is_end) {
				res->cork([this, &buf, res]() -> void {
					::flatbuffers::Verifier v{buf.data(), buf.size()};
					auto* root =
					    ::flatbuffers::GetRoot<RequestMessageT>(buf.data());
					if (!v.VerifyBuffer<RequestMessageT>(nullptr)) {
						res->end();
						return;
					}
					res->writeHeader("Content-Type", "application/octet-stream");
					res->writeStatus("200 OK");
					std::unique_ptr<typename RequestMessageT::NativeTableType>
					    fb{root->UnPack(nullptr)};
					auto lookup_response = this->svc_.handle(std::move(fb));
					res->end(std::string_view{
					    reinterpret_cast<char const*>(lookup_response.data()),
					    lookup_response.size()});
				});
			}
		});
	}

public:
	explicit Server(int const port);
	void run();
};

} // namespace web
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_SERVER_WEB_H
