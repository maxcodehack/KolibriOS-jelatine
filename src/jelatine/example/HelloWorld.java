import lejos.nxt.LCD;
import lejos.nxt.Button;

/**
 * A simple hello world program.
 * 
 * @author Lawrie Griffiths
 */
public class HelloWorld {
	public static void main(String[] aArg) throws Exception {
		LCD.drawString("Hello World", 3, 4);
		LCD.refresh();
		int but_ids;
		do{
			but_ids=Button.readButtons();
			//wait Button Escape
		}while((but_ids & Button.ID_ESCAPE)==0);
		Thread.sleep(2000);
		
	}
}