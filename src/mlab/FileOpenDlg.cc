/**********************************************************************
 **                                                                  **
 **                       FileOpenDlg.cc                             **
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

#include <stdio.h>

#include <Xm/TextF.h>
#include <Xm/FileSB.h>
#include <Xm/Protocols.h>

#include "FileOpenDlg.h"
#include "file_utils.h"

// this moves up the widget tree of w until a shell widget is found
static Widget GetTopShell( Widget w )
{
    while ( w && !XtIsWMShell( w ) )
    {
        w = XtParent( w );
    }

    return w;
}

// throws an exception if bExpression is false
static void Assert( bool bExpression )
{
    if ( !bExpression )
    {
        throw 0;
    }
}

// constructor
ClFileOpenDlg::ClFileOpenDlg( Widget widgetParent /*=NULL*/, ClFileOpenDlg** ppclThis /*=NULL*/ )
{
    m_iRet = EnNOT_RET_YET;
    m_widgetDialog = NULL;
    m_ppclThis = ppclThis;
    m_strFileName = "";
    m_strDir = "";
    m_strTitle = "";
    SetParent( widgetParent );
}

// destructor
ClFileOpenDlg::~ClFileOpenDlg()
{
    if ( m_ppclThis != NULL )
    {
        *m_ppclThis = NULL;
    }
}

// accessor to get the parent
Widget ClFileOpenDlg::GetParent()
{
    return m_widgetParent;
}

// accessor to set the parent
void ClFileOpenDlg::SetParent( Widget widgetParent )
{
    m_widgetParent = widgetParent;
}

// set up the dialog to be a FileOpen dialog. This must be called before Popup() or DoModal() are called
bool ClFileOpenDlg::Create()
{
    bool bSuccess = true;

    Arg wargs[2];
    int n = 0;

    try
    {
        Assert( ( m_widgetParent != NULL ) && 
                ( m_widgetDialog == NULL ) );

        XmString title = XmStringCreateLocalized( (char*) m_strTitle.c_str() );

        XtSetArg( wargs[ n ], XmNdialogTitle, title ); n++;
        m_widgetDialog = XmCreateFileSelectionDialog( m_widgetParent, "file-open-dialog", wargs, n );
        XmStringFree( title );
        Assert( m_widgetDialog != NULL );

        XtUnmanageChild( XmFileSelectionBoxGetChild( m_widgetDialog, XmDIALOG_HELP_BUTTON ) );

        // install the callbacks
        XtAddCallback( m_widgetDialog, XmNokCallback, OkCallback, this );
        XtAddCallback( m_widgetDialog, XmNcancelCallback, CancelCallback, this );

        Atom atomWmDeleteWindow = XmInternAtom( XtDisplay( m_widgetDialog ), "WM_DELETE_WINDOW", False );
        XmAddWMProtocolCallback( m_widgetDialog, atomWmDeleteWindow, DeleteWindowCallback, this );
    }
    catch ( int iExceptionNum )
    {
        // clean up if stuff failed
        if ( m_widgetDialog != NULL )
        {
            XtDestroyWidget( m_widgetDialog );
            m_widgetDialog = NULL;
        }
        bSuccess = false;
        printf( "caught exception number \"%d\" in ClEditLMParamsDlg::Create\n", iExceptionNum );
    }

    return bSuccess;
}

// pop up a modless dialog
void ClFileOpenDlg::Popup()
{
    if ( m_widgetDialog != NULL )
    {
        XtManageChild( m_widgetDialog );
    }
}

// pop up a modal dialog. This will not return until the user closes the dialog
int ClFileOpenDlg::DoModal()
{
    if ( m_widgetDialog != NULL )
    {
        Popup(); // display the dialog

        // flush the event queue
        XFlush( XtDisplay( m_widgetDialog ) );
        XmUpdateDisplay( XtParent( m_widgetDialog ) );

        // loop until the dialog has been closed
        while ( m_iRet == EnNOT_RET_YET )
        {
            // get the next event
            XEvent event;
            XNextEvent( XtDisplay( m_widgetDialog ), &event );

            // see who the event goes to
            Widget widgetEventSource = XtWindowToWidget( event.xany.display, event.xany.window );
            widgetEventSource = GetTopShell( widgetEventSource );

            // if the event belongs to the dialog OR is used to update the display of 
            // another window, dispatch it
            if ( ( ( widgetEventSource == XtParent( m_widgetDialog ) ) || 
                   ( event.type == Expose ) ) && 
                 ( event.type != FocusIn ) && 
                 ( event.type != FocusOut ) )
            {
                XtDispatchEvent( &event );
            }
        }
    }

    return m_iRet;
}

// kill the dialog
void ClFileOpenDlg::Destroy()
{
    // destroy the dialog box
    if ( m_widgetDialog != NULL )
    {
        XtDestroyWidget( m_widgetDialog );
        m_widgetDialog = NULL;
    }

    // if this window was dynamically allocated, deallocate it
    if ( ( m_ppclThis != NULL ) && ( *m_ppclThis != NULL ) )
    {
        printf( "deleting this\n" );
        delete this;
    }
    else printf( "not deleting this\n" );
}

// see how the dialog was dismissed
int ClFileOpenDlg::GetRetVal()
{
    return m_iRet;
}

// get the file name represented by the dialog
string ClFileOpenDlg::GetFileName()
{
    return m_strFileName;
}

// set the value of the file name and update the display appropriately
void ClFileOpenDlg::SetFileName( const string& strFileName )
{
    m_strFileName = strFileName;

    Widget widgetTextField;
    string strNewFilename;
    string strDirectory;
    char* szNewDir;
    char* szOldFileName;
    XmString dir, new_filename, new_full_filename;

    // Combine the two filenames appropriately
    if ( filename_has_directory( strFileName ) )
    {
        // We have a directory, use it to override the old one
        SetDirectory( extract_directory( strFileName ) );

        // Retrieve the new directory from the file open dialog
        // (This gets the newly expanded version)
        XtVaGetValues( m_widgetDialog, XmNdirectory, &dir, NULL );

        // Reintegrate the simple filename with the newly expanded directory
        strNewFilename = remove_directory( strFileName );
        new_filename = XmStringCreateLocalized( (char*) strNewFilename.c_str() );
        new_full_filename = XmStringConcat( dir, new_filename );

        // Install the new full filename into the file open dialog
        XtVaSetValues( m_widgetDialog, XmNtextString, new_full_filename, NULL );
        XmStringFree( new_filename );
        XmStringFree( new_full_filename );
    }
    else // No directory in the new filename
    {
        // Get the old filename
        widgetTextField = XmFileSelectionBoxGetChild( m_widgetDialog, XmDIALOG_TEXT );
        szOldFileName = XmTextFieldGetString( widgetTextField );

        // Check the directory in the old filename
        if ( ( strDirectory = extract_directory( szOldFileName ) ) == "" )
        {
            // No directory in the old file open dialog filename
            XmTextFieldSetString( widgetTextField, (char*) strFileName.c_str() );
        }
        else
        {
            // There is a directory in the old name

            // Revert the directory in the file dialog.  This makes the
            // file open dialog box reset itself to the default directory.
            SetDirectory( "" );

            // Get the newly refreshed directory
            XmStringContext context;
            XmStringCharSet charset;
            XmStringDirection direction;
            Boolean separator;
            XtVaGetValues( m_widgetDialog, XmNdirectory, &dir, NULL );
            XmStringInitContext( &context, dir );
            XmStringGetNextSegment( context, &szNewDir, &charset, &direction, &separator );
            XmStringFreeContext( context );

            // Construct the new complete filename
            strNewFilename = szNewDir;
            strNewFilename += strFileName;
            XtFree( szNewDir );
            XmTextFieldSetString( widgetTextField, (char*) strNewFilename.c_str() );
        }

        XtFree( szOldFileName );
    }
}

// get the title of the dialog
string ClFileOpenDlg::GetTitle()
{
    return m_strTitle;
}

// set the title of the dialog
void ClFileOpenDlg::SetTitle( const string& strTitle )
{
    m_strTitle = strTitle; 

    if ( m_widgetDialog != NULL )
    {
        XmString title = XmStringCreateLocalized( (char*) m_strTitle.c_str() );
        XtVaSetValues( m_widgetDialog, XmNdialogTitle, title, NULL );
        XmStringFree( title );
    }
}

// get the directory represented by the dialog
string ClFileOpenDlg::GetDirectory()
{
    return m_strDir;
}

// set the directory represented by the dialog and update the diaplay appropriately
void ClFileOpenDlg::SetDirectory( const string& strDir )
{
    m_strDir = strDir;

    if ( m_widgetDialog != NULL )
    {
        // Extract the directory and install it into the file open dialog
        XmString dir = XmStringCreateLocalized( (char*) m_strDir.c_str() );
        XtVaSetValues( m_widgetDialog, XmNdirectory, dir, NULL );
        XmStringFree( dir );

        // Retrieve the new directory from the file open dialog
        // (This gets the newly expanded version)
        XtVaGetValues( m_widgetDialog, XmNdirectory, &dir, NULL );
    }
}

// update the data from the controls
void ClFileOpenDlg::UpdateData()
{
    if ( m_widgetDialog != NULL )
    {
        XmString str;
        char* szTemp;

	    XmStringContext context;
	    XmStringCharSet charset;
	    XmStringDirection direction;
	    Boolean separator;

        // get the file name
	    XtVaGetValues( m_widgetDialog, XmNdirSpec, &str, NULL );
	    XmStringInitContext( &context, str );
	    XmStringGetNextSegment( context, &szTemp, &charset, &direction, &separator );
        m_strFileName = szTemp;
        XtFree( szTemp );
	    XmStringFreeContext( context );

        // get the directory
	    XtVaGetValues( m_widgetDialog, XmNdirectory, &str, NULL );
	    XmStringInitContext( &context, str );
	    XmStringGetNextSegment( context, &szTemp, &charset, &direction, &separator );
        m_strDir = szTemp;
        XtFree( szTemp );
	    XmStringFreeContext( context );
    }
}

// this is called when the OK button is clicked or the user double-clicks on a file name
void ClFileOpenDlg::OkCallback( Widget widgetButton, XtPointer pClientData, XtPointer pCallData )
{
    ClFileOpenDlg* pclDialog = (ClFileOpenDlg*) pClientData;

    // update the dialog's changes to the data
    if ( pclDialog != NULL )
    {
        pclDialog->UpdateData();
    }

    pclDialog->m_iRet = EnRET_OK;

    // destroy the dialog box
    pclDialog->Destroy();
}

// this is called when the user clicks the cancel button
void ClFileOpenDlg::CancelCallback( Widget widgetButton, XtPointer pClientData, XtPointer pCallData )
{
    ClFileOpenDlg* pclDialog = (ClFileOpenDlg*) pClientData;

    pclDialog->m_iRet = EnRET_CANCEL;

    // destroy the dialog box
    pclDialog->Destroy();
}

// this is called when the window dies. If a return value has not been designated, treat this 
// as a "Cancel"
void ClFileOpenDlg::DeleteWindowCallback( Widget widgetDialog, XtPointer pClientData, XtPointer pCallData )
{
    ClFileOpenDlg* pclDialog = (ClFileOpenDlg*) pClientData;

    // if neither the "OK" or "Cancel" button were used to destroy the dialog, 
    // treat the destruction as a "Cancel"
    if ( pclDialog->m_iRet == EnNOT_RET_YET )
    {
        pclDialog->m_iRet = EnRET_CANCEL;
    }

    // destroy the dialog box
    pclDialog->Destroy();
}
