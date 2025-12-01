/*********************************************************************
 **                                                                  **
 **                     acdl_to_xml_converter.cpp                    **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2006 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: acdl_to_xml_converter.cpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#include "acdl_to_xml_converter.h"
#include "ParseDefines.h"

acdl_to_xml_converter::acdl_to_xml_converter():strAgentName("AGENT_NAME"),											   strParmName("PARM_NAME"),strParmValue("PARM_VALUE") {
  iObjectiveID=0;
}

/* Move to xml_to_acdl_converter.cpp

string acdl_to_xml_converter::extractMissionDateTime(string strXMLEnvMessage) {
  
  string rString="";
  string strMStart = "<mission_start>";
  if( strXMLEnvMessage != "" ) {
    int iMissionStart = strXMLEnvMessage.find("<mission_start>", 0);
    int iMissionEnd =   strXMLEnvMessage.find("</mission_start>", 0);
    rString = strXMLEnvMessage.substr(iMissionStart+strMStart.size(), (iMissionEnd-(iMissionStart+strMStart.size())));
  } 
  return rString; 
}

string acdl_to_xml_converter::getMissionDateTime(void) {
  
  GT_MPPC_client *pMPPC = GT_MPPC_client::Instance();
  string strMessage = pMPPC->GetEnvironmentMessage(); 
  return extractMissionDateTime(strMessage); 
}

string acdl_to_xml_converter::extractMissionLocation(string strXMLEnvMessage) {
  string rString="";
  string strMCenter = "<mission_center>";
  string strMLat = "<latitude>";
  string strMLong = "<longitude>";
  if( strXMLEnvMessage != "" ) {
    int iMissionCenterStart = strXMLEnvMessage.find("<mission_center>", 0);
    int iMissionCenterEnd =   strXMLEnvMessage.find("</mission_center>", 0);
    string strLocation = strXMLEnvMessage.substr(iMissionCenterStart+strMCenter.size(), (iMissionCenterEnd-(iMissionCenterStart+strMCenter.size())));
    
    int iMissionLatitudeStart = strLocation.find("<latitude>", 0);
    int iMissionLatitudeEnd =   strLocation.find("</latitude>", 0);
    string strLatitude = strLocation.substr(iMissionLatitudeStart+strMLat.size(), (iMissionLatitudeEnd-(iMissionLatitudeStart+strMLat.size())));

    int iMissionLongitudeStart = strLocation.find("<longitude>", 0);
    int iMissionLongitudeEnd =   strLocation.find("</longitude>", 0);
    string strLongitude = strLocation.substr(iMissionLongitudeStart+strMLong.size(), (iMissionLongitudeEnd-(iMissionLongitudeStart+strMLong.size())));
    rString += "<" + strLatitude + "," + strLongitude + ">"; 
  } 
  return rString;
}

string acdl_to_xml_converter::extractMissionLatitude(string strXMLEnvMessage) {
  string strLatitude = "";
  string strMCenter = "<mission_center>";
  string strMLat = "<latitude>";
  if( strXMLEnvMessage != "" ) {
    int iMissionCenterStart = strXMLEnvMessage.find("<mission_center>", 0);
    int iMissionCenterEnd =   strXMLEnvMessage.find("</mission_center>", 0);
    string strLocation = strXMLEnvMessage.substr(iMissionCenterStart+strMCenter.size(), (iMissionCenterEnd-(iMissionCenterStart+strMCenter.size())));
    
    int iMissionLatitudeStart = strLocation.find("<latitude>", 0);
    int iMissionLatitudeEnd =   strLocation.find("</latitude>", 0);
    strLatitude = strLocation.substr(iMissionLatitudeStart+strMLat.size(), (iMissionLatitudeEnd-(iMissionLatitudeStart+strMLat.size())));
  } 
  return strLatitude;
}

string acdl_to_xml_converter::extractMissionLongitude(string strXMLEnvMessage) {
  string strLongitude = "";
  string strMCenter = "<mission_center>";
  string strMLong = "<longitude>";
  if( strXMLEnvMessage != "" ) {
    int iMissionCenterStart = strXMLEnvMessage.find("<mission_center>", 0);
    int iMissionCenterEnd =   strXMLEnvMessage.find("</mission_center>", 0);
    string strLocation = strXMLEnvMessage.substr(iMissionCenterStart+strMCenter.size(), (iMissionCenterEnd-(iMissionCenterStart+strMCenter.size())));
    
    int iMissionLongitudeStart = strLocation.find("<longitude>", 0);
    int iMissionLongitudeEnd =   strLocation.find("</longitude>", 0);
    strLongitude = strLocation.substr(iMissionLongitudeStart+strMLong.size(), (iMissionLongitudeEnd-(iMissionLongitudeStart+strMLong.size())));
  } 
  return strLongitude;
}

string acdl_to_xml_converter::getMissionLocation(void) {
  GT_MPPC_client *pMPPC = GT_MPPC_client::Instance();
  string strMessage = pMPPC->GetEnvironmentMessage();  
  return extractMissionLocation(strMessage);
}
*/

/*
 *  Parse the ACDL File for robot specific mission data
 */
string acdl_to_xml_converter::parseACDLFile(string text ) {
  string s1=text;  
  string strReturnText="";
  if( s1.size() == 0 )
	  return "";

  int iVehicleID=0;  
  GT_MPPC_client *pMPPC = GT_MPPC_client::Instance();
  int iMessageNumber = pMPPC->GetNumberOfMessagesSent();  
  int iPriority = 5;    
  strReturnText += generateXMLHeader(iMessageNumber, iPriority); 
  while(s1.find("INSTANCE START", 0) != string::npos) {
	
	size_t sInstanceStart=0;
	size_t sInstanceEnd=0;
	MOVE_TO_NEXT_INSTANCE_START_TAG( sInstanceStart )
	MOVE_TO_NEXT_INSTANCE_END_TAG( sInstanceEnd )

	string robotMission = s1.substr(sInstanceStart, (sInstanceEnd-sInstanceStart));
	s1 = s1.erase(sInstanceStart, robotMission.size());  
	iVehicleID++;
	strReturnText += parseRobotMission( robotMission );
  }

  for(int i=0;i<iVehicleID;i++)
    strReturnText += generateXMLVehicleInfo(iVehicleID); 
  
  strReturnText += generateXMLTrailer();
  return strReturnText;
}

/*
 *  Parse a mission for specific objectives
 */
string acdl_to_xml_converter::parseRobotMission( string text ) {

  string s1=text;  
  string strReturnText="";
  if( s1.size() == 0 )
	  return "";

  while(s1.find("AGENT_NAME", 0) != string::npos) {

	size_t sObjectiveStart=0;
	size_t sObjectiveEnd=0;
	sObjectiveEnd = sObjectiveStart+1;
	MOVE_TO_NEXT_AGENT_NAME_TAG( sObjectiveEnd );
	sObjectiveEnd+= (s1.find('\n', sObjectiveEnd)-sObjectiveEnd);

	string strObjectiveText = s1.substr(sObjectiveStart, (sObjectiveEnd - sObjectiveStart));	
	s1 = s1.erase(sObjectiveStart,strObjectiveText.size()+1);	
	strReturnText += parseObjective( strObjectiveText );
  } 	 
  return strReturnText;
}


/*
 *  Parse robot all robot behaviors looking for behaviors that are actually objectives
 */
string acdl_to_xml_converter::parseObjective( string text ) {

  string s1=text;
  string strReturnText="";
  
  if( s1.size() == 0 )
	return strReturnText;

  size_t sObjectiveStart=0;
  MOVE_TO_NEXT_AGENT_NAME_TAG( sObjectiveStart );
  int iSpace = s1.find(' ', sObjectiveStart);
  int iNewLine = s1.find('\n', sObjectiveStart);
  string strObjectiveName = s1.substr(iSpace+1,iNewLine);

  if(strObjectiveName == "ICARUS_MO_Steerpoint" ) {   
	iObjectiveID++; 
	strReturnText += generateXMLSteerpointObjective( text, iObjectiveID );
  }
  else if(strObjectiveName == "ICARUS_MO_SARImage" )   { 
	iObjectiveID++;
	strReturnText += generateXMLSARObjective( text, iObjectiveID );
  }
  else if(strObjectiveName == "ICARUS_MO_Loiter" )   { 
	iObjectiveID++;
	strReturnText += generateXMLLoiterObjective( text, iObjectiveID );
  }
  else if(strObjectiveName == "ICARUS_MO_PGB" )   { 
	iObjectiveID++;
	strReturnText += generateXMLPGBObjective( text, iObjectiveID );
  }
  else if(strObjectiveName == "ICARUS_MO_AGM" )   { 
	iObjectiveID++;
	strReturnText += generateXMLAGMObjective( text, iObjectiveID );
  }  
  else if(strObjectiveName == "ICARUS_MO_Communications" )   { 
	iObjectiveID++;
	strReturnText += generateXMLCommunicationObjective( text, iObjectiveID );
  }
  else if(strObjectiveName == "ICARUS_MO_EOIR" )   { 
	iObjectiveID++;
	strReturnText += generateXMLEOIRObjective( text, iObjectiveID );
  }  
  else if(strObjectiveName == "ICARUS_MO_Sentry" )   { 
	iObjectiveID++;
	strReturnText += generateXMLSentryObjective( text, iObjectiveID );
  } 
 

  // PU New objectives
  else if (strObjectiveName == "ICARUS_MO_Mad" )   { 
        iObjectiveID++;
	strReturnText += generateXMLMADObjective( text, iObjectiveID );
  } 
  else if (strObjectiveName == "ICARUS_MO_Track" ) {
        iObjectiveID++;
        strReturnText += generateXMLTrackObjective( text, iObjectiveID );
  }
  else if (strObjectiveName == "ICARUS_MO_UAVSearch" )   { 
        iObjectiveID++;
	strReturnText += generateXMLUAVSearchObjective( text, iObjectiveID );
  } 
  else if (strObjectiveName == "ICARUS_MO_UUVSearch" )   { 
    iObjectiveID++;
    strReturnText += generateXMLUUVSearchObjective( text, iObjectiveID );
  }
  else if (strObjectiveName == "ICARUS_MO_UUVEo" ) {
        iObjectiveID++;
        strReturnText += generateXMLUUVEOObjective( text, iObjectiveID );
  }
  else if (strObjectiveName == "ICARUS_MO_REImage" ) {
        iObjectiveID++;
        strReturnText += generateXMLReimageObjective( text, iObjectiveID );
  }
 
 
  return strReturnText;
}

/*
 *  Generate the vehicle information in XML
*/
string acdl_to_xml_converter::generateXMLVehicleInfo(int iVehicleID) {
  string strReturnText="";
  char sID[256];
  strReturnText += "<vehicle_id>";
  sprintf(sID, "%i",iVehicleID);
  string strID = sID;
  strReturnText += sID;
  strReturnText += "</vehicle_id>";
  return strReturnText;
}


string acdl_to_xml_converter::generateXMLHeader(int iMessageNumber, int iPriority) {
  char sNum[256];

  string strReturnText="";
  strReturnText += "<icarus>";

  strReturnText += "<priority>";
  sprintf(sNum, "%i",iPriority);
  string strNum = sNum;
  strReturnText += sNum;	
  strReturnText +="</priority>"; 

  strReturnText += "<timestamp>";
  tm localTime;
  char date[100], month[100], day[100], year[100], hour[100], min[100], sec[100], tmZone[100];    
  char chartime[1024];
  time_t currentTime = time(NULL);
  strncpy(date, ctime(&currentTime), sizeof(date));
  localTime = *localtime(&currentTime);
  strftime(month, sizeof(month), "%m", &localTime);
  strftime(day,   sizeof(day), "%d", &localTime);
  strftime(year,  sizeof(year), "%Y", &localTime);
  strftime(hour,  sizeof(hour), "%H", &localTime);
  strftime(min,   sizeof(min), "%M", &localTime);
  strftime(sec,   sizeof(sec), "%S", &localTime);
  strftime(tmZone,   sizeof(tmZone), "%z", &localTime);

  
  string strTimeZone = tmZone;
  
  strTimeZone = strTimeZone.substr(0,3) + ":" + strTimeZone.substr(3,2);
  sprintf( chartime,"%s-%s-%sT%s:%s:%s%s",year, month,day,
		                              hour, min, sec, strTimeZone.c_str());

  /*
  sprintf( chartime,"%s-%s-%sT%s:%s:%s%s",year, month,day,
		                              hour, min, sec,
			                          tmZone);
  
  */
  string strTime = chartime;
  strReturnText += strTime;
  strReturnText += "</timestamp>";

  strReturnText += "<message_number>"; 
  sprintf(sNum, "%i",iMessageNumber);
  strNum = sNum;
  strReturnText += sNum;
  strReturnText += "</message_number>";
  strReturnText += "<message_type>GroupMissionData</message_type>";
  strReturnText += "<source>MPPC</source>";
  strReturnText += "<content>";
  strReturnText += "<group_mission_data>";  
  return strReturnText;
}

/*
 *  Generate XML trailer 
 */
string acdl_to_xml_converter::generateXMLTrailer() {
  
  string strReturnText="";
  strReturnText += "</group_mission_data>";  
  strReturnText += "</content>";
  strReturnText += "</icarus>";
  return strReturnText;
}

/*
 *  Erase quotes and plus 
 */
string acdl_to_xml_converter::eraseGarbage( string text) {

  size_t x = text.find("\"");
  while( x < string::npos ) { 
	text.replace(x,1,"");
	x = text.find( "\"", x+1 );
  }
  return text;
}

/*
 *  Convert the date style and time to CIM style 
 */
string acdl_to_xml_converter::convertToCIMStyleDateTime(  string strDateTime ) {
  //incoming string format 19:00:00+12/31/1969

  string strOutputString="";

  char tmZone[100], date[100], chartime[1024];    
  tm localTime;

  time_t currentTime = time(NULL);  
  strncpy(date, ctime(&currentTime), sizeof(date));
  localTime = *localtime(&currentTime);
  strftime(tmZone,   sizeof(tmZone), "%z", &localTime);

  sprintf( chartime,"%s",tmZone);
  string strTimeZone = chartime;

  strTimeZone = strTimeZone.substr(0,3) + ":" + strTimeZone.substr(3,2);
 
  size_t plus = strDateTime.find("+");
  string strTime = strDateTime.substr(0,plus);
  string strDate = strDateTime.substr(plus+1,strDateTime.size()-plus+1);

  string strMonth = strtok((char*)strDate.c_str(),"/");
  string strDay = strtok (NULL, "/");
  string strYear =  strtok (NULL, "/");

  strOutputString = strYear+"-"+strMonth+"-"+strDay+"T"+strTime+strTimeZone;
  return strOutputString;
}

/*
 *  Get the parm the value given the parameter name 
 */
string acdl_to_xml_converter::getParmValueFromParmName( string strParmName, string text ) {

  size_t position=0;
  if((position = text.find(strParmName, position)) != string::npos) {
	position = text.find("PARM_VALUE",position);
	position += (strParmValue.size()+1);
	int iEndOfLine = text.find("\n",position);
	string strParmValue = text.substr(position, (iEndOfLine-position) );	
	strParmValue = strParmValue.substr(1,strParmValue.size()-2);
	strParmValue = eraseGarbage( strParmValue );
	return strParmValue;
  }
  return "";
}

/*
 *  Generate the complete SAR objective in XML 
 */
string acdl_to_xml_converter::generateXMLSARObjective(string text, int iID) {

  string strXMLObjective = ""; 
  
  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";
  
 
  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "synthetic_aperture_radar_image";
  strXMLObjective += "</objective_type>"; 
  
  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";            
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";           
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                
  strXMLObjective += "</priority>";
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                  
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<SAR_data>";
 
  string strRange = getParmValueFromParmName( "ground_range", text );
  int comma = strRange.find(',', 0);
  string strMinRange = strRange.substr(0,comma);
  string strMaxRange = strRange.substr(comma+1,strRange.size()-comma+1);
  strXMLObjective += "<ground_range type=\"min\" units=\"nautical miles\">";
  strXMLObjective += strMinRange;
  strXMLObjective += "</ground_range>";
  strXMLObjective += "<ground_range type=\"max\" units=\"nautical miles\">";
  strXMLObjective += strMaxRange;
  strXMLObjective += "</ground_range>";
  
  string strApproachAngle = getParmValueFromParmName( "approach_angle", text );
  comma = strApproachAngle.find(',', 0);
  string strMinApproach = strApproachAngle.substr(0,comma);
  string strMaxApproach = strApproachAngle.substr(comma+1,strApproachAngle.size()-comma+1);
  strXMLObjective += "<approach_angle type=\"min\">";
  strXMLObjective += strMinApproach;
  strXMLObjective += "</approach_angle>";
  strXMLObjective += "<approach_angle type=\"max\">";
  strXMLObjective += strMaxApproach;
  strXMLObjective += "</approach_angle>";
 
  string strResolution = getParmValueFromParmName( "image_resolution", text );
  strXMLObjective += "<resolution units=\"feet per pixel\">";
  strXMLObjective += strResolution;
  strXMLObjective += "</resolution>";
  
  string strImageQuality = getParmValueFromParmName( "image_quality", text );
  strXMLObjective += "<image_quality units=\"dB\">";
  strXMLObjective += strImageQuality;
  strXMLObjective += "</image_quality>";
  
  string strAltitude = getParmValueFromParmName( "altitude", text );
  comma = strAltitude.find(',', 0);
  string strMinAltitude = strAltitude.substr(0,comma);
  string strMaxAltitude = strAltitude.substr(comma+1,strAltitude.size()-comma+1);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";
  strXMLObjective += "<altitude type=\"max\" units=\"feet MSL\">";
  strXMLObjective += strMaxAltitude;
  strXMLObjective += "</altitude>";
  
  string strMapTime = getParmValueFromParmName( "map_time", text );
  strXMLObjective += "<map_time>";
  strXMLObjective += strMapTime;
  strXMLObjective += "</map_time>";

  string strBufferLength = getParmValueFromParmName( "buffer_length", text );
  strXMLObjective += "<buffer_length units=\"nautical miles\">";
  strXMLObjective += strBufferLength;
  strXMLObjective += "</buffer_length>";
  
  string strBufferAngle = getParmValueFromParmName( "buffer_angle", text );
  strXMLObjective += "<buffer_angle_LOS>";
  strXMLObjective += strBufferAngle;
  strXMLObjective += "</buffer_angle_LOS>";

  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";


  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>"; 

  strXMLObjective += "<ground_clearance type=\"min\" units=\"feet\">";
  string strGroundClearance = getParmValueFromParmName( "ground_clearance", text );
  strXMLObjective += strGroundClearance;
  strXMLObjective += "</ground_clearance>"; 
 
  string strClutterRange = getParmValueFromParmName( "clutter_to_noise_ratio", text );
  strXMLObjective += "<clutter_to_noise_range type=\"max\" units=\"nm\">";
  strXMLObjective += strClutterRange;
  strXMLObjective += "</clutter_to_noise_range>";

  string strPeakPower = getParmValueFromParmName( "peak_power", text );
  strXMLObjective += "<peak_power units=\"watts\">";
  strXMLObjective += strPeakPower;
  strXMLObjective += "</peak_power>";

  string strDutyCycle = getParmValueFromParmName( "duty_cycle", text );
  strXMLObjective += "<duty_cycle>";
  strXMLObjective += strDutyCycle;
  strXMLObjective += "</duty_cycle>";

  string strGain = getParmValueFromParmName( "gain", text );
  strXMLObjective += "<gain units=\"dB\">";
  strXMLObjective += strGain;
  strXMLObjective += "</gain>";

  string strTerrainBack = getParmValueFromParmName( "terrain_backscatter", text );
  strXMLObjective += "<terrain_backscatter units=\"dB\">";
  strXMLObjective += strTerrainBack;
  strXMLObjective += "</terrain_backscatter>";

  string strNoiseFigure = getParmValueFromParmName( "noise_figure", text );
  strXMLObjective += "<noise_figure units=\"dB\">";
  strXMLObjective += strNoiseFigure;
  strXMLObjective += "</noise_figure>";

  string strLoss = getParmValueFromParmName( "loss", text );
  strXMLObjective += "<loss units=\"dB\">";
  strXMLObjective += strLoss;
  strXMLObjective += "</loss>";

  string strWavelength = getParmValueFromParmName( "wavelength", text );
  strXMLObjective += "<xmitter_wavelength units=\"feet\">";
  strXMLObjective += strWavelength;
  strXMLObjective += "</xmitter_wavelength>";

  string strIntegrationTime = getParmValueFromParmName( "integration_time", text );
  strXMLObjective += "<integration_time>";
  strXMLObjective += strIntegrationTime;
  strXMLObjective += "</integration_time>";

  string strAntennaTiltAngle = getParmValueFromParmName( "antenna_tilt_angle", text );
  strXMLObjective += "<antenna_tilt_angle>";
  strXMLObjective += strAntennaTiltAngle;
  strXMLObjective += "</antenna_tilt_angle>";

  string strFOVLimitAngle = getParmValueFromParmName( "fov_limit_angle", text );
  strXMLObjective += "<FOV_limit_angle>";
  strXMLObjective += strFOVLimitAngle;
  strXMLObjective += "</FOV_limit_angle>";

  strXMLObjective += "</SAR_data>";

  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";

  return strXMLObjective;
}

/*
 *  Objective the complete Steerpoint objective in XML 
 */
string acdl_to_xml_converter::generateXMLSteerpointObjective(string text, int iID) {

  string strXMLObjective = ""; 

  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "steerpoint";
  strXMLObjective += "</objective_type>"; 

  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";             
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";           
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                   
  strXMLObjective += "</priority>"; 
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                 
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<STEER_data>";

  string strAltitude = getParmValueFromParmName( "altitude", text );
  int comma = strAltitude.find(',', 0);
  string strMinAltitude = strAltitude.substr(0,comma);
  string strMaxAltitude = strAltitude.substr(comma+1,strAltitude.size()-comma+1);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";
  strXMLObjective += "<altitude type=\"max\" units=\"feet MSL\">";
  strXMLObjective += strMaxAltitude;
  strXMLObjective += "</altitude>";

  string strRadius = getParmValueFromParmName( "radius", text );
  strXMLObjective += "<radius units=\"nautical miles\">";
  strXMLObjective += strRadius;
  strXMLObjective += "</radius>";

  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
 
  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>"; 

  strXMLObjective += "<ground_clearance type=\"min\" units=\"feet\">";
  string strGroundClearance = getParmValueFromParmName( "ground_clearance", text );
  strXMLObjective += strGroundClearance;
  strXMLObjective += "</ground_clearance>"; 
  strXMLObjective += "</STEER_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";
 
  return strXMLObjective;
}

/*
 *  Generate the complete Loiter objective in XML 
 */
string acdl_to_xml_converter::generateXMLLoiterObjective(string text, int iID) {
  
  string strXMLObjective = ""; 
  
  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "loiter";
  strXMLObjective += "</objective_type>"; 

  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";              
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";            
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                   
  strXMLObjective += "</priority>";
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                 
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<LOITER_data>";

  string strApproachAngle = getParmValueFromParmName( "approach_angle", text );
  int comma = strApproachAngle.find(',', 0);
  string strMinApproach = strApproachAngle.substr(0,comma);
  string strMaxApproach = strApproachAngle.substr(comma+1,strApproachAngle.size()-comma+1);
  strXMLObjective += "<approach_angle type=\"min\">";
  strXMLObjective += strMinApproach;
  strXMLObjective += "</approach_angle>";
  strXMLObjective += "<approach_angle type=\"max\">";
  strXMLObjective += strMaxApproach;
  strXMLObjective += "</approach_angle>";

  string strShape = getParmValueFromParmName( "shape", text ); 
  strXMLObjective += "<loiter_shape>";
  if(strShape == "circle") {
	strXMLObjective += "<circle>";
	string strRadius = getParmValueFromParmName( "radius", text );
	strXMLObjective += "<radius units=\"nautical miles\">";
	strXMLObjective += strRadius;
	strXMLObjective += "</radius></circle>";
  }
  else {	
	string strRadius = getParmValueFromParmName( "radius", text );
	strXMLObjective += "<racetrack><major_axis units=\"nautical miles\">";	
	strXMLObjective += strRadius;
	strXMLObjective += "</major_axis><minor_axis units=\"nautical miles\">";
	strXMLObjective += strRadius;
	strXMLObjective += "</minor_axis></racetrack>";
  }
  strXMLObjective += "</loiter_shape>";

  string strAltitude = getParmValueFromParmName( "altitude", text );
  comma = strAltitude.find(',', 0);
  string strMinAltitude = strAltitude.substr(0,comma);
  string strMaxAltitude = strAltitude.substr(comma+1,strAltitude.size()-comma+1);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";
  strXMLObjective += "<altitude type=\"max\" units=\"feet MSL\">";
  strXMLObjective += strMaxAltitude;
  strXMLObjective += "</altitude>";

  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
 
  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>";

  string strDwellTime = getParmValueFromParmName( "dwell_time", text );
  strXMLObjective += "<dwell_time>";
  strXMLObjective += strDwellTime;
  strXMLObjective += "</dwell_time>";

  strXMLObjective += "<ground_clearance type=\"min\" units=\"feet\">";
  string strGroundClearance = getParmValueFromParmName( "ground_clearance", text );
  strXMLObjective += strGroundClearance;
  strXMLObjective += "</ground_clearance>"; 
  strXMLObjective += "</LOITER_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";
  
  return strXMLObjective;
}

/*
 *  Generate the complete PGB objective in XML 
 */
string acdl_to_xml_converter::generateXMLPGBObjective(string text, int iID) {
  
  string strXMLObjective = ""; 
  
  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";
  
  strXMLObjective += "<objective_type>";
  strXMLObjective += "precision_guided_bomb";
  strXMLObjective += "</objective_type>"; 
  
  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";           
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";            
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                  
  strXMLObjective += "</priority>"; 
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                 
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<PGB_data>";

  string strRange = getParmValueFromParmName( "ground_range", text );
  int comma = strRange.find(',', 0);
  string strMinRange = strRange.substr(0,comma);
  string strMaxRange = strRange.substr(comma+1,strRange.size()-comma+1);
  strXMLObjective += "<ground_range type=\"min\" units=\"nautical miles\">";
  strXMLObjective += strMinRange;
  strXMLObjective += "</ground_range>";
  strXMLObjective += "<ground_range type=\"max\" units=\"nautical miles\">";
  strXMLObjective += strMaxRange;
  strXMLObjective += "</ground_range>";
  
  string strApproachAngle = getParmValueFromParmName( "approach_angle", text );
  comma = strApproachAngle.find(',', 0);
  string strMinApproach = strApproachAngle.substr(0,comma);
  string strMaxApproach = strApproachAngle.substr(comma+1,strApproachAngle.size()-comma+1);
  strXMLObjective += "<approach_angle type=\"min\">";
  strXMLObjective += strMinApproach;
  strXMLObjective += "</approach_angle>";
  strXMLObjective += "<approach_angle type=\"max\">";
  strXMLObjective += strMaxApproach;
  strXMLObjective += "</approach_angle>";
 
  string strAltitude = getParmValueFromParmName( "release_altitude", text );
  comma = strAltitude.find(',', 0);
  string strMinAltitude = strAltitude.substr(0,comma);
  string strMaxAltitude = strAltitude.substr(comma+1,strAltitude.size()-comma+1);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";
  strXMLObjective += "<altitude type=\"max\" units=\"feet MSL\">";
  strXMLObjective += strMaxAltitude;
  strXMLObjective += "</altitude>";

  string strInEnvelope = getParmValueFromParmName( "in_envelope_time", text );
  strXMLObjective += "<in_envelope_time>";
  strXMLObjective += strInEnvelope;   
  strXMLObjective += "</in_envelope_time>";
  
  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
 
  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>"; 

  strXMLObjective += "<ground_clearance type=\"min\" units=\"feet\">";
  string strGroundClearance = getParmValueFromParmName( "ground_clearance", text );
  strXMLObjective += strGroundClearance;
  strXMLObjective += "</ground_clearance>"; 

  strXMLObjective += "<track_num>";
  string strTrackNum = getParmValueFromParmName( "track_num", text );
  strXMLObjective += strTrackNum;
  strXMLObjective += "</track_num>";

  strXMLObjective += "<entityID>";
  strXMLObjective += "<site>";
  string strSite = getParmValueFromParmName( "site", text );
  strXMLObjective += strSite;
  strXMLObjective += "</site>";

  strXMLObjective += "<application>";
  string strApplication = getParmValueFromParmName( "application", text );
  strXMLObjective += strApplication;
  strXMLObjective += "</application>";

  strXMLObjective += "<entity>";
  string strEntity = getParmValueFromParmName( "entity", text );
  strXMLObjective += strEntity;
  strXMLObjective += "</entity>";
  strXMLObjective += "</entityID>"; 

  strXMLObjective += "</PGB_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";

  return strXMLObjective;
}

/*
 *  Generate the complete AGM objective in XML 
 */
string acdl_to_xml_converter::generateXMLAGMObjective(string text, int iID) {
  
  string strXMLObjective = ""; 
  
  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";
  
  strXMLObjective += "<objective_type>";
  strXMLObjective += "air_ground_missile";
  strXMLObjective += "</objective_type>"; 
  
  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";             
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";            
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                   
  strXMLObjective += "</priority>"; 
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                 
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<AGM_data>";

  string strRange = getParmValueFromParmName( "ground_range", text );
  int comma = strRange.find(',', 0);
  string strMinRange = strRange.substr(0,comma);
  string strMaxRange = strRange.substr(comma+1,strRange.size()-comma+1);
  strXMLObjective += "<ground_range type=\"min\" units=\"nautical miles\">";
  strXMLObjective += strMinRange;
  strXMLObjective += "</ground_range>";
  strXMLObjective += "<ground_range type=\"max\" units=\"nautical miles\">";
  strXMLObjective += strMaxRange;
  strXMLObjective += "</ground_range>";
  
  string strApproachAngle = getParmValueFromParmName( "approach_angle", text );
  comma = strApproachAngle.find(',', 0);
  string strMinApproach = strApproachAngle.substr(0,comma);
  string strMaxApproach = strApproachAngle.substr(comma+1,strApproachAngle.size()-comma+1);
  strXMLObjective += "<approach_angle type=\"min\">";
  strXMLObjective += strMinApproach;
  strXMLObjective += "</approach_angle>";
  strXMLObjective += "<approach_angle type=\"max\">";
  strXMLObjective += strMaxApproach;
  strXMLObjective += "</approach_angle>";

  string strAltitude = getParmValueFromParmName( "release_altitude", text );
  comma = strAltitude.find(',', 0);
  string strMinAltitude = strAltitude.substr(0,comma);
  string strMaxAltitude = strAltitude.substr(comma+1,strAltitude.size()-comma+1);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";
  strXMLObjective += "<altitude type=\"max\" units=\"feet MSL\">";
  strXMLObjective += strMaxAltitude;
  strXMLObjective += "</altitude>";

  string strInEnvelope = getParmValueFromParmName( "in_envelope_time", text );
  strXMLObjective += "<in_envelope_time>";
  strXMLObjective += strInEnvelope;   
  strXMLObjective += "</in_envelope_time>";


  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
 
  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>"; 

  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>"; 

  strXMLObjective += "<ground_clearance type=\"min\" units=\"feet\">";
  string strGroundClearance = getParmValueFromParmName( "ground_clearance", text );
  strXMLObjective += strGroundClearance;
  strXMLObjective += "</ground_clearance>"; 

  strXMLObjective += "<track_num>";
  string strTrackNum = getParmValueFromParmName( "track_num", text );
  strXMLObjective += strTrackNum;
  strXMLObjective += "</track_num>";

  strXMLObjective += "<entityID>";
  strXMLObjective += "<site>";
  string strSite = getParmValueFromParmName( "site", text );
  strXMLObjective += strSite;
  strXMLObjective += "</site>";

  strXMLObjective += "<application>";
  string strApplication = getParmValueFromParmName( "application", text );
  strXMLObjective += strApplication;
  strXMLObjective += "</application>";

  strXMLObjective += "<entity>";
  string strEntity = getParmValueFromParmName( "entity", text );
  strXMLObjective += strEntity;
  strXMLObjective += "</entity>";
  strXMLObjective += "</entityID>"; 

  strXMLObjective += "</AGM_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";

  return strXMLObjective;
}

/*
 *  Generate the complete COMM objective in XML 
 */
string acdl_to_xml_converter::generateXMLCommunicationObjective(string text, int iID) {

  string strXMLObjective = ""; 
  
  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";
  
  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "communication";
  strXMLObjective += "</objective_type>"; 
  
  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";              
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";        
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                  
  strXMLObjective += "</priority>"; 
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                 
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<COMM_data>";
 
  string strRange = getParmValueFromParmName( "ground_range", text );
  int comma = strRange.find(',', 0);
  string strMinRange = strRange.substr(0,comma);
  string strMaxRange = strRange.substr(comma+1,strRange.size()-comma+1);
  strXMLObjective += "<ground_range type=\"min\" units=\"nautical miles\">";
  strXMLObjective += strMinRange;
  strXMLObjective += "</ground_range>";
  strXMLObjective += "<ground_range type=\"max\" units=\"nautical miles\">";
  strXMLObjective += strMaxRange;
  strXMLObjective += "</ground_range>";
  
  string strApproachAngle = getParmValueFromParmName( "approach_angle", text );
  comma = strApproachAngle.find(',', 0);
  string strMinApproach = strApproachAngle.substr(0,comma);
  string strMaxApproach = strApproachAngle.substr(comma+1,strApproachAngle.size()-comma+1);
  strXMLObjective += "<approach_angle type=\"min\">";
  strXMLObjective += strMinApproach;
  strXMLObjective += "</approach_angle>";
  strXMLObjective += "<approach_angle type=\"max\">";
  strXMLObjective += strMaxApproach;
  strXMLObjective += "</approach_angle>";
 
  string strAltitude = getParmValueFromParmName( "altitude", text );
  comma = strAltitude.find(',', 0);
  string strMinAltitude = strAltitude.substr(0,comma);
  string strMaxAltitude = strAltitude.substr(comma+1,strAltitude.size()-comma+1);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";
  strXMLObjective += "<altitude type=\"max\" units=\"feet MSL\">";
  strXMLObjective += strMaxAltitude;
  strXMLObjective += "</altitude>";

  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>";   
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>";  

  string strDwellTime = getParmValueFromParmName( "dwell_time", text );
  strXMLObjective += "<dwell_time>";
  strXMLObjective += strDwellTime;
  strXMLObjective += "</dwell_time>";

  strXMLObjective += "<ground_clearance type=\"min\" units=\"feet\">";
  string strGroundClearance = getParmValueFromParmName( "ground_clearance", text );
  strXMLObjective += strGroundClearance;
  strXMLObjective += "</ground_clearance>"; 

  string strPeakPower = getParmValueFromParmName( "peak_power", text );
  strXMLObjective += "<peak_power units=\"watts\">";
  strXMLObjective += strPeakPower;
  strXMLObjective += "</peak_power>";

  string strGain = getParmValueFromParmName( "gain", text );
  strXMLObjective += "<gain units=\"dB\">";
  strXMLObjective += strGain;
  strXMLObjective += "</gain>";

  string strTerrainBack = getParmValueFromParmName( "terrain_backscatter", text );
  strXMLObjective += "<terrain_backscatter units=\"dB\">";
  strXMLObjective += strTerrainBack;
  strXMLObjective += "</terrain_backscatter>";
  
  string strSignalToNoise = getParmValueFromParmName( "signal_to_noise", text );
  strXMLObjective += "<signal_to_noise units=\"dB\">";
  strXMLObjective += strSignalToNoise;
  strXMLObjective += "</signal_to_noise>";

  string strCommToCallSign = getParmValueFromParmName( "commto_callsign", text );
  strXMLObjective += "<commto_callsign>";
  strXMLObjective += strCommToCallSign;
  strXMLObjective += "</commto_callsign>";
  
  string strLoss = getParmValueFromParmName( "loss", text );
  strXMLObjective += "<loss units=\"dB\">";
  strXMLObjective += strLoss;
  strXMLObjective += "</loss>";

  string strWavelength = getParmValueFromParmName( "wavelength", text );
  strXMLObjective += "<xmitter_wavelength units=\"feet\">";
  strXMLObjective += strWavelength;
  strXMLObjective += "</xmitter_wavelength>";

  string strTiltAngle = getParmValueFromParmName( "tilt_angle", text );
  strXMLObjective += "<antenna_tilt_angle>";
  strXMLObjective += strTiltAngle;
  strXMLObjective += "</antenna_tilt_angle>";

  string strFOVLimitAngle = getParmValueFromParmName( "fov_limit_angle", text );
  strXMLObjective += "<FOV_limit_angle>";
  strXMLObjective += strFOVLimitAngle;
  strXMLObjective += "</FOV_limit_angle>";

  strXMLObjective += "</COMM_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";
 
  return strXMLObjective;
}

/*
 *  Generate the complete Search objective in XML 
 */
string acdl_to_xml_converter::generateXMLEOIRObjective(string text, int iID) {

  string strXMLObjective = ""; 

  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";

  string strAltitude = getParmValueFromParmName( "altitude", text );
  int comma = strAltitude.find(',', 0);
  string strMinAltitude = strAltitude.substr(0,comma);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";

  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "electro_optical_infrared_image";
  strXMLObjective += "</objective_type>"; 

  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";               
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";           
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                   
  strXMLObjective += "</priority>";   
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                   
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<EOIR_data>";

  string strRange = getParmValueFromParmName( "range", text );
  comma = strRange.find(',', 0);
  string strMinRange = strRange.substr(0,comma);
  string strMaxRange = strRange.substr(comma+1,strRange.size()-comma+1);
  strXMLObjective += "<range type=\"min\" units=\"nautical miles\">";
  strXMLObjective += strMinRange;
  strXMLObjective += "</range>";
  strXMLObjective += "<range type=\"max\" units=\"nautical miles\">";
  strXMLObjective += strMaxRange;
  strXMLObjective += "</range>";

  string strRangeResolution = getParmValueFromParmName( "range_resolution", text );
  strXMLObjective += "<range_resolution units=\"nautical miles\">";
  strXMLObjective += strRangeResolution;
  strXMLObjective += "</range_resolution>";

  string strCoverageTime = getParmValueFromParmName( "coverage_time", text );
  strXMLObjective += "<coverage_time>";
  strXMLObjective += strCoverageTime;
  strXMLObjective += "</coverage_time>";

  strAltitude = getParmValueFromParmName( "altitude", text );
  comma = strAltitude.find(',', 0);
  strMinAltitude = strAltitude.substr(0,comma);
  string strMaxAltitude = strAltitude.substr(comma+1,strAltitude.size()-comma+1);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";
  strXMLObjective += "<altitude type=\"max\" units=\"feet MSL\">";
  strXMLObjective += strMaxAltitude;
  strXMLObjective += "</altitude>";

  string strAltitudeResolution = getParmValueFromParmName( "altitude_resolution", text );
  strXMLObjective += "<altitude_resolution units=\"feet MSL\">";
  strXMLObjective += strAltitudeResolution;
  strXMLObjective += "</altitude_resolution>";

  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
 
  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>"; 

  strXMLObjective += "<ground_clearance type=\"min\" units=\"feet\">";
  string strGroundClearance = getParmValueFromParmName( "ground_clearance", text );
  strXMLObjective += strGroundClearance;
  strXMLObjective += "</ground_clearance>"; 

  string strBearing = getParmValueFromParmName( "bearing", text );
  comma = strBearing.find(',', 0);
  string strMinBearing = strBearing.substr(0,comma);
  string strMaxBearing = strBearing.substr(comma+1,strBearing.size()-comma+1);
  strXMLObjective += "<bearing type=\"min\" units=\"degrees\">";
  strXMLObjective += strMinBearing;
  strXMLObjective += "</bearing>";
  strXMLObjective += "<bearing type=\"max\" units=\"degrees\">";
  strXMLObjective += strMaxBearing;
  strXMLObjective += "</bearing>";
 
  string strBearingResolution = getParmValueFromParmName( "bearing_resolution", text );
  strXMLObjective += "<bearing_resolution units=\"degrees\">";
  strXMLObjective += strBearingResolution;
  strXMLObjective += "</bearing_resolution>";

  string strAspect = getParmValueFromParmName( "aspect", text );
  comma = strAspect.find(',', 0);
  string strMinAspect = strAspect.substr(0,comma);
  string strMaxAspect = strAspect.substr(comma+1,strAspect.size()-comma+1);
  strXMLObjective += "<aspect type=\"min\">";
  strXMLObjective += strMinAspect;
  strXMLObjective += "</aspect>";
  strXMLObjective += "<aspect type=\"max\">";
  strXMLObjective += strMaxAspect;
  strXMLObjective += "</aspect>";
 
  string strAspectResolution = getParmValueFromParmName( "aspect_resolution", text );
  strXMLObjective += "<aspect_resolution>";
  strXMLObjective += strAspectResolution;
  strXMLObjective += "</aspect_resolution>";

  string strSpeed = getParmValueFromParmName( "speed", text );
  strXMLObjective += "<speed type=\"air\" units=\"knots\">";
  strXMLObjective += strSpeed;
  strXMLObjective += "</speed>";

  string strAnalysisStepSize = getParmValueFromParmName( "analysis_step_size", text );
  strXMLObjective += "<analysis_step_size>";
  strXMLObjective += strAnalysisStepSize;
  strXMLObjective += "</analysis_step_size>";
  
  string strQuality = getParmValueFromParmName( "quality", text );
  strXMLObjective += "<quality>";
  strXMLObjective += strQuality;
  strXMLObjective += "</quality>";  

  string strTargetRange = getParmValueFromParmName( "target_range", text );
  comma = strTargetRange.find(',', 0);
  string strMinTargetRange = strTargetRange.substr(0,comma);
  string strMaxTargetRange = strTargetRange.substr(comma+1,strTargetRange.size()-comma+1);
  strXMLObjective += "<target_range type=\"min\" units=\"nautical miles\">";
  strXMLObjective += strMinTargetRange;
  strXMLObjective += "</target_range>";
  strXMLObjective += "<target_range type=\"max\" units=\"nautical miles\">";
  strXMLObjective += strMaxTargetRange;
  strXMLObjective += "</target_range>";

  string strEarthRadius = getParmValueFromParmName( "earth_radius", text );
  strXMLObjective += "<earth_radius>";
  strXMLObjective += strEarthRadius;
  strXMLObjective += "</earth_radius>";

  strXMLObjective += "</EOIR_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";

  return strXMLObjective;
}

/*
 *  Generate the complete Sentry objective in XML 
 */
string acdl_to_xml_converter::generateXMLSentryObjective(string text, int iID) {

  string strXMLObjective = ""; 
  
  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";
  
 
  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "sentry";
  strXMLObjective += "</objective_type>"; 
  
  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";              
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";           
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                    
  strXMLObjective += "</priority>"; 
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                 
  strXMLObjective += "</comm_required>";  
  strXMLObjective += "<type_data>";
  strXMLObjective += "<SENTRY_data>";
  
  string strCameraAngle = getParmValueFromParmName( "camera_angle", text );
  strXMLObjective += "<camera_angle>";
  strXMLObjective += strCameraAngle;
  strXMLObjective += "</camera_angle>";

  string strSentryType = getParmValueFromParmName( "sentry_type", text );
  strXMLObjective += "<sentry_type>";
  strXMLObjective += strSentryType;
  strXMLObjective += "</sentry_type>";
 
  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  string strDateTime = getParmValueFromParmName( "time_window", text );
  int comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  strXMLObjective += "</SENTRY_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";

  return strXMLObjective;
}



/*
 *  Generate the complete MAD objective in XML 
 */
string acdl_to_xml_converter::generateXMLMADObjective(string text, int iID) {
  
  string strXMLObjective = ""; 
  
  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "magnetic_anomaly_detection";
  strXMLObjective += "</objective_type>"; 

  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";              
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";          
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                  
  strXMLObjective += "</priority>";
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                  
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<MAD_data>";

  // approach angle setup
  string strApproachAngle = getParmValueFromParmName( "approach_angle", text );
  int comma = strApproachAngle.find(',', 0);
  string strMinApproach = strApproachAngle.substr(0,comma);
  string strMaxApproach = strApproachAngle.substr(comma+1,strApproachAngle.size()-comma+1);
  strXMLObjective += "<approach_angle type=\"min\">";
  strXMLObjective += strMinApproach;
  strXMLObjective += "</approach_angle>";
  strXMLObjective += "<approach_angle type=\"max\">";
  strXMLObjective += strMaxApproach;
  strXMLObjective += "</approach_angle>";

  // search area setup
  string strSearch = getParmValueFromParmName( "search_area", text );
  strXMLObjective += "<search_area units=\"square feet\">";
  strXMLObjective += strSearch;
  strXMLObjective += "</search_area>";

  // altitude
  string strAltitude = getParmValueFromParmName( "altitude", text );
  comma = strAltitude.find(',', 0);
  string strMinAltitude = strAltitude.substr(0,comma);
  string strMaxAltitude = strAltitude.substr(comma+1,strAltitude.size()-comma+1);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";
  strXMLObjective += "<altitude type=\"max\" units=\"feet MSL\">";
  strXMLObjective += strMaxAltitude;
  strXMLObjective += "</altitude>";

  // timesetup
  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
   strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  // fuel/endurance setup
  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>";

  strXMLObjective += "<ground_clearance type=\"min\" units=\"feet\">";
  string strGroundClearance = getParmValueFromParmName( "ground_clearance", text );
  strXMLObjective += strGroundClearance;
  strXMLObjective += "</ground_clearance>"; 
  strXMLObjective += "</MAD_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";
 
  return strXMLObjective;
}

/*
 *  Generate the complete Track objective in XML 
 */
string acdl_to_xml_converter::generateXMLTrackObjective(string text, int iID) {
  
  string strXMLObjective = ""; 
  
  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "track";
  strXMLObjective += "</objective_type>"; 

  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";             
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";           
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                       
  strXMLObjective += "</priority>"; 
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                  
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<TRACK_data>";

  strXMLObjective += "<track_num>";
  string strTargetId = getParmValueFromParmName( "target_id", text );
  strXMLObjective += strTargetId;
  strXMLObjective += "</track_num>";

  // image interval (all other durations are hardcoded so i followed suit)
  string strImageInterval = getParmValueFromParmName( "image_interval", text );
  strXMLObjective += "<image_interval>";
  strXMLObjective += strImageInterval;
  strXMLObjective += "</image_interval>";

  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  int comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
 
  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  strXMLObjective += "</TRACK_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";
   
  return strXMLObjective;
}


string acdl_to_xml_converter::generateXMLUAVSearchObjective(string text, int iID) {
  string strXMLObjective = ""; 
  
  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "uav_search";
  strXMLObjective += "</objective_type>"; 

  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";              
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";            
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                       
  strXMLObjective += "</priority>"; 
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                  
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<UAVSEARCH_data>";

  // search pattern
  strXMLObjective += "<zone_id>"; 
  string strZoneID = getParmValueFromParmName( "id", text );
  strXMLObjective += "</zone_id>";
  strXMLObjective += "<search_pattern>";
  string strPattern = getParmValueFromParmName( "search_pattern", text );
  strXMLObjective += strPattern;
  strXMLObjective += "</search_pattern>"; 

  string strShape = getParmValueFromParmName( "shape", text ); 
  strXMLObjective += "<loiter_shape>";
  if(strShape == "circle") {
	strXMLObjective += "<circle>";
	string strRadius = getParmValueFromParmName( "radius", text );
	strXMLObjective += "<radius units=\"nautical miles\">";
	strXMLObjective += strRadius;
	strXMLObjective += "</radius></circle>";
  }
  else {	
	string strRadius = getParmValueFromParmName( "radius", text );
	strXMLObjective += "<racetrack><major_axis units=\"nautical miles\">";	
	strXMLObjective += strRadius;
	strXMLObjective += "</major_axis><minor_axis units=\"nautical miles\">";
	strXMLObjective += strRadius;
	strXMLObjective += "</minor_axis></racetrack>";
  }
  strXMLObjective += "</loiter_shape>";

  //search_time
  strXMLObjective += "<search_time>";
  string strSearchTime = getParmValueFromParmName( "search_time", text );
  strXMLObjective += "</search_time>";

  strXMLObjective += "<target_type>";
  string strTarget = getParmValueFromParmName( "vehicle_type", text );
  strXMLObjective += strTarget;
  strXMLObjective += "</target_type>"; 

  strXMLObjective += "<weather_type>";
  string strWeather = getParmValueFromParmName( "weather_type", text );
  strXMLObjective += strWeather;
  strXMLObjective += "</weather_type>"; 

  // altitude
  string strAltitude = getParmValueFromParmName( "altitude", text );
  int comma = strAltitude.find(',', 0);
  string strMinAltitude = strAltitude.substr(0,comma);
  string strMaxAltitude = strAltitude.substr(comma+1,strAltitude.size()-comma+1);
  strXMLObjective += "<altitude type=\"min\" units=\"feet MSL\">";
  strXMLObjective += strMinAltitude;
  strXMLObjective += "</altitude>";
  strXMLObjective += "<altitude type=\"max\" units=\"feet MSL\">";
  strXMLObjective += strMaxAltitude;
  strXMLObjective += "</altitude>";

  string strImageInterval = getParmValueFromParmName( "time_last_seen", text );
  strXMLObjective += "<time_last_detect>";
  strXMLObjective += strImageInterval;
  strXMLObjective += "</time_last_detect>";

  // timesetup
  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
   strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  // fuel/endurance setup
  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>";

  // ground clearance setup
  strXMLObjective += "<ground_clearance type=\"min\" units=\"feet\">";
  string strGroundClearance = getParmValueFromParmName( "ground_clearance", text );
  strXMLObjective += strGroundClearance;
  strXMLObjective += "</ground_clearance>"; 

  strXMLObjective += "</UAVSEARCH_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";
 
  return strXMLObjective;
}


string acdl_to_xml_converter::generateXMLUUVSearchObjective(string text, int iID) {
  string strXMLObjective = ""; 
  int comma;

  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";
  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "uuv_search";
  strXMLObjective += "</objective_type>"; 

 
  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";               
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";             
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                  
  strXMLObjective += "</priority>"; 
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                   
  strXMLObjective += "</comm_required>"; 
  strXMLObjective += "<type_data>";
  strXMLObjective += "<UUVSEARCH_data>";

  // search pattern
  strXMLObjective += "<search_type>";
  string strPattern = getParmValueFromParmName( "search_type", text );
  strXMLObjective += strPattern;
  strXMLObjective += "</search_type>"; 

  // zone
  strXMLObjective += "<zone_id>"; 
  string strZoneID = getParmValueFromParmName( "id", text );
  strXMLObjective += "</zone_id>";

  // timesetup
  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
   strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";
 
  strXMLObjective += "<target_parameters>";
  string strTargetParm = getParmValueFromParmName( "target_parameters", text );
  comma = strTargetParm.find(',', 0);
  string strTargetClass = strTargetParm.substr(0,comma);
  string strTargetPriority = strTargetParm.substr(comma+1,strTargetParm.size()-comma+1);
  strXMLObjective += "<target_class>";
  strXMLObjective += strTargetClass;
  strXMLObjective += "</target_class>";
  strXMLObjective += "<target_priority>";
  strXMLObjective += strTargetPriority;
  strXMLObjective += "</target_priority>";
  strXMLObjective += "</target_parameters>";

  string strApproachAngle = getParmValueFromParmName( "approach_angle", text );
  comma = strApproachAngle.find(',', 0);
  string strMinApproach = strApproachAngle.substr(0,comma);
  string strMaxApproach = strApproachAngle.substr(comma+1,strApproachAngle.size()-comma+1);
  strXMLObjective += "<approach_angle type=\"min\">";
  strXMLObjective += strMinApproach;
  strXMLObjective += "</approach_angle>";
  strXMLObjective += "<approach_angle type=\"max\">";
  strXMLObjective += strMaxApproach;
  strXMLObjective += "</approach_angle>";
 
  strXMLObjective += "</UUVSEARCH_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";

  return strXMLObjective;
}

// UUVEO xml generation
string acdl_to_xml_converter::generateXMLUUVEOObjective(string text, int iID) {

  string strXMLObjective = ""; 

  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";

  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "eo_uuv";
  strXMLObjective += "</objective_type>"; 

  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";              
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";             
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                    
  strXMLObjective += "</priority>";   
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                   
  strXMLObjective += "</comm_required>";
  strXMLObjective += "<type_data>";
  strXMLObjective += "<UUVEO_data>";

  // setup range
  string strRange = getParmValueFromParmName( "range", text );
  int comma = strRange.find(',', 0);
  string strMinRange = strRange.substr(0,comma);
  string strMaxRange = strRange.substr(comma+1,strRange.size()-comma+1);
  strXMLObjective += "<range type=\"min\" units=\"nautical miles\">";
  strXMLObjective += strMinRange;
  strXMLObjective += "</range>";
  strXMLObjective += "<range type=\"max\" units=\"nautical miles\">";
  strXMLObjective += strMaxRange;
  strXMLObjective += "</range>";

  string strCoverageTime = getParmValueFromParmName( "coverage_time", text );
  strXMLObjective += "<coverage_time>";
  strXMLObjective += strCoverageTime;
  strXMLObjective += "</coverage_time>";

  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
 
  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  strXMLObjective += "<endurance>";
  strXMLObjective += "<fuel units=\"lbs\">";
  string strFuel = getParmValueFromParmName( "fuel", text );
  strXMLObjective += strFuel;
  strXMLObjective += "</fuel>"; 
  strXMLObjective += "</endurance>"; 

  string strBearing = getParmValueFromParmName( "bearing", text );
  comma = strBearing.find(',', 0);
  string strMinBearing = strBearing.substr(0,comma);
  string strMaxBearing = strBearing.substr(comma+1,strBearing.size()-comma+1);
  strXMLObjective += "<bearing type=\"min\" units=\"degrees\">";
  strXMLObjective += strMinBearing;
  strXMLObjective += "</bearing>";
  strXMLObjective += "<bearing type=\"max\" units=\"degrees\">";
  strXMLObjective += strMaxBearing;
  strXMLObjective += "</bearing>";
 
  string strAspect = getParmValueFromParmName( "aspect", text );
  comma = strAspect.find(',', 0);
  string strMinAspect = strAspect.substr(0,comma);
  string strMaxAspect = strAspect.substr(comma+1,strAspect.size()-comma+1);
  strXMLObjective += "<aspect type=\"min\">";
  strXMLObjective += strMinAspect;
  strXMLObjective += "</aspect>";
  strXMLObjective += "<aspect type=\"max\">";
  strXMLObjective += strMaxAspect;
  strXMLObjective += "</aspect>";
 
  string strQuality = getParmValueFromParmName( "quality", text );
  strXMLObjective += "<quality>";
  strXMLObjective += strQuality;
  strXMLObjective += "</quality>";  

  strXMLObjective += "</UUVEO_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";
 
  return strXMLObjective;
}

// REIMAGE xml generation
string acdl_to_xml_converter::generateXMLReimageObjective(string text, int iID) {

  string strXMLObjective = ""; 

  strXMLObjective += "<objective>";
  char sID[256];
  strXMLObjective += "<id>MPPC-";
  sprintf(sID, "%i",iID);
  string strID = sID;
  strXMLObjective += sID;
  strXMLObjective += "</id>";
  strXMLObjective += "<user_id>MPPC</user_id>";
  strXMLObjective += "<index>"+strID;
  strXMLObjective += "</index>";
  strXMLObjective += "<location>";
  strXMLObjective += "<position>";

  string strLat = getParmValueFromParmName( "target_latitude", text );
  string strLong = getParmValueFromParmName( "target_longitude", text );
  strXMLObjective += "<latitude>"+strLat+"</latitude>";
  strXMLObjective += "<longitude>"+strLong+"</longitude>";

  strXMLObjective += "</position>";
  strXMLObjective += "</location>";

  strXMLObjective += "<objective_type>";
  strXMLObjective += "reimage";
  strXMLObjective += "</objective_type>"; 

  strXMLObjective += "<assigner>";
  strXMLObjective += "Unassigned";              
  strXMLObjective += "</assigner>"; 

  strXMLObjective += "<subtype>";
  strXMLObjective += "primary";             
  strXMLObjective += "</subtype>"; 

  strXMLObjective += "<priority>";
  strXMLObjective += getParmValueFromParmName( "priority", text );                    
  strXMLObjective += "</priority>";   
  strXMLObjective += "<comm_required>";
  strXMLObjective += "false";                   
  strXMLObjective += "</comm_required>";
  strXMLObjective += "<type_data>";
  strXMLObjective += "<REIMAGE_data>";

  strXMLObjective += "<time_span>";
  strXMLObjective += "<time type=\"min\">";
  strXMLObjective += "<absolute_time>"; 
  string strDateTime = getParmValueFromParmName( "time_window", text );
  int comma = strDateTime.find(',', 0);
  string strMinDateTime = strDateTime.substr(0,comma);
  string strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  string strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  string strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
 
  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>"; 
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</time_span>";

  strXMLObjective += "<exec_time_span>";
  strXMLObjective += "<time type=\"min\">"; 
  strXMLObjective += "<absolute_time>";   
  strDateTime = getParmValueFromParmName( "time_window", text );
  comma = strDateTime.find(',', 0);
  strMinDateTime = strDateTime.substr(0,comma);
  strMaxDateTime = strDateTime.substr(comma+1,strDateTime.size()-comma+1);
  strCIMStyleDateTimeMin = convertToCIMStyleDateTime( strMinDateTime );
  strCIMStyleDateTimeMax = convertToCIMStyleDateTime( strMaxDateTime );
  strXMLObjective += strCIMStyleDateTimeMin;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";

  strXMLObjective += "<time type=\"max\">";
  strXMLObjective += "<absolute_time>";   
  strXMLObjective += strCIMStyleDateTimeMax;
  strXMLObjective += "</absolute_time>";
  strXMLObjective += "</time>";
  strXMLObjective += "</exec_time_span>";

  string strCameraAngle = getParmValueFromParmName( "camera_angle", text );
  strXMLObjective += "<camera_angle>";
  strXMLObjective += strCameraAngle;
  strXMLObjective += "</camera_angle>";

  strXMLObjective += "</REIMAGE_data>";
  strXMLObjective += "</type_data>";
  strXMLObjective += "</objective>";
   fprintf(stderr, "%s\n\n",strXMLObjective.c_str() );  
  return strXMLObjective;
}

/*
 *  Load a file 
 */
string acdl_to_xml_converter::loadFileToString( string source)
{
	string s1;
	int length;
	char * buffer;

	ifstream dataFile( source.c_str(), ios::in );	//open the file

	if( !dataFile ) 
	{
		fprintf(stderr, "Error opening file: %s\n", source.c_str());
	}
	else
	{
		dataFile.seekg(0, ios::end);	//determine the size of the file
		length = dataFile.tellg();
		dataFile.seekg(0,ios::beg);
		buffer = new char[length];		//create a character buffer to
		dataFile.read(buffer,length);	//store the files contents
		s1=buffer;						//revert copy char buffer to a string

		if( s1.at(s1.size()-1) != '\n')	//sometimes junk is 
		{								//inserted at the end. MS bug?
			s1 = s1.substr(0, s1.find_last_of("\n")+1 );
		}
	}

	dataFile.close();
	delete buffer;

	return s1;
}

/*
int main() {

  fprintf(stderr, "RUNNING TESTS\n" );
  
  acdl_to_xml_converter converter;
  string file = "environment_message.xml";
  string text = converter.loadFileToString( file );
  //string strDateTime = getMissionDateTime(text);
  string strLatLong = getMissionLocation(text);

  fprintf( stderr, "%s\n", strLatLong.c_str() );  
  return 0;
}
*/

/**********************************************************************
 * $Log: acdl_to_xml_converter.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.19  2007/10/03 00:01:59  alanwags
 * Changed code for cim 2.6.4
 *
 * Revision 1.18  2007/09/18 22:37:39  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 **********************************************************************/
