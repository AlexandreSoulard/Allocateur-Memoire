CC=gcc

# uncomment to compile in 32bits mode (require gcc-*-multilib packages
# on Debian/Ubuntu)
#HOST32= -m32

CFLAGS+= $(HOST32) -Wall -Werror -std=c99 -g -DMEMORY_SIZE=128000
CFLAGS+= -DDEBUG
# pour tester avec ls
CFLAGS+= -fPIC
LDFLAGS= $(HOST32)
TEST+=test
TESTS_CMD+=test_ls test_dir test_date test_env test_locale test_id test_find test_cat
TESTS+= $(TESTS_CMD) $(TEST)
PROGRAMS=memshell $(TEST)

.PHONY: clean all tests tests_cmd

all: $(PROGRAMS) libmalloc.so

tests: $(TEST)
	for file in $(TESTS);do ./$$file; done

tests_cmd: $(TESTS_CMD)
	for file in $(TESTS_CMD);do ./$$file; done

%.o: %.c
	$(CC) -c $(CFLAGS) -MMD -MF .$@.deps -o $@ $<

# dépendences des binaires
$(PROGRAMS) libmalloc.so: %: mem.o common.o

-include $(wildcard .*.deps)

# seconde partie du sujet
libmalloc.so: malloc_stub.o
	$(CC) -shared -Wl,-soname,$@ $^ -o $@

test_ls: libmalloc.so
	LD_PRELOAD=./libmalloc.so ls

test_dir: libmalloc.so
	LD_PRELOAD=./libmalloc.so dir

test_date: libmalloc.so
	LD_PRELOAD=./libmalloc.so date

test_env: libmalloc.so
	LD_PRELOAD=./libmalloc.so env

test_cat: libmalloc.so
	LD_PRELOAD=./libmalloc.so cat

test_locale: libmalloc.so
	LD_PRELOAD=./libmalloc.so locale

test_id: libmalloc.so
	LD_PRELOAD=./libmalloc.so id

test_find: libmalloc.so
	LD_PRELOAD=./libmalloc.so find


# nettoyage
clean:
	$(RM) *.o $(PROGRAMS) libmalloc.so .*.deps
