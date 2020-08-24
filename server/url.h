#ifndef __INCLUDE_EC_PRV_SERVER_URL_H
#define __INCLUDE_EC_PRV_SERVER_URL_H
#include <string>
#include <string_view>
#include <vector>

namespace ec_prv {
namespace url {

class URL {
private:
	std::string_view url_;
	constexpr static size_t MAX_CHAR_LENGTH = 3000;
public:
	auto valid() const noexcept -> bool;
	auto get() const noexcept -> std::string_view const;
};

} // namespace url
} // namespace ec_prv
#endif // __INCLUDE_EC_PRV_SERVER_URL_H
