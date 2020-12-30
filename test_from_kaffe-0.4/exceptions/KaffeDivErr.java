class KaffeDivErr {
    public static void main ( String args[] ) {
	double v = 2.0, w = 5.0, W = 0.0;
        W = v / w;
        System.out.println("v = "+v);
        System.out.println("w = "+w);
	System.out.println("The division: W = v / w; yields : W="+W+"\n");

        v = 345.1223111;
        w = 1203.210943;
        W = v / w;
        System.out.println("v = "+v);
        System.out.println("w = "+w);
        System.out.println("The division: W = v / w; yields : W="+W+"\n");

        v = 12345.1223111;
        w = 1203.210943;
        W = v / w;
        System.out.println("v = "+v);
        System.out.println("w = "+w);
        System.out.println("The division: W = v / w; yields : W="+W+"\n");

        v = 12345.1223111;
        w = 1203.210943;
        W = w / v;
        System.out.println("v = "+v);
        System.out.println("w = "+w);
        System.out.println("The division: W = w / v; yields : W="+W+"\n");
    }
}
