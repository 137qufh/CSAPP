#ifndef EX_FORKCOUNT_H
#define EX_FORKCOUNT_H

/* How many processes exist after the pattern, counting the original.
   Return the number. Do not call fork from this function: the tests
   run inside one process and must stay that way.

   pattern 1:
       fork(); fork(); fork();
   pattern 2:
       if (fork() == 0) { fork(); }
   pattern 3:
       for (int i = 0; i < 4; i++) { if (fork() == 0) break; }
*/
int pattern_processes(int id);

#endif
