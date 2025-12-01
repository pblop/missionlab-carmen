package carmen;
/** Carmen Trajectory Point */

public class TrajPoint {
  /** current trajectory status: x, y (metres), theta (radians) */
  public double _01_x, _02_y, _03_theta;
  /** current trajectory status in translational velocity (m/s) and rotational velocity (r/s) */
  public double _04_t_vel, _05_r_vel;
}
