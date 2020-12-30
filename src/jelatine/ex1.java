
public class ex1{

	public static void main(String[] argv){
		System.out.println("try create array");
		byte[] ar=new byte[256];
		for (int i=0;i<10;i++) ar[i]=(byte)(i+65);
		System.out.println("Ok create array");
		System.out.println("str:"+new String(ar));
	} 
}