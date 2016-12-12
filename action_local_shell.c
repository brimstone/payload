#include <unistd.h>
#include <stdio.h>
#include "actions.h"

int action_local_shell(){
	execv("/bin/sh", (char *[]){"/bin/sh", 0});
	return 1;
}
void action_local_shell_help(){
	fprintf(stderr, "\nLocal Shell info:\n");
	fprintf(stderr, "This just execs /bin/sh.\n");
}
