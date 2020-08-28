#ifndef _INCLUDE_EC_PRV_SERVER_DB_H
#define _INCLUDE_EC_PRV_SERVER_DB_H
#include "rocksdb/db.h"
#include <cstdint>
#include <string_view>
#include <string>
#include <span>
#include <vector>

namespace ec_prv {
namespace db {

class RocksDBError : public std::runtime_error {
public:
	RocksDBError(std::string message) : std::runtime_error{message} {}
};

class KVStore {
private:
	rocksdb::DB* db_ = nullptr;

public:
	explicit KVStore();
	~KVStore() noexcept;
	KVStore(KVStore const&) = delete;
	auto operator=(KVStore const&) -> KVStore& = delete;
	KVStore(KVStore&&) noexcept;
	KVStore& operator=(KVStore&&) noexcept;
	auto put(std::vector<uint8_t>& key, std::vector<uint8_t>& value) -> bool;
	auto put(std::span<uint8_t> key, std::span<uint8_t> value) -> bool;
	auto get(std::vector<uint8_t>& key) -> std::vector<uint8_t>;
	void get(std::string& dst, std::span<uint8_t> const key);

	///
	/// Finds new, unused key in table using a callback which should
	/// successively try produce new options to try.
	///
	template <typename F> auto find_new_key(F key_creation_fn) -> decltype(auto) {
		while (true) {
			auto new_key = key_creation_fn();
			rocksdb::Slice kv {reinterpret_cast<char const*>(new_key.data()), new_key.size()};
			rocksdb::PinnableSlice v;
			auto status = db_->Get(rocksdb::ReadOptions(), nullptr, kv, &v);
			if (status.IsNotFound()) {
				continue;
			} else if (!status.ok()) {
				throw RocksDBError{status.ToString()};
			} else {
				return new_key;
			}
		}
	}
};

} // namespace db
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_SERVER_DB_H
