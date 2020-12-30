class Ternary {
public static void main(String args[]) {
int a = 42;
int b = 2;
int c = 99;
int d = 0;
int e = (b == 0) ? 0 : (a / b);
int f = (d == 0) ? 0 : (c / d);
System.out.println("a = " + a);
System.out.println("b = " + b);
System.out.println("c = " + c);
System.out.println("d = " + d);
System.out.println("a / b = " + e);
System.out.println("c / d = " + f);
} }
