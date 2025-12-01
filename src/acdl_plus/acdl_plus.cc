/**********************************************************************
 **                                                                  **
 **                             acdl_plus.h                          **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  ACDLPlus class contains a set of translators for converting     **
 **  ACDL+ into different languages (CDL, CMDL, etc.) and vice       **
 **  versa.                                                          **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: acdl_plus.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "acdl_plus.h"
#include "write_cdl.h"
#include "load_cdl.h"
#include "popups.h"
#include "acdl_to_xml_converter.h"

using std::ifstream;
using std::ios;

#include "mic.h"
#include "environment.h"
#include "cmdl_Root.h"
#include "cmdl_loader.h"
#include "ActionCommand.h"

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const string ACDLPlus::EMPTY_STRING_ = "";
const string ACDLPlus::ACDLPlus::ACDL2CDL_DEFAULT_ROBOT_NAME_ = "acdl2cdlDefaultRobot";
const string ACDLPlus::ACDL_STRING_MOVEMENT_ = "movement";
const string ACDLPlus::ACDL_STRING_VEHICLE_ = "vehicle";
const string ACDLPlus::ACDL_STRING_BIND_ARCH_ = "BIND_ARCH";
const string ACDLPlus::ACDL_STRING_AGENT_NAME_ = "AGENT_NAME";
const string ACDLPlus::ACDL_STRING_TASK_INDEX_ = "TASK_INDEX";
const string ACDLPlus::ACDL_STRING_TASK_DESC_ = "TASK_DESC";
const string ACDLPlus::ACDL_STRING_TRIGGERING_TASK_INDEX_ = "TRIGGERING_TASK_INDEX";
const string ACDLPlus::ACDL_STRING_CMD_ = "CMD";
const string ACDLPlus::ACDL_STRING_PARM_ = "PARM";
const string ACDLPlus::ACDL_STRING_PARM_NAME_ = "PARM_NAME";
const string ACDLPlus::ACDL_STRING_PARM_VALUE_ = "PARM_VALUE";
const string ACDLPlus::ACDL_STRING_IF_ = "IF";
const string ACDLPlus::ACDL_STRING_GOTO_ = "GOTO";
const string ACDLPlus::ACDL_STRING_INSTANCE_ = "INSTANCE";
const string ACDLPlus::ACDL_STRING_INSTANCE_INDEX_ = "INSTANCE_INDEX";
const string ACDLPlus::ACDL_STRING_INSTANCE_NAME_ = "INSTANCE_NAME";
const string ACDLPlus::ACDL_STRING_START_ = "START";
const string ACDLPlus::ACDL_STRING_END_ = "END";
const string ACDLPlus::ACDL_STRING_MAX_VELOCITY_ = "MAX_VELOCITY";
const string ACDLPlus::ACDL_STRING_BASE_VELOCITY_ = "BASE_VELOCITY";
const string ACDLPlus::ACDL_DEFAULT_CONFIG_FILE_ = "cmdli2acdl.rc";
const int    ACDLPlus::ACDL_TASK_INDEX_BASE_ = 5050;

//-----------------------------------------------------------------------
// Mutex initialization
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for ACDLPlus class.
//-----------------------------------------------------------------------
ACDLPlus::ACDLPlus(void)
{
}

//-----------------------------------------------------------------------
// Distructor for ACDLPlus class.
//-----------------------------------------------------------------------
ACDLPlus::~ACDLPlus(void)
{
}

//-----------------------------------------------------------------------
// This function reads the text from a file, and converts into a string.
//-----------------------------------------------------------------------
string ACDLPlus::readText_(string inputFileName)
{
    string textString;
    char *data = NULL;
    int dataSize = 0;

    ifstream inputFile(inputFileName.c_str());

    // Find the data size.
    inputFile.seekg (0, ios::end);
    dataSize = inputFile.tellg();
    inputFile.seekg (0, ios::beg);

    // To append NULL at the end.
    dataSize++; 

    // Read in the data.
    data = new char[dataSize];
    inputFile.read (data, dataSize-1);
    
    // Append a null, to make it a char string.
    data[dataSize-1] = '\0';

    // Close the file.
    inputFile.close();

    // Copy it as a string.
    textString = data;

    // Delete the char*.
    delete [] data;
    data = NULL;

    return textString;
}

//-----------------------------------------------------------------------
// This function dumps the text into a file.
//-----------------------------------------------------------------------
bool ACDLPlus::dumpText_(string textString, string outputFileName)
{
    FILE *outputFile = NULL;
    char buf[1024];

    // Check to see if the file exists at first.
    outputFile = fopen(outputFileName.c_str(), "r");

    if (outputFile != NULL)
    {
        sprintf(buf, "File \'%s\' exists. Overwrite it?", outputFileName.c_str());

        if (!(gPopWin->confirmUser(buf, false)))
        {
            return false;
        }
    }

    outputFile = fopen(outputFileName.c_str(), "w");

    if (outputFile == NULL)
    {
        return false;
    }

    fprintf(outputFile, "%s", textString.c_str());
    fclose(outputFile);

    return true;
}

//-----------------------------------------------------------------------
// This function converts ACDL+ into CIM XML.
//-----------------------------------------------------------------------
string ACDLPlus::acdlp2cimxmlText(string acdlpText)
{
    string cimxmlText;

    // Write the code here.
	acdl_to_xml_converter converter;
	cimxmlText = converter.parseACDLFile( acdlpText );

    return cimxmlText;
}

//-----------------------------------------------------------------------
// This function converts ACDL+ into CIM XML.
//-----------------------------------------------------------------------
bool ACDLPlus::acdlp2cimxml(string inputFileName, string outputFileName)
{
    string acdlpText, cimxmlText;
    bool dumpSuccess = false;

    // Read the input text.
    acdlpText = ACDLPlus::readText_(inputFileName);

    if (acdlpText.empty())
    {
        return false;
    }

    // Convert.
    cimxmlText = ACDLPlus::acdlp2cimxmlText(acdlpText);

    if (cimxmlText.empty())
    {
        return false;
    }

    // Dump the text.
    dumpSuccess = ACDLPlus::dumpText_(cimxmlText, outputFileName);

    return dumpSuccess;
}

//-----------------------------------------------------------------------
// This function converts ACDL+ into CMDL.
//-----------------------------------------------------------------------
string ACDLPlus::acdlp2cmdlText(string acdlpText)
{
    string cmdlText;

    // Write the code here.
    cmdlText = acdlpText;

	

    return cmdlText;
}

//-----------------------------------------------------------------------
// This function converts ACDL+ into CMDL.
//-----------------------------------------------------------------------
bool ACDLPlus::acdlp2cmdl(string inputFileName, string outputFileName)
{
    string acdlpText, cmdlText;
    bool dumpSuccess = false;

    // Read the input text.
    acdlpText = ACDLPlus::readText_(inputFileName);

    if (acdlpText.empty())
    {
        return false;
    }

    // Convert.
    cmdlText = ACDLPlus::acdlp2cmdlText(acdlpText);

    if (cmdlText.empty())
    {
        return false;
    }

    // Dump the text.
    dumpSuccess = ACDLPlus::dumpText_(cmdlText, outputFileName);

    return dumpSuccess;
}

//-----------------------------------------------------------------------
// This function converts ACDL+ into CMDLi.
//-----------------------------------------------------------------------
string ACDLPlus::acdlp2cmdliText(string acdlpText)
{
    string cmdliText;

    // Write the code here.
    cmdliText = acdlpText;

    return cmdliText;
}

//-----------------------------------------------------------------------
// This function converts ACDL+ into CMDLi.
//-----------------------------------------------------------------------
bool ACDLPlus::acdlp2cmdli(string inputFileName, string outputFileName)
{
    string acdlpText, cmdliText;
    bool dumpSuccess = false;

    // Read the input text.
    acdlpText = ACDLPlus::readText_(inputFileName);

    if (acdlpText.empty())
    {
        return false;
    }

    // Convert.
    cmdliText = ACDLPlus::acdlp2cmdliText(acdlpText);

    if (cmdliText.empty())
    {
        return false;
    }

    // Dump the text.
    dumpSuccess = ACDLPlus::dumpText_(cmdliText, outputFileName);

    return dumpSuccess;
}

//-----------------------------------------------------------------------
// This function converts ACDL+ into CDL.
//-----------------------------------------------------------------------
string ACDLPlus::acdlp2cdlText(string acdlpText)
{
    string cdlText = EMPTY_STRING_;
    char *cdlSolution = NULL;
    int status;

    cdlSolution = acdl2cdl(
        acdlpText.c_str(),
        ACDL2CDL_DEFAULT_ROBOT_NAME_,
        &status);
        
    if ((cdlSolution != NULL) && (status != ACDL2CDL_STATUS_FAILURE))
    {
        cdlText = cdlSolution;
    }

    return cdlText;
}

//-----------------------------------------------------------------------
// This function converts ACDL+ into CDL.
//-----------------------------------------------------------------------
bool ACDLPlus::acdlp2cdl(string inputFileName, string outputFileName)
{
    string acdlpText, cdlText;
    bool dumpSuccess = false;

    // Read the input text.
    acdlpText = ACDLPlus::readText_(inputFileName);

    if (acdlpText.empty())
    {
        return false;
    }

    // Convert.
    cdlText = ACDLPlus::acdlp2cdlText(acdlpText);

    if (cdlText.empty())
    {
        return false;
    }

    // Dump the text.
    dumpSuccess = ACDLPlus::dumpText_(cdlText, outputFileName);

    return dumpSuccess;
}

//-----------------------------------------------------------------------
// This function converts CIM XML into ACDL+.
//-----------------------------------------------------------------------
string ACDLPlus::cimxml2acdlpText(string cimxmlText)
{
    string acdlpText;

    // Write the code here.
    acdlpText = cimxmlText;

    return acdlpText;
}

//-----------------------------------------------------------------------
// This function converts CIM XML into ACDL+.
//-----------------------------------------------------------------------
bool ACDLPlus::cimxml2acdlp(string inputFileName, string outputFileName)
{
    string cimxmlText, acdlpText;
    bool dumpSuccess = false;

    // Read the input text.
    cimxmlText = ACDLPlus::readText_(inputFileName);

    if (cimxmlText.empty())
    {
        return false;
    }

    // Convert.
    acdlpText = ACDLPlus::cimxml2acdlpText(cimxmlText);

    if (acdlpText.empty())
    {
        return false;
    }

    // Dump the text.
    dumpSuccess = ACDLPlus::dumpText_(acdlpText, outputFileName);

    return dumpSuccess;
}

//-----------------------------------------------------------------------
// This function converts CMDL into ACDL+.
//-----------------------------------------------------------------------
string ACDLPlus::cmdl2acdlpText(string cmdlText)
{
    string acdlpText;

    // Write the code here.
    acdlpText = cmdlText;

    return acdlpText;
}

//-----------------------------------------------------------------------
// This function converts CMDL into ACDL+.
//-----------------------------------------------------------------------
bool ACDLPlus::cmdl2acdlp(string inputFileName, string outputFileName)
{
    string cmdlText, acdlpText;
    bool dumpSuccess = false;

    // Read the input text.
    cmdlText = ACDLPlus::readText_(inputFileName);

    if (cmdlText.empty())
    {
        return false;
    }

    // Convert.
    acdlpText = ACDLPlus::cmdl2acdlpText(cmdlText);

    if (acdlpText.empty())
    {
        return false;
    }

    // Dump the text.
    dumpSuccess = ACDLPlus::dumpText_(acdlpText, outputFileName);

    return dumpSuccess;
}

// Some data structures used only by the method below
typedef struct { 
  string acdl_cmd;
  short num_param;
  string cmdli_parm_names[128];
  string acdl_parm_names[128];
  string parm_types[128];
  string default_values[128];
} Parameter_Map;

typedef std::map<std::string, Parameter_Map> Command_Map;
typedef std::vector <class sara::ActionCommand *> Commands;
typedef std::map<std::string, Commands > Command_List;

//-----------------------------------------------------------------------
// This function converts CMDLi into ACDL+.
//-----------------------------------------------------------------------
string ACDLPlus::cmdli2acdlpText(string cmdliText)
{
  char buf1[256], buf2[256], buf3[256], buf4[256];
  string bufString1, bufString2, currCommand;
  FILE *acdlDefFile;
  string ourName = "minos";
  string bindArch, baseVel, maxVel;
  string acdlpText, headerText, robotText;
  char state_buf[64], index_buf[64], temp_buf[64];
  char* line_buf;
  size_t s = 0;
  Command_Map cmd_map;
  int numParam = 0;

  //****************************************************************************************
  // Read the default config file to get the binding archi. and default velocities
  // Also load in the mapping for each command and its parameters, as well as
  // default parameter values for each of these parameters

  if (!(acdlDefFile = fopen(ACDL_DEFAULT_CONFIG_FILE_.c_str(), "r")))
    {
      cerr << "Couldn't open the CMDLi_to_ACDL converter resource file\n";
      return "";
    }

  while (fscanf(acdlDefFile, "%s", buf1)!=EOF ) {

    bufString1 = buf1; 
  
    if (bufString1[0] == '#') {  // Comment Line - chomp it up
      getline(&line_buf, &s, acdlDefFile);
      free(line_buf); s=0;
    }
    else if (bufString1 == ACDL_STRING_BIND_ARCH_) {
      fscanf(acdlDefFile, "%s", buf2);
      bindArch = buf2;
    }
    else if (bufString1 == ACDL_STRING_BASE_VELOCITY_) {
      fscanf(acdlDefFile, "%s", buf2);
      baseVel = buf2;
    }
    else if (bufString1 == ACDL_STRING_MAX_VELOCITY_) {
      fscanf(acdlDefFile, "%s", buf2);
      maxVel = buf2;
    }
    else if (bufString1 == ACDL_STRING_CMD_) {
      // Saw a "CMD" token, go ahead and save the mapping for this command
      fscanf(acdlDefFile, "%s %s",buf1, buf2);
      bufString1 = buf1;
      bufString2 = buf2;
      cmd_map[bufString1].acdl_cmd = bufString2;
      currCommand = bufString1;
      cmd_map[bufString1].num_param = numParam = 0;
    }
    else if (bufString1 == ACDL_STRING_PARM_) {
      // This is parameter for the current parameter that we are reading
      fscanf(acdlDefFile, "%s %s %s %s",buf1, buf2, buf3, buf4);
      cmd_map[currCommand].cmdli_parm_names[numParam] = buf1;
      cmd_map[currCommand].acdl_parm_names[numParam] = buf2;
      cmd_map[currCommand].parm_types[numParam] = buf3;
      cmd_map[currCommand].default_values[numParam] = buf4;
      cmd_map[currCommand].num_param++; 
      numParam++;
    }
  }

  //*****************************************************************************************
  //Use the CMDLi parser to parse and load the CMDLi file
  sara::stringSet legalActions, legalTriggers;
  sara::environment *env = new sara::environment();
  sara::cmdl *thecmdl = new sara::cmdl(legalActions, legalTriggers, env);

  //  thecmdl->set_loader_debug((sara::cmdl::cmdl_DEBUG_T)4);
  thecmdl->load(ourName, cmdliText );

  // Get the main block. For now we will only support translation of commands in the main block
  // Commands inside procedures are not supported yet
  sara::Block *main_block = thecmdl->getMainBlock();

  sara::Command *cmd;
  sara::ActionCommand *actCmd = 0;
  const sara::keyValueMap *options = 0;
  Command_List cmdList;

  // Now Get all the commands and group them by Units
  for (uint i=0; main_block->isValidCommandIndex(i); i++)
  {
    //get the next command
    cmd = main_block->getCommand(i);

    //try to dynamically cast into ActionCommand - we are only interested in Action Commands
    actCmd = dynamic_cast<sara::ActionCommand*>(cmd);

    //If the cast was successful and the there is an action name associated with the command
    if ((actCmd)&&(!actCmd->getAction().empty())) {

      // Get the set of robot names
      const sara::stringSet names = actCmd->getRobotNames();

      // Now use the name of the first roobot associated with this command to index
      // this command into its own group
      if (names.size() > 0)
	cmdList[*(++names.begin())].push_back(actCmd);
    }
  }

  //****************************************************************************************
  // Generate the code for basic information
  headerText = ACDL_STRING_BIND_ARCH_ + " " + bindArch + "\n\n" 
    + ACDL_STRING_MAX_VELOCITY_ + " " + maxVel + "\n\n"
    + ACDL_STRING_BASE_VELOCITY_ + " " + baseVel + "\n\n"; 
  

  // We will treat the set of commands for each unit a separate FSA for 
  // generating the ACDL file

  for (Command_List::const_iterator it = cmdList.begin(); it != cmdList.end(); ++it) {

    // Create the start of the instance for the first robot
    robotText = ACDL_STRING_INSTANCE_NAME_ + " " + ACDL_STRING_MOVEMENT_ + "\n" 
      + ACDL_STRING_INSTANCE_ + " " + ACDL_STRING_START_ + "\n\n";  

    // Generate the code for the Start state
    robotText += ACDL_STRING_TASK_INDEX_ + " " + "Start" + "\n"
      + ACDL_STRING_TASK_DESC_ + " " + "Start" + "\n"
      + ACDL_STRING_AGENT_NAME_ + " " + "Stop" + "\n\n";

    int index_counter = ACDL_TASK_INDEX_BASE_;
    int numCommands = it->second.size();
    int numRobots = (it->second)[0]->numRobotNames();

    for (int j=0; j<numCommands; j++) {

      sprintf(state_buf, "State%d", j+1);
      sprintf(index_buf, "$AN_%d", index_counter++);

      robotText += ACDL_STRING_TASK_INDEX_ + " " + index_buf + "\n"
	+ ACDL_STRING_TASK_DESC_ + " " + state_buf + "\n";

      currCommand = ((it->second)[j])->getAction();
      // Get the parameters for this command
      options =  ((it->second)[j])->getOptions();
      
      string parmVal, parmVal1, parmVal2;
      sara::keyValueMap::const_iterator km;

      string temp_parm_name;

      // Now dump the parameters out
      for (int p=0; p < cmd_map[currCommand].num_param; p++){

	// initialise to default value
	parmVal = cmd_map[currCommand].default_values[p];

	// Now check if the command passed in a value for this parameter
	// If yes, use it

	if (options && !options->empty()) {  

	  // If we are dealing with string pair need to extract two separate
	  // strings from the CMDLi paprameters

	  if (cmd_map[currCommand].parm_types[p] == "string_pair") {
	    temp_parm_name = "min_" + cmd_map[currCommand].cmdli_parm_names[p];
	    if ((km = options->find(temp_parm_name)) != options->end()) {
	      parmVal1 = km->second;
	      temp_parm_name = "max_" + cmd_map[currCommand].cmdli_parm_names[p];
	      if ((km = options->find(temp_parm_name)) != options->end()) {
		parmVal2 = km->second;
		parmVal = "\"" + parmVal1 + "\"" + "," +  "\"" + parmVal2 + "\"";
	      }
	    }
	  }
	  else if ((km = options->find(cmd_map[currCommand].cmdli_parm_names[p])) != options->end())
	    parmVal = km->second;
	}

	robotText += ACDL_STRING_PARM_NAME_ + " %" + cmd_map[currCommand].acdl_parm_names[p] + "\n";;
	
	// format the paramater value according to parameter type
	/*	if (cmd_map[currCommand].parm_types[p] == "string")
		robotText += ACDL_STRING_PARM_VALUE_ + " {\"" + parmVal + "\"}\n"; */

	if ((cmd_map[currCommand].parm_types[p] == "number")
		 ||(cmd_map[currCommand].parm_types[p] == "string")
		 ||(cmd_map[currCommand].parm_types[p] == "string_pair"))
	  robotText += ACDL_STRING_PARM_VALUE_ + " {" + parmVal + "}\n"; 

	else if (cmd_map[currCommand].parm_types[p] == "vector") {
	  sara::Vector *v = sara::Vector::load(parmVal);
	  sprintf(temp_buf, "%.1f,%.1f", v->x, v->y);
	  robotText += ACDL_STRING_PARM_VALUE_ + " {" + temp_buf + "}\n";
	}
      }

      robotText += ACDL_STRING_AGENT_NAME_ + " " 
	+ cmd_map[currCommand].acdl_cmd + "\n";
    }

    // Now that all the commands are added, add in the "Immediate" Triggers to get a 
    // straight line FSA
    if (numCommands > 0) {
     
      sprintf(index_buf, "$AN_%d", ACDL_TASK_INDEX_BASE_);

      robotText += ACDL_STRING_TRIGGERING_TASK_INDEX_ + " " + "Start" + "\n"
	+ ACDL_STRING_IF_ + " " + ACDL_STRING_START_ + "\n"
	+ ACDL_STRING_AGENT_NAME_ + " " + "Immediate" + "\n"
	+ ACDL_STRING_GOTO_ + " " + index_buf + "\n"
	+ ACDL_STRING_IF_ + " " + ACDL_STRING_END_ + "\n";

      for (int j=1; j < numCommands; j++) {

	robotText += ACDL_STRING_TRIGGERING_TASK_INDEX_ + " " +  index_buf + "\n";

	sprintf(index_buf, "$AN_%d", ACDL_TASK_INDEX_BASE_+j);

	robotText += ACDL_STRING_IF_ + " " + ACDL_STRING_START_ + "\n"
	  + ACDL_STRING_AGENT_NAME_ + " " + "Immediate" + "\n"
	  + ACDL_STRING_GOTO_ + " " + index_buf + "\n"
	  + ACDL_STRING_IF_ + " " + ACDL_STRING_END_ + "\n";

      }
    }

    // Now close this instance and append an instance of vehicle
    robotText += ACDL_STRING_INSTANCE_ + " " + ACDL_STRING_END_ + "\n\n"
      + ACDL_STRING_INSTANCE_NAME_ + " " + ACDL_STRING_VEHICLE_ + "\n"
      + ACDL_STRING_INSTANCE_ + " " + ACDL_STRING_START_ + "\n"
      + ACDL_STRING_INSTANCE_ + " " + ACDL_STRING_END_ + "\n\n";

    // Finally repeat the robotText for the number of robots
    for (int r=1; r < numRobots; r++)
      robotText += robotText;

    acdlpText += headerText + robotText;
  }

  //Close the defaults file before returning
  fclose(acdlDefFile);

  return acdlpText;
}

//-----------------------------------------------------------------------
// This function converts CMDLi into ACDL+.
//-----------------------------------------------------------------------
bool ACDLPlus::cmdli2acdlp(string inputFileName, string outputFileName)
{
   string cmdliText, acdlpText;

   const string cmdl_filename(inputFileName);

   // Create an input stream from the file.
   std::ifstream input( cmdl_filename.c_str() );
   if( !input )
   {
      cerr << "Unable to open the CMDL file " << cmdl_filename << " for reading!" << endl;
      exit(1);
   }

   // Read it in.
   char ch;
   while( input.get(ch) )
      cmdliText += ch;

    bool dumpSuccess = false;

    if (cmdliText.empty())
    {
        return false;
    }

    // Convert.
    acdlpText = ACDLPlus::cmdli2acdlpText(cmdliText);

    if (acdlpText.empty())
    {
        return false;
    }

    // Dump the text.
    dumpSuccess = ACDLPlus::dumpText_(acdlpText,outputFileName);

    return dumpSuccess;
}

//-----------------------------------------------------------------------
// This function converts CDL into ACDL+.
//-----------------------------------------------------------------------
string ACDLPlus::cdl2acdlpText(string cdlText)
{
    Symbol * agent;
    string acdlpText = EMPTY_STRING_;
    string tmpCDLFilenameString, tmpACDLFilenameString;
    char tmpACDLFilename[256], tmpCDLFilename[256];
    const char *tempFilelist[2];
    int fd;
    bool errors;
    bool saved = false;

    // Name the temporary files that will be used in this function.
    sprintf(tmpCDLFilename, "/tmp/%s-cdl2acdlpText-cdl-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpCDLFilename);
    unlink(tmpCDLFilename);
    tmpCDLFilenameString = tmpCDLFilename;

    sprintf(tmpACDLFilename, "/tmp/%s-cdl2acdlpText-acdl-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpACDLFilename);
    unlink(tmpACDLFilename);
    tmpACDLFilenameString = tmpACDLFilename;

    // Write the CDL in a temporary file first.
    saved = ACDLPlus::dumpText_(cdlText, tmpCDLFilenameString);
    
    if (!saved)
    {
        return acdlpText;
    }

    // Load the CDL as an agent.
    tempFilelist[0] = tmpCDLFilenameString.c_str();
    tempFilelist[1] = NULL;
    agent = load_cdl(tempFilelist, errors, false, false);

    // Save as ACDL.
    saved = save_abstracted_workspace(agent, tmpACDLFilenameString.c_str());

    if (!saved)
    {
        return acdlpText;
    }
    
    // Read off the ACDL text.
    acdlpText = ACDLPlus::readText_(tmpACDLFilenameString);

    // Remove the temporary files.
    unlink(tmpCDLFilenameString.c_str());
    unlink(tmpACDLFilenameString.c_str());

    return acdlpText;
}

//-----------------------------------------------------------------------
// This function converts CDL into ACDL+.
//-----------------------------------------------------------------------
bool ACDLPlus::cdl2acdlp(string inputFileName, string outputFileName)
{
    string cdlText, acdlpText;
    bool dumpSuccess = false;

    // Read the input text.
    cdlText = ACDLPlus::readText_(inputFileName);

    if (cdlText.empty())
    {
        return false;
    }

    // Convert.
    acdlpText = ACDLPlus::cdl2acdlpText(cdlText);

    if (acdlpText.empty())
    {
        return false;
    }

    // Dump the text.
    dumpSuccess = ACDLPlus::dumpText_(acdlpText, outputFileName);

    return dumpSuccess;
}

/**********************************************************************
 * $Log: acdl_plus.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.6  2006/09/04 01:13:41  nadeem
 * Fixed a small bug
 *
 * Revision 1.5  2006/09/04 00:35:16  nadeem
 * *** empty log message ***
 *
 * Revision 1.4  2006/08/25 20:11:40  alanwags
 * Added acdl to cim xml conversion
 *
 * Revision 1.3  2006/08/04 22:26:48  endo
 * cdl2acdlp implemented.
 *
 * Revision 1.2  2006/08/02 21:38:05  endo
 * acdlp2cdl() implemented.
 *
 * Revision 1.1  2006/07/26 18:05:46  endo
 * ACDLPlus class added.
 *
 *
 **********************************************************************/
