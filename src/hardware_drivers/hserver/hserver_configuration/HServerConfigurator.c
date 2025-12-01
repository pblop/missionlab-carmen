/**********************************************************************
 **                                                                  **
 **                       HServerConfigurator.c                      **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: HServerConfigurator.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include <limits>
#include "HServerConfigurator.h"
#include "../gps.h"
#include "../compass.h"
#include "../gyro.h"
#include "../PoseCalculatorTypes.h"

using std::numeric_limits;

const double ClHServerConfigurator::HSCONFIG_HUGE_VAL_ = numeric_limits<double>::infinity();


ClHServerConfigurator::ClHServerConfigurator(ClHServerConfig& clConfig)
{
    m_pclConfig = &clConfig;
}

ClHServerConfigurator::~ClHServerConfigurator()
{
}

SuCognachromeInitData ClHServerConfigurator::GetCognachromeOptions(int iSection)
{
    SuCognachromeInitData suData;
    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnCognachromeValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}

SuCameraInitData ClHServerConfigurator::GetCameraOptions(int iSection)
{
    SuCameraInitData suData;
    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnCameraValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}

SuWebCamInitData ClHServerConfigurator::GetWebCamOptions(int iSection)
{
    SuWebCamInitData suData;
    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnWebCamValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}

SuNomadInitData ClHServerConfigurator::GetNomadOptions(int iSection)
{
    SuNomadInitData suData;
    suData.enType = Nomad::EnNomadType_150;
    suData.strPortString = "";
    suData.strHost = "";

    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnNomadValueType_TYPE,
                                strValue) &&
         (strValue != ""))
    {
        if (strValue == "150")
        {
            suData.enType = Nomad::EnNomadType_150;
        }
        else if (strValue == "200")
        {
            suData.enType = Nomad::EnNomadType_200;
        }
        else
        {
            string strName;
            m_pclConfig->GetSectionName(iSection, strName);
            fprintf(stderr, "hserver error: bad nomad type \"%s\" for\n", strValue.c_str());
            fprintf(stderr, "    nomad section labeled: %s\n", strName.c_str());
            fprintf(stderr, "    Legal values are \"150\" and \"200\".\n");
            exit(-1);
        }
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnNomadValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnNomadValueType_HOST_NAME,
                                strValue) &&
         (strValue != ""))
    {
        suData.strHost = strValue;
    }

    return suData;
}

SuPioneerInitData ClHServerConfigurator::GetPioneerOptions(int iSection)
{
    SuPioneerInitData suData;
    suData.strPortString = "";

    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnPioneerValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}

SuPioneer2InitData ClHServerConfigurator::GetPioneer2Options(int iSection)
{
    SuPioneer2InitData suData;
    suData.strPortString = "";

    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnPioneer2ValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}

SuAmigoBotInitData ClHServerConfigurator::GetAmigoBotOptions(int iSection)
{
    SuAmigoBotInitData suData;
    suData.strPortString = "";

    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnAmigoBotValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}


SuRobotLaserInitData ClHServerConfigurator::GetRobotLaserOptions(int iSection)
{
    SuRobotLaserInitData suData;
    suData.strPortString = "";

    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnRobotLaserValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}

SuRoombaInitData ClHServerConfigurator::GetRoombaOptions(int iSection)
{
    SuRoombaInitData suData;
    suData.strPortString = "";

    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnRoombaValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}

SuRoomba560InitData ClHServerConfigurator::GetRoomba560Options(int iSection)
{
    SuRoomba560InitData suData;
    suData.strPortString = "";

    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnRoomba560ValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}

SuCarmenInitData ClHServerConfigurator::GetCarmenOptions(int iSection)
{
	SuCarmenInitData suData;
    suData.strCentralServer = "";

    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnCarmenValueType_CENTRAL_SERVER,
                                strValue) &&
         (strValue != ""))
    {
        suData.strCentralServer = strValue;
    }

    return suData;
}

SuFrameGrabberInitData ClHServerConfigurator::GetFrameGrabberOptions(int iSection)
{
    SuFrameGrabberInitData suData;
    suData.strPortString = "";

    string strValue;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnFrameGrabberValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }

    return suData;
}

SuGpsInitData ClHServerConfigurator::GetGpsOptions(int iSection)
{
    SuGpsInitData suData;
    suData.strPortString = "";
    suData.strGpsType = "";
    suData.bUseBase = false;
    suData.dBaseLat = DEFAULT_GPS_BASE_LAT;
    suData.dBaseLong = DEFAULT_GPS_BASE_LON;
    suData.dXDiff = DEFAULT_GPS_BASE_X;
    suData.dYDiff = DEFAULT_GPS_BASE_Y;
    suData.dMPerLat = DEFAULT_GPS_M_PER_LON;
    suData.dMPerLong = DEFAULT_GPS_M_PER_LAT;

    string strValue;
    char* pcEnd;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGpsValueType_USE_BASE,
                                strValue) &&
         (strValue != ""))
    {
        if ((strcasecmp(strValue.c_str(), "true") == 0) ||
             (strcasecmp(strValue.c_str(), "1") == 0))
        {
            suData.bUseBase = true;
        }
        else if ((strcasecmp(strValue.c_str(), "false") == 0) ||
                  (strcasecmp(strValue.c_str(), "0") == 0))
        {
            suData.bUseBase = false;
        }
        else
        {
            fprintf(stderr, "hserver error: a GPS's \"use_base\" value had an invalid value.\n");
            fprintf(stderr, "  Valid values: \"true\", \"false\", \"1\", \"0\"\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGpsValueType_GPS_TYPE,
                                strValue) &&
         (strValue != ""))
    {
        suData.strGpsType = strValue;
    }

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGpsValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPortString = strValue;
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGpsValueType_BASE_LAT,
                                strValue) &&
         (strValue != ""))
    {
        suData.dBaseLat = strtod(strValue.c_str(), &pcEnd);
        if (*pcEnd != '\0')
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"base_lat\" value, %s, was not valid.\n",
                     strValue.c_str());
            exit(-1);
        }
        if ((suData.dBaseLat == HSCONFIG_HUGE_VAL_) || (suData.dBaseLat == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"base_lat\" value, %s, caused an overflow.\n",
                     strValue.c_str());
            exit(-1);
        }
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGpsValueType_BASE_LONG,
                                strValue) &&
         (strValue != ""))
    {
        suData.dBaseLong = strtod(strValue.c_str(), &pcEnd);
        if (*pcEnd != '\0')
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"base_long\" value, %s, was not valid.\n",
                     strValue.c_str());
            exit(-1);
        }
        if ((suData.dBaseLong == HSCONFIG_HUGE_VAL_) || (suData.dBaseLong == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"base_long\" value, %s, caused an overflow.\n",
                     strValue.c_str());
            exit(-1);
        }
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGpsValueType_X_DIFF,
                                strValue) &&
         (strValue != ""))
    {
        suData.dXDiff = strtod(strValue.c_str(), &pcEnd);
        if (*pcEnd != '\0')
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"x_diff\" value, %s, was not valid.\n",
                     strValue.c_str());
            exit(-1);
        }
        if ((suData.dXDiff == HSCONFIG_HUGE_VAL_) || (suData.dXDiff == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"x_diff\" value, %s, caused an overflow.\n",
                     strValue.c_str());
            exit(-1);
        }
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGpsValueType_Y_DIFF,
                                strValue) &&
         (strValue != ""))
    {
        suData.dYDiff = strtod(strValue.c_str(), &pcEnd);
        if (*pcEnd != '\0')
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"y_diff\" value, %s, was not valid.\n",
                     strValue.c_str());
            exit(-1);
        }
        if ((suData.dYDiff == HSCONFIG_HUGE_VAL_) || (suData.dYDiff == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"y_diff\" value, %s, caused an overflow.\n",
                     strValue.c_str());
            exit(-1);
        }
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGpsValueType_M_PER_LAT,
                                strValue) &&
         (strValue != ""))
    {
        suData.dMPerLat = strtod(strValue.c_str(), &pcEnd);
        if (*pcEnd != '\0')
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"m_per_lat\" value, %s, was not valid.\n",
                     strValue.c_str());
            exit(-1);
        }
        if ((suData.dMPerLat == HSCONFIG_HUGE_VAL_) || (suData.dMPerLat == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"m_per_lat\" value, %s, caused an overflow.\n",
                     strValue.c_str());
            exit(-1);
        }
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGpsValueType_M_PER_LONG,
                                strValue) &&
         (strValue != ""))
    {
        suData.dMPerLong = strtod(strValue.c_str(), &pcEnd);
        if (*pcEnd != '\0')
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"m_per_long\" value, %s, was not valid.\n",
                     strValue.c_str());
            exit(-1);
        }
        if ((suData.dBaseLong == HSCONFIG_HUGE_VAL_) || (suData.dBaseLong == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"m_per_long\" value, %s, caused an overflow.\n",
                     strValue.c_str());
            exit(-1);
        }
    }

    return suData;
}

SuJboxInitData ClHServerConfigurator::GetJboxOptions(int iSection)
{
    SuJboxInitData suData;
    suData.jboxID = 0;
    suData.disableNetworkQuery = false;
    suData.gpsSection = "";

    string strValue;

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnJboxValueType_JBOX_ID,
            strValue) &&
        (strValue != ""))
    {
        suData.jboxID = atoi(strValue.c_str());
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnJboxValueType_DISABLE_NETWORK_QUERY,
            strValue) &&
        (strValue != ""))
    {
        if ((strcasecmp(strValue.c_str(), "true") == 0) ||
             (strcasecmp(strValue.c_str(), "1") == 0))
        {
            suData.disableNetworkQuery = true;
        }
        else if ((strcasecmp(strValue.c_str(), "false") == 0) ||
                  (strcasecmp(strValue.c_str(), "0") == 0))
        {
            suData.disableNetworkQuery = false;
        }
        else
        {
            fprintf(stderr, "hserver error: a jbox's \"disable_network_query\" value had an invalid value.\n");
            fprintf(stderr, "  Valid values: \"true\", \"false\", \"1\", and \"0\"\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnJboxValueType_GPS_SECTION,
            strValue) &&
        (strValue != ""))
    {
        suData.gpsSection = strValue;
    }

    return suData;
}

SuCompassInitData ClHServerConfigurator::GetCompassOptions(int iSection)
{
    SuCompassInitData suData;
    string strValue;
    string strName;

    suData.portString = "";
    suData.type = 0;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnCompassValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.portString = strValue;
    }

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnCompassValueType_TYPE,
                                strValue) &&
         (strValue != ""))
    {
        if (strValue == "KVH-C100")
        {
            suData.type = Compass::TYPE_KVHC100;
        }
        else if (strValue == "3DM-G")
        {
            suData.type = Compass::TYPE_3DMG;
        }
        else
        {
            m_pclConfig->GetSectionName(iSection, strName);
            fprintf(stderr, "hserver error: bad compass type \"%s\" for\n", strValue.c_str());
            fprintf(stderr, "    compass section labeled: %s\n", strName.c_str());
            fprintf(stderr, "    Legal values are \"KVH-C100\" and \"3DM-G\".\n");
            exit(-1);
        }
    }

    return suData;
}

SuGyroInitData ClHServerConfigurator::GetGyroOptions(int iSection)
{
    SuGyroInitData suData;
    string strValue;
    string strName;

    suData.portString = "";
    suData.type = 0;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnGyroValueType_TYPE,
                                strValue) &&
         (strValue != ""))
    {
        if (strValue == "DMU-VGX")
        {
            suData.type = Gyro::TYPE_DMUVGX;
        }
        else if(strValue == "CAN")
        {
        	suData.type = Gyro::TYPE_CAN;
        }
        else
        {
            m_pclConfig->GetSectionName(iSection, strName);
            fprintf(stderr, "hserver error: bad gyro type \"%s\" for\n", strValue.c_str());
            fprintf(stderr, "    gyro section labeled: %s\n", strName.c_str());
            fprintf(stderr, "    Legal values are \"DMU-VGX\" and \"CAN\".\n");
            exit(-1);
        }
    }

    return suData;
}

SuPoseCalcInitData ClHServerConfigurator::GetPoseCalcOptions(int iSection)
{
    SuPoseCalcInitData suData;
    string strValue;
    string strName;
    char* pcEnd;

    memset((SuPoseCalcInitData *)&suData, 0x0, sizeof(SuPoseCalcInitData));

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnPoseCalcValueType_FUSER_TYPE,
            strValue) &&
        (strValue != ""))
    {
        if (strValue == "maxconf")
        {
            suData.fuserType = POSECALC_FUSER_TYPE_MAXMUM_CONFIDENCE;
        }
        else if (strValue == "ekf")
        {
            suData.fuserType = POSECALC_FUSER_TYPE_EXTENDED_KALMAN_FILTER;
        }
        else if (strValue == "pfilter")
        {
            suData.fuserType = POSECALC_FUSER_TYPE_PARTICLE_FILTER;
        }
        else
        {
            m_pclConfig->GetSectionName(iSection, strName);
            fprintf(stderr, "hserver error: bad posecal fuser type \"%s\" for\n", strValue.c_str());
            fprintf(stderr, "    posecalc section labeled: %s\n", strName.c_str());
            fprintf(stderr, "    Legal values are \"maxconf\", \"ekf\" and \"pfilter\".\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnPoseCalcValueType_VARIANCE_SCHEME,
            strValue) &&
        (strValue != ""))
    {
        if (strValue == "dynamic")
        {
            suData.varianceScheme = POSECALC_VARIANCE_SCHEME_DYNAMIC;
        }
        else if (strValue == "static")
        {
            suData.varianceScheme = POSECALC_VARIANCE_SCHEME_STATIC;
        }
        else
        {
            m_pclConfig->GetSectionName(iSection, strName);
            fprintf(stderr, "hserver error: bad posecal variance scheme \"%s\" for\n", strValue.c_str());
            fprintf(stderr, "    posecalc section labeled: %s\n", strName.c_str());
            fprintf(stderr, "    Legal values are \"dynamic\", and \"static\".\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnPoseCalcValueType_COMPASS2MLAB_HEADING_FACTOR,
            strValue) &&
        (strValue != ""))
    {
        suData.compass2MlabHeadingFacor = strtod(strValue.c_str(), &pcEnd);

        if (*pcEnd != '\0')
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (compass2mlab_headingFacor).\n");
            exit(-1);
        }
        if ((suData.compass2MlabHeadingFacor == HSCONFIG_HUGE_VAL_) || (suData.compass2MlabHeadingFacor == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (compass2mlab_headingFacor).\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnPoseCalcValueType_COMPASS2MLAB_HEADING_OFFSET,
            strValue) &&
        (strValue != ""))
    {
        suData.compass2MlabHeadingOffset = strtod(strValue.c_str(), &pcEnd);

        if (*pcEnd != '\0')
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (compass2mlab_headingFacor).\n");
            exit(-1);
        }
        if ((suData.compass2MlabHeadingOffset == HSCONFIG_HUGE_VAL_) || (suData.compass2MlabHeadingOffset == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (compass2mlab_headingFacor).\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnPoseCalcValueType_COMPASS_MAX_VALID_ANGSPEED4HEADING,
            strValue) &&
        (strValue != ""))
    {
        suData.compassMaxValidAngSpeed4Heading = strtod(strValue.c_str(), &pcEnd);

        if (*pcEnd != '\0')
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (compass_max_valid_angspeed4heading).\n");
            exit(-1);
        }
        if ((suData.compassMaxValidAngSpeed4Heading == HSCONFIG_HUGE_VAL_) || (suData.compassMaxValidAngSpeed4Heading == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (compass_max_valid_angspeed4heading).\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnPoseCalcValueType_GPS2MLAB_HEADING_FACTOR,
            strValue) &&
        (strValue != ""))
    {
        suData.gps2MlabHeadingFacor = strtod(strValue.c_str(), &pcEnd);

        if (*pcEnd != '\0')
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (gps2mlab_headingFacor).\n");
            exit(-1);
        }
        if ((suData.gps2MlabHeadingFacor == HSCONFIG_HUGE_VAL_) || (suData.gps2MlabHeadingFacor == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (gps2mlab_headingFacor).\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnPoseCalcValueType_GPS2MLAB_HEADING_OFFSET,
            strValue) &&
        (strValue != ""))
    {
        suData.gps2MlabHeadingOffset = strtod(strValue.c_str(), &pcEnd);

        if (*pcEnd != '\0')
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (gps2mlab_headingFacor).\n");
            exit(-1);
        }
        if ((suData.gps2MlabHeadingOffset == HSCONFIG_HUGE_VAL_) || (suData.gps2MlabHeadingOffset == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (gps2mlab_headingFacor).\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnPoseCalcValueType_GPS_MIN_VALID_TRANSSPEED4HEADING,
            strValue) &&
        (strValue != ""))
    {
        suData.gpsMinValidTransSpeed4Heading = strtod(strValue.c_str(), &pcEnd);

        if (*pcEnd != '\0')
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (gps_min_valid_transspeed4heading).\n");
            exit(-1);
        }
        if ((suData.gpsMinValidTransSpeed4Heading == HSCONFIG_HUGE_VAL_) || (suData.gpsMinValidTransSpeed4Heading == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (gps_min_valid_transspeed4heading).\n");
            exit(-1);
        }
    }

    if (m_pclConfig->GetValue(
            iSection,
            ClHServerConfig::EnPoseCalcValueType_GPS_MAX_VALID_ANGSPEED4HEADING,
            strValue) &&
        (strValue != ""))
    {
        suData.gpsMaxValidAngSpeed4Heading = strtod(strValue.c_str(), &pcEnd);

        if (*pcEnd != '\0')
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (gps_max_valid_angspeed4heading).\n");
            exit(-1);
        }
        if ((suData.gpsMaxValidAngSpeed4Heading == HSCONFIG_HUGE_VAL_) || (suData.gpsMaxValidAngSpeed4Heading == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr, "hserver error: invalid value in the RC file (gps_max_valid_angspeed4heading).\n");
            exit(-1);
        }
    }

    return suData;
}

SuLaserInitData ClHServerConfigurator::GetLaserOptions(int iSection)
{
    SuLaserInitData suData;
    suData.strPort = "";
    suData.bSend = false;
    suData.bListen = false;
    suData.fAngleOffset = 0.0;
    suData.fXOffset = 0.0;
    suData.fYOffset = 0.0;
    suData.strName = "";
    suData.strStreamAddress = "";

    string strValue = "";

    char* pcEnd;

    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnLaserValueType_NAME,
                                strValue) &&
         (strValue != ""))
    {
        suData.strName = strValue;
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnLaserValueType_PORT_STRING,
                                strValue) &&
         (strValue != ""))
    {
        suData.strPort = strValue;
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnLaserValueType_ANGLE_OFFSET,
                                strValue) &&
         (strValue != ""))
    {
        suData.fAngleOffset = strtod(strValue.c_str(), &pcEnd);
        if (*pcEnd != '\0')
        {
            fprintf(stderr,
                     "hserver error: a laser's \"angle_offset\" value, %s, was not valid.\n",
                     strValue.c_str());
            exit(-1);
        }
        if ((suData.fAngleOffset == HSCONFIG_HUGE_VAL_) || (suData.fAngleOffset == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr,
                     "hserver error: a laser's \"angle_offset\" value, %s, caused an overflow.\n",
                     strValue.c_str());
            exit(-1);
        }
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnLaserValueType_X_OFFSET,
                                strValue) &&
         (strValue != ""))
    {
        suData.fXOffset = strtod(strValue.c_str(), &pcEnd);
        if (*pcEnd != '\0')
        {
            fprintf(stderr,
                     "hserver error: a laser's \"x_offset\" value, %s, was not valid.\n",
                     strValue.c_str());
            exit(-1);
        }
        if ((suData.fXOffset == HSCONFIG_HUGE_VAL_) || (suData.fXOffset == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr,
                     "hserver error: a laser's \"x_offset\" value, %s, caused an overflow.\n",
                     strValue.c_str());
            exit(-1);
        }
        suData.fXOffset /= 100.0; // convert cm to meters
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnLaserValueType_Y_OFFSET,
                                strValue) &&
         (strValue != ""))
    {
        suData.fYOffset = strtod(strValue.c_str(), &pcEnd);
        if (*pcEnd != '\0')
        {
            fprintf(stderr,
                     "hserver error: a laser's \"y_offset\" value, %s, was not valid.\n",
                     strValue.c_str());
            exit(-1);
        }
        if ((suData.fYOffset == HSCONFIG_HUGE_VAL_) || (suData.fYOffset == -HSCONFIG_HUGE_VAL_))
        {
            fprintf(stderr,
                     "hserver error: a GPS's \"y_offset\" value, %s, caused an overflow.\n",
                     strValue.c_str());
            exit(-1);
        }
        suData.fYOffset /= 100.0; // convert cm to meters
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnLaserValueType_LISTEN_IPT,
                                strValue) &&
         (strValue != ""))
    {
        if ((strcasecmp(strValue.c_str(), "true") == 0) ||
             (strcasecmp(strValue.c_str(), "1") == 0))
        {
            suData.bListen = true;
        }
        else if ((strcasecmp(strValue.c_str(), "false") == 0) ||
                  (strcasecmp(strValue.c_str(), "0") == 0))
        {
            suData.bListen = false;
        }
        else
        {
            fprintf(stderr, "hserver error: a laser's \"listen_ipt\" value had an invalid value.\n");
            fprintf(stderr, "  Valid values: \"true\", \"false\", \"1\", and \"0\"\n");
            exit(-1);
        }
    }
    if (m_pclConfig->GetValue(iSection, ClHServerConfig::EnLaserValueType_SEND_IPT, strValue) &&
         (strValue != ""))
    {
        if ((strcasecmp(strValue.c_str(), "true") == 0) ||
             (strcasecmp(strValue.c_str(), "1") == 0))
        {
            suData.bSend = true;
        }
        else if ((strcasecmp(strValue.c_str(), "false") == 0) ||
                  (strcasecmp(strValue.c_str(), "0") == 0))
        {
            suData.bSend = false;
        }
        else
        {
            fprintf(stderr, "hserver error: a laser's \"send_ipt\" value had an invalid value.\n");
            fprintf(stderr, "  Valid values: \"true\", \"false\", \"1\", and \"0\"\n");
            exit(-1);
        }
    }
    if (m_pclConfig->GetValue(iSection,
                                ClHServerConfig::EnLaserValueType_STREAM_HOST,
                                strValue) &&
         (strValue != ""))
    {
        suData.strStreamAddress = strValue;
    }

    return suData;
}

/**********************************************************************
# $Log: HServerConfigurator.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2006/05/17 16:46:15  endo
# HUGE_VAL replaced by HSCONFIG_HUGE_VAL
#
# Revision 1.1.1.1  2005/02/06 22:59:45  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.3  2003/04/26 18:31:06  ebeowulf
# Checked in Pioneer2-dxe module.
#
# Revision 1.2  2002/02/18 13:52:47  endo
# AmigoBot added.
#
#
#**********************************************************************/
