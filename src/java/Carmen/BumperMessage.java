package carmen;


/**  Carmen BumperHandler's message
   */
public class BumperMessage extends Message 
{  
  public int _01_num_bumpers; 
  /** state of bumper */
  public char _02_bumper[]; 
  /** robot pose at the center of the robot */
  public Point _03_robot_location; 
  /** commanded translational velocity (m/s) */
  public double _04_tv;
  /** commanded rotational velocity (r/s) */
  public double _05_rv;

  private static final String CARMEN_ROBOT_BUMPER_NAME =
    "carmen_robot_bumper";
  private static final String CARMEN_ROBOT_BUMPER_FMT = 
    "{int, <char:1>, {double,double,double}, double, double, double, string}";

  public static void subscribe(BumperHandler handler) {
    subscribe(CARMEN_ROBOT_BUMPER_NAME, CARMEN_ROBOT_BUMPER_FMT, handler,
	      BumperMessage.class, "handle");
  }

}

