
all:preverifier
OBJS=check_class.o  classloader.o    convert_md.o  inlinejsr.o  jar_support.o  stubs.o        utf.o \
check_code.o   classresolver.o  file.o        jar.o        main.o         sys_support.o  util.o


CFLAGS=
#-std=c99
LDFLAGS=-lglib-2.0.dll -lm -lpthread -liconv -lwsock32 
#-lmingwex
%.o:%.c
	gcc -DHAVE_CONFIG_H -I. -c $(CFLAGS) $< -o $@
preverifier:$(OBJS) 
	gcc -o $@ $(OBJS) $(LDFLAGS)
clean:
	rm -f preverifier $(OBJS)
install_win32:preverifier
	mkdir -p c:/Programs/jelatine/classpath/
	mkdir -p c:/Programs/jelatine/bin/
	install preverifier c:/Programs/jelatine/bin/
install:preverifier
	install $< /usr/local/bin
	
