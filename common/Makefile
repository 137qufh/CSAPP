include flags.mk

LIB = libscraft.a
OBJS = src/sc_error.o src/sc_bio.o src/sc_time.o

.PHONY: all test test-solution clean

all: $(LIB) test_bio

$(LIB): $(OBJS)
	ar rcs $@ $(OBJS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -I$(SC_INC) -c $< -o $@

test_bio: tests/test_bio.c $(LIB)
	$(CC) $(CFLAGS) -I$(SC_INC) -o $@ tests/test_bio.c $(LIB)

test: test_bio
	./test_bio

test-solution: test

clean:
	rm -f $(OBJS) $(LIB) test_bio
