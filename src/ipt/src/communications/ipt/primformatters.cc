///////////////////////////////////////////////////////////////////////////////
//
//                              primformatters.cc
//
// This file implements the primitive formatter subclasses.  The algorithms 
// are inherited from TCA v8.0 (authors Reid Simmons (reids@cs.cmu.edu), and 
// Rich Goodwinn (rich@cs.cmu.edu)), but the form has been changed to a 
// consistent C++ style to make the code more readable and efficient
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./libc.h"

#include <ipt/primformatters.h>
#include <ipt/form_macros.h>

static inline int iptStrLen(const char *s)
{
    if (s)
        return(strlen(s));
    else
        return 0;
}


int IPStringFormat::encode(const char* datastruct, int dstart,
                           char *buffer, int bstart) const
{ 
    char *pString;
    int current_byte, length;
    
    current_byte = bstart;
    pString = REF(char *, datastruct, dstart);
    if (pString) {
        length = iptStrLen(pString)*sizeof(char)+1;
        TO_BUFFER_AND_ADVANCE(pString, buffer, current_byte, length);
    } else
        TO_BUFFER_AND_ADVANCE("", buffer, current_byte, 1);
    
    return current_byte - bstart;
}

int IPStringFormat::decode(char* datastruct, int dstart,
                           char *buffer, int bstart, int bsize,
                           int, int) const
{ 
    char *pString;
    int current_byte, length;
    
    current_byte = bstart;
    length = iptStrLen(buffer+current_byte) + 1;
    if (bsize)
        pString = buffer+current_byte;
    else {
        pString = new char[(unsigned)(length+1)];
        FROM_BUFFER_AND_ADVANCE(pString, buffer, current_byte, length);
    }
    *((char **)(datastruct+dstart)) = pString;
    
    return length;
}

int IPStringFormat::e_length(const char* datastruct, int dstart,
                             char *, int) const
{ 
    int length;
    
    length = iptStrLen(REF(char *, datastruct, dstart)) * sizeof(char);
    
    if (length)
        return(length+sizeof(int));
    else
        return(sizeof(char)+sizeof(int));
}

int IPStringFormat::r_length(const char*, int, char *, int) const
{ 
    return sizeof(char *);
}

int IPStringFormat::a_length(const char*, int, char *, int) const
{ 
    return sizeof(char *);
}

int IPStringFormat::actData(const IPFormatAction* act,
                            char* dataStruct, int dStart,
                            const IPStructFormat*, int) const
{
    act->string_act(REF(char*, dataStruct, dStart));
    return sizeof(char*);
}

int IPStringFormat::free_data(char* datastruct, int dstart,
                              char * buffer, int, int bsize) const
{ 
    char *pString;
    
    /* Free the pString */
    pString = REF(char *, datastruct, dstart);
    if (pString && !DataInPlace(pString, buffer, bsize))
        delete(pString);
    return 1;
}

int IPCharFormat::encode(const char* datastruct, int dstart,
                               char *buffer, int bstart) const
{
    charToNetBytes(*(char *)(datastruct+dstart), (buffer+bstart));
    return sizeof(char);
}

int IPCharFormat::decode(char* datastruct, int dstart,
                               char *buffer, int bstart, int, int, int) const
{ 
    netBytesToChar((buffer+bstart), (short *)(datastruct+dstart));
    return sizeof(char);
}

int IPCharFormat::e_length(const char*, int, char *, int) const
{ 
    return sizeof(char);
}

int IPCharFormat::a_length(const char*, int, char *, int) const
{ 
    return sizeof(char);
}

int IPCharFormat::r_length(const char*, int, char *, int ) const
{ 
    return sizeof(char);
}

int IPCharFormat::actData(const IPFormatAction* act,
                          char* dataStruct, int dStart,
                          const IPStructFormat*, int) const
{
    act->char_act(*(char*) (dataStruct+dStart));
    return sizeof(char);
}

int IPShortFormat::encode(const char* datastruct, int dstart,
                                char *buffer, int bstart) const
{ 
    shortToNetBytes(*(short *)(datastruct+dstart), (buffer+bstart));
    return sizeof(short);
}

int IPShortFormat::decode(char* datastruct, int dstart,
                                char *buffer, int bstart, int, 
                                int byteOrder, int) const
{ 
    netBytesToShort((buffer+bstart), (short *)(datastruct+dstart));
    return sizeof(short);
}

int IPShortFormat::e_length(const char*, int, char *, int) const
{ 
    return sizeof(short);
}

int IPShortFormat::a_length(const char*, int, char *, int) const
{ 
    return sizeof(short);
}

int IPShortFormat::r_length(const char*, int, char *, int) const
{ 
    return sizeof(short);
}

int IPShortFormat::actData(const IPFormatAction* act,
                           char* dataStruct, int dStart,
                           const IPStructFormat*, int) const
{
    short s;
    bcopy((char*) (dataStruct+dStart), (char*) &s, sizeof(short));
    act->short_act(s);
    return sizeof(short);
}

/*************************************/

int IPLongFormat::encode(const char* datastruct, int dstart,
                               char *buffer, int bstart) const
{ 
    longToNetBytes(*(long *)(datastruct+dstart), (buffer+bstart));
    return sizeof(long);
}

int IPLongFormat::decode(char* datastruct, int dstart,
                               char *buffer, int bstart, int, 
                               int byteOrder, int) const
{ 
    netBytesToLong((buffer+bstart), (long *)(datastruct+dstart));
    return sizeof(long);
}

int IPLongFormat::e_length(const char*, int, char *, int) const
{ 
    return sizeof(long);
}

int IPLongFormat::a_length(const char*, int, char *, int) const
{ 
    return sizeof(long);
}

int IPLongFormat::r_length(const char*, int , char *, int) const
{ 
    return sizeof(long);
}

int IPLongFormat::actData(const IPFormatAction* act,
                          char* dataStruct, int dStart,
                          const IPStructFormat*, int) const
{
    long l;
    bcopy((char*) (dataStruct+dStart), (char*) &l, sizeof(long));
    act->long_act(l);
    return sizeof(long);
}

/*************************************/

int IPIntFormat::encode(const char* datastruct, int dstart,
                              char *buffer, int bstart) const
{ 
    intToNetBytes(*(int *)(datastruct+dstart), (buffer+bstart));
    return sizeof(int);
}

int IPIntFormat::decode(char* datastruct, int dstart,
                              char *buffer, int bstart, int, 
                              int byteOrder, int) const
{ 
    netBytesToInt((buffer+bstart), (int *)(datastruct+dstart));
    return sizeof(int);
}

int IPIntFormat::e_length(const char*, int, char *, int) const
{ 
    return sizeof(int);
}

int IPIntFormat::a_length(const char*, int, char *, int) const
{ 
    return sizeof(int);
}

int IPIntFormat::r_length(const char*, int, char *, int) const
{ 
    return sizeof(int);
}

int IPIntFormat::actData(const IPFormatAction* act,
                         char* dataStruct, int dStart,
                         const IPStructFormat*, int) const
{
    int i;
    bcopy((char*) (dataStruct+dStart), (char*) &i, sizeof(int));
    act->int_act(i);
    return sizeof(int);
}

/*************************************/

int IPFloatFormat::encode(const char* datastruct, int dstart,
                                char *buffer, int bstart) const
{ 
    floatToNetBytes(*(float *)(datastruct+dstart), (buffer+bstart));
    return sizeof(float);
}

int IPFloatFormat::decode(char* datastruct, int dstart,
                                char *buffer, int bstart, int, 
                                int byteOrder, int) const
{ 
    netBytesToFloat((buffer+bstart), (float *)(datastruct+dstart));
    return sizeof(float);
}

int IPFloatFormat::e_length(const char*, int, char *, int) const
{ 
    return sizeof(float);
}

int IPFloatFormat::a_length(const char*, int, char *, int) const
{ 
    return sizeof(float);
}

int IPFloatFormat::r_length(const char*, int, char *, int) const
{ 
    return sizeof(float);
}

int IPFloatFormat::actData(const IPFormatAction* act,
                           char* dataStruct, int dStart,
                           const IPStructFormat*, int) const
{
    float f;
    bcopy((char*) (dataStruct+dStart), (char*) &f, sizeof(float));
    act->float_act(f);
    return sizeof(float);
}

/*************************************/

int IPDoubleFormat::encode(const char* datastruct, int dstart,
                                 char *buffer, int bstart) const
{ 
    doubleToNetBytes(*(double *)(datastruct+dstart), (buffer+bstart));
    return sizeof(double);
}

int IPDoubleFormat::decode(char* datastruct, int dstart,
                                 char *buffer, int bstart, int, 
                                 int byteOrder, int) const
{ 
    netBytesToDouble((buffer+bstart), (double *)(datastruct+dstart));
    return sizeof(double);
}

int IPDoubleFormat::e_length(const char*, int, char *, int) const
{ 
    return sizeof(double);
}

int IPDoubleFormat::a_length(const char*, int, char *, int) const
{ 
    return sizeof(double);
}

int IPDoubleFormat::r_length(const char*, int, char *, int) const
{ 
    return sizeof(double);
}

int IPDoubleFormat::actData(const IPFormatAction* act,
                            char* dataStruct, int dStart,
                            const IPStructFormat*, int) const
{
    double d;
    bcopy((char*) (dataStruct+dStart), (char*) &d, sizeof(double));
    act->double_act(d);
    return sizeof(double);
}

/*************************************/

int IPBooleanFormat::encode(const char* datastruct, int dstart,
                                  char *buffer, int bstart) const
{ 
    intToNetBytes(*(int *)(datastruct+dstart), (buffer+bstart));
    return sizeof(int);
}

int IPBooleanFormat::decode(char* datastruct, int dstart,
                            char *buffer, int bstart, int, 
                            int byteOrder, int) const
{ 
    netBytesToInt((buffer+bstart), (int *)(datastruct+dstart));
    return sizeof(int);
}

int IPBooleanFormat::e_length(const char*, int, char *, int) const
{ 
    return sizeof(int);
}

int IPBooleanFormat::a_length(const char*, int, char *, int) const
{ 
    return sizeof(int);
}

int IPBooleanFormat::r_length(const char*, int, char *, int) const
{ 
    return sizeof(int);
}

int IPBooleanFormat::actData(const IPFormatAction* act,
                             char* dataStruct, int dStart,
                             const IPStructFormat*, int) const
{
    int b;
    bcopy((char*) (dataStruct+dStart), (char*) &b, sizeof(int));
    act->bool_act(b);
    return sizeof(int);
}

