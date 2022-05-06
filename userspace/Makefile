CC := i686-alloy-gcc
CFLAGS := -g

all: user.exe write.exe

user.exe: main.c
	$(CC) $(CFLAGS) $^ -o $@
	cp user.exe /Volumes/NO\ NAME/USER.EXE
	sync

write.exe: write.c
	$(CC) $(CFLAGS) $^ -o $@
	cp write.exe /Volumes/NO\ NAME/WRITE.EXE
	sync

clean:
	rm user.exe
