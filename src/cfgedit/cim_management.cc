/**********************************************************************
 **                                                                  **
 **                          cim_management.cc                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: cim_management.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <stdio.h>
#include <Xm/Label.h>
#include <signal.h>
#include <unistd.h>

#include "prototypes.h"
#include "globals.h"
#include "mission_expert.h"
#include "EventLogging.h"
#include "assistantDialog.h"
#include "GT_MPPC_client.h"
#include "popups.h"


using std::string;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
#define MAXARGS         20
#define MAXSUBSCRIPTIONS 5

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define RES_CONVERT(res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1
#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)



//-----------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------

Widget cim_run_window;
Widget w_use_cim;
Widget w_cim_host_name;
Widget w_cim_port_number;
Widget w_mppc_port_number;
Widget w_mppc_host_name;
Widget text_output;	
Widget w_set_connection_button;
Widget w_disconnect_button;
Widget w_subscription_type;
Widget w_message_type;
Widget w_namespace_type;
Widget scroll;
Widget text_w;
Widget w_set_subscribe_button;
Widget w_set_unsubscribe_button;



string str_subscription = "onetime"; 
string str_message_type = "Environment";
string str_namespace_type = "xmlstring";

static bool doneCIMDialog;
static bool okCIMDialog;
int iUseCIM;
//GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

//-----------------------------------------------------------------------
static void done_run(void)
{
    gEventLogging->log("SET-button in CIM-Dialog pressed.");
    doneCIMDialog = true;
    okCIMDialog = true;
}

void cim_send_message( string strXMLMessage )
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();
    pMPPC->SendString((char *)(strXMLMessage.c_str()));
    return;
}

//-----------------------------------------------------------------------

static void set_connection(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() == true && pMPPC->GetListeningToServerSocket()==true ) {
        //alert the user
        warn_user("MissionLab is already connected to the CIM");
    }
    else {
        char* pEnd;
        int iCIMPort=-1;
        int iMPPCPort=-1;

        iCIMPort = strtol(XmTextGetString(w_cim_port_number), &pEnd,0);
        iMPPCPort = strtol(XmTextGetString(w_mppc_port_number), &pEnd,0);

        if(iCIMPort<1 || iMPPCPort<1 ) {
            fprintf(stderr,"Error: Invalid port number\n");
            return;
        }

        pMPPC->SetCIMHostname( XmTextGetString(w_cim_host_name) );
        pMPPC->SetMPPCHostname( XmTextGetString(w_mppc_host_name) );
        pMPPC->SetCIMPortNumber( iCIMPort );
        pMPPC->SetMPPCPortNumber( iMPPCPort );
        pMPPC->SetupServerSocket();
        pMPPC->SetupClientSocket();
        //	pMPPC->ListenForMessages();
    }
}
//-----------------------------------------------------------------------

static void disconnect(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() != true && pMPPC->GetListeningToServerSocket()!=true ) {
        //alert the user
        warn_user("MissionLab is already disconnected from the CIM");
    }	
    else { 
        char buf[1024];
        pMPPC->UnsubscribeAll();
        sprintf(buf, "Currently unsubscribed to CIM"); 
        XmTextSetString( text_w, buf);
        sleep(1);
        pMPPC->CloseSockets();
    }
}

static void set_subscribe(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() != true || pMPPC->GetListeningToServerSocket()!=true ) {
        //alert the user
        warn_user("MissionLab is not connected to the CIM. Please connect first.");
    }
    else {
	
        if(pMPPC->GetNumberOfSubscriptions() == 0 )
            XmTextSetString( text_w, "");

        string one = XmTextGetString(w_subscription_type);
        string two = XmTextGetString(w_message_type);
        string three = XmTextGetString(w_namespace_type);
        string strSub = one +";"+ two +";" + three;


        if( pMPPC->SubscriptionExist( strSub ) == true) {  
            warn_user("This subscription exists already.");
        }
        else {

            pMPPC->Subscribe(XmTextGetString(w_subscription_type),
                             XmTextGetString(w_message_type),
                             XmTextGetString(w_namespace_type));

            char buf[1024];
            sprintf(buf, "Subscription: %s %s %s\n", XmTextGetString(w_subscription_type), XmTextGetString(w_message_type), XmTextGetString(w_namespace_type));
            XmTextInsert( text_w, 0,buf);	
        }
    }
}

static void set_unsubscribe(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() != true && pMPPC->GetListeningToServerSocket()!=true ) {
        //alert the user
        warn_user("MissionLab is not connected to the CIM. Please connect first.");
    }
    else {
        string one = XmTextGetString(w_subscription_type);
        string two = XmTextGetString(w_message_type);
        string three = XmTextGetString(w_namespace_type);
        string strSub = one +";"+ two +";" + three;

        if( pMPPC->SubscriptionExist( strSub ) != true) {   
            warn_user("Unsubscribe could not find this subscription.");
        }
        else {

            pMPPC->Unsubscribe(XmTextGetString(w_subscription_type),
                               XmTextGetString(w_message_type),
                               XmTextGetString(w_namespace_type));


            char buf[1024];
            sprintf(buf, "Subscription: %s %s %s\n", XmTextGetString(w_subscription_type), XmTextGetString(w_message_type), XmTextGetString(w_namespace_type));
	 
            string strSubscriptionText = XmTextGetString( text_w );
            string strUnSubscribeText( buf );
            string::size_type iFrom = strSubscriptionText.find( strUnSubscribeText,0 );
            string::size_type iTo = iFrom +  strUnSubscribeText.size();
            XmTextReplace( text_w, iFrom, iTo,"");	
        }
    }
}

//-----------------------------------------------------------------------
static void create_run_popup(Widget parent)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();
    XmString label;
    Widget super_frm, frm;
    Arg args[MAXARGS];
    Cardinal argcount = 0;
    char buf[1024];

	//if(pMPPC == NULL )
    //    pMPPC = GT_MPPC_client::Instance();

	//Get default values from MPPC
	bool bSubscribedToCIM;
	if(pMPPC->GetNumberOfSubscriptions() == 0 )
        bSubscribedToCIM = false;
	else
        bSubscribedToCIM = true;

    sprintf(buf,"CIM Configuration Management");
    XmString title = XSTRING(buf);
	XmString ok  = XSTRING("Done");	

    static XtCallbackRec ok_cb_list[] =
        {
            {(XtCallbackProc) done_run, NULL},
            {(XtCallbackProc) NULL, NULL}
        };
	
    // Create the dialog
    argcount = 0;
    XtSetArg(args[argcount], XmNdialogTitle, title);
    argcount++;
    XtSetArg(args[argcount], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
    argcount++;
    XtSetArg(args[argcount], XmNokLabelString, ok);
    argcount++;
    XtSetArg(args[argcount], XmNokCallback, ok_cb_list);
    argcount++;
    cim_run_window = XmCreateMessageDialog(parent, "log_window", args, argcount);
  
	XtUnmanageChild(XmMessageBoxGetChild(cim_run_window, XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(cim_run_window, XmDIALOG_HELP_BUTTON));
    XmStringFree(title);
    XmStringFree(ok);
 
    super_frm = XtVaCreateManagedWidget("frm", xmFormWidgetClass, cim_run_window,  
										XmNwidth,300, 
										NULL);

	
    // This widget will create an option for the user whether to ask or
    // not ask overlay upon the execution of the mlab.


	frm = XtVaCreateManagedWidget(
        "cim-frm",
        xmFormWidgetClass, super_frm, 
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNalignment, XmALIGNMENT_BEGINNING,
		XmNrightAttachment,	XmATTACH_FORM,         
		NULL);  
	
    // This widget is for the user to type in the default overlay.
    // At first, it uses the overlay specified in .cfgeditrc.
    sprintf(buf, "CIM Host name");
    label = XmStringCreateLocalized(buf);
    Widget w = XtVaCreateManagedWidget(
        "CIM-host-name-label", 
        xmLabelGadgetClass, frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_FORM,
        NULL);
    XmStringFree(label);

    w_cim_host_name = XtVaCreateManagedWidget(
        "cim-host-name", 
        xmTextWidgetClass, frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	
	XmTextSetString(w_cim_host_name,(char*)pMPPC->GetCIMHostname());

    // This widget is for the user to type in the value for
    // the mission expiration time.
    sprintf(buf, "CIM Port number");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "cim-port-number-label", 
        xmLabelGadgetClass, frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_cim_host_name,
        NULL);
    XmStringFree(label);

    w_cim_port_number = XtVaCreateManagedWidget(
        "cim-port-number", 
        xmTextWidgetClass, frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	char buffer[256];
	sprintf (buffer, "%i", pMPPC->GetCIMPortNumber());
    XmTextSetString(w_cim_port_number, (char*)buffer);

	// This widget is for the user to type in the default overlay.
    // At first, it uses the overlay specified in .cfgeditrc.
    sprintf(buf, "MPPC Host name");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "mppc-host-label", 
        xmLabelGadgetClass, frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_cim_port_number,
        NULL);
    XmStringFree(label);

    w_mppc_host_name = XtVaCreateManagedWidget(
        "mppc-host-name", 
        xmTextWidgetClass, frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

    XmTextSetString(w_mppc_host_name,(char*)pMPPC->GetMPPCHostname());

    // This widget is for the user to type in the value for
    // the mission expiration time.
    sprintf(buf, "MPPC Port number");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "mppc-port-label", 
        xmLabelGadgetClass, frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_mppc_host_name,
        NULL);
    XmStringFree(label);

    w_mppc_port_number = XtVaCreateManagedWidget(
        "mppc-port-number", 
        xmTextWidgetClass, frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	char buffer2[256];
	sprintf (buffer2, "%i", pMPPC->GetMPPCPortNumber());
	XmTextSetString(w_mppc_port_number, (char*)buffer2);

	label = XmStringCreateLocalized( "Connect" );
	w_set_connection_button = XtVaCreateManagedWidget(
        "set-connection", 
        xmPushButtonWidgetClass, frm, 
		XmNlabelString, label, 
		XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_mppc_port_number,
		XmNleftAttachment, XmATTACH_FORM,
        NULL);
	XmStringFree(label);
	XtAddCallback(w_set_connection_button, XmNarmCallback, (XtCallbackProc)set_connection,(XtPointer)NULL);


	label = XmStringCreateLocalized( "Disconnect" );
	w_disconnect_button = XtVaCreateManagedWidget(
        "disconnect", 
        xmPushButtonWidgetClass, frm, 
		XmNlabelString, label, 
		XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_mppc_port_number,
		XmNrightAttachment,	XmATTACH_FORM,
        NULL);
	XmStringFree(label);
	
	XtAddCallback(w_disconnect_button, XmNarmCallback, (XtCallbackProc)disconnect,(XtPointer)NULL);


	Widget frm2 = XtVaCreateManagedWidget(
        "cim-frm2",
        xmFormWidgetClass, super_frm, 
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, frm,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNalignment, XmALIGNMENT_BEGINNING,
		XmNrightAttachment,	XmATTACH_FORM,        
		NULL);  

// This widget is for the user to type in the default overlay.
    // At first, it uses the overlay specified in .cfgeditrc.
    sprintf(buf, "Subscription Type");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "subscription-type-label", 
        xmLabelGadgetClass, frm2, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_cim_port_number,
        NULL);
    XmStringFree(label);

    w_subscription_type = XtVaCreateManagedWidget(
        "subscription-type", 
        xmTextWidgetClass, frm2, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

    XmTextSetString(w_subscription_type,(char*)str_subscription.c_str());
	
	sprintf(buf, "Message Type");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "message-type-label", 
        xmLabelGadgetClass, frm2, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_subscription_type,
        NULL);
    XmStringFree(label);

    w_message_type = XtVaCreateManagedWidget(
        "message-type", 
        xmTextWidgetClass, frm2, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	XmTextSetString(w_message_type,(char*)str_message_type.c_str());

	sprintf(buf, "Data Type");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "subscription-namespace-label", 
        xmLabelGadgetClass, frm2, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_message_type,
        NULL);
    XmStringFree(label);

    w_namespace_type = XtVaCreateManagedWidget(
        "subscription-namespace", 
        xmTextWidgetClass, frm2, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	XmTextSetString(w_namespace_type,(char*)str_namespace_type.c_str());
	
    scroll = XtVaCreateManagedWidget (
        "scrolled_w",
        xmScrolledWindowWidgetClass, frm2,
		XmNwidth, 100, 
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, w_namespace_type,
        XmNscrollingPolicy, XmAS_NEEDED,
		XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
		XmNshadowType, XmSHADOW_OUT,
        NULL);

	Widget frm3 = XtVaCreateManagedWidget("frm3", xmFormWidgetClass, scroll, NULL); 
	
	Arg args2[10];
	int n2=0;
	XtSetArg( args2[n2], XmNrows, 5);n2++;
	XtSetArg( args2[n2], XmNwidth, 500);n2++;
	XtSetArg( args2[n2], XmNeditable, False);n2++;
	XtSetArg( args2[n2], XmNeditMode, XmMULTI_LINE_EDIT);n2++;
	XtSetArg( args2[n2], XmNscrollHorizontal, False);n2++;
	XtSetArg( args2[n2], XmNwordWrap, True);n2++;	
	XtSetArg( args2[n2], XmNleftAttachment, XmATTACH_FORM);n2++;	
	XtSetArg( args2[n2], XmNtopAttachment,  XmATTACH_FORM);n2++;	
	XtSetArg( args2[n2], XmNcursorPositionVisible, false);n2++;	
	XtSetArg( args2[n2], XmNhighlightThickness, 0);n2++;
	
	text_w = XmCreateScrolledText (frm3, "text_w", args2, n2);
	XtManageChild (text_w);
	
	string strSubscriptionText="";
	if(pMPPC->GetNumberOfSubscriptions() == 0 ) {
        string strSub;
        if( bSubscribedToCIM == true ) 
            strSub = "subscribed";
        else 
            strSub = "unsubscribed";
        sprintf(buf, "Currently %s to CIM",strSub.c_str());
        XmTextSetString( text_w, buf);
	}
	else {
        string strSubscriptionText;
        for(int i = 0; i < pMPPC->GetNumberOfSubscriptions(); i++)	{ 
            string strCompleteSub = pMPPC->GetSubscription( i );
            char* pch; 
            pch = strtok((char*)strCompleteSub.c_str(),";");
            string strSubscription = pch;
            pch = strtok (NULL, ";");
            string strMessage = pch;
            pch = strtok (NULL, ";");
            string strNamespace = pch;

            sprintf(buf, "%i-Subscription: %s %s %s\n",i,strSubscription.c_str(),strMessage.c_str(),strNamespace.c_str());
            strSubscriptionText += buf;
        }
        XmTextInsert( text_w, 0,(char*)strSubscriptionText.c_str());	
	}
	   
	label = XmStringCreateLocalized( "Subscribe" );
	w_set_subscribe_button = XtVaCreateManagedWidget(
        "subscribe", 
        xmPushButtonWidgetClass, frm2, 
		XmNlabelString, label, 
		XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, scroll,
		XmNleftAttachment, XmATTACH_FORM,
        NULL);
	XmStringFree(label);	

	XtAddCallback(w_set_subscribe_button, XmNarmCallback, (XtCallbackProc)set_subscribe,(XtPointer)NULL);

	label = XmStringCreateLocalized( "Unsubscribe" );
	w_set_unsubscribe_button = XtVaCreateManagedWidget(
        "unsubscribe", 
        xmPushButtonWidgetClass, frm2, 
		XmNlabelString, label, 
		XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, scroll,
		XmNrightAttachment,	XmATTACH_FORM,
        NULL);
	XmStringFree(label);
	
	XtAddCallback(w_set_unsubscribe_button, XmNarmCallback, (XtCallbackProc)set_unsubscribe,(XtPointer)NULL);

    XtManageChild(super_frm);
    XtManageChild(w_cim_host_name);
	XtManageChild(w_mppc_host_name);
    XtManageChild(w_cim_port_number);
    XtManageChild(w_mppc_port_number);
    XtManageChild(cim_run_window);

	XtManageChild(w_set_subscribe_button);		
	XtManageChild(w_set_unsubscribe_button);
	XtManageChild(w_subscription_type);
	XtManageChild(w_message_type);	
	XtManageChild(w_namespace_type);
	XtManageChild(w_set_connection_button);
	XtManageChild(w_disconnect_button);
	XtManageChild(scroll);
	
  
    doneCIMDialog = false;
	gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_RUN);
}

/*-----------------------------------------------------------------------*/
static void
close_run_window()
{
    XtUnmanageChild(cim_run_window);

    if (config->currentPageIsFSA())
    {
        // It is the FSA level.
        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_ASSEMBLAGE);
    }
    else
    {
        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_TOP_LEVEL);
    }
}

/*-----------------------------------------------------------------------*/
void cim_run(void)
{
	gEventLogging->start("CIM-Dialog");

    create_run_popup(main_window);
     
    while(!doneCIMDialog)
    {
	  XtAppProcessEvent(app, XtIMAll);
    }

    close_run_window();

    if(!okCIMDialog)
    {
	  gEventLogging->cancel("CIM-Dialog");
      return;
    }
}

/*-----------------------------------------------------------------------*/
bool cim_connected(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() == true && pMPPC->GetListeningToServerSocket()==true)
    {
        return true;
    }

    return false;
}

/*-----------------------------------------------------------------------*/
string cim_get_environment_message(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    return (pMPPC->GetEnvironmentMessage());
}

///////////////////////////////////////////////////////////////////////
// $Log: cim_management.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.7  2007/02/14 00:40:59  alanwags
// Set defaults to Environment and onetime
//
// Revision 1.6  2006/10/23 22:14:53  endo
// ICARUS Wizard revised to incorporate CIM's latitude/longitude.
//
// Revision 1.5  2006/09/27 17:16:20  alanwags
// Changed default values
//
// Revision 1.4  2006/09/21 14:47:54  endo
// ICARUS Wizard improved.
//
// Revision 1.3  2006/09/20 22:08:28  alanwags
// Add Meyers singleton pattern for actual cim communications
//
// Revision 1.2  2006/09/18 18:35:11  alanwags
// cim_send_message() implemented.
//
// Revision 1.1  2006/08/19 16:50:45  alanwags
// This is the MPPC management dialog for cfgedit
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
///////////////////////////////////////////////////////////////////////
