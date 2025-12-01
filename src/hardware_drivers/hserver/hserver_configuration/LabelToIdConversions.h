/**********************************************************************
 **                                                                  **
 **                       LabelToIdConversions.h                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: LabelToIdConversions.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef LABELTOIDCONVERSIONS_H
#define LABELTOIDCONVERSIONS_H

#include "HServerConfig.h"

struct SuLabelIdPair
{
    char* szLabel;
    int iId;
};

const SuLabelIdPair asuCAMERA_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnCameraValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuLASER_LABEL_CONVERSIONS[] = 
{
    {"name",           ClHServerConfig::EnLaserValueType_NAME},
    {"port_string",    ClHServerConfig::EnLaserValueType_PORT_STRING},
    {"angle_offset",   ClHServerConfig::EnLaserValueType_ANGLE_OFFSET},
    {"x_offset",       ClHServerConfig::EnLaserValueType_X_OFFSET},
    {"y_offset",       ClHServerConfig::EnLaserValueType_Y_OFFSET},
    {"listen_ipt",     ClHServerConfig::EnLaserValueType_LISTEN_IPT},
    {"send_ipt",       ClHServerConfig::EnLaserValueType_SEND_IPT},
    {"stream_host",    ClHServerConfig::EnLaserValueType_STREAM_HOST},
    {NULL,             0}
};

const SuLabelIdPair asuCOGNACHROME_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnCognachromeValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuNOMAD_LABEL_CONVERSIONS[] = 
{
    {"type",           ClHServerConfig::EnNomadValueType_TYPE},
    {"port_string",    ClHServerConfig::EnNomadValueType_PORT_STRING},
    {"host_name",      ClHServerConfig::EnNomadValueType_HOST_NAME},
    {NULL,             0}
};

const SuLabelIdPair asuPIONEER_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnPioneerValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuPIONEER2_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnPioneer2ValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuAMIGOBOT_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnAmigoBotValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuROBOTLASER_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnRobotLaserValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuROOMBA_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnRoombaValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuROOMBA560_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnRoomba560ValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuCARMEN_LABEL_CONVERSIONS[] =
{
    {"central_server",    ClHServerConfig::EnCarmenValueType_CENTRAL_SERVER},
    {NULL,             0}
};

const SuLabelIdPair asuWEBCAM_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnWebCamValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuFRAMEGRABBER_LABEL_CONVERSIONS[] = 
{
    {"port_string",    ClHServerConfig::EnFrameGrabberValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuGPS_LABEL_CONVERSIONS[] = 
{
    {"use_base",       ClHServerConfig::EnGpsValueType_USE_BASE},
    {"type",    	   ClHServerConfig::EnGpsValueType_GPS_TYPE},
    {"port_string",    ClHServerConfig::EnGpsValueType_PORT_STRING},
    {"base_lat",       ClHServerConfig::EnGpsValueType_BASE_LAT},
    {"base_long",      ClHServerConfig::EnGpsValueType_BASE_LONG},
    {"x_diff",         ClHServerConfig::EnGpsValueType_X_DIFF},
    {"y_diff",         ClHServerConfig::EnGpsValueType_Y_DIFF},
    {"m_per_lat",      ClHServerConfig::EnGpsValueType_M_PER_LAT},
    {"m_per_long",     ClHServerConfig::EnGpsValueType_M_PER_LONG},
    {NULL,             0}
};

const SuLabelIdPair asuJBOX_LABEL_CONVERSIONS[] = 
{
    {"jbox_id",        ClHServerConfig::EnJboxValueType_JBOX_ID},
    {"disable_network_query",        ClHServerConfig::EnJboxValueType_DISABLE_NETWORK_QUERY},
    {"gps_section",    ClHServerConfig::EnJboxValueType_GPS_SECTION},
    {NULL,             0}
};

const SuLabelIdPair asuCOMPASS_LABEL_CONVERSIONS[] = 
{
    {"type",           ClHServerConfig::EnCompassValueType_TYPE},
    {"port_string",    ClHServerConfig::EnCompassValueType_PORT_STRING},
    {NULL,             0}
};

const SuLabelIdPair asuGYRO_LABEL_CONVERSIONS[] = 
{
    {"type",           ClHServerConfig::EnGyroValueType_TYPE},
    {NULL,             0}
};

const SuLabelIdPair asuPOSECALC_LABEL_CONVERSIONS[] = 
{
    {"fuser_type",
      ClHServerConfig::EnPoseCalcValueType_FUSER_TYPE},
    {"variance_scheme",
      ClHServerConfig::EnPoseCalcValueType_VARIANCE_SCHEME},
    {"compass2mlab_headingFacor",
     ClHServerConfig::EnPoseCalcValueType_COMPASS2MLAB_HEADING_FACTOR},
    {"compass2mlab_headingOffset",
     ClHServerConfig::EnPoseCalcValueType_COMPASS2MLAB_HEADING_OFFSET},
    {"compass_max_valid_angspeed4heading",
     ClHServerConfig::EnPoseCalcValueType_COMPASS_MAX_VALID_ANGSPEED4HEADING},
    {"gps2mlab_headingFactor",
     ClHServerConfig::EnPoseCalcValueType_GPS2MLAB_HEADING_FACTOR},
    {"gps2mlab_headingOffset",
     ClHServerConfig::EnPoseCalcValueType_GPS2MLAB_HEADING_OFFSET},
    {"gps_min_valid_transspeed4heading",
     ClHServerConfig::EnPoseCalcValueType_GPS_MIN_VALID_TRANSSPEED4HEADING},
    {"gps_max_valid_angspeed4heading",
     ClHServerConfig::EnPoseCalcValueType_GPS_MAX_VALID_ANGSPEED4HEADING},
    {NULL,
     0}
};

const SuLabelIdPair asuSECTION_LABEL_CONVERSIONS[] = 
{
    {"camera",         ClHServerConfig::EnHServerConfigSectionType_CAMERA},
    {"laser",          ClHServerConfig::EnHServerConfigSectionType_LASER},
    {"carmen",         ClHServerConfig::EnHServerConfigSectionType_CARMEN},
    {"cognachrome",    ClHServerConfig::EnHServerConfigSectionType_COGNACHROME},
    {"nomad",          ClHServerConfig::EnHServerConfigSectionType_NOMAD},
    {"pioneer",        ClHServerConfig::EnHServerConfigSectionType_PIONEER},
    {"pioneerd",       ClHServerConfig::EnHServerConfigSectionType_PIONEER2},
    {"amigobot",       ClHServerConfig::EnHServerConfigSectionType_AMIGOBOT},
    {"RobotLaser",     ClHServerConfig::EnHServerConfigSectionType_ROBOTLASER},
	{"Roomba",         ClHServerConfig::EnHServerConfigSectionType_ROOMBA},
	{"Roomba560",      ClHServerConfig::EnHServerConfigSectionType_ROOMBA560},
    {"framegrabber",   ClHServerConfig::EnHServerConfigSectionType_FRAMEGRABBER},
    {"gps",            ClHServerConfig::EnHServerConfigSectionType_GPS},
    {"jbox",           ClHServerConfig::EnHServerConfigSectionType_JBOX},
    {"compass",        ClHServerConfig::EnHServerConfigSectionType_COMPASS},
    {"gyro",           ClHServerConfig::EnHServerConfigSectionType_GYRO},
    {"webCam",       ClHServerConfig::EnHServerConfigSectionType_WEBCAM},
    {"posecalc",       ClHServerConfig::EnHServerConfigSectionType_POSECALC},
    {NULL,             0}
};

const int NUM_LABEL_ID_PAIRS = 18;

/*
const SuLabelIdPair* aasuVALUE_TYPE_CONVERSIONS[] = 
{
    asuCAMERA_LABEL_CONVERSIONS,
    asuLASER_LABEL_CONVERSIONS,
    asuCOGNACHROME_LABEL_CONVERSIONS,
    asuNOMAD_LABEL_CONVERSIONS,
    asuPIONEER_LABEL_CONVERSIONS,
    asuPIONEER2_LABEL_CONVERSIONS,
    asuAMIGOBOT_LABEL_CONVERSIONS,
    asuROBOTLASER_LABEL_CONVERSIONS,
	asuROOMBA_LABEL_CONVERSIONS,
	asuROOMBA560_LABEL_CONVERSIONS,
    asuFRAMEGRABBER_LABEL_CONVERSIONS,
    asuGPS_LABEL_CONVERSIONS,
    asuJBOX_LABEL_CONVERSIONS,
    asuCOMPASS_LABEL_CONVERSIONS,
    asuGYRO_LABEL_CONVERSIONS,
    asuPOSECALC_LABEL_CONVERSIONS
};
*/

extern const SuLabelIdPair* aasuVALUE_TYPE_CONVERSIONS[NUM_LABEL_ID_PAIRS];

#endif // ifndef LABELTOIDCONVERSIONS_H

/**********************************************************************
# $Log: LabelToIdConversions.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:54  endo
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
# Revision 1.3  2003/04/26 18:31:06  ebeowulf
# Checked in Pioneer2-dxe module.
#
# Revision 1.2  2002/02/18 13:52:47  endo
# AmigoBot added.
#
#
#**********************************************************************/
