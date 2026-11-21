/*
 * Runtime interposition via LD_PRELOAD. The dynamic loader searches
 * this library before libc, so the executable's malloc calls land here.
 *
 * dlsym can itself allocate before the real malloc pointer is known.
 * Those early blocks come from a static buffer; free ignores them.
 * The counter only includes callers whose object name is not a .so,
 * which keeps libc and this library out of the total.
 */
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void *(*real_malloc)(size_t) = NULL;
static void (*real_free)(void *) = NULL;
static int malloc_count = 0;
static int in_count = 0;

static char early[8192];
static size_t early_used = 0;

static int ptr_in_early(const void *p) {
    const char *c = p;
    return c >= early && c < early + sizeof early;
}

static void *early_alloc(size_t n) {
    size_t align = (early_used + 15u) & ~(size_t)15u;
    if (n > sizeof early || align > sizeof early - n) {
        return NULL;
    }
    void *p = early + align;
    early_used = align + n;
    return p;
}

static void note(void *caller) {
    if (in_count) {
        return;
    }
    in_count = 1;
    Dl_info info;
    if (dladdr(caller, &info) && info.dli_fname) {
        const char *base = strrchr(info.dli_fname, '/');
        base = base ? base + 1 : info.dli_fname;
        if (strstr(base, ".so") == NULL) {
            malloc_count++;
        }
    }
    in_count = 0;
}

void *malloc(size_t n) {
    if (!real_malloc) {
        return early_alloc(n);
    }
    void *p = real_malloc(n);
    note(__builtin_return_address(0));
    return p;
}

void free(void *p) {
    if (p == NULL || ptr_in_early(p)) {
        return;
    }
    if (!real_free) {
        return;
    }
    real_free(p);
}

__attribute__((constructor)) static void init(void) {
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_free = dlsym(RTLD_NEXT, "free");
    if (!real_malloc || !real_free) {
        const char msg[] = "preload: dlsym failed\n";
        ssize_t w = write(STDERR_FILENO, msg, sizeof msg - 1);
        if (w < 0) {
            /* stderr is unavailable; the exit status still reports failure */
        }
        _exit(1);
    }
}

__attribute__((destructor)) static void report(void) {
    fprintf(stderr, "preload-mallocs=%d\n", malloc_count);
}
