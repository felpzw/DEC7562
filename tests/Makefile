CC = cc
CPPFLAGS = -DOS_HOST_TEST -I../AdidOS.X -Ihost
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -g -fsanitize=address,undefined
SOURCES = test_rtos.c ../AdidOS.X/kernel.c ../AdidOS.X/scheduler.c ../AdidOS.X/sync.c \
          ../AdidOS.X/mem.c ../AdidOS.X/message.c ../AdidOS.X/timer.c ../AdidOS.X/int.c
.PHONY: test clean
test: test_rtos
	./test_rtos
test_rtos: $(SOURCES) $(wildcard ../AdidOS.X/*.h) host/xc.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $@
clean:
	rm -f test_rtos
