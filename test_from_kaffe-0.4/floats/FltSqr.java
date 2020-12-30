class FltSqr {
	public static void main(String argv[]) {
            int i;
            double s = 0;

            for (i=1; i<1000000; i++) 
                s+= 1.0/Math.sqrt(i);

            System.out.println("s = " + s  + "\n");
        }
}
