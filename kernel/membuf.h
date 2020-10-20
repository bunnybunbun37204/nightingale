#pragma once
#ifndef NG_K_MEMBUF_H
#define NG_K_MEMBUF_H

#include <basic.h>
#include <ng/fs.h>

ssize_t membuf_read(struct open_file *n, void *data, size_t len);
ssize_t membuf_write(struct open_file *n, const void *data, size_t len);
off_t membuf_seek(struct open_file *n, off_t offset, int whence);

#endif // NG_K_MEMBUF_H
