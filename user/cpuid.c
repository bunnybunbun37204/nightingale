#include <stdio.h>
#include <stdlib.h>
#include <x86/cpu.h>

int main(int argc, char **argv)
{
    int request = 0;
    int sub = 0;

    if (argc >= 2)
        request = strtol(argv[1], NULL, 10);
    if (argc >= 3)
        request = strtol(argv[2], NULL, 10);

    unsigned id[4];
    cpuid(request, sub, id);
    printf("%08x %08x %08x %08x\n", id[0], id[1], id[2], id[3]);
}
