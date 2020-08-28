#include "server/xorshift.h"
#include <random>
#include <ctime>

namespace ec_prv::xorshift {

XORShiftU64::XORShiftU64() {
	// seed with real randomness from system call
	// clang-format off
#if defined(__linux__)
	std::random_device rd("/dev/urandom");
#elif defined(__APPLE__)
	std::random_device rd("/dev/urandom");
#else
	std::random_device rd();
#endif
	// clang-format on
	std::uniform_int_distribution<uint64_t> d(0);
	state_ = d(rd);
}

auto XORShiftU64::rand() noexcept -> uint64_t {
	uint64_t x = state_;
	x ^= x >> 12; // a
	x ^= x << 25; // b
	x ^= x >> 27; // c
	state_ = x;
	return x * 0x2545F4914F6CDD1DULL;
}

XORShiftU32::XORShiftU32() {
	auto t = std::time(nullptr);
	y_ = static_cast<uint32_t>(t);
}

auto XORShiftU32::rand() noexcept -> uint32_t {
	y_ ^= y_ << 13;
	y_ ^= y_ << 17;
	y_ ^= y_ << 5;
	return y_;
}

} // namespace ec_prv::xorshift
