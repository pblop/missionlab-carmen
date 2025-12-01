package carmen;

import java.util.HashMap;

import IPC.IPC;

/**  Carmen class for handling robot parameters in file carmen.ini 
   */
public class Param {


  @SuppressWarnings("unused")
private static final String CARMEN_PARAM_VARIABLE_CHANGE_NAME = 
    "carmen_param_variable_change";
  
  @SuppressWarnings("unused")
private static final String CARMEN_PARAM_VARIABLE_CHANGE_FMT =
    "{string, string, string, int, int, double, string}";

  private static final String CARMEN_PARAM_QUERY_NAME =
    "carmen_param_query_string";
  private static final String CARMEN_PARAM_QUERY_FMT = 
    "{string, string, double, string}";

  private static final String CARMEN_PARAM_RESPONSE_STRING_NAME =
    "carmen_param_respond_string";
  private static final String CARMEN_PARAM_RESPONSE_STRING_FMT =
    "{string, string, string, int, int, double, string}";

  private static final String CARMEN_PARAM_SET_NAME =
    "carmen_param_set";
  private static final String CARMEN_PARAM_SET_FMT =
    "{string, string, string, double, string}";

  private static final String CARMEN_PARAM_QUERY_ALL_NAME =
	    "carmen_param_query_all";
  
  private static final String CARMEN_PARAM_RESPONSE_ALL_NAME =
	    "carmen_param_respond_all";
  private static final String CARMEN_PARAM_RESPONSE_ALL_FMT =
	    "{string, int, <string:2>, <string:2>, <int:2>, int, double, string}";  
  
  private static final String CARMEN_PARAM_QUERY_MODULES_NAME =
	    "carmen_param_query_modules";

  private static final String CARMEN_PARAM_RESPONSE_MODULES_NAME =
	    "carmen_param_respond_modules";
  private static final String CARMEN_PARAM_RESPONSE_MODULES_FMT =
	    "{<string:2>, int, int, double, string}";  

  
  /** Class method that handles query of current parameter values by module and variable */
  public static class ParamQuery extends Message {
    public String _01_moduleName;
    public String _02_variableName;

    ParamQuery(String moduleName, String variableName) {
      this._01_moduleName = moduleName;
      this._02_variableName = variableName;
    }
  }
  /** inner class of Param */
  public static class ParamResponse extends Message {
    public String _01_moduleName;
    public String _02_variableName;
    public String _03_value;
    public int _04_expert;
    public int _05_status;
  }
  
  /** inner class of Param */
  public static class ParamResponseAll extends Message {
	public String _01_moduleName;
	public int _02_listLength;
    public String[] _03_variables;
    public String[] _04_values;
    public int[] _05_expert;
    public int _06_status;
  }

  public static class ParamResponseModules extends Message {
	  public String[] _01_modules;
	  public int _02_num_modules;
	  public int _03_status;  
  }  

  /** inner class of Param */
  public static class ParamSet extends Message {
    public String _01_moduleName;
    public String _02_variableName;
    public String _03_value;

    ParamSet(String moduleName, String variableName, String value) {
      this._01_moduleName = moduleName;
      this._02_variableName = variableName;
      this._03_value = value;
    }
  }

  /** Class method for parameter queryring */
  public static String query(String moduleName, String variableName) {
    IPC.defineMsg(CARMEN_PARAM_QUERY_NAME, CARMEN_PARAM_QUERY_FMT);
    IPC.defineMsg(CARMEN_PARAM_RESPONSE_STRING_NAME, 
		  CARMEN_PARAM_RESPONSE_STRING_FMT);
    ParamQuery query = new ParamQuery(moduleName, variableName);
    ParamResponse response = (ParamResponse)IPC.queryResponseData
      (CARMEN_PARAM_QUERY_NAME, query, ParamResponse.class, 5000);
    return response._03_value;
  }
  /** Class method to set a variable with a new value */
  public static boolean set(String moduleName, String variable, 
			    String newValue) {
	IPC.defineMsg(CARMEN_PARAM_SET_NAME, CARMEN_PARAM_SET_FMT);
	ParamSet msg = new ParamSet(moduleName, variable, newValue);
    ParamResponse response = (ParamResponse)IPC.queryResponseData
      (CARMEN_PARAM_SET_NAME, msg, ParamResponse.class, 5000);
    if (response._05_status == 0)
      return true;
    return false;
  }

  public static boolean parseBoolean(String value) {
    if (value.equalsIgnoreCase("1"))
      return true;
    return false;
  }

  public static boolean parseOnoff(String value) {
    if (value.equalsIgnoreCase("on"))
      return true;
    return false;
  }
  
  public static String[] getModules() {
    IPC.defineMsg(CARMEN_PARAM_QUERY_MODULES_NAME, CARMEN_PARAM_QUERY_FMT);
    IPC.defineMsg(CARMEN_PARAM_RESPONSE_MODULES_NAME, 
    		CARMEN_PARAM_RESPONSE_MODULES_FMT);
    
    ParamQuery query = new ParamQuery("paramServer", "modules");
    ParamResponseModules response = (ParamResponseModules)IPC.queryResponseData
      (CARMEN_PARAM_QUERY_MODULES_NAME, query, ParamResponseModules.class, 5000);
    return response._01_modules;
  }
  
  public static HashMap<String, String> getAll(String module) {
	    IPC.defineMsg(CARMEN_PARAM_QUERY_ALL_NAME, CARMEN_PARAM_QUERY_FMT);
	    IPC.defineMsg(CARMEN_PARAM_RESPONSE_ALL_NAME, 
	    		CARMEN_PARAM_RESPONSE_ALL_FMT);
	    
	    ParamQuery query = new ParamQuery(module, "*");
	    ParamResponseAll response = (ParamResponseAll)IPC.queryResponseData
	      (CARMEN_PARAM_QUERY_ALL_NAME, query, ParamResponseAll.class, 5000);
	    
	    HashMap<String, String> hash = new HashMap<String, String>();
	    for(int i=0; i<response._03_variables.length; i++)
	    	hash.put(response._03_variables[i], response._04_values[i]);
	    
	    return hash;
	  }
}
