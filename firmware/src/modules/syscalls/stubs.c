#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

int _close(int fd __attribute__((unused)))
{
    errno = ENOSYS;
    return -1;
}

int _fstat(int fd __attribute__((unused)), struct stat *st __attribute__((unused)))
{
    errno = ENOSYS;
    return -1;
}

int _isatty(int fd __attribute__((unused)))
{
    errno = ENOSYS;
    return 0;
}

int _lseek(int fd __attribute__((unused)), int offset __attribute__((unused)), int whence __attribute__((unused)))
{
    errno = ENOSYS;
    return -1;
}

int _read(int fd __attribute__((unused)), void *buf __attribute__((unused)), size_t count __attribute__((unused)))
{
    errno = ENOSYS;
    return -1;
}

int _kill(int pid __attribute__((unused)), int sig __attribute__((unused)))
{
    errno = EINVAL;
    return -1;
}

int _getpid(void)
{
    return 1;
}
