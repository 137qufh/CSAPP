#ifndef EX_SYMBOLS_H
#define EX_SYMBOLS_H

/* Tiny model of one symbol resolved across two object files.
   kind: 0 = undefined, 1 = weak, 2 = strong.
   resolve_one returns 1 when the link succeeds and 0 on error.
   On success, *winner is 0 or 1: the file whose definition is kept.
   Rules:
   - two strong -> error (return 0)
   - strong and weak -> strong wins
   - two weak -> file 0 wins
   - undefined plus strong or weak -> the defined file wins
   - both undefined -> error
   Do not call the linker. Decide from the two kind values only.
*/
int resolve_one(int kind0, int kind1, int *winner);

#endif
