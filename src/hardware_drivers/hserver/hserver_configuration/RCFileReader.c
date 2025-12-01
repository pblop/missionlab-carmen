/**********************************************************************
 **                                                                  **
 **                          RCFileReader.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: RCFileReader.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include <string>
#include "RCFileReader.h"

const char* g_szERR_UNEXPECTED_EOF      = "Unexpected end of file";
const char* g_szERR_NO_LABEL            = "Couldn't find a label";
const char* g_szERR_ORPHANED_END        = "END without a section";
const char* g_szERR_TAG_WITHIN_SECTION  = "A non-end tag was found in a section";
const char* g_szERR_UNCLOSED_TAG        = "A tag was not closed";
const char* g_szERR_EXTRA_TEXT          = "Extra text found on line";
const char* g_szERR_LABEL_WITHOUT_VALUE = "Label without value";
const char* g_szERR_MISSING_EQUALS      = "Missing equals sign";

bool IsBlank( char c )
{
    return ( ( c == ' ' ) || ( c == '\t' ) );
}

ClRCFileReader::ClRCFileReader()
{
    m_pfInput = NULL;
    m_iLine = 0;
    m_bInSection = false;
}

ClRCFileReader::ClRCFileReader( const string& strFileName )
{
    m_pfInput = NULL;
    m_iLine = 0;
    m_bInSection = false;
    OpenFile( strFileName );
}

ClRCFileReader::~ClRCFileReader()
{
    Cleanup();
}

bool ClRCFileReader::OpenFile( const string& strFileName )
{
    Cleanup();

    m_pfInput = fopen( strFileName.c_str(), "r" );

    if ( IsOpen() )
    {
        m_iLine = 1;
    }

    return IsOpen();
}

bool ClRCFileReader::ReadLine( SuRCFileLine& suLine )
{
    bool bSuccess = IsOpen();

    suLine.strLabel = "";
    suLine.strValue = "";
    suLine.enType = EnRCFileLineType_ERROR;

    if ( bSuccess )
    {
        char c;

        // kill front white space
        if ( !MovePastWhiteSpace() )
        {
            bSuccess = !m_bInSection;
            if ( bSuccess )
            {
                suLine.enType = EnRCFileLineType_EOF;
            }
            else
            {
                suLine.strLabel = g_szERR_UNEXPECTED_EOF;
            }
        }
        else
        {
            switch ( c = fgetc( m_pfInput ) )
            {
            case '#': // if we find a comment, move to the next line
                if ( !MoveToNextLine() )
                {
                    bSuccess = !m_bInSection;
                    if ( bSuccess )
                    {
                        suLine.enType = EnRCFileLineType_EOF;
                    }
                    else
                    {
                        suLine.strLabel = g_szERR_UNEXPECTED_EOF;
                    }
                    break;
                }
                // intentional fall-through

            case '\n': // effectively empty line, move on
                m_iLine++;
                bSuccess = ReadLine( suLine );
                break;

            case '<':  // this should be a "tag line", so get the tag label
                bSuccess = GetTagLabel( suLine );
                break;

            default: // if we get here, then we must have a <label = value> line
                suLine.strLabel += c;
                bSuccess = GetLabelValuePair( suLine );
                break;
            }
        }
    }

    return bSuccess;
}

bool ClRCFileReader::GetTagLabel( SuRCFileLine& suLine )
{
    bool bSuccess = false;
    char c;
    string strTagType = "";

    // get rid of white space
    bSuccess = MovePastWhiteSpace();

    // get the tag type
    while ( bSuccess )
    {
        c = fgetc( m_pfInput );

        // we can't finish the line or file in the middle of a label
        if ( PastEOF() || ( c == '\n' ) || ( c == '#' ) )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_NO_LABEL;
            bSuccess = false;
            break;
        }

        if ( IsBlank( c ) || ( c == '>' ) )
        {
            break;
        }

        strTagType += c;
    };

    // see what type of label it was
    if ( bSuccess )
    {
        // it better not have been empty...
        if ( strTagType == "" )
        {
            bSuccess = false;
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_NO_LABEL;
        }
        // it could have been a <end> tag
        else if ( strcasecmp( strTagType.c_str(), "end" ) == 0 )
        {
            // make sure it's not an orphaned <end>
            if ( !m_bInSection )
            {
                bSuccess = false;
                suLine.enType = EnRCFileLineType_ERROR;
                suLine.strLabel = g_szERR_ORPHANED_END;
            }
            else
            {
                suLine.enType = EnRCFileLineType_END;
            }
        }
        else if ( strcasecmp( strTagType.c_str(), "start" ) == 0 )
        {
            // <end> tags are the only ones allowed if we're already in a section, 
            // so if we found a tag that's not <end>, and we're in a section, then error
            if ( m_bInSection )
            {
                bSuccess = false;
                suLine.enType = EnRCFileLineType_ERROR;
                suLine.strLabel = g_szERR_TAG_WITHIN_SECTION;
            }
            else
            {
                suLine.enType = EnRCFileLineType_START;
            }
        }
    }

    // get rid of white space. if we hit EOF before a label, then error.
    // this moves up to the label, so if the tag has been closed off or it's 
    // an "end" tag, don't do it
    if ( bSuccess && ( suLine.enType != EnRCFileLineType_END ) && ( c != '>' ) )
    {
        bSuccess = MovePastWhiteSpace();
        if ( !bSuccess )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_UNCLOSED_TAG;
        }
    }

    // get the label. if it was an "end" tag, then there is no label to get.
    // if the tag has been closed off already, then there's no label to get
    while ( bSuccess && ( suLine.enType != EnRCFileLineType_END ) && ( c != '>' ) )
    {
        c = fgetc( m_pfInput );

        // we can't finish the line or file in the middle of a label
        if ( PastEOF() || ( c == '\n' ) || ( c == '#' ) )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_NO_LABEL;
            bSuccess = false;
            break;
        }

        if ( ( IsBlank( c ) ) || ( c == '>' ) )
        {
            break;
        }

        suLine.strLabel += c;
    };

    // get rid of white space. if we hit EOF before a label, then error.
    // this moves up to the section name, so if the tag has been closed off or it's 
    // an "end" tag, don't do it
    if ( bSuccess && ( suLine.enType != EnRCFileLineType_END ) && ( c != '>' ) )
    {
        bSuccess = MovePastWhiteSpace();
        if ( !bSuccess )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_UNCLOSED_TAG;
        }
    }

    // get the section. if it was an "end" tag, then there is no section name to get.
    // if the tag has been closed off already, then there's no label to get
    while ( bSuccess && ( suLine.enType != EnRCFileLineType_END ) && ( c != '>' ) )
    {
        c = fgetc( m_pfInput );

        // we can't finish the line or file in the middle of a label
        if ( PastEOF() || ( c == '\n' ) || ( c == '#' ) )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_NO_LABEL;
            bSuccess = false;
            break;
        }

        if ( ( IsBlank( c ) ) || ( c == '>' ) )
        {
            break;
        }

        suLine.strValue += c;
    };

    // get rid of white space and move up to the closing '>'. 
    // if we hit EOF before a '>', then error don't worry 
    // about it if we've already hit '>', though
    if ( bSuccess && ( c != '>' ) )
    {
        bSuccess = MovePastWhiteSpace();
        if ( !bSuccess )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_UNCLOSED_TAG;
        }
    }

    // if a character isn't '>', error. don't worry about it if we've
    // already hit '>', though
    if ( bSuccess && ( c != '>' ) )
    {
        if ( ( c = fgetc( m_pfInput ) ) != '>' )
        {
            bSuccess = false;
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_UNCLOSED_TAG;
        }
    }

    // get rid of white space
    if ( bSuccess )
    {
        bool bMoreToRead = MovePastWhiteSpace();

        // we only need to worry about hitting EOF if this is a start tag;
        // if we just ended a section, it's OK
        if ( ( suLine.enType == EnRCFileLineType_START ) && !bMoreToRead )
        {
            bSuccess = false;
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_UNEXPECTED_EOF;
        }
    }

    // make sure we're ending on an end-of-line or a comment
    if ( bSuccess )
    {
        c = fgetc( m_pfInput );

        // don't worry about anything after the comment
        if ( c == '#' )
        {
            bool bMoreToRead = MoveToNextLine();

            // we only need to worry about hitting EOF if this is a start tag;
            // if we just ended a section, it's OK
            if ( ( suLine.enType == EnRCFileLineType_START ) && !bMoreToRead )
            {
                bSuccess = false;
                suLine.enType = EnRCFileLineType_ERROR;
                suLine.strLabel = g_szERR_UNEXPECTED_EOF;
            }
        }
        // nothing is allowed after the closing '>' except a comment 
        // or an end-of-line
        else if ( c != '\n' )
        {
            bSuccess = false;
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_EXTRA_TEXT;
        }
    }

    if ( bSuccess )
    {
        m_bInSection = ( suLine.enType == EnRCFileLineType_START );

        if ( !PastEOF() )
        {
            m_iLine++;
        }
    }

    return bSuccess;
}

bool ClRCFileReader::GetLabelValuePair( SuRCFileLine& suLine )
{
    bool bSuccess = true;
    char c;
    string strValue = "";

    // read until we hit white space
    while ( true )
    {
        c = fgetc( m_pfInput );

        // can't end the file on a label/value pair, and can't have a comment
        // until the end of the line
        if ( PastEOF() || ( c == '#' ) )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_LABEL_WITHOUT_VALUE;
            bSuccess = false;
            break;
        }

        if ( IsBlank( c ) )
        {
            break;
        }

        suLine.strLabel += c;
    }

    // get rid of white space
    if ( bSuccess )
    {
        bSuccess = MovePastWhiteSpace();
        if ( !bSuccess )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_UNEXPECTED_EOF;
        }
    }

    // if the next character isn't '=', error
    if ( bSuccess )
    {
        c = fgetc( m_pfInput );
        if ( c != '=' )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_MISSING_EQUALS;
            bSuccess = false;
        }
    }

    // get rid of white space
    if ( bSuccess )
    {
        bSuccess = MovePastWhiteSpace();
        if ( !bSuccess )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_UNEXPECTED_EOF;
        }
    }

    // read the rest of the line
    while ( bSuccess )
    {
        c = fgetc( m_pfInput );

        if ( PastEOF() )
        {
            suLine.enType = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_UNEXPECTED_EOF;
            bSuccess = false;
            break;
        }

        if ( c == '\n' )
        {
            break;
        }

        if ( c == '#' )
        {
            break;
        }

        strValue += c;
    }

    // if there's a comment at the end, read to end of line
    if ( c == '#' )
    {
        bSuccess = MoveToNextLine();
        if ( !bSuccess )
        {
            suLine.strValue = EnRCFileLineType_ERROR;
            suLine.strLabel = g_szERR_UNEXPECTED_EOF;
        }
    }

    // trim trailing white space
    if ( bSuccess )
    {
        int iLastChar = strValue.length() - 1;

        while ( ( iLastChar >= 0 ) && IsBlank( strValue[ iLastChar ] ) )
        {
            iLastChar--;
        }
        suLine.strValue = strValue.erase( iLastChar + 1, strValue.length() );
    }

    if ( bSuccess )
    {
        m_iLine++;
        suLine.enType = EnRCFileLineType_VALUE;
    }

    return bSuccess;
}

// assume file is open
bool ClRCFileReader::MoveToNextLine()
{
    while ( ( fgetc( m_pfInput ) != '\n' ) && !PastEOF() );

    return !PastEOF();
}

// this stays on the same line
bool ClRCFileReader::MovePastWhiteSpace()
{
    char c;

    do
    {
        c = fgetc( m_pfInput );
    } while ( IsBlank( c ) && !PastEOF() );

    // we've read a non-white space character; put it back
    // if we've moved past the EOF, the last line was
    // all white space, so stay past the EOF
    if ( !PastEOF() )
    {
        fseek( m_pfInput, -1, SEEK_CUR );
    }

    return !PastEOF();
}

void ClRCFileReader::Cleanup()
{
    if ( IsOpen() )
    {
        fclose( m_pfInput );
    }

    m_pfInput = NULL;
    m_iLine = 0;
    m_bInSection = false;
}

/**********************************************************************
# $Log: RCFileReader.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
