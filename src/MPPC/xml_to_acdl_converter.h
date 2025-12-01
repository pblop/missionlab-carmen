/*********************************************************************
 **                                                                  **
 **                      acdl_to_xml_converter.h                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2006 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: xml_to_acdl_converter.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef XML_TO_ACDL_CONVERTER_H
#define XML_TO_ACDL_CONVERTER_H

#include <string>
#include <stdlib.h>	//Old compilation error: atoi not defined
#include <string.h>
#include <stdio.h>

using std::string;

class Xml2Acdl {

protected:
    static const string EMPTY_STRING_;
    static const string DATETIME_END_KEY_;
    static const string DATETIME_START_KEY_;
    static const string LATITUDE_END_KEY_;
    static const string LATITUDE_START_KEY_;
    static const string LOCATION_END_KEY_;
    static const string LOCATION_START_KEY_;
    static const string LONGITUDE_END_KEY_;
    static const string LONGITUDE_START_KEY_;
    static const string STRING_COLON_;
    static const string STRING_DASH_;
    static const string STRING_PLUS_;
    static const string STRING_SLASH_;
    static const string DATE_TO_TIME_SEPARATOR_;

    static string convertFromCIMStyleDateTime_(string xmlDateTimeString);

public:
    Xml2Acdl(void);
    ~Xml2Acdl(void);

    static string createACDLStyleDateTimeString(
        int year,
        int month,
        int day,
        int hour,
        int minute,
        int second);
    static string extractMissionDateTime(string xmlEnvMessage);
    static string extractMissionLatitude(string xmlEnvMessage);
    static string extractMissionLongitude(string xmlEnvMessage);
};


#endif

/**********************************************************************
 * $Log: xml_to_acdl_converter.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1  2007/09/18 22:37:40  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 **********************************************************************/
