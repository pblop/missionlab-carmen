/**********************************************************************
 **                                                                  **
 **                       HServerConfigurator.h                      **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: HServerConfigurator.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef HSERVERCONFIGURATOR_H
#define HSERVERCONFIGURATOR_H

#include <string>
#include "HServerConfig.h"
#include "nomad.h"

// this holds initialization data for the lasers
struct SuLaserInitData
{
    string strPort;
    bool bSend;
    bool bListen;
    float fAngleOffset;
    float fXOffset;
    float fYOffset;
    string strName;
    string strStreamAddress;

    SuLaserInitData() {}
    SuLaserInitData( const SuLaserInitData& suOriginal ) { *this = suOriginal; }
    inline SuLaserInitData& operator=( const SuLaserInitData& suOriginal );
};

inline SuLaserInitData& SuLaserInitData::operator=( const SuLaserInitData& suOriginal )
{
    if ( this != & suOriginal )
    {
        strPort = suOriginal.strPort;
        bSend = suOriginal.bSend;
        bListen = suOriginal.bListen;
        fAngleOffset = suOriginal.fAngleOffset;
        fXOffset = suOriginal.fXOffset;
        fYOffset = suOriginal.fYOffset;
        strName = suOriginal.strName;
        strStreamAddress = suOriginal.strStreamAddress;
    }

    return *this;
}

struct SuCameraInitData
{
    string strPortString;

    SuCameraInitData() {}
    SuCameraInitData( const SuCameraInitData& suOriginal ) { *this = suOriginal; }
    inline SuCameraInitData& operator=( const SuCameraInitData& suOriginal );
};

inline SuCameraInitData& SuCameraInitData::operator=( const SuCameraInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuWebCamInitData
{
    string strPortString;

    SuWebCamInitData() {}
    SuWebCamInitData( const SuWebCamInitData& suOriginal ) { *this = suOriginal; }
    inline SuWebCamInitData& operator=( const SuWebCamInitData& suOriginal );
};

inline SuWebCamInitData& SuWebCamInitData::operator=( const SuWebCamInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuCognachromeInitData
{
    string strPortString;

    SuCognachromeInitData() {}
    SuCognachromeInitData( const SuCognachromeInitData& suOriginal ) { *this = suOriginal; }
    inline SuCognachromeInitData& operator=( const SuCognachromeInitData& suOriginal );
};

inline SuCognachromeInitData& SuCognachromeInitData::operator=( const SuCognachromeInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuNomadInitData
{
    Nomad::EnNomadType enType;
    string strPortString;
    string strHost;

    SuNomadInitData() {}
    SuNomadInitData( const SuNomadInitData& suOriginal ) { *this = suOriginal; }
    inline SuNomadInitData& operator=( const SuNomadInitData& suOriginal );
};

inline SuNomadInitData& SuNomadInitData::operator=( const SuNomadInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        enType = suOriginal.enType;
        strPortString = suOriginal.strPortString;
        strHost = suOriginal.strHost;
    }

    return *this;
}

struct SuPioneerInitData
{
    string strPortString;

    SuPioneerInitData() {}
    SuPioneerInitData( const SuPioneerInitData& suOriginal ) { *this = suOriginal; }
    inline SuPioneerInitData& operator=( const SuPioneerInitData& suOriginal );
};

inline SuPioneerInitData& SuPioneerInitData::operator=( const SuPioneerInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuPioneer2InitData
{
    string strPortString;

    SuPioneer2InitData() {}
    SuPioneer2InitData( const SuPioneer2InitData& suOriginal ) { *this = suOriginal; }
    inline SuPioneer2InitData& operator=( const SuPioneer2InitData& suOriginal );
};

inline SuPioneer2InitData& SuPioneer2InitData::operator=( const SuPioneer2InitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuAmigoBotInitData
{
    string strPortString;

    SuAmigoBotInitData() {}
    SuAmigoBotInitData( const SuAmigoBotInitData& suOriginal ) { *this = suOriginal; }
    inline SuAmigoBotInitData& operator=( const SuAmigoBotInitData& suOriginal );
};

inline SuAmigoBotInitData& SuAmigoBotInitData::operator=( const SuAmigoBotInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuRobotLaserInitData
{
    string strPortString;

    SuRobotLaserInitData() {}
    SuRobotLaserInitData( const SuRobotLaserInitData& suOriginal ) { *this = suOriginal; }
    inline SuRobotLaserInitData& operator=( const SuRobotLaserInitData& suOriginal );
};

inline SuRobotLaserInitData& SuRobotLaserInitData::operator=( const SuRobotLaserInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuRoombaInitData
{
    string strPortString;

    SuRoombaInitData() {}
    SuRoombaInitData( const SuRoombaInitData& suOriginal ) { *this = suOriginal; }
    inline SuRoombaInitData& operator=( const SuRoombaInitData& suOriginal );
};

inline SuRoombaInitData& SuRoombaInitData::operator=( const SuRoombaInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuRoomba560InitData
{
    string strPortString;

    SuRoomba560InitData() {}
    SuRoomba560InitData( const SuRoomba560InitData& suOriginal ) { *this = suOriginal; }
    inline SuRoomba560InitData& operator=( const SuRoomba560InitData& suOriginal );
};

inline SuRoomba560InitData& SuRoomba560InitData::operator=( const SuRoomba560InitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuCarmenInitData
{
    string strCentralServer;

    SuCarmenInitData() {}
    SuCarmenInitData( const SuCarmenInitData& suOriginal ) { *this = suOriginal; }
    inline SuCarmenInitData& operator=( const SuCarmenInitData& suOriginal );
};

inline SuCarmenInitData& SuCarmenInitData::operator=( const SuCarmenInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
    	strCentralServer = suOriginal.strCentralServer;
    }

    return *this;
}

struct SuFrameGrabberInitData
{
    string strPortString;

    SuFrameGrabberInitData() {}
    SuFrameGrabberInitData( const SuFrameGrabberInitData& suOriginal ) { *this = suOriginal; }
    inline SuFrameGrabberInitData& operator=( const SuFrameGrabberInitData& suOriginal );
};

inline SuFrameGrabberInitData& SuFrameGrabberInitData::operator=( const SuFrameGrabberInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        strPortString = suOriginal.strPortString;
    }

    return *this;
}

struct SuGpsInitData
{
    bool bUseBase;
    string strGpsType;
    string strPortString;
    double dBaseLat;
    double dBaseLong;
    double dXDiff;
    double dYDiff;
    double dMPerLat;
    double dMPerLong;

    SuGpsInitData() {}
    SuGpsInitData( const SuGpsInitData& suOriginal ) { *this = suOriginal; }
    inline SuGpsInitData& operator=( const SuGpsInitData& suOriginal );
};

inline SuGpsInitData& SuGpsInitData::operator=( const SuGpsInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        bUseBase = suOriginal.bUseBase;
        strGpsType = suOriginal.strGpsType;
        strPortString = suOriginal.strPortString;
        dBaseLat = suOriginal.dBaseLat;
        dBaseLong = suOriginal.dBaseLong;
        dXDiff = suOriginal.dXDiff;
        dYDiff = suOriginal.dYDiff;
        dMPerLat = suOriginal.dMPerLat;
        dMPerLong = suOriginal.dMPerLong;
    }

    return *this;
}

struct SuJboxInitData
{
    int jboxID;
    bool disableNetworkQuery;
    string gpsSection;

    SuJboxInitData() {}
    SuJboxInitData( const SuJboxInitData& suOriginal ) { *this = suOriginal; }
    inline SuJboxInitData& operator=( const SuJboxInitData& suOriginal );
};

inline SuJboxInitData& SuJboxInitData::operator=( const SuJboxInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        jboxID = suOriginal.jboxID;
        disableNetworkQuery = suOriginal.disableNetworkQuery;
        gpsSection = suOriginal.gpsSection;
    }

    return *this;
}

struct SuCompassInitData
{
    int type;
    string portString;

    SuCompassInitData() {}
    SuCompassInitData( const SuCompassInitData& suOriginal ) { *this = suOriginal; }
    inline SuCompassInitData& operator=( const SuCompassInitData& suOriginal );
};

inline SuCompassInitData& SuCompassInitData::operator=( const SuCompassInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        type = suOriginal.type;
        portString = suOriginal.portString;
    }

    return *this;
}

struct SuGyroInitData
{
    int type;
    string portString;

    SuGyroInitData() {}
    SuGyroInitData( const SuGyroInitData& suOriginal ) { *this = suOriginal; }
    inline SuGyroInitData& operator=( const SuGyroInitData& suOriginal );
};

inline SuGyroInitData& SuGyroInitData::operator=( const SuGyroInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        type = suOriginal.type;
        portString = suOriginal.portString;
    }

    return *this;
}

struct SuPoseCalcInitData
{
    int fuserType;
    int varianceScheme;
    double compass2MlabHeadingFacor;
    double compass2MlabHeadingOffset;
    double compassMaxValidAngSpeed4Heading;
    double gps2MlabHeadingFacor;
    double gps2MlabHeadingOffset;
    double gpsMinValidTransSpeed4Heading;
    double gpsMaxValidAngSpeed4Heading;

    SuPoseCalcInitData() {}
    SuPoseCalcInitData( const SuPoseCalcInitData& suOriginal ) { *this = suOriginal; }
    inline SuPoseCalcInitData& operator=( const SuPoseCalcInitData& suOriginal );
};

inline SuPoseCalcInitData& SuPoseCalcInitData::operator=( const SuPoseCalcInitData& suOriginal )
{
    if ( this != &suOriginal )
    {
        fuserType = suOriginal.fuserType;
        varianceScheme = suOriginal.varianceScheme;
        compass2MlabHeadingFacor = suOriginal.compass2MlabHeadingFacor;
        compass2MlabHeadingOffset = suOriginal.compass2MlabHeadingOffset;
        compassMaxValidAngSpeed4Heading = suOriginal.compassMaxValidAngSpeed4Heading;
        gps2MlabHeadingFacor = suOriginal.gps2MlabHeadingFacor;
        gps2MlabHeadingOffset = suOriginal.gps2MlabHeadingOffset;
        gpsMinValidTransSpeed4Heading = suOriginal.gpsMinValidTransSpeed4Heading;
        gpsMaxValidAngSpeed4Heading = suOriginal.gpsMaxValidAngSpeed4Heading;
    }

    return *this;
}

class ClHServerConfigurator
{
public:
    ClHServerConfigurator( ClHServerConfig& clConfig );
    ~ClHServerConfigurator();

    SuCognachromeInitData GetCognachromeOptions( int iSection );
    SuCameraInitData GetCameraOptions( int iSection );
    SuNomadInitData GetNomadOptions( int iSection );
    SuPioneerInitData GetPioneerOptions( int iSection );
    SuPioneer2InitData GetPioneer2Options( int iSection );
    SuAmigoBotInitData GetAmigoBotOptions( int iSection );
    SuRobotLaserInitData GetRobotLaserOptions( int iSection );
	SuRoombaInitData GetRoombaOptions( int iSection );
	SuRoomba560InitData GetRoomba560Options( int iSection );
	SuCarmenInitData GetCarmenOptions( int iSection );
    SuFrameGrabberInitData GetFrameGrabberOptions( int iSection );
    SuGpsInitData GetGpsOptions( int iSection );
    SuJboxInitData GetJboxOptions( int iSection );
    SuCompassInitData GetCompassOptions( int iSection );
    SuGyroInitData GetGyroOptions( int iSection );
    SuPoseCalcInitData GetPoseCalcOptions( int iSection );
    SuLaserInitData GetLaserOptions( int iSection );
    SuWebCamInitData GetWebCamOptions( int iSection );

protected:
    ClHServerConfig* m_pclConfig;
    static const double HSCONFIG_HUGE_VAL_;
};

#endif // _HSERVERCONFIGURATOR_H

/**********************************************************************
# $Log: HServerConfigurator.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1.1.1  2005/02/06 22:59:46  endo
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
