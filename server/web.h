#ifndef _INCLUDE_EC_PRV_SERVER_WEB_H
#define _INCLUDE_EC_PRV_SERVER_WEB_H
#include <string_view>
#include "db.h"
#include "server/shortening_service.h"
#include <uWebSockets/App.h>

namespace ec_prv {
namespace web {

class Server {
private:
	int const port_;
	db::KVStore store_;
	shortening_service::ServiceHandle svc_;

public:
	explicit Server(int const port);
	void run();
};


} // namespace web
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_SERVER_WEB_H


