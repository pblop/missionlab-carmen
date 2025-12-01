import carmen.*;

public class TestBase implements OdometryHandler
{
  private boolean initialized = false;
  private double startTime = 0;

  public void handleOdometry(OdometryMessage message) {
    if (!initialized) {
      startTime = message.getTimestamp();
      initialized = true;
      return;
    }
    message.setTimestamp(message.getTimestamp() - startTime);
    System.out.println("OdometryHandler: Got message at "+message.getTimestamp()+
		       " from "+message.getHost());
    if (message.getTimestamp() > 2) {
      System.exit(0);
    }
  }

  public static void main (String args[]) {
    RobotModule.initialize("TestBase", "localhost");
    RobotModule.resetRobotBase();
    OdometryMessage.subscribe(new TestBase());
    RobotModule.dispatch();
  }
}
