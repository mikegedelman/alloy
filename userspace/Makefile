CC := i686-alloy-gcc
CFLAGS := -g

all: user.exe

user.exe: main.c
	$(CC) $(CFLAGS) $^ -o $@
	cp user.exe /Volumes/NO\ NAME/USER.EXE
	sync

clean:
	rm user.exe
