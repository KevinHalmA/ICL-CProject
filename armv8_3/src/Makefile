CC      ?= gcc
CFLAGS  ?= -std=c17 -g \
		   -D_POSIX_SOURCE -D_DEFAULT_SOURCE \
		   -Wall -Werror -pedantic \
		   -MMD -MP

.SUFFIXES: .c .o

.PHONY: all clean

ASSEMBLE_DIR 	:= assemble_
EMULATE_DIR  	:= emulate_
COMMON_DIR      := common

EXECS 		 	:= assemble emulate
ASSEMBLE_SRCS 	:= $(wildcard $(ASSEMBLE_DIR)/*.c)
ASSEMBLE_OBJS 	:= $(ASSEMBLE_SRCS:.c=.o)
EMULATE_SRCS 	:= $(wildcard $(EMULATE_DIR)/*.c)
EMULATE_OBJS 	:= $(EMULATE_SRCS:.c=.o)
COMMON_SRCS     := $(wildcard $(COMMON_DIR)/*.c)
COMMON_OBJS 	:= $(COMMON_SRCS:.c=.o)

all: $(EXECS)

assemble: $(ASSEMBLE_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

emulate: $(EMULATE_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(EXECS) *.o */*.o *.d */*.d

-include $(ASSEMBLE_OBJS:.o=.d)
-include $(EMULATE_OBJS:.o=.d)
-include $(COMMON_OBJS:.o=.d)