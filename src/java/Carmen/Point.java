package carmen;
/**  Carmen Point 
   */

public class Point {
  /** pose information in metres for x, y and radians for theta */
  public double _01_x, _02_y, _03_theta;
  
  public double getX()
  { return _01_x; }
  public double getY()
  { return _02_y; }
  public double getTheta()
  { return _03_theta; }
  
  public void setX(double x)
  { _01_x=x; }
  public void setY(double y)
  { _02_y=y; }
  public void setTheta(double theta)
  { _03_theta=theta; }
  
}
