CC = kos32-gcc
LD = kos32-ld

SDK_DIR = /home/autobuild/tools/win32/sdk

CFLAGS = -std=c99 -DHAVE_CONFIG_H -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I. -I $(SDK_DIR)/sources/SDL-1.2.2_newlib/include
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

OBJS =	array.o \
		bytecode.o \
		class.o \
		classfile.o \
		constantpool.o \
		field.o \
		header.o \
		interpreter.o \
		jstring.o \
		kni.o \
		loader.o \
		main.o \
		memory.o \
		method.o \
		native.o \
		print.o \
		thread.o \
		utf8_string.o \
		util.o \
		verifier.o \
		vm.o \
		native_exts/lejos_nxj_ext.o

all: $(OBJS) jelatine

jelatine: $(OBJS)
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem native -o $@ $(OBJS) -lSDLn -lsound -lgcc -lc.dll
	objcopy -O binary $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
	
clean:
	rm -f $(OBJS)
