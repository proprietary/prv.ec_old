#include "b58/b58.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <iostream>
#include <span>
#include <string>
#include <string_view>

int main(int argc, char** argv) {
	using namespace std::literals;

	if (argc < 2) {
		return 1;
	}
	std::u8string in;
	size_t n = 0;
	constexpr size_t chunk_size = 1024;
	do {
		std::array<char8_t, chunk_size> buf;
		n = fread(buf.data(), 1, chunk_size, stdin);
		in.append(buf.begin(), buf.begin() + n);
	} while (n >= chunk_size);
	if ("-e"sv == argv[1]) {
		std::span<uint8_t> inbuf{reinterpret_cast<uint8_t*>(in.data()), in.size()};
		auto r = ec_prv::b58::enc(inbuf);
		fwrite(r.data(), r.size(), 1, stdout);
	} else if ("-d"sv == argv[1]) {
		std::string_view insv{reinterpret_cast<char const*>(in.data()), in.size()};
		auto r = ec_prv::b58::dec(insv);
		fwrite(r.data(), r.size(), 1, stdout);
	} else {
		printf(R"(Usage: %s [OPTION]
Encode or decode base-58 strings from standard input. Output is returned on standard output.

-d decode base-58 encoded data
-e encode binary data to base-58

Example:

    head -c 100 /dev/urandom | %s -e

)",
		       argv[0], argv[0]);
		return 1;
	}
	return 1;
}
