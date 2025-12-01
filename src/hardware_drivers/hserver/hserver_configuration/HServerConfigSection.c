/**********************************************************************
 **                                                                  **
 **                      HServerConfigSection.c                      **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: HServerConfigSection.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include <stdlib.h>
#include "HServerConfigSection.h"

ClHServerConfigSection::ClHServerConfigSection()
{
    m_iType = -1;
    m_iLastError = EnHServerConfigSectionError_NONE;
    m_iValueCount = 0;
    m_strName = "";
    m_astrValues = NULL;
}

ClHServerConfigSection::ClHServerConfigSection( int iType, int iValueCount )
{
    m_iType = -1;
    m_iLastError = EnHServerConfigSectionError_NONE;
    m_iValueCount = 0;
    m_astrValues = NULL;

    Init( iType, iValueCount, "" );
}

ClHServerConfigSection::ClHServerConfigSection( int iType, int iValueCount, string strName )
{
    m_iType = -1;
    m_iLastError = EnHServerConfigSectionError_NONE;
    m_iValueCount = 0;
    m_astrValues = NULL;

    Init( iType, iValueCount, strName );
}

ClHServerConfigSection::ClHServerConfigSection( const ClHServerConfigSection& clOriginal )
{
    m_astrValues = NULL;
    *this = clOriginal;
}

ClHServerConfigSection& ClHServerConfigSection::operator=( const ClHServerConfigSection& clOriginal )
{
    if ( &clOriginal != this )
    {
        Cleanup();

        m_iType = clOriginal.m_iType;
        m_iLastError = clOriginal.m_iLastError;
        m_iValueCount = clOriginal.m_iValueCount;
        m_strName = clOriginal.m_strName;

        if ( clOriginal.m_astrValues != NULL )
        {
            if ( ( m_astrValues = new string[ m_iValueCount ] ) != NULL )
            {
                for ( int i = 0; i < m_iValueCount; i++ )
                {
                    m_astrValues[ i ] = clOriginal.m_astrValues[ i ];
                }
            }
            else
            {
                m_iLastError = EnHServerConfigSectionError_NO_MEM;
            }
        }
    }

    return *this;
}

ClHServerConfigSection::~ClHServerConfigSection()
{
    Cleanup();
}

void ClHServerConfigSection::Cleanup()
{
    if ( m_astrValues != NULL )
    {
        delete [] m_astrValues;
        m_astrValues = NULL;
    }
}

bool ClHServerConfigSection::Init( int iType, int iValueCount, string strName )
{
    bool bSuccess = true;

    Cleanup();

    m_iType = iType;
    m_iLastError = EnHServerConfigSectionError_NONE;
    m_iValueCount = iValueCount;
    m_strName = strName;

    if ( m_iValueCount > 0 )
    {
        if ( ( m_astrValues = new string[ m_iValueCount ] ) == NULL )
        {
            m_iLastError = EnHServerConfigSectionError_NO_MEM;
            bSuccess = false;
        }
    }

    return bSuccess;
}

bool ClHServerConfigSection::InsertValue( int iType, const string& strValue )
{
    bool bSuccess = false;
    m_iLastError = EnHServerConfigSectionError_NONE;

    if ( m_astrValues == NULL )
    {
        m_iLastError = EnHServerConfigSectionError_UNINITIALIZED;
    }
    else if ( ( iType < 0 ) || ( iType >= m_iValueCount ) )
    {
        m_iLastError = EnHServerConfigSectionError_BAD_TYPE;
    }
    else
    {
        m_astrValues[ iType ] = strValue;
        bSuccess = true;
    }

    return bSuccess;
}

bool ClHServerConfigSection::GetValue( int iType, string& strValue )
{
    bool bSuccess = false;
    m_iLastError = EnHServerConfigSectionError_NONE;

    if ( m_astrValues == NULL )
    {
        m_iLastError = EnHServerConfigSectionError_UNINITIALIZED;
    }
    else if ( ( iType < 0 ) || ( iType >= m_iValueCount ) )
    {
        m_iLastError = EnHServerConfigSectionError_BAD_TYPE;
    }
    else
    {
        strValue = m_astrValues[ iType ];
        bSuccess = true;
    }

    return bSuccess;
}

/**********************************************************************
# $Log: HServerConfigSection.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
