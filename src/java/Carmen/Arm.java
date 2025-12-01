package carmen;

import IPC.IPC;

public class Arm {
  private static final String CARMEN_ARM_QUERY_NAME = "carmen_arm_query";

  public static class ArmCommandMessage extends Message {
    public int _01_numJoints;
    public double _02_jointAngles [];

    private static final String CARMEN_ARM_COMMAND_NAME = "carmen_arm_command";
    private static final String CARMEN_ARM_COMMAND_FMT = 
      "{int, <double:1>, double, string}";

    ArmCommandMessage(double jointAngles[]) {
      this._02_jointAngles = jointAngles;
      this._01_numJoints = jointAngles.length;
    }
    public void publish() {
      publish(CARMEN_ARM_COMMAND_NAME, CARMEN_ARM_COMMAND_FMT, this);
    }    
  }    

  public static void move(double jointAngles[])
  {
    if (jointAngles.length > 4) {
      System.out.println("Arm has only 4 joints. You called Arm.move with "+
			 jointAngles.length+" doubles. Ignoring.");
      return;
    }
    ArmCommandMessage command = new ArmCommandMessage(jointAngles);
    command.publish();
  }

  public static ArmMessage query()
  {
    Message msg = new Message();
    ArmMessage response = (ArmMessage)IPC.queryResponseData
      (CARMEN_ARM_QUERY_NAME, msg, Message.class, 5000);
    return response;
  }
}
