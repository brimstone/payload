// https://stackoverflow.com/questions/16091382/pass-arguments-to-execve-program-in-shellcode

// http://blog.binamuse.com/2013/01/about-shellcodes-in-c.html
// docker run --rm -i -v $PWD:/opt -w /opt brimstone/kali msfvenom -p - -f elf -a x86 --platform linux -o execv

#include <unistd.h>
int main(int argc, char* argv[]){
	argv++;
	execv(argv[0], argv);
}
