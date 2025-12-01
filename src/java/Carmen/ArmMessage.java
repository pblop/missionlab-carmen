package carmen;

 
public class ArmMessage extends Message {
  public int _01_flags;
  public int _02_num_joints;
  public double _03_joint_angles[];
  public int _04_num_currents;
  public double _05_joint_currents[];
  public int _06_num_vels;
  public double _07_joint_angular_vels[];
  public int _08_gripper_closed;
  
  private static final String CARMEN_ARM_STATE_NAME = 
    "carmen_arm_state";
  private static final String CARMEN_ARM_STATE_FMT =  
    "{int,int,<double:2>,int,<double:4>,int,<double:6>,int,double,string}";

  public static void subscribe(CameraHandler handler) {
    subscribe(CARMEN_ARM_STATE_NAME, CARMEN_ARM_STATE_FMT, handler, 
	      ArmMessage.class, "handle");
  }
}
