
public class TestString1{

	public static void main(String argv[]){
		String target="abcdefgh";
		String src="def";
		char []target_ar=target.toCharArray();
		char []src_ar=src.toCharArray();
		System.out.println("Try find:");
		
	int ix=String.lastIndexOf(src_ar,0,src_ar.length,target_ar,0,target_ar.length,20
				);
		System.out.println("Found at:"+ix);
		ix=String.lastIndexOf(target_ar,0,target_ar.length,
				src_ar,0,src_ar.length,20
				);
		System.out.println("Found at:"+ix);
		ix=target.lastIndexOf("de");
		System.out.println("Found at:"+ix);
	}
}