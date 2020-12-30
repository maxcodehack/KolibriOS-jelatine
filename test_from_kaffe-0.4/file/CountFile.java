import java.io.*;

class CountFile {

        public static void main ( String args[] )
        throws  java.io.IOException,
                java.io.FileNotFoundException
        {

                int count = 0 ;
                int tmp = 0 ;
                InputStream is;
                byte[]  buffer = new byte[1024] ;

                if ( args.length == 1 ) {
                        is = new FileInputStream(args[0]) ;
                } else {
                        is = System.in ;
                }

                while ( ( tmp = is.read(buffer) ) != -1 ) {
                        count += tmp ;
                        System.out.println( "Read " + tmp + " chars" ) ;
                }

                System.out.println ( "Input has " + count + " chars" ) ;
        }
}
