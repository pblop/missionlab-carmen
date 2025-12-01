/****************************************************************************
*
*                                     tcx.h
*
* This header file defined TCX cover functions for IPT
*
*  "1995, Carnegie Mellon University. All Rights Reserved." This
*  software is made available for academic and research purposes only. No
*  commercial license is hereby granted.  Copying and other reproduction is
*  authorized only for research, education, and other non-commercial
*  purposes.  No warranties, either expressed or implied, are made
*  regarding the operation, use, or results of the software.
*
****************************************************************************/

#ifndef ip_tcx_h
#define ip_tcx_h

#include <stdio.h>
#include <ipt/tcx_cover.h>

#if defined(__cplusplus)
extern "C" {
#endif

extern void tcxInitialize P_((char *module, char *server));
extern int tcxInitialized();
extern void tcxDeinitialize ();

extern TCX_MODULE_PTR tcxConnectModule P_((char *module));
extern TCX_MODULE_PTR tcxConnectOptional P_((char *module));

extern char *tcxModuleName P_((TCX_MODULE_PTR module));

extern void tcxSendData P_((TCX_MODULE_PTR module, int id, int ins,
                            void *data, int len, TCX_FMT_PTR fmt));

extern int tcxRecvData P_((TCX_MODULE_PTR *module, int *id, int *ins,
                           void *data, TCX_FMT_PTR fmt, int allowHnd,
                           void *timeout));

extern TCX_FMT_PTR tcxParseFormat P_((char *format));

extern void tcxFree P_((char *msgName, void *data));
extern void tcxFreeReply P_((char *msgName, void *data));
extern void tcxFreeByRef P_((TCX_REF_PTR ref, void *data));
extern void tcxFreeData P_((TCX_FMT_PTR fmt, void *data));

extern void tcxRegisterMessages P_((TCX_REG_MSG_TYPE *msgArray, int size));
extern void tcxRegisterHandlers P_((TCX_REG_HND_TYPE *hndArray, int size));

extern int tcxSendMsg P_((TCX_MODULE_PTR module, char *msgName, void *data));

extern int tcxRecvLoop P_((void *timeout));

extern void tcxQuery P_((TCX_MODULE_PTR module, char *msgName, 
                         void *data, char *replyMsgName, void *reply));

extern void tcxReply P_((TCX_REF_PTR ref, char *replyMsgName, void *reply));

extern char *tcxMessageName P_((int id));
extern int tcxMessageId P_((char *name));

extern TCX_MODULE_PTR tcxRefModule P_((TCX_REF_PTR ref));
extern int tcxRefId P_((TCX_REF_PTR ref));
extern int tcxRefIns P_((TCX_REF_PTR ref));
extern TCX_REF_PTR tcxRefDup P_((TCX_REF_PTR ref, TCX_REF_PTR out));

extern int tcxRecvMsg P_((char *msgName, int *ins, void *data, void *timeout));
extern int tcxRecvMsgNoHnd P_((char *msgName, int *ins, void *data,
                               void *timeout));

extern int tcxRecvMsgE P_((TCX_MODULE_PTR *module, char *msgName, int *ins, 
                           void *data, int allowHnd, void *timeout));

extern int tcxRecv P_((void* timeout));

extern int tcxNRecvMsgE P_((TCX_MODULE_PTR *module, char **msgName,
                            void *data, void *timeout, int *inst,
                            int hndMask));

extern char *tcxCurrentModuleName P_((void));

extern int tcxTestActiveModule P_((TCX_MODULE_PTR module));

extern void tcxCloseAll ();

extern void tcxSetConnectCallback P_((void (*connect_func) P_((TCX_MODULE_PTR,
                                                               void*)),
                                      void* connect_data));
extern void tcxSetDisconnectCallback
    P_((void (*disconnect_func) P_((TCX_MODULE_PTR, void*)),
        void* disconnect_data));

MSG_DEF_PTR messageFindByID P_((int));

#ifdef VXWORKS
extern void tcxInitializeInherited P_((char* task_name));
extern void tcxDeinitializeInherited();
#endif

#if defined(__cplusplus)
}
#endif

#endif
