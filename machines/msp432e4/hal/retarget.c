#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "uart.h"

int _close(int file) { (void)file; return -1; }
int _fstat(int file, struct stat *status) { (void)file; (void)status; return -1; }
int _isatty(int file) { (void)file; return 0; }
off_t _lseek(int file, off_t offset, int whence)
{
    (void)file; (void)offset; (void)whence; return (off_t)-1;
}
ssize_t _read(int file, void *buffer, size_t length)
{
    (void)file; (void)buffer; (void)length; return -1;
}
ssize_t _write(int file, const void *buffer, size_t length)
{
    if (file != STDOUT_FILENO && file != STDERR_FILENO) {
        return -1;
    }
    const char *characters = buffer;
    for (size_t index = 0U; index < length; ++index) {
        if (characters[index] == '\n') {
            uart_putc('\r');
        }
        uart_putc(characters[index]);
    }
    return (ssize_t)length;
}
void *_sbrk(ptrdiff_t increment)
{
    (void)increment;
    errno = ENOMEM;
    return (void *)-1;
}
