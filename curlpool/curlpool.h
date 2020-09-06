#ifndef _INCLUDE_EC_PRV_CURLPOOL_H
#define _INCLUDE_EC_PRV_CURLPOOL_H

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <curl/multi.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <uv.h>
#include <vector>

namespace ec_prv {
namespace curlpool {

class cURLPool {
private:
	CURLM* curlm_;
	std::unique_ptr<uv_loop_t> loop_;
	uv_timer_t timeout_;
	uv_async_t wakeup_;
	std::queue<CURL*> additions_;
	std::queue<CURL*> removals_;
	std::mutex m_;

public:
	explicit cURLPool();
	~cURLPool() noexcept;
	void swap(cURLPool& b) noexcept;
	cURLPool(cURLPool const& other) = delete;
	cURLPool(cURLPool&& other) noexcept;
	cURLPool& operator=(cURLPool&& other) noexcept;
	cURLPool& operator=(cURLPool const&) = delete;
	void run();

	static void handle_socket(CURL* easy_handle, curl_socket_t socket, int action, void* userp,
				  void* socketp);

	static void handle_timeout(CURLM* _, long timeout_ms, void* userp);

	void check_multi_info();

	void handle_wakeup();

	void wakeup();

	void add_handle(CURL* easy_handle);

	void remove_handle(CURL* easy_handle);
};

struct cURLContext {
	std::unique_ptr<uv_poll_t> poll_handle;
	curl_socket_t sockfd;
	cURLPool* pool;
	explicit cURLContext(cURLPool& parent, uv_loop_t* loop, curl_socket_t socket);
	~cURLContext() noexcept;
	cURLContext(cURLContext&& other) noexcept;
	cURLContext(cURLContext const& other) = delete;
	cURLContext& operator=(cURLContext const& other) = delete;
	cURLContext& operator=(cURLContext&& other);
};

} // namespace curlpool
} // namespace ec_prv

#endif // _INCLUDE_EC_PRV_CURLPOOL_H
