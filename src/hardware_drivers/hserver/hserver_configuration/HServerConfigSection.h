/**********************************************************************
 **                                                                  **
 **                     HServerConfigSection.h                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  James Brian Lee                                    **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: HServerConfigSection.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef HSERVERCONFIGSECTION_H
#define HSERVERCONFIGSECTION_H

#include <string>

using std::string;

class ClHServerConfigSection
{
public:
    enum EnHServerConfigSectionError
    {
        EnHServerConfigSectionError_NONE = 0, 
        EnHServerConfigSectionError_NO_MEM,
        EnHServerConfigSectionError_UNINITIALIZED,
        EnHServerConfigSectionError_BAD_TYPE
    };

    ClHServerConfigSection();
    ClHServerConfigSection( int iType, int iValueCount );
    ClHServerConfigSection( int iType, int iValueCount, string strName );
    ClHServerConfigSection( const ClHServerConfigSection& clOriginal );
    ~ClHServerConfigSection();

    ClHServerConfigSection& operator=( const ClHServerConfigSection& clOriginal );

    bool Init( int iType, int iValueCount );
    bool Init( int iType, int iValueCount, string strName );
    bool InsertValue( int iType, const string& strValue );
    bool GetValue( int iType, string& strValue );
    int GetType() const;
    int GetLastError() const;
    string GetName() const;
    void SetName( string strName );

protected:
    int m_iType;
    string m_strName;
    int m_iLastError;
    int m_iValueCount;
    string* m_astrValues;

    void Cleanup();
};

inline bool ClHServerConfigSection::Init( int iType, int iValueCount )
{
    return Init( iType, iValueCount, "" );
}

inline int ClHServerConfigSection::GetType() const
{
    return m_iType;
}

inline int ClHServerConfigSection::GetLastError() const
{
    return m_iLastError;
}

inline string ClHServerConfigSection::GetName() const
{
    return m_strName;
}

inline void ClHServerConfigSection::SetName( string strName )
{
    m_strName = strName;
}

#endif // ifndef _HSERVERCONFIGSECTION_H

/**********************************************************************
# $Log: HServerConfigSection.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1.1.1  2005/02/06 22:59:46  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.2  2003/04/06 13:35:53  endo
# gcc 3.1.1
#
#**********************************************************************/
