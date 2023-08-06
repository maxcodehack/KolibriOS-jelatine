srcdir=src
dstdir=bin
mkdir bin
/usr/java/jdk1.5.0/bin/javac -classpath $srcdir -bootclasspath $srcdir -d $dstdir -source 1.4 `find ./ -iname '*.java'` 
