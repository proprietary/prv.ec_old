#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <uv.h>

static int64_t counter = 0;

int main() {
	::uv_loop_t *loop = new uv_loop_t;
	::uv_loop_init(loop);
	auto wait = +[](::uv_idle_t *handle) {
		counter++;
		if (counter >= (1 << 20)) {
			::uv_idle_stop(handle);
		}
	};
	::uv_idle_t idler;
	::uv_idle_init(loop, &idler);
	::uv_idle_start(&idler, wait);
	::puts("Idling...");
	::uv_run(loop, UV_RUN_DEFAULT);
	::uv_loop_close(loop);
	delete loop;
	return 0;
}
