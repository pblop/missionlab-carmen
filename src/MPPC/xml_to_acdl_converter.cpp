/*********************************************************************
 **                                                                  **
 **                     xml_to_acdl_converter.cpp                    **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: xml_to_acdl_converter.cpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "xml_to_acdl_converter.h"

//-----------------------------------------------------------------------
// Constants.
//-----------------------------------------------------------------------
const string Xml2Acdl::EMPTY_STRING_ = "";
const string Xml2Acdl::DATETIME_END_KEY_ = "</mission_start>";
const string Xml2Acdl::DATETIME_START_KEY_ = "<mission_start>";
const string Xml2Acdl::LATITUDE_END_KEY_ = "</latitude>";
const string Xml2Acdl::LATITUDE_START_KEY_ = "<latitude>";
const string Xml2Acdl::LOCATION_END_KEY_ = "</mission_center>";
const string Xml2Acdl::LOCATION_START_KEY_ = "<mission_center>";
const string Xml2Acdl::LONGITUDE_END_KEY_ = "</longitude>";
const string Xml2Acdl::LONGITUDE_START_KEY_ = "<longitude>";
const string Xml2Acdl::STRING_COLON_ = ":";
const string Xml2Acdl::STRING_DASH_ = "-";
const string Xml2Acdl::STRING_PLUS_ = "+";
const string Xml2Acdl::STRING_SLASH_ = "/";
const string Xml2Acdl::DATE_TO_TIME_SEPARATOR_ = "T";

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
Xml2Acdl::Xml2Acdl(void)
{
}

//-----------------------------------------------------------------------
// Distructor
//-----------------------------------------------------------------------
Xml2Acdl::~Xml2Acdl(void)
{
}

//-----------------------------------------------------------------------
// This function translates the CIM style date/time string
// (YYYY-MM-DDThh:mm:ss.sss-xx:xx) to ACDL type (hh:mm:ss+MM/DD/YYYY).
//-----------------------------------------------------------------------
string Xml2Acdl::convertFromCIMStyleDateTime_(string xmlDateTimeString)
{
    string acdlDateString, year, month, day, hour, minute, second;
    int startPos, endPos;

    // Get the year.
    startPos = 0;
    endPos = xmlDateTimeString.find(STRING_DASH_, startPos);
    year = xmlDateTimeString.substr(startPos, endPos - startPos);

    // Get the month.
    startPos = endPos + STRING_DASH_.size();
    endPos = xmlDateTimeString.find(STRING_DASH_, startPos);
    month = xmlDateTimeString.substr(startPos, endPos - startPos);

    // Get the day.
    startPos = endPos + STRING_DASH_.size();
    endPos = xmlDateTimeString.find(DATE_TO_TIME_SEPARATOR_, startPos);
    day = xmlDateTimeString.substr(startPos, endPos - startPos);

    // Get the hour.
    startPos = endPos + DATE_TO_TIME_SEPARATOR_.size();
    endPos = xmlDateTimeString.find(STRING_COLON_, startPos);
    hour = xmlDateTimeString.substr(startPos, endPos - startPos);

    // Get the minute.
    startPos = endPos + STRING_COLON_.size();
    endPos = xmlDateTimeString.find(STRING_COLON_, startPos);
    minute = xmlDateTimeString.substr(startPos, endPos - startPos);

    // Get the second.
    startPos = endPos + STRING_COLON_.size();
    endPos = xmlDateTimeString.find(STRING_DASH_, startPos);
    second = xmlDateTimeString.substr(startPos, endPos - startPos);


    acdlDateString = createACDLStyleDateTimeString(
        atoi(year.c_str()),
        atoi(month.c_str()),
        atoi(day.c_str()),
        atoi(hour.c_str()),
        atoi(minute.c_str()),
        atoi(second.c_str()));

    return acdlDateString;
}

//-----------------------------------------------------------------------
// This function translates the CIM style date/time string
// (YYYY-MM-DDThh:mm:ss.sss-xx:xx) to ACDL type (hh:mm:ss+MM/DD/YYYY).
//-----------------------------------------------------------------------
string Xml2Acdl::createACDLStyleDateTimeString(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second)
{
    string timeString;
    char buf[1024];

    sprintf(
        buf,
        "%d:%d:%d+%d/%d/%d",
        hour,
        minute,
        second,
        month,
        day,
        year);

    timeString = buf;

    return timeString;
}

//-----------------------------------------------------------------------
// This function extracts the mission date/time from the XML environment
// file.
//-----------------------------------------------------------------------
string Xml2Acdl::extractMissionDateTime(string xmlEnvMessage)
{
    string dateString = EMPTY_STRING_;
    int startPos, endPos;

    if(xmlEnvMessage != EMPTY_STRING_)
    {
        startPos = xmlEnvMessage.find(DATETIME_START_KEY_, 0) + DATETIME_START_KEY_.size();
        endPos =   xmlEnvMessage.find(DATETIME_END_KEY_, startPos);

        dateString = xmlEnvMessage.substr(startPos, endPos - startPos);

        dateString = convertFromCIMStyleDateTime_(dateString);
    }

    return dateString; 
}

//-----------------------------------------------------------------------
// This function extracts the latitude from the XML environment file.
//-----------------------------------------------------------------------
string Xml2Acdl::extractMissionLatitude(string xmlEnvMessage)
{
    string locString, latString = EMPTY_STRING_;
    int startPos, endPos;

    if(xmlEnvMessage != EMPTY_STRING_)
    {
        // Extract the location first.
        startPos = xmlEnvMessage.find(LOCATION_START_KEY_, 0) + LOCATION_START_KEY_.size();
        endPos =   xmlEnvMessage.find(LOCATION_END_KEY_, startPos);
        locString = xmlEnvMessage.substr(startPos, endPos - startPos);

        // Extract the latitude.
        startPos = locString.find(LATITUDE_START_KEY_, 0) + LATITUDE_START_KEY_.size();
        endPos =   locString.find(LATITUDE_END_KEY_, startPos);
        latString = locString.substr(startPos, endPos - startPos);
    }

    return latString;
}

//-----------------------------------------------------------------------
// This function extracts the longitude from the XML environment file.
//-----------------------------------------------------------------------
string Xml2Acdl::extractMissionLongitude(string xmlEnvMessage)
{
    string locString, latString = EMPTY_STRING_;
    int startPos, endPos;

    if(xmlEnvMessage != EMPTY_STRING_)
    {
        // Extract the location first.
        startPos = xmlEnvMessage.find(LOCATION_START_KEY_, 0) + LOCATION_START_KEY_.size();
        endPos =   xmlEnvMessage.find(LOCATION_END_KEY_, startPos);
        locString = xmlEnvMessage.substr(startPos, endPos - startPos);

        // Extract the longitude.
        startPos = locString.find(LONGITUDE_START_KEY_, 0) + LONGITUDE_START_KEY_.size();
        endPos =   locString.find(LONGITUDE_END_KEY_, startPos);
        latString = locString.substr(startPos, endPos - startPos);
    }

    return latString;
}

/**********************************************************************
 * $Log: xml_to_acdl_converter.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1  2007/09/18 22:37:39  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 **********************************************************************/
