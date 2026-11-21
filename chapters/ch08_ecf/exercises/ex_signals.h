#ifndef EX_SIGNALS_H
#define EX_SIGNALS_H

/* Block SIGUSR1, raise it, and return 1 iff sigpending shows SIGUSR1.
   Before returning, restore the mask this process had on entry and
   leave SIGUSR1 not pending (sigwait, or SIG_IGN long enough to discard
   it, then put the old disposition back). Do not kill the caller.
*/
int usr1_became_pending(void);

#endif
