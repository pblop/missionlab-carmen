package carmen;


/**  Carmen OdometryHandler's message
   */

public class OdometryMessage extends Message {
  /** robot pose at the center of the robot */
  public double _01_x, _02_y, _03_theta;
  /** commanded translational (m/s) and rotational (r/s) velocity */
  public double _04_tv, _05_rv;
  /** robot acceleration (m/s^2) */
  public double _06_acceleration;

  private static final String CARMEN_ROBOT_ODOMETRY_NAME = "carmen_base_odometry";
  private static final String CARMEN_ROBOT_ODOMETRY_FMT = 
  	"{double, double, double, double, double, double, double, string}";

  /** Application module calls this to subscribe to OdometryMessage.
   *  Application module must extend OdometryHandler.
   */
  public static void subscribe(OdometryHandler handler) {
    subscribe(CARMEN_ROBOT_ODOMETRY_NAME, CARMEN_ROBOT_ODOMETRY_FMT, handler, 
    		OdometryMessage.class, "handleOdometry");
  }

  public static void unsubscribe()
  {
  	unsubscribe(CARMEN_ROBOT_ODOMETRY_NAME);	
  }
    
  public double getX()
  {
	  return _01_x;
  }

  public double getY()
  {
	  return _02_y;
  }

  public double getTheta()
  {
	  return _03_theta;
  }

  public double getTv()
  {
	  return _04_tv;
  }

  public double getRv()
  {
	  return _05_rv;
  }

  public double getAcceleration()
  {
	  return _06_acceleration;
  }

  public static String getMessageName()
  {
	  return CARMEN_ROBOT_ODOMETRY_NAME;
  }

  public static String getMessageFormat()
  {
	  return CARMEN_ROBOT_ODOMETRY_FMT;
  }

  public void setX(double x)
  {
	  this._01_x = x;
  }

  public void setY(double y)
  {
	  this._02_y = y;
  }

  public void setTheta(double theta)
  {
	  this._03_theta = theta;
  }

  public void setTv(double tv)
  {
	  this._04_tv = tv;
  }

  public void setRv(double rv)
  {
	  this._05_rv = rv;
  }

  public void setAcceleration(double acceleration)
  {
	  this._06_acceleration = acceleration;
  }
}

