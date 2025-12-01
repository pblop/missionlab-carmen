/**********************************************************************
 **                                                                  **
 **                          RCFileReader.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: RCFileReader.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef RCFILEREADER_H
#define RCFILEREADER_H

#include <stdio.h>
#include <string.h>

using std::string;

// types of lines in the RC file
enum EnRCFileLineType
{
    EnRCFileLineType_START,
    EnRCFileLineType_END,
    EnRCFileLineType_VALUE,
    EnRCFileLineType_EOF,
    EnRCFileLineType_ERROR
};

// information about a line in the RC file
struct SuRCFileLine
{
    EnRCFileLineType enType;
    string strLabel;
    string strValue;
};

class ClRCFileReader
{
public:
    ClRCFileReader();
    ClRCFileReader( const string& strFileName );
    ~ClRCFileReader();

    int GetLine() const;
    bool IsOpen() const;
    bool OpenFile( const string& strFileName );
    bool ReadLine( SuRCFileLine& suLine );
    bool PastEOF() const;

protected:
    FILE* m_pfInput;    // a pointer to the file being read
    bool m_bInSection;  // whether or not a section is being read
    int m_iLine;        // the line number

    void Cleanup();
    bool GetLabelValuePair( SuRCFileLine& suLine );
    bool GetTagLabel( SuRCFileLine& suLine );
    bool MovePastWhiteSpace();
    bool MoveToNextLine();
};

inline int ClRCFileReader::GetLine() const
{
    return m_iLine;
}

// check to see if the file is open
inline bool ClRCFileReader::IsOpen() const
{
    return ( m_pfInput != NULL );
}

// check to see if we have read past the end of the file
inline bool ClRCFileReader::PastEOF() const
{
    bool bEof = false;

    if ( IsOpen() )
    {
        bEof = ( feof( m_pfInput ) != 0 );
    }

    return bEof;
}

#endif // #ifndef RCFILEREADER_H

/**********************************************************************
# $Log: RCFileReader.h,v $
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
