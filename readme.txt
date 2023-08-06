Get JAVA 1.5 at
http://ftp.scientificlinux.org/linux/contrib/obsolete/java/jdk-1.5.0-fcs.i586.rpm
install:
sudo rpm -ivh --nodeps jdk-1.5.0-fcs.i586.rpm

Build classpath:
cd classpath
./build.sh
classpath/bin is your classpath
