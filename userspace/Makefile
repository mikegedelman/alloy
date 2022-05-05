user.exe: main.c
	i686-alloy-gcc -g main.c -o user.exe
	cp user.exe /Volumes/NO\ NAME/USER.EXE
	sync

all: user.exe

clean:
	rm user.exe

.PHONY: clean
