#pragma comment (lib, "ws2_32.lib")

#include <stdio.h>
#include "wrapper.hpp"

int main() {
	Wrapper wrapper;

	Credentials creds = {"<auth username>", "<auth password>"};

	if (!wrapper.connect_to_proxy("<proxy ip>", 0, &creds)) {
		printf("Could not connect to the proxy.\n");
		return 1;
	}

	if (!wrapper.set_destination("192.178.25.174", 80)) {
		printf("Could not set the destination.\n");
		return 1;
	}


	return 0;
}