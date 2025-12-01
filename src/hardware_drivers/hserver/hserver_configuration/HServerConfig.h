/**********************************************************************
 **                                                                  **
 **                         HServerConfig.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: HServerConfig.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef HSERVERCONFIG_H
#define HSERVERCONFIG_H

#include <string>
#include <vector>
#include "HServerConfigSection.h"

using std::string;
using std::vector;

class ClHServerConfig
{
public:
    enum EnCameraValueType
    {
        EnCameraValueType_PORT_STRING = 0,
        EnCameraValueType_COUNT     // this must be declared last
    };

    enum EnWebCamValueType
    {
        EnWebCamValueType_PORT_STRING = 0,
        EnWebCamValueType_COUNT     // this must be declared last
    };

    enum EnLaserValueType
    {
        EnLaserValueType_NAME = 0,
        EnLaserValueType_PORT_STRING,
        EnLaserValueType_ANGLE_OFFSET,
        EnLaserValueType_X_OFFSET,
        EnLaserValueType_Y_OFFSET,
        EnLaserValueType_LISTEN_IPT,
        EnLaserValueType_SEND_IPT,
        EnLaserValueType_STREAM_HOST,
        EnLaserValueType_COUNT     // this must be declared last
    };

    enum EnCognachromeValueType
    {
        EnCognachromeValueType_PORT_STRING = 0,
        EnCognachromeValueType_COUNT     // this must be declared last
    };

    enum EnNomadValueType
    {
        EnNomadValueType_TYPE = 0,
        EnNomadValueType_PORT_STRING,
        EnNomadValueType_HOST_NAME,
        EnNomadValueType_COUNT     // this must be declared last
    };

    enum EnPioneerValueType
    {
        EnPioneerValueType_PORT_STRING = 0,
        EnPioneerValueType_COUNT     // this must be declared last
    };

    enum EnPioneer2ValueType
    {
        EnPioneer2ValueType_PORT_STRING = 0,
        EnPioneer2ValueType_COUNT     // this must be declared last
    };

    enum EnAmigoBotValueType
    {
        EnAmigoBotValueType_PORT_STRING = 0,
        EnAmigoBotValueType_COUNT     // this must be declared last
    };

    enum EnRobotLaserValueType
    {
        EnRobotLaserValueType_PORT_STRING = 0,
        EnRobotLaserValueType_COUNT     // this must be declared last
    };

	enum EnRoombaValueType
    {
        EnRoombaValueType_PORT_STRING = 0,
        EnRoombaValueType_COUNT     // this must be declared last
    };

	enum EnRoomba560ValueType
    {
        EnRoomba560ValueType_PORT_STRING = 0,
        EnRoomba560ValueType_COUNT     // this must be declared last
    };

	enum EnCarmenValueType
    {
        EnCarmenValueType_CENTRAL_SERVER = 0,
        EnCarmenValueType_COUNT     // this must be declared last
    };

    enum EnFrameGrabberValueType
    {
        EnFrameGrabberValueType_PORT_STRING = 0,
        EnFrameGrabberValueType_COUNT     // this must be declared last
    };

    enum EnGpsValueType
    {
        EnGpsValueType_USE_BASE = 0,
        EnGpsValueType_PORT_STRING,
        EnGpsValueType_GPS_TYPE,
        EnGpsValueType_BASE_LAT,
        EnGpsValueType_BASE_LONG,
        EnGpsValueType_X_DIFF,
        EnGpsValueType_Y_DIFF,
        EnGpsValueType_M_PER_LAT,
        EnGpsValueType_M_PER_LONG,
        EnGpsValueType_COUNT     // this must be declared last
    };

    enum EnJboxValueType
    {
        EnJboxValueType_JBOX_ID = 0,
        EnJboxValueType_DISABLE_NETWORK_QUERY,
        EnJboxValueType_GPS_SECTION,
        EnJboxValueType_COUNT     // this must be declared last
    };

    enum EnCompassValueType
    {
        EnCompassValueType_TYPE = 0,
        EnCompassValueType_PORT_STRING,
        EnCompassValueType_COUNT     // this must be declared last
    };

    enum EnGyroValueType
    {
        EnGyroValueType_TYPE = 0,
        EnGyroValueType_COUNT     // this must be declared last
    };

    enum EnPoseCalcValueType
    {
        EnPoseCalcValueType_FUSER_TYPE = 0,
        EnPoseCalcValueType_VARIANCE_SCHEME,
        EnPoseCalcValueType_COMPASS2MLAB_HEADING_FACTOR,
        EnPoseCalcValueType_COMPASS2MLAB_HEADING_OFFSET,
        EnPoseCalcValueType_COMPASS_MAX_VALID_ANGSPEED4HEADING,
        EnPoseCalcValueType_GPS2MLAB_HEADING_FACTOR,
        EnPoseCalcValueType_GPS2MLAB_HEADING_OFFSET,
        EnPoseCalcValueType_GPS_MIN_VALID_TRANSSPEED4HEADING,
        EnPoseCalcValueType_GPS_MAX_VALID_ANGSPEED4HEADING,
        EnPoseCalcValueType_COUNT     // this must be declared last
    };

    enum EnHServerConfigSectionType
    {
        EnHServerConfigSectionType_CAMERA = 0,
        EnHServerConfigSectionType_LASER,
        EnHServerConfigSectionType_COGNACHROME,
        EnHServerConfigSectionType_NOMAD,
        EnHServerConfigSectionType_PIONEER,
        EnHServerConfigSectionType_PIONEER2,
        EnHServerConfigSectionType_AMIGOBOT,
		EnHServerConfigSectionType_ROBOTLASER,
		EnHServerConfigSectionType_ROOMBA,
		EnHServerConfigSectionType_ROOMBA560,
        EnHServerConfigSectionType_FRAMEGRABBER,
        EnHServerConfigSectionType_GPS,
        EnHServerConfigSectionType_JBOX,
        EnHServerConfigSectionType_COMPASS,
        EnHServerConfigSectionType_GYRO,
        EnHServerConfigSectionType_POSECALC,
        EnHServerConfigSectionType_WEBCAM,
        EnHServerConfigSectionType_CARMEN,
        EnHServerConfigSectionType_COUNT     // this must be declared last
    };

    enum EnHServerConfigError
    {
        EnHServerConfigError_NONE,
        EnHServerConfigError_BAD_CONFIG_TYPE
    };

    static const int m_aiSECTION_VALUE_COUNTS[];

    ClHServerConfig();
    ~ClHServerConfig();

    bool AddNewSection( int iSectionType, string strName );
    unsigned int GetSectionCount() const;
    int GetSectionType( int iSection ) const;
    bool GetValue( int iSection, int iValueType, string& strValue );
    bool InsertValue( int iSection, int iValueType, const string& strValue );
    bool GetSectionName( int iSection, string& strName ) const;
    bool SetSectionName( int iSection, string strName );

protected:
    vector<ClHServerConfigSection> m_clSectionList;

    bool ValidSection( int iSection ) const;
};

inline unsigned int ClHServerConfig::GetSectionCount() const
{
    return m_clSectionList.size();
}

inline bool ClHServerConfig::ValidSection( int iSection ) const
{
    return ( ( iSection >= 0 ) && ( iSection < (int) m_clSectionList.size() ) );
}

inline int ClHServerConfig::GetSectionType( int iSection ) const
{
    return ( ValidSection( iSection ) ? m_clSectionList[ iSection ].GetType() : -1 );
}

#endif // #ifndef _HSERVERCONFIG_H

/**********************************************************************
# $Log: HServerConfig.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.5  2004/11/22 00:19:22  endo
# Network query of JBox can now be disabled.
#
# Revision 1.4  2004/09/22 04:00:58  endo
# *** empty log message ***
#
# Revision 1.3  2004/09/10 20:05:05  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/04/24 11:20:54  endo
# 3DM-G added.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.4  2003/04/26 18:31:06  ebeowulf
# Checked in Pioneer2-dxe module.
#
# Revision 1.3  2003/04/06 13:35:53  endo
# gcc 3.1.1
#
# Revision 1.2  2002/02/18 13:52:47  endo
# AmigoBot added.
#
#
#**********************************************************************/
