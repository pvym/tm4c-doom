#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

extern char __heap_start__;
extern char __heap_end__;

static char *s_heap_end;

int _close(int file)
{
    (void)file;
    return 0;
}

int _fstat(int file, struct stat *st)
{
    (void)file;

    if (st == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _open(const char *name, int flags, int mode)
{
    (void)name;
    (void)flags;
    (void)mode;
    errno = ENOSYS;
    return -1;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

caddr_t _sbrk(int incr)
{
    char *prev_heap_end;
    char *new_heap_end;

    if (s_heap_end == NULL)
    {
        s_heap_end = &__heap_start__;
    }

    prev_heap_end = s_heap_end;
    new_heap_end = s_heap_end + incr;

    if ((new_heap_end < &__heap_start__) || (new_heap_end > &__heap_end__))
    {
        errno = ENOMEM;
        return (void *)-1;
    }

    s_heap_end = new_heap_end;
    return (caddr_t) prev_heap_end;
}

int _write(int file, const char *ptr, int len)
{
    (void)file;
    (void)ptr;
    return len;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _getpid(void)
{
    return 1;
}

void _exit(int status)
{
    (void)status;

    while (1)
    {
    }
}
