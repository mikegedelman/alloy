CC := i686-alloy-gcc
CFLAGS := -g

all: user.exe

user.exe: main.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm user.exe