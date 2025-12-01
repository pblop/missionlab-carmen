import carmen.*;

public class TestRobot implements FrontLaserHandler, OdometryHandler {
  private boolean initialized = false;
  private double startTime = 0;

  public void handleOdometry (OdometryMessage message) {
    if (!initialized) {
      startTime = message.getTimestamp();
      initialized = true;
      return;
    }
    message.setTimestamp(message.getTimestamp() - startTime);
    System.out.println("Odometry: "+message.getTv()+" m/s "+
		       Math.toDegrees(message.getRv())+" deg/s");
    if (message.getTimestamp() > 2) {
      System.exit(0);
    }
  }

  public void handleFrontLaser (LaserMessage message) {
    if (!initialized) {
      startTime = message.getTimestamp();
      initialized = true;
      return;
    }
    message.setTimestamp(message.getTimestamp() - startTime);
    System.out.println("LaserHandler: Got "+message.getNumReadings());
    if (message.getTimestamp() > 2) {
      System.exit(0);
    }
  }

  public static void main (String args[]) {
    RobotModule.initialize("TestRobot", "localhost");
    TestRobot test = new TestRobot();
    OdometryMessage.subscribe(test);
    LaserMessage.subscribeFront(test);
    RobotModule.setVelocity(0, Math.toRadians(30));
    RobotModule.dispatch();
  }
}
