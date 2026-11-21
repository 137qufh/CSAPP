#ifndef MINITEST_H
#define MINITEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Tiny single-file test runner. One translation unit should include this. */

static int mt_passed = 0;
static int mt_failed = 0;

#define MT_ASSERT(cond)                                                         \
    do {                                                                        \
        if (cond) {                                                             \
            mt_passed++;                                                        \
        } else {                                                                \
            mt_failed++;                                                        \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);    \
        }                                                                       \
    } while (0)

#define MT_EQ_INT(a, b)                                                         \
    do {                                                                        \
        long long _mt_a = (long long)(a);                                      \
        long long _mt_b = (long long)(b);                                      \
        if (_mt_a == _mt_b) {                                                   \
            mt_passed++;                                                        \
        } else {                                                                \
            mt_failed++;                                                        \
            fprintf(stderr,                                                     \
                    "FAIL %s:%d: %s == %s (%lld vs %lld)\n",                    \
                    __FILE__,                                                   \
                    __LINE__,                                                   \
                    #a,                                                         \
                    #b,                                                         \
                    _mt_a,                                                      \
                    _mt_b);                                                     \
        }                                                                       \
    } while (0)

#define MT_EQ_UINT(a, b)                                                        \
    do {                                                                        \
        unsigned long long _mt_a = (unsigned long long)(a);                    \
        unsigned long long _mt_b = (unsigned long long)(b);                    \
        if (_mt_a == _mt_b) {                                                   \
            mt_passed++;                                                        \
        } else {                                                                \
            mt_failed++;                                                        \
            fprintf(stderr,                                                     \
                    "FAIL %s:%d: %s == %s (%llu vs %llu)\n",                    \
                    __FILE__,                                                   \
                    __LINE__,                                                   \
                    #a,                                                         \
                    #b,                                                         \
                    _mt_a,                                                      \
                    _mt_b);                                                     \
        }                                                                       \
    } while (0)

#define MT_EQ_STR(a, b)                                                         \
    do {                                                                        \
        const char *_mt_a = (a);                                               \
        const char *_mt_b = (b);                                               \
        if (_mt_a && _mt_b && strcmp(_mt_a, _mt_b) == 0) {                      \
            mt_passed++;                                                        \
        } else {                                                                \
            mt_failed++;                                                        \
            fprintf(stderr,                                                     \
                    "FAIL %s:%d: %s == %s (\"%s\" vs \"%s\")\n",                \
                    __FILE__,                                                   \
                    __LINE__,                                                   \
                    #a,                                                         \
                    #b,                                                         \
                    _mt_a ? _mt_a : "(null)",                                   \
                    _mt_b ? _mt_b : "(null)");                                  \
        }                                                                       \
    } while (0)

#define MT_REPORT()                                                             \
    do {                                                                        \
        printf("%d passed, %d failed\n", mt_passed, mt_failed);                \
        return mt_failed ? 1 : 0;                                              \
    } while (0)

#endif
