#ifndef _INCLUDE_EC_PRV_SERVER_DB_H
#define _INCLUDE_EC_PRV_SERVER_DB_H
#include "url_index/url_index.h"
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
	auto put(url_index::URLIndex key, std::span<uint8_t> value) -> bool;
	auto get(rocksdb::PinnableSlice& dst, url_index::URLIndex key) -> bool;
};

} // namespace db
} // namespace ec_prv
#endif // _INCLUDE_EC_PRV_SERVER_DB_H
