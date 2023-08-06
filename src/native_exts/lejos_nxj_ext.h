//Base of this source been was generated by jni_interfaces_generator.py

#include "wrappers.h"

#include "interpreter.h"
#include "jstring.h"
#include "kni.h"
#include "loader.h"
#include "native.h"
#include "thread.h"
#include "utf8_string.h"
#include "util.h"
#include "vm.h"

#include "java_lang_String.h"
#include "java_lang_Thread.h"

enum NXJButtonKey{
	ID_ENTER =	1,
	ID_ESCAPE = 8,
	ID_LEFT =	2,
	ID_RIGHT =	4
};
/**
 * NXTEvent class as native structure
 */
typedef struct S_NXTEvent
{
  //Object _super;	     // Superclass object storage
  struct S_NXTEvent *sync;
  header_t header;
  jint state;
 
  jint updatePeriod;
  jint updateCnt;
  jint typ;
  jint filter;
  jint eventData;
  jint userEvents;
} NXTEvent;

#define NXT_EVENT_WAITING 1

//TO DO NXJ functions 

#if 0
void monitor_notify_unchecked(jobject//Object *
	obj, jboolean//const boolean
 all);
#if FAST_DISPATCH
typedef const unsigned short DISPATCH_LABEL;
extern DISPATCH_LABEL * volatile dispatchTable;
extern DISPATCH_LABEL *checkEvent;
#define FORCE_EVENT_CHECK() (gMakeRequest = true, dispatchTable = checkEvent)
#else
#define FORCE_EVENT_CHECK() (gMakeRequest = true)
#endif

static inline void schedule_request (const byte aCode)
{
  FORCE_EVENT_CHECK();
  gRequestCode = aCode;
}
#endif 
//

#define get_monitor_count(SYNC_)  ((SYNC_)->monitorCount)



//r:is uintptr_t or (char*)!!
#define NXTEvent_REF2PTR(r) ((NXTEvent*)(r-offsetof(NXTEvent, header))) 
#define NXTEvent_PTR2REF(ptr) (((uintptr_t)r)+offsetof(NXTEvent, header))
	
 //JNIFunc={  static long java/lang/Double:: doubleToRawLongBits (double d);}
//public static native long doubleToRawLongBits(double d)
extern KNI_RETURNTYPE_LONG java_lang_Double_doubleToRawLongBits(void);
 //JNIFunc={  static double java/lang/Double:: longBitsToDouble (long l);}
//public static native double longBitsToDouble(long l)
#if 0
extern KNI_RETURNTYPE_DOUBLE java_lang_Double_longBitsToDouble(void);
#endif
 //JNIFunc={  static int java/lang/Float:: floatToRawIntBits (float value);}
//public static native int floatToRawIntBits(float value)
extern KNI_RETURNTYPE_INT java_lang_Float_floatToRawIntBits(void);
 //JNIFunc={  static float java/lang/Float:: intBitsToFloat (int value);}
//public static native float intBitsToFloat(int value)
	//extern KNI_RETURNTYPE_FLOAT java_lang_Float_intBitsToFloat(void);
 //JNIFunc={  static Object java/lang/Object:: cloneObject (Object old);}
//private static final native Object cloneObject(Object old)
extern KNI_RETURNTYPE_OBJECT java_lang_Object_cloneObject(void);
 //JNIFunc={  void java/lang/Object:: notify ();}
//public final native void notify()
	//extern KNI_RETURNTYPE_VOID java_lang_Object_notify(void);
 //JNIFunc={  void java/lang/Object:: notifyAll ();}
//public final native void notifyAll()
	//extern KNI_RETURNTYPE_VOID java_lang_Object_notifyAll(void);
 //JNIFunc={  void java/lang/Object:: wait ();}
//public final native void wait() throws InterruptedException
extern KNI_RETURNTYPE_VOID java_lang_Object_wait(void);
 //JNIFunc={  void java/lang/Object:: wait (long timeout);}
//public final native void wait(long timeout) throws InterruptedException
extern KNI_RETURNTYPE_VOID java_lang_Object_wait(void);
 //JNIFunc={  long java/lang/Runtime:: freeMemory ();}
//public native long freeMemory()
	//extern KNI_RETURNTYPE_LONG java_lang_Runtime_freeMemory(void);

//JNIFunc={  long java/lang/Runtime:: totalMemory ();}
//public native long totalMemory()
#if 0
extern KNI_RETURNTYPE_LONG java_lang_Runtime_totalMemory(void);
#endif
//JNIFunc={  static void java/lang/Shutdown:: halt (int code);}
//public static native void halt(int code)
extern KNI_RETURNTYPE_VOID java_lang_Shutdown_halt(void);
 //JNIFunc={  static void java/lang/Shutdown:: shutdown ();}
//private static native void shutdown()
extern KNI_RETURNTYPE_VOID java_lang_Shutdown_shutdown(void);
 //JNIFunc={  static void java/lang/System:: arraycopy (Object src,int srcOffset,Object dest,int destOffset,int length);}
//public static native void arraycopy (Object src, int srcOffset, Object dest, int destOffset, int length)
#if 0
extern KNI_RETURNTYPE_VOID java_lang_System_arraycopy(void);
#endif
 //JNIFunc={  static long java/lang/System:: currentTimeMillis ();}
//public static native long currentTimeMillis()
#if 0
extern KNI_RETURNTYPE_LONG java_lang_System_currentTimeMillis(void);
#endif 
//JNIFunc={  static int java/lang/System:: getDataAddress (Object obj);}
//private native static int getDataAddress (Object obj)
extern KNI_RETURNTYPE_INT java_lang_System_getDataAddress(void);
 //JNIFunc={  static void java/lang/System:: gc ();}
//public static native void gc()
extern KNI_RETURNTYPE_VOID java_lang_System_gc(void);
 //JNIFunc={  static long java/lang/System:: nanoTime ();}
//public static native long nanoTime()
extern KNI_RETURNTYPE_LONG java_lang_System_nanoTime(void);
 //JNIFunc={  void java/lang/Thread:: start ();}
//public final native void start()
#if 0
extern KNI_RETURNTYPE_VOID java_lang_Thread_start(void);
#endif
 //JNIFunc={  static void java/lang/Thread:: yield ();}
//public static native void yield()
#if 0
extern KNI_RETURNTYPE_VOID java_lang_Thread_yield(void);
#endif
 //JNIFunc={  static void java/lang/Thread:: sleep (long aMilliseconds);}
//public static native void sleep (long aMilliseconds) throws InterruptedException
#if 0
extern KNI_RETURNTYPE_VOID java_lang_Thread_sleep(void);
#endif
 //JNIFunc={  static Thread java/lang/Thread:: currentThread ();}
//public static native Thread currentThread()
#if 0
extern KNI_RETURNTYPE_OBJECT java_lang_Thread_currentThread(void);
#endif
 //JNIFunc={  int java/lang/Thread:: getPriority ();}
//public final native int getPriority()
extern KNI_RETURNTYPE_INT java_lang_Thread_getPriority(void);
 //JNIFunc={  void java/lang/Thread:: setPriority (int priority);}
//public final native void setPriority(int priority)
extern KNI_RETURNTYPE_VOID java_lang_Thread_setPriority(void);
 //JNIFunc={  void java/lang/Thread:: interrupt ();}
//public native void interrupt()
#if 0
extern KNI_RETURNTYPE_VOID java_lang_Thread_interrupt(void);
#endif
 //JNIFunc={  static boolean java/lang/Thread:: interrupted ();}
//public static native boolean interrupted()
extern KNI_RETURNTYPE_BOOLEAN java_lang_Thread_interrupted(void);
 //JNIFunc={  boolean java/lang/Thread:: isInterrupted ();}
//public final native boolean isInterrupted()
extern KNI_RETURNTYPE_BOOLEAN java_lang_Thread_isInterrupted(void);
 //JNIFunc={  boolean java/lang/Thread:: isDaemon ();}
//public final native boolean isDaemon()
extern KNI_RETURNTYPE_BOOLEAN java_lang_Thread_isDaemon(void);
 //JNIFunc={  void java/lang/Thread:: setDaemon (boolean on);}
//public final native void setDaemon(boolean on)
extern KNI_RETURNTYPE_VOID java_lang_Thread_setDaemon(void);
 //JNIFunc={  void java/lang/Thread:: join ();}
//public final native void join() throws InterruptedException
#if 0
extern KNI_RETURNTYPE_VOID java_lang_Thread_join(void);
#endif
 //JNIFunc={  void java/lang/Thread:: join (long timeout);}
//public final native void join(long timeout) throws InterruptedException
	//extern KNI_RETURNTYPE_VOID java_lang_Thread_join(void);
 //JNIFunc={  static void java/lang/Thread:: exitThread ();}
//private final static native void exitThread()
extern KNI_RETURNTYPE_VOID java_lang_Thread_exitThread(void);
 //JNIFunc={  static int lejos/nxt/Battery:: getBatteryStatus ();}
//private static native int getBatteryStatus()
extern KNI_RETURNTYPE_INT lejos_nxt_Battery_getBatteryStatus(void);
 //JNIFunc={  static int lejos/nxt/Button:: getButtons ();}
//private static native int getButtons()
extern KNI_RETURNTYPE_INT lejos_nxt_Button_getButtons(void);
 //JNIFunc={  static int lejos/nxt/Flash:: flashReadPage (byte[] buf,int pageNum);}
//static native int flashReadPage(byte[] buf, int pageNum)
extern KNI_RETURNTYPE_INT lejos_nxt_Flash_flashReadPage(void);
 //JNIFunc={  static int lejos/nxt/Flash:: flashWritePage (byte[] buf,int pageNum);}
//static native int flashWritePage(byte[] buf, int pageNum)
extern KNI_RETURNTYPE_INT lejos_nxt_Flash_flashWritePage(void);
 //JNIFunc={  static int lejos/nxt/Flash:: flashExec (int pageNum,int size);}
//static native int flashExec(int pageNum, int size)
extern KNI_RETURNTYPE_INT lejos_nxt_Flash_flashExec(void);
 //JNIFunc={  static void lejos/nxt/LCD:: drawString (String str,int x,int y);}
//public static native void drawString(String str, int x, int y)
extern KNI_RETURNTYPE_VOID lejos_nxt_LCD_drawString(void);
 //JNIFunc={  static void lejos/nxt/LCD:: drawInt (int i,int x,int y);}
//public static native void drawInt(int i, int x, int y)
extern KNI_RETURNTYPE_VOID lejos_nxt_LCD_drawInt3(void);
 //JNIFunc={  static void lejos/nxt/LCD:: drawInt (int i,int places,int x,int y);}
//public static native void drawInt(int i, int places, int x, int y)
extern KNI_RETURNTYPE_VOID lejos_nxt_LCD_drawInt4(void);
 //JNIFunc={  static void lejos/nxt/LCD:: asyncRefresh ();}
//public static native void asyncRefresh()
extern KNI_RETURNTYPE_VOID lejos_nxt_LCD_asyncRefresh(void);
 //JNIFunc={  static int lejos/nxt/LCD:: getRefreshCompleteTime ();}
//public static native int getRefreshCompleteTime()
extern KNI_RETURNTYPE_INT lejos_nxt_LCD_getRefreshCompleteTime(void);
 //JNIFunc={  static void lejos/nxt/LCD:: clear ();}
//public static native void clear()
extern KNI_RETURNTYPE_VOID lejos_nxt_LCD_clear(void);
 //JNIFunc={  static byte[] lejos/nxt/LCD:: getDisplay ();}
//public static native byte[] getDisplay()
extern KNI_RETURNTYPE_OBJECT lejos_nxt_LCD_getDisplay(void);
 //JNIFunc={  static byte[] lejos/nxt/LCD:: getSystemFont ();}
//public static native byte[] getSystemFont()
extern KNI_RETURNTYPE_OBJECT lejos_nxt_LCD_getSystemFont(void);
 //JNIFunc={  static int lejos/nxt/LCD:: setAutoRefreshPeriod (int period);}
//public static native int setAutoRefreshPeriod(int period)
extern KNI_RETURNTYPE_INT lejos_nxt_LCD_setAutoRefreshPeriod(void);
 //JNIFunc={  static void lejos/nxt/LCD:: bitBlt (byte[] src,int sw,int sh,int sx,int sy,byte[] dst,int dw,int dh,int dx,int dy,int w,int h,int rop);}
//public native static void bitBlt(byte[] src, int sw, int sh, int sx, int sy, byte dst[], int dw, int dh, int dx, int dy, int w, int h, int rop)
extern KNI_RETURNTYPE_VOID lejos_nxt_LCD_bitBlt(void);
 //JNIFunc={  static void lejos/nxt/LCD:: setContrast (int contrast);}
//public native static void setContrast(int contrast)
extern KNI_RETURNTYPE_VOID lejos_nxt_LCD_setContrast(void);
 //JNIFunc={  static void lejos/nxt/MotorPort:: controlMotorById (int id,int power,int mode);}
//private static synchronized native void controlMotorById(int id, int power, int mode)
extern KNI_RETURNTYPE_VOID lejos_nxt_MotorPort_controlMotorById(void);
 //JNIFunc={  static int lejos/nxt/MotorPort:: getTachoCountById (int aMotor);}
//private static native int getTachoCountById(int aMotor)
extern KNI_RETURNTYPE_INT lejos_nxt_MotorPort_getTachoCountById(void);
 //JNIFunc={  static void lejos/nxt/MotorPort:: resetTachoCountById (int aMotor);}
//private static synchronized native void resetTachoCountById(int aMotor)
extern KNI_RETURNTYPE_VOID lejos_nxt_MotorPort_resetTachoCountById(void);
 //JNIFunc={  static void lejos/nxt/NXT:: shutDown ();}
//public static native void shutDown()
extern KNI_RETURNTYPE_VOID lejos_nxt_NXT_shutDown(void);
 //JNIFunc={  static void lejos/nxt/NXT:: boot ();}
//public static native void boot()
extern KNI_RETURNTYPE_VOID lejos_nxt_NXT_boot(void);
 //JNIFunc={  static int lejos/nxt/NXT:: getProgramExecutionsCount ();}
//public static native int getProgramExecutionsCount()
extern KNI_RETURNTYPE_INT lejos_nxt_NXT_getProgramExecutionsCount(void);
 //JNIFunc={  static int lejos/nxt/NXT:: getFirmwareRawVersion ();}
//public static native int getFirmwareRawVersion()
extern KNI_RETURNTYPE_INT lejos_nxt_NXT_getFirmwareRawVersion(void);
 //JNIFunc={  static int lejos/nxt/NXT:: getFirmwareRevision ();}
//public static native int getFirmwareRevision()
extern KNI_RETURNTYPE_INT lejos_nxt_NXT_getFirmwareRevision(void);
 //JNIFunc={  static int lejos/nxt/NXT:: getUserPages ();}
//public static native int getUserPages()
extern KNI_RETURNTYPE_INT lejos_nxt_NXT_getUserPages(void);
 //JNIFunc={  int lejos/nxt/NXTEvent:: registerEvent ();}
//public native int registerEvent()
extern KNI_RETURNTYPE_INT lejos_nxt_NXTEvent_registerEvent(void);
 //JNIFunc={  int lejos/nxt/NXTEvent:: unregisterEvent ();}
//public native int unregisterEvent()
extern KNI_RETURNTYPE_INT lejos_nxt_NXTEvent_unregisterEvent(void);
 //JNIFunc={  int lejos/nxt/NXTEvent:: changeEvent (int set,int clear);}
//private native int changeEvent(int set, int clear)
extern KNI_RETURNTYPE_INT lejos_nxt_NXTEvent_changeEvent(void);
 //JNIFunc={  static int lejos/nxt/SensorPort:: readSensorValue (int aPortId);}
//private static native int readSensorValue(int aPortId)
extern KNI_RETURNTYPE_INT lejos_nxt_SensorPort_readSensorValue(void);
 //JNIFunc={  static void lejos/nxt/SensorPort:: setPowerTypeById (int aPortId,int aPortType);}
//private static native void setPowerTypeById(int aPortId, int aPortType)
extern KNI_RETURNTYPE_VOID lejos_nxt_SensorPort_setPowerTypeById(void);
 //JNIFunc={  static void lejos/nxt/SensorPort:: i2cEnableById (int aPortId,int mode);}
//private static native void i2cEnableById(int aPortId, int mode)
extern KNI_RETURNTYPE_VOID lejos_nxt_SensorPort_i2cEnableById(void);
 //JNIFunc={  static void lejos/nxt/SensorPort:: i2cDisableById (int aPortId);}
//private static native void i2cDisableById(int aPortId)
extern KNI_RETURNTYPE_VOID lejos_nxt_SensorPort_i2cDisableById(void);
 //JNIFunc={  static int lejos/nxt/SensorPort:: i2cStatusById (int aPortId);}
//private static native int i2cStatusById(int aPortId)
extern KNI_RETURNTYPE_INT lejos_nxt_SensorPort_i2cStatusById(void);
 //JNIFunc={  static int lejos/nxt/SensorPort:: i2cStartById (int aPortId,int address,byte[] writeBuffer,int writeOffset,int writeLen,int readLen);}
//private static native int i2cStartById(int aPortId, int address,byte[] writeBuffer, int writeOffset, int writeLen, int readLen)
extern KNI_RETURNTYPE_INT lejos_nxt_SensorPort_i2cStartById(void);
 //JNIFunc={  static int lejos/nxt/SensorPort:: i2cCompleteById (int aPortId,byte[] readBuffer,int offset,int readLen);}
//private static native int i2cCompleteById(int aPortId, byte[] readBuffer, int offset, int readLen)
extern KNI_RETURNTYPE_INT lejos_nxt_SensorPort_i2cCompleteById(void);
 //JNIFunc={  static void lejos/nxt/SensorPort:: setSensorPinMode (int port,int pin,int mode);}
//private static native void setSensorPinMode(int port, int pin, int mode)
extern KNI_RETURNTYPE_VOID lejos_nxt_SensorPort_setSensorPinMode(void);
 //JNIFunc={  static void lejos/nxt/SensorPort:: setSensorPin (int port,int pin,int val);}
//private static native void setSensorPin(int port, int pin, int val)
extern KNI_RETURNTYPE_VOID lejos_nxt_SensorPort_setSensorPin(void);
 //JNIFunc={  static int lejos/nxt/SensorPort:: getSensorPin (int port,int pin);}
//private static native int getSensorPin(int port, int pin)
extern KNI_RETURNTYPE_INT lejos_nxt_SensorPort_getSensorPin(void);
 //JNIFunc={  static int lejos/nxt/SensorPort:: readSensorPin (int port,int pin);}
//private static native int readSensorPin(int port, int pin)
extern KNI_RETURNTYPE_INT lejos_nxt_SensorPort_readSensorPin(void);
 //JNIFunc={  static int lejos/nxt/Sound:: getTime ();}
//public static native int getTime()
extern KNI_RETURNTYPE_INT lejos_nxt_Sound_getTime(void);
 //JNIFunc={  static void lejos/nxt/Sound:: playFreq (int aFrequency,int aDuration,int aVolume);}
//static native void playFreq(int aFrequency, int aDuration, int aVolume)
extern KNI_RETURNTYPE_VOID lejos_nxt_Sound_playFreq(void);
 //JNIFunc={  static void lejos/nxt/Sound:: playSample (int page,int offset,int len,int freq,int vol);}
//static native void playSample(int page, int offset, int len, int freq, int vol)
extern KNI_RETURNTYPE_VOID lejos_nxt_Sound_playSample(void);
 //JNIFunc={  static int lejos/nxt/Sound:: playQueuedSample (byte[] ,int offset,int len,int freq,int vol);}
//static native int playQueuedSample(byte [] data, int offset, int len, int freq, int vol)
extern KNI_RETURNTYPE_INT lejos_nxt_Sound_playQueuedSample(void);
 //JNIFunc={  static int lejos/nxt/VM:: memPeek (int base,int offset,int typ);}
//private static native int memPeek(int base, int offset, int typ)
extern KNI_RETURNTYPE_INT lejos_nxt_VM_memPeek(void);
 //JNIFunc={  static void lejos/nxt/VM:: memCopy (Object obj,int objoffset,int base,int offset,int len);}
//private static native void memCopy(Object obj, int objoffset, int base, int offset, int len)
extern KNI_RETURNTYPE_VOID lejos_nxt_VM_memCopy(void);
 //JNIFunc={  static int lejos/nxt/VM:: getDataAddress (Object obj);}
//private native static int getDataAddress (Object obj)
extern KNI_RETURNTYPE_INT lejos_nxt_VM_getDataAddress(void);
 //JNIFunc={  static int lejos/nxt/VM:: getObjectAddress (Object obj);}
//private native static int getObjectAddress(Object obj)
extern KNI_RETURNTYPE_INT lejos_nxt_VM_getObjectAddress(void);
 //JNIFunc={  static Object lejos/nxt/VM:: memGetReference (int base,int offset);}
//private native static Object memGetReference(int base, int offset)
extern KNI_RETURNTYPE_OBJECT lejos_nxt_VM_memGetReference(void);
 //JNIFunc={  static boolean lejos/nxt/VM:: isAssignable (int src,int dst);}
//private static native boolean isAssignable(int src, int dst)
extern KNI_RETURNTYPE_BOOLEAN lejos_nxt_VM_isAssignable(void);
 //JNIFunc={  static void lejos/nxt/VM:: suspendThread (Object thread);}
//public native static final void suspendThread(Object thread)
extern KNI_RETURNTYPE_VOID lejos_nxt_VM_suspendThread(void);
 //JNIFunc={  static void lejos/nxt/VM:: resumeThread (Object thread);}
//public native static final void resumeThread(Object thread)
extern KNI_RETURNTYPE_VOID lejos_nxt_VM_resumeThread(void);
 //JNIFunc={  static void lejos/nxt/VM:: executeProgram (int progNo);}
//public native static final void executeProgram(int progNo)
extern KNI_RETURNTYPE_VOID lejos_nxt_VM_executeProgram(void);
 //JNIFunc={  static void lejos/nxt/VM:: setVMOptions (int options);}
//public static final native void setVMOptions(int options)
extern KNI_RETURNTYPE_VOID lejos_nxt_VM_setVMOptions(void);
 //JNIFunc={  static int lejos/nxt/VM:: getVMOptions ();}
//public static final native int getVMOptions()
extern KNI_RETURNTYPE_INT lejos_nxt_VM_getVMOptions(void);
 //JNIFunc={  static int[] lejos/nxt/VM:: createStackTrace (Thread thread,Object ignore);}
//public static native int[] createStackTrace(Thread thread, Object ignore)
extern KNI_RETURNTYPE_OBJECT lejos_nxt_VM_createStackTrace(void);
 //JNIFunc={  static void lejos/nxt/VM:: firmwareExceptionHandler (Throwable exception,int method,int pc);}
//public static native void firmwareExceptionHandler(Throwable exception, int method, int pc)
extern KNI_RETURNTYPE_VOID lejos_nxt_VM_firmwareExceptionHandler(void);
 //JNIFunc={  static int lejos/nxt/comm/Bluetooth:: btWrite (byte[] buf,int off,int len);}
//public static native int btWrite(byte[] buf, int off, int len)
extern KNI_RETURNTYPE_INT lejos_nxt_comm_Bluetooth_btWrite(void);
 //JNIFunc={  static int lejos/nxt/comm/Bluetooth:: btRead (byte[] buf,int off,int len);}
//public static native int btRead(byte[] buf, int off, int len)
extern KNI_RETURNTYPE_INT lejos_nxt_comm_Bluetooth_btRead(void);
 //JNIFunc={  static int lejos/nxt/comm/Bluetooth:: btPending ();}
//public static native int btPending()
extern KNI_RETURNTYPE_INT lejos_nxt_comm_Bluetooth_btPending(void);
 //JNIFunc={  static void lejos/nxt/comm/Bluetooth:: btSetArmCmdMode (int mode);}
//public static native void btSetArmCmdMode(int mode)
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_Bluetooth_btSetArmCmdMode(void);
 //JNIFunc={  static int lejos/nxt/comm/Bluetooth:: btGetBC4CmdMode ();}
//public static native int btGetBC4CmdMode()
extern KNI_RETURNTYPE_INT lejos_nxt_comm_Bluetooth_btGetBC4CmdMode(void);
 //JNIFunc={  static void lejos/nxt/comm/Bluetooth:: btSetResetLow ();}
//public static native void btSetResetLow()
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_Bluetooth_btSetResetLow(void);
 //JNIFunc={  static void lejos/nxt/comm/Bluetooth:: btSetResetHigh ();}
//public static native void btSetResetHigh()
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_Bluetooth_btSetResetHigh(void);
 //JNIFunc={  static void lejos/nxt/comm/Bluetooth:: btSend (byte[] buf,int len);}
//public static native void btSend(byte[] buf, int len)
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_Bluetooth_btSend(void);
 //JNIFunc={  static void lejos/nxt/comm/Bluetooth:: btReceive (byte[] buf);}
//public static native void btReceive(byte[] buf)
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_Bluetooth_btReceive(void);
 //JNIFunc={  static void lejos/nxt/comm/Bluetooth:: btEnable ();}
//public static native void btEnable()
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_Bluetooth_btEnable(void);
 //JNIFunc={  static void lejos/nxt/comm/Bluetooth:: btDisable ();}
//public static native void btDisable()
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_Bluetooth_btDisable(void);
 //JNIFunc={  static int lejos/nxt/comm/RS485:: hsSend (byte address,byte control,byte[] data,int offset,int len,char[] crc);}
//static native int hsSend(byte address, byte control, byte[]data, int offset, int len, char[]crc)
extern KNI_RETURNTYPE_INT lejos_nxt_comm_RS485_hsSend(void);
 //JNIFunc={  static int lejos/nxt/comm/RS485:: hsRecv (byte[] data,int len,char[] crc,int reset);}
//static native int hsRecv(byte[]data, int len, char[]crc, int reset)
extern KNI_RETURNTYPE_INT lejos_nxt_comm_RS485_hsRecv(void);
 //JNIFunc={  static void lejos/nxt/comm/RS485:: hsEnable (int baudRate,int bufferSize);}
//public static native void hsEnable(int baudRate, int bufferSize)
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_RS485_hsEnable(void);
 //JNIFunc={  static void lejos/nxt/comm/RS485:: hsDisable ();}
//public static native void hsDisable()
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_RS485_hsDisable(void);
 //JNIFunc={  static int lejos/nxt/comm/RS485:: hsRead (byte[] ,int offset,int len);}
//public static native int hsRead(byte [] buf, int offset, int len)
extern KNI_RETURNTYPE_INT lejos_nxt_comm_RS485_hsRead(void);
 //JNIFunc={  static int lejos/nxt/comm/RS485:: hsWrite (byte[] buf,int offset,int len);}
//public static native int hsWrite(byte[] buf, int offset, int len)
extern KNI_RETURNTYPE_INT lejos_nxt_comm_RS485_hsWrite(void);
 //JNIFunc={  static void lejos/nxt/comm/USB:: usbEnable (int reset);}
//public static native void usbEnable(int reset)
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_USB_usbEnable(void);
 //JNIFunc={  static void lejos/nxt/comm/USB:: usbDisable ();}
//public static native void usbDisable()
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_USB_usbDisable(void);
 //JNIFunc={  static void lejos/nxt/comm/USB:: usbReset ();}
//public static native void usbReset()
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_USB_usbReset(void);
 //JNIFunc={  static int lejos/nxt/comm/USB:: usbRead (byte[] ,int off,int len);}
//public static native int usbRead(byte [] buf, int off, int len)
extern KNI_RETURNTYPE_INT lejos_nxt_comm_USB_usbRead(void);
 //JNIFunc={  static int lejos/nxt/comm/USB:: usbWrite (byte[] ,int off,int len);}
//public static native int usbWrite(byte [] buf, int off, int len)
extern KNI_RETURNTYPE_INT lejos_nxt_comm_USB_usbWrite(void);
 //JNIFunc={  static int lejos/nxt/comm/USB:: usbStatus ();}
//public static native int usbStatus()
extern KNI_RETURNTYPE_INT lejos_nxt_comm_USB_usbStatus(void);
 //JNIFunc={  static void lejos/nxt/comm/USB:: usbSetSerialNo (String serNo);}
//public static native void usbSetSerialNo(String serNo)
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_USB_usbSetSerialNo(void);
 //JNIFunc={  static void lejos/nxt/comm/USB:: usbSetName (String name);}
//public static native void usbSetName(String name)
extern KNI_RETURNTYPE_VOID lejos_nxt_comm_USB_usbSetName(void);
 //JNIFunc={  void lejos/nxt/debug/DebugInterface:: setDebug ();}
//private native final void setDebug()
extern KNI_RETURNTYPE_VOID lejos_nxt_debug_DebugInterface_setDebug(void);
 //JNIFunc={  static int lejos/nxt/debug/DebugInterface:: eventOptions (int event,int option);}
//private native static final int eventOptions(int event, int option)
extern KNI_RETURNTYPE_INT lejos_nxt_debug_DebugInterface_eventOptions(void);