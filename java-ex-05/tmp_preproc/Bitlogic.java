class Bitlogic {

public static void main(String args []) {

char []__wrap_const_string14={'1','1','0','1'};

 

String binary[] = {

char []__wrap_const_string16={'1','1','1','1'};

 (new String(__wrap_const_string1) ), (new String(__wrap_const_string2) ), (new String(__wrap_const_string3) ), (new String(__wrap_const_string4) ), (new String(__wrap_const_string5) ), (new String(__wrap_const_string6) ), (new String(__wrap_const_string7) ), (new String(__wrap_const_string8) ), (new String(__wrap_const_string9) ), (new String(__wrap_const_string10) ),	(new String(__wrap_const_string11) ), (new String(__wrap_const_string12) ), (new String(__wrap_const_string13) ), (new String(__wrap_const_string14) ),
(new String(__wrap_const_string15) ), (new String(__wrap_const_string16) ) };
int a = 3;	//	0+2+1	или двоичное 0011

int b = 6;	//	4+2+0	или двоичное 0110

int c = a | b;

int d = a & b;

int e = a ^ b;

int f = (~a & b) | (a & ~b);

int g = ~a & 0x0f;

char []__wrap_const_string17={' ','a',' ','=',' '};



System.out.println((new String(__wrap_const_string17) ) + binary[a]);

char []__wrap_const_string18={' ','b',' ','=',' '};


System.out.println((new String(__wrap_const_string18) ) + binary[b]);

char []__wrap_const_string19={' ','a','b',' ','=',' '};


System.out.println((new String(__wrap_const_string19) ) + binary[c]);

char []__wrap_const_string20={' ','a','&','b',' ','=',' '};


System.out.println((new String(__wrap_const_string20) ) + binary[d]);

char []__wrap_const_string21={' ','a','^','b',' ','=',' '};


System.out.println((new String(__wrap_const_string21) ) + binary[e]);

char []__wrap_const_string22={'~','a','&','b',' ','|',' ','а','^','~','b',' ','=',' '};


System.out.println((new String(__wrap_const_string22) ) + binary[f]);

char []__wrap_const_string23={' ','~','a',' ','=',' '};


System.out.println((new String(__wrap_const_string23) ) + binary[g]);
} }

