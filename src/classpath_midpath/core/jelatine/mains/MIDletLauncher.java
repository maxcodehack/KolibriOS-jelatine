package jelatine.mains;

import jelatine.*;

public class MIDletLauncher {
	public static void main(String argv[]){
	String mainClass;
	//mainClass="org.thenesis.midpath.main.MIDletLauncher";
	mainClass="jelatine.mains.MIDletLauncherImpl";
		try{
			MainLauncher.launch(mainClass,argv);
		}catch(Exception err){
			System.err.println("Error!"+err+ " msg:"+err.getMessage());
		}
	}
	
};