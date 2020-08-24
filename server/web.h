#ifndef __INCLUDE_EC_SERVER_PRV_WEB_H
#define __INCLUDE_EC_SERVER_PRV_WEB_H
#include <uWebSockets/App.h>
#include <string_view>

namespace ec_prv {
namespace web {

// class Server {
// public:
// 	const char* address;
// };

void rpc_dispatch(std::string_view inbuf);

} // namespace web
} // namespace ec_prv
#endif // __INCLUDE_EC_SERVER_PRV_WEB_H


