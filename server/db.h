#ifndef __INCLUDE_EC_PRV_DB_H
#define __INCLUDE_EC_PRV_DB_H
#include <cstdint>
#include <rocksdb/db.h>
#include <span>
#include <vector>

namespace ec_prv {
namespace db {

class KVStore {
private:
	rocksdb::DB* db_ = nullptr;

public:
	explicit KVStore();
	~KVStore() noexcept;
	KVStore(KVStore const&) = delete;
	KVStore& operator=(KVStore const&) = delete;
	KVStore(KVStore&&) noexcept;
	KVStore& operator=(KVStore&&) noexcept;
	bool put(std::vector<uint8_t>& key, std::vector<uint8_t>& value);
	bool put(std::span<uint8_t> key, std::span<uint8_t> value);
	auto get(std::vector<uint8_t>& key) -> std::vector<uint8_t>;
	void get(std::string& dst, std::span<uint8_t> const key);
};

} // namespace db
} // namespace ec_prv
#endif // __INCLUDE_EC_PRV_DB_H
