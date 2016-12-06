#include <unistd.h>

int action_local_shell(){
	execv("/bin/sh", (char *[]){"/bin/sh"});
	return 1;
}
