package jelatine.midp_emu;

import jelatine.*;

public class MIDletLauncher {
	public static void main(String argv[]){
		String mainClass=
		try{
			MainLauncher.launch("",argv);
		}catch(Exception err){
			System.err.println("Error!"+err+ " msg:"+err.getMessage());
		}
	}
	
}