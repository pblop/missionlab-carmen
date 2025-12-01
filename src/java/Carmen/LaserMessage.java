package carmen;


/**  Carmen FrontLaserHandler's and RearLaserHandler's message
   */

public class LaserMessage extends Message {
  public int _00_id;
  public LaserConfig _01_config;
  public int _02_num_readings;
  public float _03_range[];
  public char _04_tooclose[];
  public int _05_num_remissions;
  public float _06_remission[];
  /** position of the center of the laser **/
  public Point _07_laser_pose; 
  /** position of the center of the robot **/
  public Point _08_robot_pose; 
  /** robot state: translational velocity and rotational velocity */
  public double _09_tv, _10_rv;
  /** application defined safety distance in metres */
  public double _11_forward_safety_dist, _12_side_safety_dist;
  public double _13_turn_axis;

  private static final String CARMEN_ROBOT_FRONTLASER_NAME =
    "carmen_robot_frontlaser";

  private static final String CARMEN_ROBOT_REARLASER_NAME =
    "carmen_robot_rearlaser";

  private static final String CARMEN_ROBOT_LASER_FMT = 
    "{int,{int,double,double,double,double,double,int},int,<float:3>,<char:3>,int,<float:6>,{double,double,double},{double,double,double},double,double,double,double,double,double,string}";

  /** Application module calls this to subscribe to LaserMessage.
   *  Application module must extend either FrontLaserHandler or RearLaserHandler.
   */
  public static void subscribeFront(FrontLaserHandler handler) {
    subscribe(CARMEN_ROBOT_FRONTLASER_NAME, CARMEN_ROBOT_LASER_FMT, handler, 
	      LaserMessage.class, "handleFrontLaser");
  }
 
  /** Application module calls this to subscribe to LaserMessage.
   *  Application module must extend either FrontLaserHandler or RearLaserHandler.
   */
  public static void subscribeRear(RearLaserHandler handler) {
    subscribe(CARMEN_ROBOT_REARLASER_NAME, CARMEN_ROBOT_LASER_FMT, handler, 
	      LaserMessage.class, "handleRearLaser");
  }

public static void unsubscribeFront()
{
	unsubscribe(CARMEN_ROBOT_FRONTLASER_NAME);		
}

public int getNumReadings()
{
	return _02_num_readings;
}

}

