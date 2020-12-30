package jelatine;

public class MainLauncher{
	public static void launch(String className,String argv[]) 
		throws ClassNotFoundException, 
		InstantiationException,
        IllegalAccessException
	{
		Class cl=Class.forName(className);
		Object obj=cl.newInstance();
		MainRunner mainObj=(MainRunner)obj;
		mainObj.main(argv);
	}
};
