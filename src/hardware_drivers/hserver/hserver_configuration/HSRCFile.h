/**********************************************************************
 **                                                                  **
 **                              HSRCFile.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: HSRCFile.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef HSRCFILE_H
#define HSRCFILE_H

#include <string.h>
#include "HServerConfig.h"
#include "RCFileReader.h"
#include "LabelToIdConversions.h"

class ClHSRCFile
{
public:
    ClHSRCFile();
    ClHSRCFile( string strFileName );

    void SetFileName( string strFileName );

    bool CreateConfiguration( ClHServerConfig& clConfig );
    bool CreateConfiguration( const string& strFileName, ClHServerConfig& clConfig );

protected:
    string m_strFileName;
    int m_iCurrentSection;

    int LabelToId( const SuLabelIdPair* asuLabelIdPairs, const string& strLabel );
    bool ProcessLine( const SuRCFileLine& suLine, ClHServerConfig& clConfig, int iLineNum );
    bool ProcessStartSectionLine( const SuRCFileLine& suLine, ClHServerConfig& clConfig, int iLineNum );
    bool ProcessValueLine( const SuRCFileLine& suLine, ClHServerConfig& clConfig, int iLineNum );
};

inline void ClHSRCFile::SetFileName( string strFileName )
{
    m_strFileName = strFileName;
}

inline bool ClHSRCFile::CreateConfiguration( const string& strFileName, ClHServerConfig& clConfig )
{
    SetFileName( strFileName );
    return CreateConfiguration( clConfig );
}

#endif // ifndef HSRCFILE_H

/**********************************************************************
# $Log: HSRCFile.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
