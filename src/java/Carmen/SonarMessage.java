package carmen;


/**  Carmen SonarHandler's message
   */

public class SonarMessage extends Message {
  public int _01_num_sonars;
  /** width of sonar cone */
  public double _02_sensor_angle;   
  /** for each sonar, the range reading */
  public double _03_range[];
  /** location of each sonar with respect to robot as a Point */
  public Point _04_sonar_positions[];
  /** robot state: point location */
  public Point _05_robot_pose;
  public double _06_tv;
  public double _07_rv;

  private static final String CARMEN_ROBOT_SONAR_NAME = "robot_sonar";
  private static final String CARMEN_ROBOT_SONAR_FMT =
    "{int,double,<double:1>,<{double,double,double}:1>,{double,double,double},double,double,double,string}";

  /** Application module calls this to subscribe to SonarMessage.
   *  Application module must extend SonarHandler
   */
  public static void subscribe(SonarHandler handler) {
    subscribe(CARMEN_ROBOT_SONAR_NAME, CARMEN_ROBOT_SONAR_FMT, handler, 
	      SonarMessage.class, "handle");
  }

public static void unsubscribe()
{
	unsubscribe(CARMEN_ROBOT_SONAR_NAME);	
}

}

