#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Load libscgreet.so from the directory that holds this executable. */
int main(void) {
    char exe[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof exe - 1);
    if (n < 0 || (size_t)n >= sizeof exe - 1) {
        perror("readlink");
        return 1;
    }
    exe[n] = '\0';

    char *slash = strrchr(exe, '/');
    if (!slash) {
        fprintf(stderr, "demo_dlopen: no directory in %s\n", exe);
        return 1;
    }
    const char *lib = "libscgreet.so";
    if ((size_t)(slash - exe) + 1 + strlen(lib) + 1 > sizeof exe) {
        fprintf(stderr, "demo_dlopen: path too long\n");
        return 1;
    }
    memcpy(slash + 1, lib, strlen(lib) + 1);

    void *handle = dlopen(exe, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        return 1;
    }

    void *sym = dlsym(handle, "greet");
    if (!sym) {
        fprintf(stderr, "dlsym: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    void (*greet)(const char *);
    memcpy(&greet, &sym, sizeof greet);
    greet("dynamic");

    if (dlclose(handle) != 0) {
        fprintf(stderr, "dlclose: %s\n", dlerror());
        return 1;
    }
    return 0;
}
