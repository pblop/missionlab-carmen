/**********************************************************************
 **                                                                  **
 **                         HServerConfig.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: HServerConfig.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include "HServerConfig.h"

const int ClHServerConfig::m_aiSECTION_VALUE_COUNTS[] = 
{
    EnCameraValueType_COUNT,
    EnLaserValueType_COUNT,
    EnCognachromeValueType_COUNT,
    EnNomadValueType_COUNT,
    EnPioneerValueType_COUNT,
    EnPioneer2ValueType_COUNT,
    EnAmigoBotValueType_COUNT,
    EnRobotLaserValueType_COUNT,
	EnRoombaValueType_COUNT,
	EnRoomba560ValueType_COUNT,
    EnFrameGrabberValueType_COUNT,
    EnGpsValueType_COUNT,
    EnJboxValueType_COUNT,
    EnCompassValueType_COUNT,
    EnGyroValueType_COUNT,
    EnPoseCalcValueType_COUNT,
    EnWebCamValueType_COUNT,
    EnCarmenValueType_COUNT
};

ClHServerConfig::ClHServerConfig()
{
}

ClHServerConfig::~ClHServerConfig()
{
}

bool ClHServerConfig::AddNewSection( int iSectionType, string strName )
{
    bool bSuccess = false;

    if ( ( iSectionType >= 0 ) && 
         ( iSectionType < EnHServerConfigSectionType_COUNT ) )
    {
        ClHServerConfigSection clNewSection( iSectionType, m_aiSECTION_VALUE_COUNTS[ iSectionType ], strName );
        m_clSectionList.push_back( clNewSection );
        bSuccess = true;
    }

    return bSuccess;
}

bool ClHServerConfig::InsertValue( int iSection, int iValueType, const string& strValue )
{
    bool bSuccess = false;

    if ( ValidSection( iSection ) )
    {
        bSuccess = m_clSectionList[ iSection ].InsertValue( iValueType, strValue );
    }

    return bSuccess;
}

bool ClHServerConfig::GetValue( int iSection, int iValueType, string& strValue )
{
    bool bSuccess = false;

    if ( ValidSection( iSection ) )
    {
        bSuccess = m_clSectionList[ iSection ].GetValue( iValueType, strValue );
    }

    return bSuccess;
}

bool ClHServerConfig::GetSectionName( int iSection, string& strName ) const
{
    bool bSuccess = ValidSection( iSection );

    if ( bSuccess )
    {
        strName = m_clSectionList[ iSection ].GetName();
    }

    return bSuccess;
}

bool ClHServerConfig::SetSectionName( int iSection, string strName )
{
    bool bSuccess = false;

    if ( ValidSection( iSection ) )
    {
        m_clSectionList[ iSection ].SetName( strName );
        bSuccess = true;
    }

    return bSuccess;
}

/**********************************************************************
# $Log: HServerConfig.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.4  2004/09/22 04:00:03  endo
# *** empty log message ***
#
# Revision 1.3  2004/09/10 20:05:04  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/04/24 11:20:54  endo
# 3DM-G added.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.3  2003/04/26 18:31:06  ebeowulf
# Checked in Pioneer2-dxe module.
#
# Revision 1.2  2002/02/18 13:52:47  endo
# AmigoBot added.
#
#
#**********************************************************************/
