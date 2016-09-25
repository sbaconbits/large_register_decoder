

C_OPTS=-g -Wall
LD_OPTS=-lconfig -lm

all: lrd


C_FILES=config_parser.c \
		register_info.c \
		lrd.c

lrd: lrd.c config_parser.c
	gcc $(C_OPTS) -o lrd $(C_FILES) $(LD_OPTS)

todo:
	grep -n TODO *.c *.h

clean:
	rm -f lrd


