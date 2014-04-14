#

CFLAGS := -g -c -D_GNU_SOURCE
SH_LIB := -shared
LFLAGS := -Wl,-rpath='$$ORIGIN'

default: all

%.o: %.c
	@echo "Compiling.. $<"
	@$(CC) $(CFLAGS) $<

libtml.so: tml.o
	@echo "Buiding.. $@"
	@$(CC) $(SH_LIB) -ldl -o $@ $<

all: libtml.so

clean:
	$(RM) *.o libtml.so
