/*********************************************************************
 **                                                                  **
 **                      acdl_to_xml_converter.h                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2006 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: acdl_to_xml_converter.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef ACDL_TO_XML_CONVERTER_H
#define ACDL_TO_XML_CONVERTER_H

#include <vector>
#include <string>
#include <fstream>
#include "ParseDefines.h"
#include "GT_MPPC_client.h"

using namespace std;

using std::string;

class acdl_to_xml_converter {

    //public interface
public:
    acdl_to_xml_converter(void);
    ~acdl_to_xml_converter(void)             {};
    string parseACDLFile(string text );
    string loadFileToString( string source);

    /* Move to xml_to_acdl_coverter.h
    static string getMissionDateTime(void);
    static string getMissionLocation(void);
    static string extractMissionDateTime(string strXMLEnvMessage);
    static string extractMissionLocation(string strXMLLEnvMessage);
    static string extractMissionLatitude(string strXMLLEnvMessage);
    static string extractMissionLongitude(string strXMLLEnvMessage);
    */

    //internal methods
private:
    string getParmValueFromParmName( string strParmName, string text );
    string convertToCIMStyleDateTime(  string strDateTime);
    string eraseGarbage( string test );
    string parseRobotMission(string text );
    string parseObjective( string text );
    string generateXMLSteerpointObjective(string text, int iID);  
    string generateXMLAGMObjective(string text, int ID);
    string generateXMLSARObjective(string text, int ID);
    string generateXMLCommunicationObjective(string text, int ID);
    string generateXMLEOIRObjective(string text, int ID);
    string generateXMLLoiterObjective(string text, int ID);
    string generateXMLMADObjective(string text, int ID);
    string generateXMLPGBObjective(string text, int ID);
    string generateXMLSearchObjective(string text, int ID);
    string generateXMLUAVSearchObjective(string text, int ID);
    string generateXMLUUVSearchObjective(string text, int ID);
    string generateXMLSentryObjective(string text, int ID);
    string generateXMLReimageObjective(string text, int ID);
    string generateXMLTrackObjective(string text, int ID);
    string generateXMLUUVEOObjective(string text, int ID);
    string generateXMLVehicleInfo(int ID);
    string generateXMLHeader(int iMessageNumber, int iPriority);
    string generateXMLTrailer();
 

    //data members
private:
    const string strAgentName;
    const string strParmName;
    const string strParmValue;
    int iObjectiveID;
};


#endif

/**********************************************************************
 * $Log: acdl_to_xml_converter.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.11  2007/09/18 22:37:39  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 **********************************************************************/
