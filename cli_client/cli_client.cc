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
	std::string lookup_arg;
	std::string shorten_arg;
	std::string upstream_server{"https://prv.ec"};
	int32_t pbkdf2_rounds = 2'000'000;
	int verbose_flag = 0;
	static struct option long_options[] = {
	    {"verbose", no_argument, &verbose_flag, 1},
	    {"brief", no_argument, &verbose_flag, 0},
	    {"help", no_argument, nullptr, 'h'},
	    {"lookup", required_argument, nullptr, 'l'},
	    {"shorten", required_argument, nullptr, 's'},
	    {"upstream-server", required_argument, nullptr, 'u'},
	    {"rounds", required_argument, &pbkdf2_rounds, 'r'},
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
				lookup_arg = optarg;
			break;
		case 's':
			if (optarg != nullptr)
				shorten_arg = optarg;
			break;
		case 'u':
			if (optarg != nullptr)
				upstream_server = optarg;
			break;
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
	if ((shorten_arg.length() == 0 && lookup_arg.length() == 0) ||
	    (shorten_arg.length() > 0 && lookup_arg.length() > 0)) {
		puts("\nEither --shorten or --lookup must be provided but not both");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
