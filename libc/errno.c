#include <basic.h>
#include <errno.h>
#include <stdio.h>

// TODO: errno should be thread-local
int errno;

const char *errno_names[] = {
    [SUCCESS] = "SUCCESS",
    [ETODO] = "ETODO",
    [EINVAL] = "EINVAL",
    [EAGAIN] = "EAGAIN",
    [ENOEXEC] = "ENOEXEC",
    [ENOENT] = "ENOENT",
    [EAFNOSUPPORT] = "EAFNOSUPPORT",
    [EPROTONOSUPPORT] = "EPROTONOSUPPORT",
    [ECHILD] = "ECHILD",
    [EPERM] = "EPERM",
    [EFAULT] = "EFAULT",
    [EBADF] = "EBADF",
    [ERANGE] = "ERANGE",
    [EDOM] = "EDOM",
    [EACCES] = "EACCES",
    [ESPIPE] = "ESPIPE",
    [EISDIR] = "EISDIR",
    [ESRCH] = "ESRCH",
    [ENOSYS] = "ENOSYS",
    [ENOTTY] = "ENOTTY",
    [ENOTDIR] = "ENOTDIR",
    [ECONNREFUSED] = "ECONNREFUSED",
    [ENODEV] = "ENODEV",
};

const char *const perror_strings[] = {
    [SUCCESS] = "No error",
    [EINVAL] = "(EINVAL) Invalid argument",
    [EWOULDBLOCK] = "(EAGAIN) Would block",
    [ENOEXEC] = "(ENOEXEC) Argument is not executable",
    [ENOENT] = "(ENOENT) Entity does not exist",
    [EAFNOSUPPORT] = "(EAFNOSUPPORT) Unsupported protocol",
    [EPROTONOSUPPORT] = "(EPROTONOSUPPORT) Unsupported protocol",
    [ECHILD] = "(ECHILD) No such child",
    [EPERM] = "(EPERM) No permission",
    [EFAULT] = "(EFAULT) Fault occurred",
    [EBADF] = "(EBADF) Bad fd",
    [ERANGE] = "(ERANGE) Out of range",
    [EACCES] = "(EACCES) File access disallowed",
    [ESPIPE] = "(ESPIPE) File is not seekable",
    [EISDIR] = "(EISDIR) Is directory",
    [ESRCH] = "(ESRCH) Entity does not exist",
    [ENOSYS] = "(ENOSYS) Function not implemented",
    [ENOTTY] = "(ENOTTY) Not a TTY",
    [ENOTDIR] = "(ENOTDIR) Not a directory",
    [ECONNREFUSED] = "(ECONNREFUSED) Connection refused",
    [ETODO] = "(ETODO) Work in progress",
    [ENODEV] =
        "(ENODEV) No such device (or mmap does not support mapping that)",
};

#ifndef __kernel__
void perror(const char *const message) {
    if (errno >= 0 && errno <= ETODO) {
        fprintf(stderr, "%s: %s\n", message, perror_strings[errno]);
    } else {
        fprintf(stderr, "%s: Unknown Error (%i)\n", message, errno);
    }
}

const char *strerror(enum errno_value errno) {
    return perror_strings[errno];
}
#endif // ifndef __kernel__
