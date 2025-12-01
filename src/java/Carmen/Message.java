package carmen;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashSet;

import IPC.IPC;

/** Carmen generic or base Message class */

public class Message {

	public double _98_timestamp;

	public String _99_host;  

  private static HashSet<String> defined_messages = new HashSet<String>(); 

  /** New message has timestamp from this host and host name */
  public Message() {
    _98_timestamp = Util.getTime();
    _99_host = Util.getHostName();
  }

  private static void verifyFormatString(String msgFormat, Class<? extends Object> msgClass)
  {
    
  }

  private static void verifyFormatString(String msgFormat, Object msgInstance)
  {
    Class<? extends Object> className = msgInstance.getClass();
    verifyFormatString(msgFormat, className);
  }

  private static class PrivateHandler implements IPC.HANDLER_TYPE {
    private Object userHandler;
    private Method handleMethod;

    PrivateHandler(Object userHandler, Method handleMethod,
		   Class<? extends Object> handlerClass, Class<?> messageClass) 
    {
      this.userHandler = userHandler;
      this.handleMethod = handleMethod;
    }

    public void handle(IPC.MSG_INSTANCE msgInstance, Object callData) 
    {
      try {
	handleMethod.invoke(userHandler, callData);
      }
      catch (IllegalAccessException e) {
	System.err.println(e.toString());
	System.exit(-1);
      }
      catch (IllegalArgumentException e) {
    	  e.printStackTrace();
	System.exit(-1);
      }
      catch (InvocationTargetException e) {
    	  e.printStackTrace();
	System.exit(-1);
      }
    }
  }  

  protected static void subscribe(String messageName, String messageFmt, 
				  Object handler, Class<?> messageClass, 
				  String handlerFuncName) 
  {
    if (!defined_messages.contains(messageName)) {
      verifyFormatString(messageFmt, messageClass);
      IPC.defineMsg(messageName, messageFmt);
      defined_messages.add(messageName);
    }

    Class<? extends Object> handlerClass = handler.getClass();

    Method handleMethod;
    try {
      handleMethod = handlerClass.getMethod(handlerFuncName, messageClass);
    }
    catch (NoSuchMethodException e) {
      System.err.println("You subscribed to "+messageClass+" but you used a "+
			 "handler that doesn't\nhave a method \n"+
			 "handle("+messageClass+")\n");
      throw new Error(e.toString());
    }

    PrivateHandler pvtHandler = new PrivateHandler(handler, handleMethod,
						   handlerClass, messageClass);

    IPC.subscribeData(messageName, pvtHandler, messageClass);
    IPC.setMsgQueueLength(messageName, 1);
  }
  
  protected static void unsubscribe(String messageName) 
  {
	  IPC.unsubscribe(messageName, PrivateHandler.class);
  }

  public void publish(String msgName, String msgFmt, Object message) 
  {
    if (!defined_messages.contains(msgName)) {
      verifyFormatString(msgFmt, message);
      IPC.defineMsg(msgName, msgFmt);
      defined_messages.add(msgName);
    }

    System.out.println("Envio: " + msgName);
    IPC.publishData(msgName, message);
  }
  
  public double getTimestamp()
  {
	  return _98_timestamp;
  }
  
  public void setTimestamp(double timestamp)
  {
	  _98_timestamp = timestamp;
  }
  
  public String getHost()
  {
	  return _99_host;
  }
}

