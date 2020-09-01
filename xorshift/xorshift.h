#ifndef _INCLUDE_EC_PRV_XORSHIFT_XORSHIFT
#define _INCLUDE_EC_PRV_XORSHIFT_XORSHIFT
#include <cstdint>

namespace ec_prv {
namespace xorshift {

///
/// Fast, non-cryptographically secure random number generator
/// https://en.wikipedia.org/wiki/Xorshift
/// https://www.jstatsoft.org/article/view/v008i14
///
class XORShiftU64 {
private:
	uint64_t state_ = 0;

public:
	explicit XORShiftU64();

	///
	/// Get next pseudo-random number.
	///
	auto rand() noexcept -> uint64_t;
};

///
/// Fast, non-cryptographically secure random number generator.
///
class XORShiftU32 {
private:
	uint32_t y_ = 0;

public:
	explicit XORShiftU32();

	///
	/// Get next pseudo-random number.
	///
	auto rand() noexcept -> uint32_t;
};

} // namespace xorshift
} // namespace ec_prv

#endif // _INCLUDE_EC_PRV_XORSHIFT_XORSHIFT
