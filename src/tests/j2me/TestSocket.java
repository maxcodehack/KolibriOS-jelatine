
import javax.microedition.io.*;
import java.io.*;
import jelatine.cldc.io.*;

public class TestSocket{

	public TestSocket(){
	}
	public static void println(String s){
		System.out.println(s);
	}
	public void test(){
		try{
			System.out.println("try open");
			int port=5000;
			//SocketConnection socket;
			//socket=(SocketConnection)Connector.open("socket://127.0.0.1/"+port);
			StreamConnection socket;
			String s_url="socket://127.0.0.1:"+port;
			URL url=new URL(s_url);
			println("name:"+url.getName());
			println("scheme:"+url.getScheme());
			println("target:"+url.getTarget());
			socket=(StreamConnection)Connector.open(s_url);
			//socket=(StreamConnection)jelatine.cldc.io.socket.ProtocolImpl.open();
			System.out.println("try get io streams...");
			//socket.setSocketOption(SocketConnection.LINGER, 5);
			InputStream is  = socket.openInputStream();
			OutputStream os = socket.openOutputStream();
			System.out.println("try write sample string...");
			os.write("HelloWorld!\n".getBytes());
			os.flush();
			byte bytes[]=new byte[10];
			System.out.println("try read...");
			is.read(bytes);
			String s=new String(bytes);
			System.out.println("ok read:"+s);
			
			os.close();
			is.close();
			socket.close();
			System.out.println("ok close streams...");
		}catch(Exception err){
			System.out.println("Error!"+err+":"+err.getMessage());
			err.printStackTrace();
		}
		System.out.println("Ok quit!");
	}
	public static void main(String argv[]){
		TestSocket tsocket=new TestSocket();
		tsocket.test();
	
	}

}