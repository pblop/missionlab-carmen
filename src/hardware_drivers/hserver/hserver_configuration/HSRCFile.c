/**********************************************************************
 **                                                                  **
 **                              HSRCFile.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: HSRCFile.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include "HSRCFile.h"

ClHSRCFile::ClHSRCFile()
{
    SetFileName( "" );
    m_iCurrentSection = -1;
}

ClHSRCFile::ClHSRCFile( string strFileName )
{
    m_iCurrentSection = -1;
    SetFileName( strFileName );
}

bool ClHSRCFile::CreateConfiguration( ClHServerConfig& clConfig )
{
    bool bSuccess = false;

    if ( m_strFileName != "" )
    {
        ClRCFileReader clFile( m_strFileName );
        if ( clFile.IsOpen() )
        {
            bSuccess = true;
            SuRCFileLine suLine;
            while ( !clFile.PastEOF() && bSuccess )
            {
                if ( clFile.ReadLine( suLine ) && 
                     ( suLine.enType != EnRCFileLineType_ERROR ) )
                {
                    bSuccess = ProcessLine( suLine, clConfig, clFile.GetLine() );
                }
                else
                {
                    fprintf( stderr, "Error: hserver config file line %d:\n", clFile.GetLine() );
                    fprintf( stderr, "    %s\n", suLine.strLabel.c_str() );
                    bSuccess = false;
                }
            }
        }
        else
        {
            fprintf( stderr, "Error: couldn't open file: %s\n", m_strFileName.c_str() );
        }
    }

    return bSuccess;
}

int ClHSRCFile::LabelToId( const SuLabelIdPair* asuLabelIdPairs, const string& strLabel )
{
    int iId = -1;

    if ( asuLabelIdPairs != NULL )
    {
        int i = 0;
        while ( asuLabelIdPairs[ i ].szLabel != NULL )
        {
            if ( strcasecmp( strLabel.c_str(), asuLabelIdPairs[ i ].szLabel ) == 0 )
            {
                iId = asuLabelIdPairs[ i ].iId;
                break;
            }
            i++;
        }
    }

    return iId;
}

bool ClHSRCFile::ProcessStartSectionLine( const SuRCFileLine& suLine, ClHServerConfig& clConfig, int iLineNum )
{
    bool bSuccess = false;

    if ( m_iCurrentSection == -1 )
    {
        m_iCurrentSection = LabelToId( asuSECTION_LABEL_CONVERSIONS, suLine.strLabel );
        if ( bSuccess = ( m_iCurrentSection != -1 ) )
        {
            clConfig.AddNewSection( m_iCurrentSection, suLine.strValue );
        }
        else
        {
            fprintf( stderr, "Error: hserver config file line %d\n", iLineNum );
            fprintf( stderr, "    Bad section name: %s\n", suLine.strLabel.c_str() );
        }
    }
    else
    {
        fprintf( stderr, "Error: hserver config file line %d\n", iLineNum );
        fprintf( stderr, "    Can't start new section inside of old one\n" );
    }

    return bSuccess;
}

bool ClHSRCFile::ProcessValueLine( const SuRCFileLine& suLine, ClHServerConfig& clConfig, int iLineNum )
{
    bool bSuccess = false;

    if ( m_iCurrentSection > -1 )
    {
        int iValueId = LabelToId( aasuVALUE_TYPE_CONVERSIONS[ m_iCurrentSection ], suLine.strLabel );

        if ( iValueId != -1 )
        {
            bSuccess = clConfig.InsertValue( clConfig.GetSectionCount() - 1, iValueId, suLine.strValue );
            if ( !bSuccess )
            {
                fprintf( stderr, "Error: hserver config file line %d\n", iLineNum );
                fprintf( stderr, "    unknown error\n" );
            }
        }
        else
        {
            fprintf( stderr, "Error: hserver config file line %d\n", iLineNum );
            fprintf( stderr, "    Invalid value type for the current section: %s\n", suLine.strLabel.c_str() );
        }
    }
    else
    {
        fprintf( stderr, "Error: hserver config file line %d\n", iLineNum );
        fprintf( stderr, "    A value is not in a valid section\n" );
    }

    return bSuccess;
}

bool ClHSRCFile::ProcessLine( const SuRCFileLine& suLine, ClHServerConfig& clConfig, int iLineNum )
{
    bool bSuccess = false;

    switch ( suLine.enType )
    {
    case EnRCFileLineType_START:
        bSuccess = ProcessStartSectionLine( suLine, clConfig, iLineNum );
        break;

    case EnRCFileLineType_END:
        m_iCurrentSection = -1;
        bSuccess = true;
        break;

    case EnRCFileLineType_VALUE:
        bSuccess = ProcessValueLine( suLine, clConfig, iLineNum );
        break;

    case EnRCFileLineType_EOF:
        // do nothing since the processing should be over
        bSuccess = true;
        break;

    case EnRCFileLineType_ERROR:
    default:
        fprintf( stderr, "Error: hserver config file line %d\n", iLineNum );
        fprintf( stderr, "    unknown error\n" );
        break;
    }

    return bSuccess;
}

/**********************************************************************
# $Log: HSRCFile.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
