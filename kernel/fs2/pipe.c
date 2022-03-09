#include <basic.h>
#include <fcntl.h>
#include <ng/sync.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "file_system.h"
#include "inode.h"
#include "pipe.h"

#define wait_on wq_block_on
#define wake_from wq_notify_all

struct inode_operations pipe2_ops;
struct file_operations pipe2_file_ops;

struct inode *new_pipe(void) {
    struct inode *inode = new_inode(initfs_file_system, S_IFIFO | 0777);
    inode->capacity = 16384;
    inode->data = malloc(inode->capacity);
    inode->ops = &pipe2_ops;
    inode->file_ops = &pipe2_file_ops;
    return inode;
}

int pipe2_close(struct inode *pipe, struct fs2_file *file) {
    if (pipe->read_refcnt == 0) {
        wake_from(&pipe->write_queue);
    }
    if (pipe->write_refcnt == 0) {
        wake_from(&pipe->read_queue);
    }
    return 0;
}

struct inode_operations pipe2_ops = {
    .close = pipe2_close,
};

ssize_t pipe2_read(struct fs2_file *file, char *buffer, size_t len) {
    struct inode *inode = file->inode;
    while (inode->len == 0 && inode->write_refcnt)
        wait_on(&inode->read_queue);

    size_t to_read = umin(len, inode->len);
    memcpy(buffer, inode->data, to_read);
    memmove(inode->data, PTR_ADD(inode->data, to_read), inode->len - to_read);
    inode->len -= to_read;
    wake_from(&inode->write_queue);
    return to_read;
}

ssize_t pipe2_write(struct fs2_file *file, const char *buffer, size_t len) {
    struct inode *inode = file->inode;
    while (inode->len == inode->capacity && inode->read_refcnt)
        wait_on(&inode->write_queue);

    size_t to_write = umin(len, inode->capacity - inode->len);
    memcpy(PTR_ADD(inode->data, inode->len), buffer, to_write);
    inode->len += to_write;
    wake_from(&inode->read_queue);
    return to_write;
}

struct file_operations pipe2_file_ops = {
    .read = pipe2_read,
    .write = pipe2_write,
};
