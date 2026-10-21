CC = cc
CPPFLAGS = -DOS_HOST_TEST -I../AdidOS.X -Ihost
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -g -fsanitize=address,undefined
SOURCES = test_rtos.c ../AdidOS.X/kernel.c ../AdidOS.X/scheduler.c ../AdidOS.X/sync.c \
          ../AdidOS.X/mem.c ../AdidOS.X/message.c ../AdidOS.X/timer.c ../AdidOS.X/int.c
.PHONY: test clean syntax
test: test_rtos test_io test_demo
	./test_rtos
	./test_io
	./test_demo
test_rtos: $(SOURCES) $(wildcard ../AdidOS.X/*.h) host/xc.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $@
test_io: test_io.c ../AdidOS.X/adc.c ../AdidOS.X/pwm.c ../AdidOS.X/sync.c $(wildcard ../AdidOS.X/*.h) host/xc.h
	$(CC) $(CPPFLAGS) $(CFLAGS) test_io.c ../AdidOS.X/adc.c ../AdidOS.X/pwm.c ../AdidOS.X/sync.c -o $@
DEMO_SOURCES = $(filter-out test_rtos.c,$(SOURCES)) ../AdidOS.X/adc.c ../AdidOS.X/pwm.c ../AdidOS.X/uart.c ../AdidOS.X/user_app.c
test_demo: test_demo.c $(DEMO_SOURCES) $(wildcard ../AdidOS.X/*.h) host/xc.h
	$(CC) $(CPPFLAGS) $(CFLAGS) test_demo.c $(DEMO_SOURCES) -o $@
syntax:
	$(CC) $(CPPFLAGS) $(CFLAGS) -fsyntax-only ../AdidOS.X/main.c ../AdidOS.X/user_app.c ../AdidOS.X/uart.c
clean:
	rm -f test_rtos test_io test_demo
