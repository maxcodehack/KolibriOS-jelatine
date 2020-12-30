public class T16 implements Runnable
{
 public static void main(String args[])
 {
  T16 inst = new T16();
  for (int i = 0;; i++)
  {
   Thread t = new Thread(inst, "" + i);
      t.start();
      try
   {
    t.join();
   }  catch(Exception e)
   {
    System.out.println("Thread join interrupted??");
   }
  }
 }

 public void run()
 {
  System.out.println(Thread.currentThread().getName());
   // try { Thread.sleep(20); } catch (Exception e) { /*NOP */ 
 }
}
