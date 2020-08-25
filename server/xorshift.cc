#include "server/xorshift.h"
#include <random>

namespace ec_prv::xorshift {

XORShift::XORShift() {
	// seed with real randomness from system call
#if defined(__linux__)
	std::random_device rd("/dev/urandom");
#elif defined(__APPLE__)
	std::random_device rd("/dev/urandom");
#else
	std::random_device rd();
#endif
	std::uniform_int_distribution<uint64_t> d(0);
	state_ = d(rd);
}

auto XORShift::rand() noexcept -> uint64_t {
	uint64_t x = state_;
	x ^= x >> 12; // a
	x ^= x << 25; // b
	x ^= x >> 27; // c
	state_ = x;
	return x * 0x2545F4914F6CDD1DULL;
}

} // namespace ec_prv::xorshift
