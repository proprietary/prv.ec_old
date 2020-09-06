#ifndef _INCLUDE_EC_PRV_CURLPOOL_H
#define _INCLUDE_EC_PRV_CURLPOOL_H

#include <atomic>
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

///
/// Implements an asynchronous, event-driven (libuv) runner for cURL requests.
///
///
class cURLPool {
private:
	CURLM* curlm_;
	std::unique_ptr<uv_loop_t> loop_;
	uv_timer_t timeout_;
	uv_async_t wakeup_;
	std::queue<CURL*> additions_;
	std::queue<CURL*> removals_;
	std::mutex m_;
	std::atomic_bool stopping_;

public:
	explicit cURLPool();
	~cURLPool() noexcept;
	void swap(cURLPool& b) noexcept;
	cURLPool(cURLPool const& other) = delete;
	cURLPool(cURLPool&& other) noexcept;
	auto operator=(cURLPool&& other) noexcept -> cURLPool&;
	auto operator=(cURLPool const&) -> cURLPool& = delete;

	///
	/// Start executing event loop.
	/// To stop the event loop, call the destructor from another thread.
	///
	void run();

	///
	/// Enqueue a cURL easy handle. You can call this from another thread while the loop is
	/// running.
	///
	void add_handle(CURL* easy_handle);

	///
	/// Remove a cURL easy handle. You can call this from another thead while the loop is
	/// running.
	///
	void remove_handle(CURL* easy_handle);

private:
	static void handle_socket(CURL* easy_handle, curl_socket_t socket, int action, void* userp,
				  void* socketp);

	static void handle_timeout(CURLM* easy_handle, long timeout_ms, void* userp);

	void check_multi_info();

	void handle_wakeup();

	void wakeup();
};

struct cURLContext {
	std::unique_ptr<uv_poll_t> poll_handle;
	curl_socket_t sockfd;
	cURLPool* pool;
	explicit cURLContext(cURLPool& parent, uv_loop_t* loop, curl_socket_t socket);
	~cURLContext() noexcept;
	cURLContext(cURLContext&& other) noexcept;
	cURLContext(cURLContext const& other) = delete;
	auto operator=(cURLContext const& other) -> cURLContext& = delete;
	auto operator=(cURLContext&& other) noexcept -> cURLContext&;
};

} // namespace curlpool
} // namespace ec_prv

#endif // _INCLUDE_EC_PRV_CURLPOOL_H
