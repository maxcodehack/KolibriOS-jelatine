class ByteUShift	{

static public void main(String args[]) {

char hex[] = { '0', '1', '2', '3', '4',	'5', '6', '7', '8', '9', ' ', 'b', 'á', 'd',	'e', 'f' };

byte b = (byte) 0xf1;

byte c = (byte) (b >> 4);

byte d = (byte) (b >> 4);

byte e = (byte) ((b & 0xff) >> 4);

char []__wrap_const_string1={' ','b',' ','=',' ','0','x'};



System.out.println((new String(__wrap_const_string1) ) + hex[(b >> 4) & 0x0f] + hex[b & 0x0f]);

char []__wrap_const_string2={' ','b',' ','>','>',' ','4',' ','=','\t','0','x'};


System.out.println((new String(__wrap_const_string2) ) + hex[(c >> 4) & 0x0f] + hex[c & 0x0f]);

char []__wrap_const_string3={'b',' ','>','>','>',' ','4',' ','=',' ','0','x'};


System.out.println((new String(__wrap_const_string3) ) + hex[(d >> 4) & 0x0f] + hex[d & 0x0f]);

char []__wrap_const_string4={'(','b',' ','&',' ','0','x','f','f',')',' ','>','>',' ','4',' ','=',' ','0','x'};


System.out.println((new String(__wrap_const_string4) ) + hex[(e >> 4) & 0x0f] + hex[e & 0x0f]);
} }

