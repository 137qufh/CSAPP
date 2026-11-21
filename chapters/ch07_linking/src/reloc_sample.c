/* External names still need a relocation in the .o file. */
extern int shared_counter;
extern int bump(int x);

int use_external(int n) {
    shared_counter += n;
    return bump(shared_counter);
}
