#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

void fail_unless(int a)
{
    if (a < 0)
        exit(1);
}

int exec(const char *stdio_file, char **argv)
{
    pid_t child;

    child = fork();
    if (child == -1) {
        perror("fork()");
        return -1;
    }
    if (child == 0) {
        if (stdio_file) {
            close(0);
            close(1);
            close(2);
            fail_unless(open(stdio_file, O_RDONLY));
            fail_unless(open(stdio_file, O_WRONLY));
            fail_unless(open(stdio_file, O_WRONLY));
        }

        printf("Welcome to Nightingale\n");
        execve(argv[0], argv, NULL);

        printf("init failed to run sh\n");
        perror("execve");
        exit(127);
    }
    return child;
}

void run_sh_forever(const char *stdio_file)
{
    while (true) {
        int child = exec(stdio_file, (char *[]) { "/bin/sh", NULL });
        int return_code;
        while (true) {
            int pid = waitpid(child, &return_code, 0);
            if (pid < 0 && errno == EINTR)
                continue;
            if (pid < 0 && errno == ECHILD)
                break;
            if (pid < 0) {
                perror("waitpid");
                exit(1);
            }
            break;
        }
    }
}

void cleanup_children(int arg)
{
    (void)arg;
    int status;
    pid_t pid = 0;
    while (pid != -1) {
        pid = waitpid(-1, &status, 0);
        // fprintf(stderr, "init reaped %i\n", pid);
    }
}

int main()
{
    fail_unless(mkdir("/dev", 0755));
    fail_unless(mknod("/dev/null", 0666, 0));
    fail_unless(mknod("/dev/zero", 0666, 1));
    fail_unless(mknod("/dev/random", 0666, 2));
    fail_unless(mknod("/dev/inc", 0666, 3));
    fail_unless(mknod("/dev/serial", 0666, 1 << 16));
    fail_unless(mknod("/dev/serial2", 0666, 1 << 16 | 1));

    fail_unless(mkdir("/proc", 0755));
    fail_unless(mount("/proc", _FS_PROCFS, "proc"));

    fail_unless(open("/dev/serial", O_RDONLY));
    fail_unless(open("/dev/serial", O_WRONLY));
    fail_unless(open("/dev/serial", O_WRONLY));

    signal(SIGCHLD, cleanup_children);

    if (fork())
        run_sh_forever("/dev/serial2");
    run_sh_forever(NULL);
    assert(0);
}
