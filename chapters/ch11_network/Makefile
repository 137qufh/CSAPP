include ../../common/flags.mk

CFLAGS += -I../../common/include -Iexercises

.PHONY: all test test-solution clean

DEMOS = demo_byteorder demo_getaddr demo_echo demo_http_server demo_http_client demo_http_demo

all: $(DEMOS) www/cgi-bin/hello

demo_byteorder: demo_byteorder.c
	$(CC) $(CFLAGS) -o $@ $<

demo_getaddr: demo_getaddr.c
	$(CC) $(CFLAGS) -o $@ $<

demo_echo: demo_echo.c
	$(CC) $(CFLAGS) -o $@ $<

demo_http_server: demo_http_server.c http_server.c http_client.c
	$(CC) $(CFLAGS) -o $@ demo_http_server.c http_server.c http_client.c

demo_http_client: demo_http_client.c http_client.c http_server.c
	$(CC) $(CFLAGS) -o $@ demo_http_client.c http_client.c http_server.c

demo_http_demo: demo_http_demo.c http_server.c http_client.c www/cgi-bin/hello
	$(CC) $(CFLAGS) -o $@ demo_http_demo.c http_server.c http_client.c

www/cgi-bin/hello: www/cgi-bin/hello.c
	mkdir -p www/cgi-bin
	$(CC) $(CFLAGS) -o $@ $<

test_student: tests/test_parse.c exercises/ex_parse.c
	$(CC) $(CFLAGS) -o $@ $^

test_solution_bin: tests/test_parse.c solutions/ex_parse.c
	$(CC) $(CFLAGS) -o $@ $^

test: test_student
	./test_student

test-solution: test_solution_bin $(DEMOS) www/cgi-bin/hello
	./test_solution_bin
	./demo_byteorder
	./demo_getaddr
	./demo_echo
	./demo_http_server
	./demo_http_client
	./demo_http_demo

clean:
	rm -f $(DEMOS) test_student test_solution_bin www/cgi-bin/hello
