package carmen;

public class LaserConfig 
{
  /** what kind of laser is this */
  public int _01_laser_type;  
  /** angle of the first beam relative 
      to to the center of the laser **/
  public double _02_start_angle;                     
  /** field of view of the laser **/
  public double _03_fov;
  /** angular resolution of the laser **/
  public double _04_angular_resolution;
  /** the maximum valid range of a measurement  **/
  public double _05_maximum_range;
  /** error in the range measurements**/
  public double _06_accuracy;

  /** if and what kind of remission values are used **/
  public int _07_remission_mode;
}
