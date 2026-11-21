/* Make a temporary directory, scan it, and print entry names in sorted order.
 * readdir does not promise alphabetical order, so the names are sorted here.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME_MAX_LOCAL 256
#define MAX_ENTRIES 32

static int cmp_name(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

static int create_file(const char *path) {
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    const char mark[] = "x";
    if (fd < 0) {
        return -1;
    }
    if (write(fd, mark, 1) != 1) {
        close(fd);
        return -1;
    }
    if (close(fd) < 0) {
        return -1;
    }
    return 0;
}

int main(void) {
    char dir[] = "/tmp/scraft-dir-XXXXXX";
    char path_a[512];
    char path_b[512];
    char names[MAX_ENTRIES][NAME_MAX_LOCAL];
    int count = 0;
    DIR *dp;
    struct dirent *ent;
    int i;

    if (!mkdtemp(dir)) {
        perror("mkdtemp");
        return 1;
    }
    if (snprintf(path_a, sizeof(path_a), "%s/alpha.dat", dir) >= (int)sizeof(path_a) ||
        snprintf(path_b, sizeof(path_b), "%s/beta.dat", dir) >= (int)sizeof(path_b)) {
        fprintf(stderr, "path too long\n");
        rmdir(dir);
        return 1;
    }
    if (create_file(path_a) < 0 || create_file(path_b) < 0) {
        perror("create");
        unlink(path_a);
        unlink(path_b);
        rmdir(dir);
        return 1;
    }

    dp = opendir(dir);
    if (!dp) {
        perror("opendir");
        unlink(path_a);
        unlink(path_b);
        rmdir(dir);
        return 1;
    }
    while ((ent = readdir(dp)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        if (count >= MAX_ENTRIES || strlen(ent->d_name) >= NAME_MAX_LOCAL) {
            fprintf(stderr, "too many directory entries\n");
            closedir(dp);
            unlink(path_a);
            unlink(path_b);
            rmdir(dir);
            return 1;
        }
        memcpy(names[count], ent->d_name, strlen(ent->d_name) + 1);
        count++;
    }
    if (closedir(dp) < 0) {
        perror("closedir");
    }
    qsort(names, (size_t)count, NAME_MAX_LOCAL, cmp_name);
    for (i = 0; i < count; i++) {
        printf("%s\n", names[i]);
    }

    unlink(path_a);
    unlink(path_b);
    if (rmdir(dir) < 0) {
        perror("rmdir");
        return 1;
    }
    if (count != 2) {
        fprintf(stderr, "expected 2 names, got %d\n", count);
        return 1;
    }
    printf("dir-ok\n");
    return 0;
}
