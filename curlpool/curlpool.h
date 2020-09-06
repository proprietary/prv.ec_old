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

class cURLPool;

struct cURLContext {
	std::unique_ptr<uv_poll_t> poll_handle;
	curl_socket_t sockfd;
	cURLPool* pool;
	explicit cURLContext(cURLPool& parent, uv_loop_t* loop, curl_socket_t socket)
	    : poll_handle{std::make_unique<uv_poll_t>()}, pool{&parent} {
		sockfd = socket;
		uv_poll_init_socket(loop, poll_handle.get(), sockfd);
		poll_handle->data = this;
	}
	~cURLContext() noexcept {
		if (poll_handle) {
			uv_close(
			    reinterpret_cast<uv_handle_t*>(poll_handle.get()),
			    +[](uv_handle_t* handle) {
				    // auto* that = static_cast<cURLContext*>(handle->data);
			    });
		}
	}
	cURLContext(cURLContext&& other) noexcept {
		pool = other.pool;
		poll_handle = std::move(other.poll_handle);
		sockfd = other.sockfd;
	}
	cURLContext(cURLContext const& other) = delete;
	cURLContext& operator=(cURLContext const& other) = delete;
	cURLContext& operator=(cURLContext&& other) {
		pool = other.pool;
		poll_handle = std::move(other.poll_handle);
		sockfd = other.sockfd;
		return *this;
	}
};

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
	explicit cURLPool() : loop_{std::make_unique<uv_loop_t>()} {
		uv_loop_init(loop_.get());
		loop_->data = this;
		uv_timer_init(loop_.get(), &timeout_);
		timeout_.data = this;
		uv_async_init(
		    loop_.get(), &wakeup_, +[](uv_async_t* handle) -> void {
			    assert(handle->data != nullptr);
			    auto* that = static_cast<cURLPool*>(handle->data);
			    that->handle_wakeup();
		    });
		wakeup_.data = this;
		curlm_ = curl_multi_init();
		curl_multi_setopt(curlm_, CURLMOPT_SOCKETDATA, this);
		curl_multi_setopt(curlm_, CURLMOPT_SOCKETFUNCTION, handle_socket);
		curl_multi_setopt(curlm_, CURLMOPT_TIMERDATA, this);
		curl_multi_setopt(curlm_, CURLMOPT_TIMERFUNCTION, handle_timeout);
	}
	~cURLPool() noexcept {
		if (loop_) {
			uv_loop_close(loop_.get());
		}
		if (curlm_ != nullptr) {
			curl_multi_cleanup(curlm_);
		}
	}
	void swap(cURLPool& b) noexcept {
		using std::swap;
		swap(curlm_, b.curlm_);
		swap(loop_, b.loop_);
		swap(timeout_, b.timeout_);
		swap(wakeup_, b.wakeup_);
		swap(additions_, b.additions_);
		swap(removals_, b.removals_);
	}
	cURLPool(cURLPool const& other) noexcept;
	cURLPool(cURLPool&& other) noexcept {
		curlm_ = other.curlm_;
		other.curlm_ = nullptr;
		loop_ = std::move(other.loop_);
		timeout_ = other.timeout_;
		wakeup_ = other.wakeup_;
		additions_ = std::move(other.additions_);
		removals_ = std::move(other.removals_);
	}
	cURLPool& operator=(cURLPool&& other) noexcept {
		using std::swap;
		cURLPool{std::move(other)}.swap(*this);
		return *this;
	}
	cURLPool& operator=(cURLPool const&) = delete;
	void run() { uv_run(loop_.get(), UV_RUN_DEFAULT); }

	static void handle_socket(CURL* easy_handle, curl_socket_t socket, int action, void* userp,
				  void* socketp) {
		assert(userp != nullptr);
		auto* that = static_cast<cURLPool*>(userp);
		cURLContext* curl_context = nullptr;
		int events = 0;
		switch (action) {
		case CURL_POLL_IN:
		case CURL_POLL_OUT:
		case CURL_POLL_INOUT: {
			if (socketp != nullptr) {
				curl_context = static_cast<cURLContext*>(socketp);
			} else {
				curl_context = new cURLContext{*that, that->loop_.get(), socket};
			}
			curl_multi_assign(that->curlm_, socket, curl_context);
			if (action != CURL_POLL_IN) {
				events |= UV_WRITABLE;
			}
			if (action != CURL_POLL_OUT) {
				events |= UV_READABLE;
			}
			uv_poll_start(
			    curl_context->poll_handle.get(), events,
			    +[](uv_poll_t* req, int status, int events) -> void {
				    assert(req->data != nullptr);
				    auto* context = static_cast<cURLContext*>(req->data);
				    int flags = 0;
				    if (events & UV_READABLE) {
					    flags |= CURL_CSELECT_IN;
				    }
				    if (events & UV_WRITABLE) {
					    flags |= CURL_CSELECT_OUT;
				    }
				    int running_handles = 0;
				    auto ec = curl_multi_socket_action(context->pool->curlm_,
								       context->sockfd, flags,
								       &running_handles);
				    if (ec != CURLM_OK) {
					    fputs(curl_multi_strerror(ec), stderr);
					    // TODO: handle curl multi error
				    }
				    printf("%d running\n", running_handles);
				    context->pool->check_multi_info();
			    });
			break;
		case CURL_POLL_REMOVE:
			if (socketp != nullptr) {
				curl_context = static_cast<cURLContext*>(socketp);
				uv_poll_stop(curl_context->poll_handle.get());
				delete curl_context;
				curl_multi_assign(that->curlm_, socket, nullptr);
			}
			break;
		}
		default:
			std::terminate();
		}
	}

	static void handle_timeout(CURLM* _, long timeout_ms, void* userp) {
		assert(userp != nullptr);
		auto* that = static_cast<cURLPool*>(userp);
		if (timeout_ms < 0) {
			uv_timer_stop(&that->timeout_);
		} else {
			if (timeout_ms == 0) {
				timeout_ms = 1;
			}
			uv_timer_start(
			    &that->timeout_,
			    +[](uv_timer_t* req) -> void {
				    assert(req->data != nullptr);
				    auto* that = static_cast<cURLPool*>(req->data);
				    int running_handles = 0;
				    curl_multi_socket_action(that->curlm_, CURL_SOCKET_TIMEOUT, 0,
							     &running_handles);
				    that->check_multi_info();
			    },
			    timeout_ms, 0);
		}
	}

	void check_multi_info() {
		CURLMsg* message = nullptr;
		int pending = 0;
		CURL* easy_handle = nullptr;
		while ((message = curl_multi_info_read(curlm_, &pending)) != nullptr) {
			switch (message->msg) {
			case CURLMSG_DONE: {
				easy_handle = message->easy_handle;
				char* done_url = nullptr;
				curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
				assert(done_url != nullptr);
				if (done_url != nullptr) {
					fprintf(stdout, "%s DONE\n", done_url);
				}
				curl_multi_remove_handle(curlm_, easy_handle);
				curl_easy_cleanup(easy_handle);
				break;
			}
			default:
				fprintf(stderr, "CURLMSG default\n");
				assert(false);
				std::terminate();
				break;
			}
		}
	}

	void handle_wakeup() {
		std::lock_guard<std::mutex> lock{m_};
		while (!additions_.empty()) {
			auto* a = additions_.front();
			auto ec = curl_multi_add_handle(curlm_, a);
			if (ec != CURLM_OK) {
				fputs(curl_multi_strerror(ec), stderr);
			}
			additions_.pop();
		}
		while (!removals_.empty()) {
			auto* a = removals_.front();
			auto ec = curl_multi_remove_handle(curlm_, a);
			if (ec != CURLM_OK) {
				fputs(curl_multi_strerror(ec), stderr);
			}
			removals_.pop();
		}
		// TODO: graceful shutdown when a `shutdown` boolean is set
	}

	void wakeup() { uv_async_send(&wakeup_); }

	void add_handle(CURL* easy_handle) {
		{
			std::lock_guard<std::mutex> lock{m_};
			additions_.push(easy_handle);
		}
		wakeup();
	}

	void remove_handle(CURL* easy_handle) {
		{
			std::lock_guard<std::mutex> lock{m_};
			removals_.push(easy_handle);
		}
		wakeup();
	}
};

} // namespace curlpool
} // namespace ec_prv

#endif // _INCLUDE_EC_PRV_CURLPOOL_H
