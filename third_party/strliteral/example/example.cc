#include "third_party/strliteral/example/example.txt_generated.h"
#include <stdio.h>

int main(int argc, char** argv) {
	puts(reinterpret_cast<const char*>(example_txt));
	return 1;
}
