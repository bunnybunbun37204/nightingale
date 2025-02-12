#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int yield_ = 0;
    if (argc > 1 && strcmp(argv[1], "-y") == 0)
        yield_ = 1;

    if (fork()) {
        while (true) {
            fprintf(stderr, "a");
            if (yield_)
                yield();
        }
    } else {
        while (true) {
            fprintf(stderr, "b");
            if (yield_)
                yield();
        }
    }
}
