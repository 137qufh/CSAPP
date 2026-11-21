#include <setjmp.h>
#include <stdio.h>

static sigjmp_buf jump_env;

static void dive(void) {
    siglongjmp(jump_env, 42);
}

/* setjmp returns once with 0. siglongjmp comes back with 42. */
int main(void) {
    jmp_buf once_env;
    int once = setjmp(once_env);
    if (once != 0) {
        return 1;
    }
    printf("setjmp\n");

    int jumped = sigsetjmp(jump_env, 1);
    if (jumped == 0) {
        dive();
        return 1;
    }
    printf("jumped=%d\n", jumped);
    return 0;
}
