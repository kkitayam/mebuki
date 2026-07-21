#include <errno.h>
#include <reent.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "uart.h"

int _close_r(struct _reent *r, int fd)
{
  (void)fd;
  r->_errno = EBADF;
  return -1;
}

int _fstat_r(struct _reent *r, int fd, struct stat *st)
{
  (void)fd;
  (void)st;
  r->_errno = EBADF;
  return -1;
}

int _isatty_r(struct _reent *r, int fd)
{
  (void)fd;
  r->_errno = EBADF;
  return 0;
}

off_t _lseek_r(struct _reent *r, int fd, off_t offset, int whence)
{
  (void)fd;
  (void)offset;
  (void)whence;
  r->_errno = EBADF;
  return (off_t)-1;
}

ssize_t _read_r(struct _reent *r, int fd, void *buf, size_t cnt)
{
  (void)fd;
  (void)buf;
  (void)cnt;
  r->_errno = EBADF;
  return -1;
}

ssize_t _write_r(struct _reent *r, int fd, const void *buf, size_t cnt)
{
  if (fd != STDOUT_FILENO && fd != STDERR_FILENO) {
    r->_errno = EBADF;
    return -1;
  }

  const char *p = (const char *)buf;
  for (size_t i = 0U; i < cnt; ++i) {
    if (p[i] == '\n') {
      uart_putc('\r');
    }

    uart_putc(p[i]);
  }

  return (ssize_t)cnt;
}

void *_sbrk_r(struct _reent *r, ptrdiff_t incr)
{
  (void)incr;
  r->_errno = ENOMEM;
  return (void *)-1;
}