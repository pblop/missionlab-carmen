///////////////////////////////////////////////////////////////////////////////
//
//                            internal_messages.h
//
// This header file defines the constants needed for administrative messages
// between IPT modules and the IPT server (and between pairs of IPT modules)
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_internal_messages_h
#define ip_internal_messages_h

#define SERVER_NAME "IPT Server"
#define SERVER_PORT 1387
#define LOWEST_MESSAGE_NUMBER 1000
#define MAX_BACKUP 100000

#define IPT_NONE 0
#define IPT_SAME 1
#define IPT_DIFFERENT 2
#define IPT_HOSTNAME 3

/* Global, fixed, message types */
enum {
    IPTVersionInfoMsgNum = 0,
    IPTModuleConnectingMsgNum,
    IPTRequestConnectionMsgNum,
    IPTRegisterMessageMsgNum,
    IPTMessageRegisteredMsgNum,
    IPTRequestDomainNameMsgNum,
    IPTDomainNameMsgNum,
    IPTRegisterMessageSetMsgNum,
    IPTMessageSetRegisteredMsgNum,
    IPTSubscriptionMsgNum,
    IPTSubscribedMsgNum,
    IPTServerRegisterMsgNum,
    IPTRemoteSubscriptionMsgNum,
    IPTLoggingMsgNum,
    IPTUnsubscribeMsgNum,
    IPTDeletionActionMsgNum,
    IPTServerDumpMsgNum,
    IPTServerFlushMsgNum,
    IPTServerDumpNetMsgNum,
    IPTClearRemoteConnectionMsgNum,
    IPTServerFlushNetMsgNum,
    IPTRequestTranslationMsgNum,
    IPTTranslationMsgNum,
    IPTPeerConnectingMsgNum,
    IPTPeerDisconnectingMsgNum,
    IPTRoutingTableMsgNum,
    IPTConfirmRemoteConnectionMsgNum,
    IPTRemoteConnectionConfirmedMsgNum,
    IPTRoutingTableRequestMsgNum,
    IPTRequestedRoutingTableMsgNum,
    IPTConnectionRoutingMsgNum,
    IPTReportRemoteConnectionMsgNum,
    IPTReportConnectionsMsgNum,
    IPTReregisterMessagesMsgNum,
    IPTLastBuiltInMsgNum
};

#define IPT_VERSION_INFO_MSG "IPTVersionInfoMsg"
#define IPT_VERSION_INFO_FORM "{ char, char, string }"

struct IPTVersionInfoStruct {
    unsigned char major_version;
    unsigned char minor_version;
    const char* domain_name;
};

#define IPT_MODULE_CONNECTING_MSG "IPTModuleConnectingMsg"
#define IPT_MODULE_CONNECTING_FORM "{ char, char, string, string }"

struct IPTModuleConnectingStruct {
    unsigned char byte_order;
    unsigned char alignment;
    const char* name;
    const char* host;
};

#define IPT_REQUEST_CONNECTION_MSG "IPTRequestConnectionMsg"
#define IPT_REQUEST_CONNECTION_FORM "{ string, int }"

struct IPTRequestConnectionStruct {
    const char* name;
    int required;
};

#define IPT_REGISTER_MESSAGE_MSG "IPTRegisterMessageMsg"
#define IPT_REGISTER_MESSAGE_FORM "string"

struct IPTRegisterMessageSetStruct {
    int n;
    char** names;
};

#define IPT_REGISTER_MESSAGE_SET_MSG "IPTRegisterMessageSetMsg"
#define IPT_REGISTER_MESSAGE_SET_FORM "{ int, < string : 1 > }"

struct IPTMessageSetRegisteredStruct {
    int n;
    int* ids;
};

#define IPT_MESSAGE_SET_REGISTERED_MSG "IPTMessageSetRegisteredMsg"
#define IPT_MESSAGE_SET_REGISTERED_FORM "{ int, < int : 1> }"

#define IPT_MESSAGE_REGISTERED_MSG "IPTMessageRegisteredMsg"
#define IPT_MESSAGE_REGISTERED_FORM "int"

#define IPT_REQUEST_DOMAIN_NAME_MSG "IPTRequestDomainNameMsg"
#define IPT_REQUEST_DOMAIN_NAME_FORM 0

#define IPT_DOMAIN_NAME_MSG "IPTDomainNameMsg"
#define IPT_DOMAIN_NAME_FORM "string"

struct IPTSubscriptionStruct {
    char* msg_name;
    int size_info;
    unsigned char* info;
};

#define IPT_SUBSCRIPTION_MSG "IPTSubscriptionMsg"
#define IPT_SUBSCRIPTION_FORM "{ string, int, <char: 2> }"

#define IPT_SUBSCRIBED_MSG "IPTSubscribedMsg"
#define IPT_SUBSCRIBED_FORM "int"

#define IPT_SERVER_REGISTER_MSG "IPTServerRegisterMsg"
#define IPT_SERVER_REGISTER_FORM NULL

struct IPTRemoteSubscriptionStruct {
    const char* publisher;
    const char* msg_name;
    int size_info;
    unsigned char* info;
};

#define IPT_REMOTE_SUBSCRIPTION_MSG "IPTRemoteSubscriptionMsg"
#define IPT_REMOTE_SUBSCRIPTION_FORM "{ string, string, int, <char : 3> }"

struct IPTLoggingStruct {
    int operation;
    const char* msg_name;
    int msg_id;
    int instance;
    const char* origin;
    const char* destination;
    int size;
    const char* extra;
};

#define IPT_LOGGING_MSG "IPTLoggingMsg"
#define IPT_LOGGING_FORM "{ int, string, int, int, string, string, int, string }"

#define IPT_UNSUBSCRIBE_MSG "IPTUnsubscribeMsg"
#define IPT_UNSUBSCRIBE_FORM "{ string, string }"

struct IPTUnsubscribeStruct {
    const char* subscriber;
    const char* msg_name;
};

#define IPT_DELETION_ACTION_MSG "IPTDeletionActionMsg"
#define IPT_DELETION_ACTION_FORM "{ string, string }"

struct IPTDeletionActionStruct {
    const char* module;
    const char* action;
};

#define IPT_SERVER_DUMP_MSG "IPTServerDumpMsg"
#define IPT_SERVER_DUMP_FORM "string"
#define IPT_SERVER_FLUSH_MSG "IPTServerFlushMsg"
#define IPT_SERVER_FLUSH_FORM 0

#define IPT_SERVER_DUMP_NET_MSG "IPTServerDumpNetMsg"
#define IPT_SERVER_DUMP_NET_FORM "string"

#define IPT_CLEAR_REMOTE_CONNECTION_MSG "IPTClearRemoteConnectionMsg"
#define IPT_CLEAR_REMOTE_CONNECTION_FORM "string"

#define IPT_SERVER_FLUSH_NET_MSG "IPTServerFlushNetMsg"
#define IPT_SERVER_FLUSH_NET_FORM NULL

struct IPTRequestTranslationStruct {
    char* domain_name;
    char* conn;
    int id;
};

#define IPT_REQUEST_TRANSLATION_MSG "IPTRequestTranslationMsg"
#define IPT_REQUEST_TRANSLATION_FORM "{ string, string, int }"

struct IPTTranslationStruct {
    char* conn;
    int id, translated_id;
};

#define IPT_TRANSLATION_MSG "IPTTranslationMsg"
#define IPT_TRANSLATION_FORM "{ string, int, int }"

#define IPT_PEER_CONNECTING_MSG "IPTPeerConnectingMsg"
#define IPT_PEER_CONNECTING_FORM "string"

#define IPT_PEER_DISCONNECTING_MSG "IPTPeerDisconnectingMsg"
#define IPT_PEER_DISCONNECTING_FORM "string"

struct IPTRoutingStruct {
    char* type;
    char* sub_type;
    int comparison;
    char* parameters;
};

struct IPTRoutingTableStruct {
    int num_routes;
    IPTRoutingStruct* routes;
};

#define IPT_ROUTING_TABLE_MSG "IPTRoutingTableMsg"
#define IPT_ROUTING_TABLE_FORM "{int, < { string, string, int, string } : 1 >}"
    
struct IPTConfirmRemoteConnectionStruct {
    char* initiator;
    char* target;
};

#define IPT_CONFIRM_REMOTE_CONNECTION_MSG "IPTConfirmRemoteConnectionMsg"
#define IPT_CONFIRM_REMOTE_CONNECTION_FORM "{ string, string}"

#define IPT_REMOTE_CONNECTION_CONFIRMED_MSG "IPTRemoteConnectionConfirmedMsg"
#define IPT_REMOTE_CONNECTION_CONFIRMED_FORM "int"

struct IPTRoutingTableRequestStruct {
    char* name;
    int required;
};

#define IPT_ROUTING_TABLE_REQUEST_MSG "IPTRoutingTableRequestMsg"
#define IPT_ROUTING_TABLE_REQUEST_FORM "{string, int }"

struct IPTRequestedRoutingTableStruct {
    int num_routes;
    IPTRoutingStruct* routes;
};

#define IPT_REQUESTED_ROUTING_TABLE_MSG "IPTRequestedRoutingTableMsg"
#define IPT_REQUESTED_ROUTING_TABLE_FORM "{int, < { string, string, int, string } : 1 >}"

struct IPTConnectionRoutingStruct {
    char* name;
    char* type;
    char* sub_type;
    char* parameters;
};

#define IPT_CONNECTION_ROUTING_MSG "IPTConnectionRoutingMsg"
#define IPT_CONNECTION_ROUTING_FORM "{ string, string, string, string }"

struct IPTReportRemoteConnectionStruct {
    char* initiator;
    char* target;
};

#define IPT_REPORT_REMOTE_CONNECTION_MSG "IPTReportRemoteConnectionMsg"
#define IPT_REPORT_REMOTE_CONNECTION_FORM "{ string, string}"

struct IPTReportConnectionsStruct {
    int num_conns;
    const char** conn_names;
};

#define IPT_REPORT_CONNECTIONS_MSG "IPTReportConnectionsMsg"
#define IPT_REPORT_CONNECTIONS_FORM "{ int, < string : 1 > }"

struct IPTMessageIDStruct {
    const char* msg_name;
    int msg_id;
};

struct IPTReregisterMessagesStruct {
    int num_messages;
    IPTMessageIDStruct *messages;
};

#define IPT_REREGISTER_MESSAGES_MSG "IPTReregisterMessagesMsg"
#define IPT_REREGISTER_MESSAGES_FORM "{ int, < { string, int } : 1 > }"

#endif
