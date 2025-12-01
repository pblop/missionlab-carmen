package carmen;
/** Carmen ParamChangeHandler's message */

public class ParamChangeMessage extends Message {
  public String _01_moduleName;
  public String _02_variableName;
  public String _03_newValue;
  public int _04_expert;
  public int _05_status;

  private static final String CARMEN_PARAM_CHANGE_NAME = 
    "carmen_param_variable_change";
  private static final String CARMEN_PARAM_CHANGE_FMT =
    "{string, string, string, int, int, double, string}";

  public static void subscribe(ParamChangeHandler handler)
  {
    subscribe(CARMEN_PARAM_CHANGE_NAME, CARMEN_PARAM_CHANGE_FMT, handler, 
	      ParamChangeMessage.class, "handle");
  }
}

