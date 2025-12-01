/**********************************************************************
 **                                                                  **
 **                       FileOpenDlg.h                              **
 **                                                                  **
 **  A class for a "file open" dialog box.                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  J. Brian Lee                                       **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: FileOpenDlg.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef _FILEOPENDLG_H
#define _FILEOPENDLG_H

#include <string>
#include <Xm/Xm.h>

using std::string;

class ClFileOpenDlg
{
public:
    enum { EnNOT_RET_YET, EnRET_OK, EnRET_CANCEL };

    ClFileOpenDlg( Widget widgetParent = NULL, ClFileOpenDlg** ppclThis = NULL );
    ~ClFileOpenDlg();

    Widget GetParent();
    void SetParent( Widget widgetParent );

    bool Create();

    void Popup();
    int DoModal();
    void Destroy();

    int GetRetVal();

    string GetFileName();
    void SetFileName( const string& strFileName );

    string GetTitle();
    void SetTitle( const string& strTitle );

    string GetDirectory();
    void SetDirectory( const string& strDir );

protected:
    int m_iRet;
    Widget m_widgetDialog;
    Widget m_widgetParent;
    string m_strFileName;
    string m_strDir;
    string m_strTitle;
    ClFileOpenDlg** m_ppclThis;

    void UpdateData();

    static void OkCallback( Widget widgetButton, XtPointer pClientData, XtPointer pCallData );
    static void CancelCallback( Widget widgetButton, XtPointer pClientData, XtPointer pCallData );
    static void DeleteWindowCallback( Widget widgetDialog, XtPointer pClientData, XtPointer pCallData );
};

#endif // ndef _FILEOPENDLG_H

/**********************************************************************
 * $Log: FileOpenDlg.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:12  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 **********************************************************************/
