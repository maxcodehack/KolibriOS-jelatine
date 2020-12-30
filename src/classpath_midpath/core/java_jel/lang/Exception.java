package java.lang;

public class Exception extends Throwable
{
	public Exception() {
		super();
	}
	
	public Exception(String message) {
		super(message);
	}
    
    public Exception(String message, Throwable cause)
    {
        //my mod
		//super(message, cause);
		
		super(message);
    }
    
    public Exception(Throwable cause)
    {
        super(cause);
    }
}
