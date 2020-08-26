#include "server/web.h"

int main(int argc, char** argv) {
	::ec_prv::web::Server server{8000};
	server.run();
	return 0;
}
