#include "cli_client/shortening_client.h"
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <string>
#include <string_view>

void print_help_message(char** const argv) {
	printf(R"(Usage: %s [OPTION...]

  -l, --lookup=URL            shortened URL to lookup and resolve to a full URL
  -s, --shorten=URL           full, original URL to shorten
  -r, --rounds=NUMBER         (optional, default 2000000) number of PBKDF2 rounds
  -u, --upstream-server=URL   (optional, default "https://prv.ec") change the default server
)",
	       argv[0]);
}

int main(int argc, char** argv) {
	int c = 0;
	int lookup_arg_set = 0;
	int shorten_arg_set = 0;
	std::string lookup_url{};
	std::string shorten_url{};
	std::string upstream_server{"https://prv.ec"};
	int32_t pbkdf2_rounds = 2'000'000;
	int pbkdf2_rounds_set = 0;
	int verbose_flag = 0;
	static option long_options[] = {
	    {"verbose", no_argument, &verbose_flag, 1},
	    {"brief", no_argument, &verbose_flag, 0},
	    {"help", no_argument, nullptr, 'h'},
	    {"lookup", required_argument, &lookup_arg_set, 'l'},
	    {"shorten", required_argument, &shorten_arg_set, 1},
	    {"upstream-server", required_argument, nullptr, 'u'},
	    {"rounds", required_argument, &pbkdf2_rounds_set, 'r'},
	    {nullptr, 0, nullptr, 0},
	};
	while (true) {
		int option_index = 0;
		c = getopt_long(argc, argv, "hl:s:u:", long_options, &option_index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 'l':
			if (optarg != nullptr)
				lookup_url = optarg;
			break;
		case 's':
			if (optarg != nullptr)
				shorten_url = optarg;
			break;
		case 'u':
			if (optarg != nullptr) {
				upstream_server = optarg;
			}
			break;
		case 'r':
			if (optarg != nullptr)
				pbkdf2_rounds = *optarg;
		case 'h':
			// show help message
			print_help_message(argv);
			break;
		default:
			return EXIT_FAILURE;
		}
	}
	// std::cout << "lookup arg " << lookup_arg << "\nshorten arg " << shorten_arg <<
	// "\nhost_server_arg " << upstream_server << std::endl;
	if ((!shorten_url.empty() && !lookup_url.empty()) ||
	    (shorten_url.empty() && lookup_url.empty())) {
		puts("\nEither --shorten or --lookup must be provided but not both");
		return EXIT_FAILURE;
	}

	::ec_prv::shortening_client::v1::ClientV1 client{
	    .upstream_server = upstream_server.c_str(),
	    .pbkdf2_rounds = pbkdf2_rounds,
	};

	if (!shorten_url.empty()) {
		auto shortened_url = client.shorten(shorten_url);
		puts(shortened_url.c_str());
	} else if (!lookup_url.empty()) {
		auto [identifier, pass] =
		    ::ec_prv::shortening_client::parse_shortened_url(lookup_url);
		auto plaintext_url = client.lookup(identifier, pass);
		puts(plaintext_url.c_str());
	}
	return EXIT_SUCCESS;
}
