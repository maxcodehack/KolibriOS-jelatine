srcdir="./"
dstdir=c:/Programs/jelatine/classpath/
javac -classpath $srcdir -bootclasspath $srcdir -d $dstdir -source 1.4 `find ./ -iname '*.java'` 
