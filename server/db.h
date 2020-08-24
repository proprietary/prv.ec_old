#ifndef __INCLUDE_EC_PRV_DB_H
#define __INCLUDE_EC_PRV_DB_H
#include <rocksdb/db.h>
#include <vector>
#include <cstdint>

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
	void put(std::vector<uint8_t>& key, std::vector<uint8_t>& value);
	auto get(std::vector<uint8_t>& key) -> std::vector<uint8_t>;
};

} // namespace db
} // namespace ec_prv
#endif // __INCLUDE_EC_PRV_DB_H
