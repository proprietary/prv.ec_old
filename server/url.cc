#include <url.h>
#include <string_view>

namespace ec_prv {
namespace url {

auto URL::valid() const noexcept -> bool {
	auto const sz = url_.size();
	if (sz > URL::MAX_CHAR_LENGTH) {
		return false;
	}
	constexpr static std::vector<std::string_view> protocols = {"http", "https", "mailto"};
	for (auto p : protocols) {
		auto const psz = p.size();
		if (sz <= psz) {
			return false;
		}
		bool full_match = true;
		for (auto i = 0ULL; i < p.size(); ++i) {
			if (url_[i] != p[i]) {
				full_match = false;
				break;
			}
		}
		if (full_match) {
			break;
		}
	}
	return true;
}

auto URL::get() const noexcept -> std::string_view const {
	return url_;
}

} // namespace url
} // namespace ec_prv

