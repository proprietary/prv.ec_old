#include "server/db.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <span>
#include <exception>

namespace ec_prv {
namespace db {

auto KVStore::open_default() -> KVStore {
	const char* EC_PRV_ROCKSDB_DATADIR_PATH = std::getenv("EC_PRV_ROCKSDB_DATADIR_PATH");
	if (nullptr == EC_PRV_ROCKSDB_DATADIR_PATH || strlen(EC_PRV_ROCKSDB_DATADIR_PATH) == 0) {
		throw std::runtime_error{"Environment variable EC_PRV_ROCKSDB_DATADIR_PATH is missing"};
	}
	return KVStore{EC_PRV_ROCKSDB_DATADIR_PATH};
}

KVStore::KVStore(std::string_view path) {
	rocksdb::Options options;
	options.create_if_missing = true;
	options.IncreaseParallelism();
	options.OptimizeLevelStyleCompaction();
	std::string datadir_path {path};
	auto status = rocksdb::DB::Open(options, datadir_path, &db_);
	if (!status.ok()) {
		throw RocksDBError{status.ToString()};
	}
}

KVStore::~KVStore() noexcept { delete db_; }

KVStore::KVStore(KVStore&& other) noexcept {
	if (this->db_ != nullptr) {
		delete this->db_;
	}
	this->db_ = other.db_;
	other.db_ = nullptr;
	other.~KVStore();
}

KVStore& KVStore::operator=(KVStore&& other) noexcept {
	if (this->db_ != nullptr) {
		delete this->db_;
	}
	this->db_ = other.db_;
	other.db_ = nullptr;
	other.~KVStore();
	return *this;
}

bool KVStore::put(std::vector<uint8_t>& key, std::vector<uint8_t>& value) {
	rocksdb::Slice k{reinterpret_cast<char*>(key.data()), key.size()};
	rocksdb::Slice v{reinterpret_cast<char*>(value.data()), value.size()};
	auto s = db_->Put(rocksdb::WriteOptions(), k, v);
	return s.ok();
}

bool KVStore::put(std::span<uint8_t> key, std::span<uint8_t> value) {
	rocksdb::Slice k{reinterpret_cast<char*>(key.data()), key.size()};
	rocksdb::Slice v{reinterpret_cast<char*>(value.data()), value.size()};
	auto s = db_->Put(rocksdb::WriteOptions(), k, v);
	return s.ok();
}

auto KVStore::get(std::vector<uint8_t>& key) -> std::vector<uint8_t> {
	rocksdb::Slice k{reinterpret_cast<char*>(key.data()), key.size()};
	std::string v;
	auto s = db_->Get(rocksdb::ReadOptions(), k, &v);
	assert(s.ok());
	if (!s.ok()) {
		// TODO: think about how to handle this in context of its use
		return {};
	}
	std::vector<uint8_t> out(v.size());
	std::copy(v.begin(), v.end(), out.begin());
	return out;
}

void KVStore::get(std::string& dst, std::span<uint8_t> const key) {
	rocksdb::Slice k{reinterpret_cast<char const*>(key.data()), key.size()};
	auto s = db_->Get(rocksdb::ReadOptions(), k, &dst);
	if (!s.ok()) {
		assert(false);
		// TODO
	}
}

auto KVStore::put(url_index::URLIndex key, std::span<uint8_t> value) -> bool {
	auto b = key.as_bytes();
	rocksdb::Slice k {reinterpret_cast<char*>(b.data()), b.size()};
	rocksdb::Slice v {reinterpret_cast<char*>(value.data()), value.size()};
	auto s = db_->Put(rocksdb::WriteOptions(), k, v);
	return s.ok();
}

auto KVStore::get(rocksdb::PinnableSlice& dst, url_index::URLIndex key) -> rocksdb::Status {
	auto b = key.as_bytes();
	rocksdb::Slice k {reinterpret_cast<char*>(b.data()), b.size()};
	auto s = db_->Get(rocksdb::ReadOptions(), db_->DefaultColumnFamily(), k, &dst);
	return s;
}

} // namespace db
} // namespace ec_prv
