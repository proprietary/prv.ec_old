#include "server/db.h"
#include "idl/generated/direct_url_generated.h"
#include <cassert>
#include <cstdio>
#include <algorithm>

namespace ec_prv {
namespace db {

KVStore::KVStore() {
	rocksdb::Options options;
	options.IncreaseParallelism();
	options.OptimizeLevelStyleCompaction();
	auto status = rocksdb::DB::Open(options, "/tmp/testdb", &db_);
	assert(status.ok());
}

KVStore::~KVStore() noexcept {
	if (db_ != nullptr) {
		delete db_;
	}
}

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

void KVStore::put(std::vector<uint8_t>& key, std::vector<uint8_t>& value) {
	rocksdb::Slice k{reinterpret_cast<char*>(key.data()), key.size()};
	rocksdb::Slice v{reinterpret_cast<char*>(value.data()), value.size()};
	db_->Put(rocksdb::WriteOptions(), k, v);
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

} // namespace db
} // namespace ec_prv

int main(int argc, char** argv) {
	using namespace rocksdb;
	DB* db;
	Options options;
	// Optimize RocksDB. This is the easiest way to get RocksDB to perform well
	options.IncreaseParallelism();
	options.OptimizeLevelStyleCompaction();
	// create the DB if it's not already present
	options.create_if_missing = true;

	// open DB
	Status s = DB::Open(options, "/tmp/testdb", &db);
	assert(s.ok());

	// Put key-value
	s = db->Put(WriteOptions(), "key1", "value");
	assert(s.ok());
	std::string value;
	// get value
	s = db->Get(ReadOptions(), "key1", &value);
	assert(s.ok());
	assert(value == "value");

	// atomically apply a set of updates
	{
		WriteBatch batch;
		batch.Delete("key1");
		batch.Put("key2", value);
		s = db->Write(WriteOptions(), &batch);
	}

	s = db->Get(ReadOptions(), "key1", &value);
	assert(s.IsNotFound());

	db->Get(ReadOptions(), "key2", &value);
	assert(value == "value");

	{
		PinnableSlice pinnable_val;
		db->Get(ReadOptions(), db->DefaultColumnFamily(), "key2", &pinnable_val);
		assert(pinnable_val == "value");
	}

	{
		std::string string_val;
		// If it cannot pin the value, it copies the value to its internal buffer.
		// The intenral buffer could be set during construction.
		PinnableSlice pinnable_val(&string_val);
		db->Get(ReadOptions(), db->DefaultColumnFamily(), "key2", &pinnable_val);
		assert(pinnable_val == "value");
		// If the value is not pinned, the internal buffer must have the value.
		assert(pinnable_val.IsPinned() || string_val == "value");
	}

	PinnableSlice pinnable_val;
	s = db->Get(ReadOptions(), db->DefaultColumnFamily(), "key1", &pinnable_val);
	assert(s.IsNotFound());
	// Reset PinnableSlice after each use and before each reuse
	pinnable_val.Reset();
	db->Get(ReadOptions(), db->DefaultColumnFamily(), "key2", &pinnable_val);
	assert(pinnable_val == "value");
	pinnable_val.Reset();
	// The Slice pointed by pinnable_val is not valid after this point

	delete db;

	return 0;
}
