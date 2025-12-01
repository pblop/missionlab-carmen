/**********************************************************************
 **                                                                  **
 **                          sgig_management.cc                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: sgig_management.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

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

Widget sgig_run_window;
Widget w_use_sgig;
Widget w_sgig_host_name;
Widget w_sgig_port_number;
Widget w_mppc_port_number_sgig;
Widget w_mppc_host_name_sgig;
Widget text_output_sgig;	
Widget w_set_connection_button_sgig;
Widget w_disconnect_button_sgig;
Widget w_subscription_type_sgig;
Widget w_message_type_sgig;
Widget w_namespace_type_sgig;
Widget scroll_sgig;
Widget text_w_sgig;
Widget w_set_subscribe_button_sgig;
Widget w_set_unsubscribe_button_sgig;



string str_subscription_sgig = "onetime"; 
string str_message_type_sgig = "Environment";
string str_namespace_type_sgig = "xmlstring";

static bool doneSGIGDialog;
static bool okSGIGDialog;
int iUseSGIG;
//GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

//-----------------------------------------------------------------------
static void done_run_sgig(void)
{
    gEventLogging->log("SET-button in SGIG-Dialog pressed.");
    doneSGIGDialog = true;
    okSGIGDialog = true;
}

void sgig_send_message( string strXMLMessage )
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();
    pMPPC->SendString((char *)(strXMLMessage.c_str()));
    return;
}

//-----------------------------------------------------------------------

static void set_connection_sgig(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() == true && pMPPC->GetListeningToServerSocket()==true ) {
        //alert the user
        warn_user("MissionLab is already connected to the SGIG");
    }
    else {
        char* pEnd;
        int iSGIGPort=-1;
        int iMPPCPort=-1;

        iSGIGPort = strtol(XmTextGetString(w_sgig_port_number), &pEnd,0);
        iMPPCPort = strtol(XmTextGetString(w_mppc_port_number_sgig), &pEnd,0);

        if(iSGIGPort<1 || iMPPCPort<1 ) {
            fprintf(stderr,"Error: Invalid port number\n");
            return;
        }
	int ret;

        pMPPC->SetCIMHostname( XmTextGetString(w_sgig_host_name) );
        pMPPC->SetMPPCHostname( XmTextGetString(w_mppc_host_name_sgig) );
        pMPPC->SetCIMPortNumber( iSGIGPort );
        pMPPC->SetMPPCPortNumber( iMPPCPort );
        pMPPC->SetupServerSocket();
        ret = pMPPC->SetupClientSocket();
	if (ret == -1) {
	   warn_user("Cannot connect to MPPC");
	}
        //	pMPPC->ListenForMessages();
    }
}
//-----------------------------------------------------------------------

static void disconnect_sgig(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() != true && pMPPC->GetListeningToServerSocket()!=true ) {
        //alert the user
        warn_user("MissionLab is already disconnected from the SGIG");
    }	
    else { 
        char buf[1024];
        pMPPC->UnsubscribeAll();
        sprintf(buf, "Currently unsubscribed to SGIG"); 
        XmTextSetString( text_w_sgig, buf);
        sleep(1);
        pMPPC->CloseSockets();
    }
}

static void set_subscribe_sgig(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() != true || pMPPC->GetListeningToServerSocket()!=true ) {
        //alert the user
        warn_user("MissionLab is not connected to the SGIG. Please connect first.");
    }
    else {
	
        if(pMPPC->GetNumberOfSubscriptions() == 0 )
            XmTextSetString( text_w_sgig, "");

        string one = XmTextGetString(w_subscription_type_sgig);
        string two = XmTextGetString(w_message_type_sgig);
        string three = XmTextGetString(w_namespace_type_sgig);
        string strSub = one +";"+ two +";" + three;


        if( pMPPC->SubscriptionExist( strSub ) == true) {  
            warn_user("This subscription exists already.");
        }
        else {

            pMPPC->Subscribe(XmTextGetString(w_subscription_type_sgig),
                             XmTextGetString(w_message_type_sgig),
                             XmTextGetString(w_namespace_type_sgig));

            char buf[1024];
            sprintf(buf, "Subscription: %s %s %s\n", XmTextGetString(w_subscription_type_sgig), XmTextGetString(w_message_type_sgig), XmTextGetString(w_namespace_type_sgig));
            XmTextInsert( text_w_sgig, 0,buf);	
        }
    }
}

static void set_unsubscribe_sgig(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() != true && pMPPC->GetListeningToServerSocket()!=true ) {
        //alert the user
        warn_user("MissionLab is not connected to the SGIG. Please connect first.");
    }
    else {
        string one = XmTextGetString(w_subscription_type_sgig);
        string two = XmTextGetString(w_message_type_sgig);
        string three = XmTextGetString(w_namespace_type_sgig);
        string strSub = one +";"+ two +";" + three;

        if( pMPPC->SubscriptionExist( strSub ) != true) {   
            warn_user("Unsubscribe could not find this subscription.");
        }
        else {

            pMPPC->Unsubscribe(XmTextGetString(w_subscription_type_sgig),
                               XmTextGetString(w_message_type_sgig),
                               XmTextGetString(w_namespace_type_sgig));


            char buf[1024];
            sprintf(buf, "Subscription: %s %s %s\n", XmTextGetString(w_subscription_type_sgig), XmTextGetString(w_message_type_sgig), XmTextGetString(w_namespace_type_sgig));
	 
            string strSubscriptionText = XmTextGetString( text_w_sgig );
            string strUnSubscribeText( buf );
            string::size_type iFrom = strSubscriptionText.find( strUnSubscribeText,0 );
            string::size_type iTo = iFrom +  strUnSubscribeText.size();
            XmTextReplace( text_w_sgig, iFrom, iTo,"");	
        }
    }
}

//-----------------------------------------------------------------------
static void create_run_popup_sgig(Widget parent)
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
	bool bSubscribedToSGIG;
	if(pMPPC->GetNumberOfSubscriptions() == 0 )
        bSubscribedToSGIG = false;
	else
        bSubscribedToSGIG = true;

    sprintf(buf,"SGIG Configuration Management");
    XmString title = XSTRING(buf);
	XmString ok  = XSTRING("Done");	

    static XtCallbackRec ok_cb_list[] =
        {
            {(XtCallbackProc) done_run_sgig, NULL},
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
    sgig_run_window = XmCreateMessageDialog(parent, "log_window", args, argcount);
  
	XtUnmanageChild(XmMessageBoxGetChild(sgig_run_window, XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(sgig_run_window, XmDIALOG_HELP_BUTTON));
    XmStringFree(title);
    XmStringFree(ok);
 
    super_frm = XtVaCreateManagedWidget("frm", xmFormWidgetClass, sgig_run_window,  
										XmNwidth,300, 
										NULL);

	
    // This widget will create an option for the user whether to ask or
    // not ask overlay upon the execution of the mlab.


	frm = XtVaCreateManagedWidget(
        "sgig-frm",
        xmFormWidgetClass, super_frm, 
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNalignment, XmALIGNMENT_BEGINNING,
		XmNrightAttachment,	XmATTACH_FORM,         
		NULL);  
	
    // This widget is for the user to type in the default overlay.
    // At first, it uses the overlay specified in .cfgeditrc.
    sprintf(buf, "SGIG Host name");
    label = XmStringCreateLocalized(buf);
    Widget w = XtVaCreateManagedWidget(
        "SGIG-host-name-label", 
        xmLabelGadgetClass, frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_FORM,
        NULL);
    XmStringFree(label);

    w_sgig_host_name = XtVaCreateManagedWidget(
        "sgig-host-name", 
        xmTextWidgetClass, frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	
	XmTextSetString(w_sgig_host_name,(char*)pMPPC->GetCIMHostname());

    // This widget is for the user to type in the value for
    // the mission expiration time.
    sprintf(buf, "SGIG Port number");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "sgig-port-number-label", 
        xmLabelGadgetClass, frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_sgig_host_name,
        NULL);
    XmStringFree(label);

    w_sgig_port_number = XtVaCreateManagedWidget(
        "sgig-port-number", 
        xmTextWidgetClass, frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	char buffer[256];
	sprintf (buffer, "%i", pMPPC->GetCIMPortNumber());
    XmTextSetString(w_sgig_port_number, (char*)buffer);

	// This widget is for the user to type in the default overlay.
    // At first, it uses the overlay specified in .cfgeditrc.
    sprintf(buf, "MPPC Host name");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "mppc-host-label", 
        xmLabelGadgetClass, frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_sgig_port_number,
        NULL);
    XmStringFree(label);

    w_mppc_host_name_sgig = XtVaCreateManagedWidget(
        "mppc-host-name", 
        xmTextWidgetClass, frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

    XmTextSetString(w_mppc_host_name_sgig,(char*)pMPPC->GetMPPCHostname());

    // This widget is for the user to type in the value for
    // the mission expiration time.
    sprintf(buf, "MPPC Port number");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "mppc-port-label", 
        xmLabelGadgetClass, frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_mppc_host_name_sgig,
        NULL);
    XmStringFree(label);

    w_mppc_port_number_sgig = XtVaCreateManagedWidget(
        "mppc-port-number", 
        xmTextWidgetClass, frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	char buffer2[256];
	sprintf (buffer2, "%i", pMPPC->GetMPPCPortNumber());
	XmTextSetString(w_mppc_port_number_sgig, (char*)buffer2);

	label = XmStringCreateLocalized( "Connect" );
	w_set_connection_button_sgig = XtVaCreateManagedWidget(
        "set-connection", 
        xmPushButtonWidgetClass, frm, 
		XmNlabelString, label, 
		XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_mppc_port_number_sgig,
		XmNleftAttachment, XmATTACH_FORM,
        NULL);
	XmStringFree(label);
	XtAddCallback(w_set_connection_button_sgig, XmNarmCallback, (XtCallbackProc)set_connection_sgig,(XtPointer)NULL);


	label = XmStringCreateLocalized( "Disconnect" );
	w_disconnect_button_sgig = XtVaCreateManagedWidget(
        "disconnect", 
        xmPushButtonWidgetClass, frm, 
		XmNlabelString, label, 
		XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_mppc_port_number_sgig,
		XmNrightAttachment,	XmATTACH_FORM,
        NULL);
	XmStringFree(label);
	
	XtAddCallback(w_disconnect_button_sgig, XmNarmCallback, (XtCallbackProc)disconnect_sgig,(XtPointer)NULL);


	Widget frm2 = XtVaCreateManagedWidget(
        "sgig-frm2",
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
        XmNtopWidget, w_sgig_port_number,
        NULL);
    XmStringFree(label);

    w_subscription_type_sgig = XtVaCreateManagedWidget(
        "subscription-type", 
        xmTextWidgetClass, frm2, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

    XmTextSetString(w_subscription_type_sgig,(char*)str_subscription_sgig.c_str());
	
	sprintf(buf, "Message Type");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "message-type-label", 
        xmLabelGadgetClass, frm2, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_subscription_type_sgig,
        NULL);
    XmStringFree(label);

    w_message_type_sgig = XtVaCreateManagedWidget(
        "message-type", 
        xmTextWidgetClass, frm2, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	XmTextSetString(w_message_type_sgig,(char*)str_message_type_sgig.c_str());

	sprintf(buf, "Data Type");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "subscription-namespace-label", 
        xmLabelGadgetClass, frm2, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_message_type_sgig,
        NULL);
    XmStringFree(label);

    w_namespace_type_sgig = XtVaCreateManagedWidget(
        "subscription-namespace", 
        xmTextWidgetClass, frm2, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

	XmTextSetString(w_namespace_type_sgig,(char*)str_namespace_type_sgig.c_str());
	
    scroll_sgig = XtVaCreateManagedWidget (
        "scrolled_w",
        xmScrolledWindowWidgetClass, frm2,
		XmNwidth, 100, 
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, w_namespace_type_sgig,
        XmNscrollingPolicy, XmAS_NEEDED,
		XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
		XmNshadowType, XmSHADOW_OUT,
        NULL);

	Widget frm3 = XtVaCreateManagedWidget("frm3", xmFormWidgetClass, scroll_sgig, NULL); 
	
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
	
	text_w_sgig = XmCreateScrolledText (frm3, "text_w", args2, n2);
	XtManageChild (text_w_sgig);
	
	string strSubscriptionText="";
	if(pMPPC->GetNumberOfSubscriptions() == 0 ) {
        string strSub;
        if( bSubscribedToSGIG == true ) 
            strSub = "subscribed";
        else 
            strSub = "unsubscribed";
        sprintf(buf, "Currently %s to SGIG",strSub.c_str());
        XmTextSetString( text_w_sgig, buf);
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
        XmTextInsert( text_w_sgig, 0,(char*)strSubscriptionText.c_str());	
	}
	   
	label = XmStringCreateLocalized( "Subscribe" );
	w_set_subscribe_button_sgig = XtVaCreateManagedWidget(
        "subscribe", 
        xmPushButtonWidgetClass, frm2, 
		XmNlabelString, label, 
		XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, scroll_sgig,
		XmNleftAttachment, XmATTACH_FORM,
        NULL);
	XmStringFree(label);	

	XtAddCallback(w_set_subscribe_button_sgig, XmNarmCallback, (XtCallbackProc)set_subscribe_sgig,(XtPointer)NULL);

	label = XmStringCreateLocalized( "Unsubscribe" );
	w_set_unsubscribe_button_sgig = XtVaCreateManagedWidget(
        "unsubscribe", 
        xmPushButtonWidgetClass, frm2, 
		XmNlabelString, label, 
		XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, scroll_sgig,
		XmNrightAttachment,	XmATTACH_FORM,
        NULL);
	XmStringFree(label);
	
	XtAddCallback(w_set_unsubscribe_button_sgig, XmNarmCallback, (XtCallbackProc)set_unsubscribe_sgig,(XtPointer)NULL);

    XtManageChild(super_frm);
    XtManageChild(w_sgig_host_name);
	XtManageChild(w_mppc_host_name_sgig);
    XtManageChild(w_sgig_port_number);
    XtManageChild(w_mppc_port_number_sgig);
    XtManageChild(sgig_run_window);

	XtManageChild(w_set_subscribe_button_sgig);		
	XtManageChild(w_set_unsubscribe_button_sgig);
	XtManageChild(w_subscription_type_sgig);
	XtManageChild(w_message_type_sgig);	
	XtManageChild(w_namespace_type_sgig);
	XtManageChild(w_set_connection_button_sgig);
	XtManageChild(w_disconnect_button_sgig);
	XtManageChild(scroll_sgig);
	
  
    doneSGIGDialog = false;
	gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_RUN);
}

/*-----------------------------------------------------------------------*/
static void
close_run_window()
{
    XtUnmanageChild(sgig_run_window);

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
void sgig_run(void)
{
	gEventLogging->start("SGIG-Dialog");

    create_run_popup_sgig(main_window);
     
    while(!doneSGIGDialog)
    {
	  XtAppProcessEvent(app, XtIMAll);
    }

    close_run_window();

    if(!okSGIGDialog)
    {
	  gEventLogging->cancel("SGIG-Dialog");
      return;
    }
}

/*-----------------------------------------------------------------------*/
bool sgig_connected(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    if(pMPPC->GetClientSocketConnected() == true && pMPPC->GetListeningToServerSocket()==true)
    {
        return true;
    }

    return false;
}

/*-----------------------------------------------------------------------*/
string sgig_get_environment_message(void)
{
    GT_MPPC_client* pMPPC = GT_MPPC_client::Instance();

    return (pMPPC->GetEnvironmentMessage());
}

///////////////////////////////////////////////////////////////////////
// $Log: sgig_management.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2007/06/21 02:53:51  pulam
// Added message for failure to connect to MPPC
//
// Revision 1.1  2007/05/14 20:40:49  pulam
// Added interface for SGIG
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
