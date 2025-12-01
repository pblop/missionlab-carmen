///////////////////////////////////////////////////////////////////////////////
//
//                                 primformatters.h
//
// This header file declares the primitive formatter subclasses
//
// Classes defined for export:
//    IPStringFormat, IPCharFormat, IPShortFormat, IPLongFormat, IPIntFormat,
//    IPFloatFormat, IPDoubleFormat, IPBooleanFormat, 
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef primFmttrs_h
#define primFmttrs_h

#include <ipt/format.h>
#include <ipt/formconfig.h>

#define STR_TRANS_SIMPLETYPE 0
#define CHAR_TRANS_SIMPLETYPE 1
#define SHORT_TRANS_SIMPLETYPE 1
#define LONG_TRANS_SIMPLETYPE 1
#define INT_TRANS_SIMPLETYPE 1
#define FLOAT_TRANS_SIMPLETYPE 1
#define BOOLEAN_TRANS_SIMPLETYPE 1
#define DOUBLE_TRANS_SIMPLETYPE 1

class IPStringFormat : public IPPrimFormat {
  public:
    IPStringFormat(IPFormatParser* p)
        : IPPrimFormat(p, STR_TRANS_SIMPLETYPE, StringFMT) {}

    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;
    virtual int encode(const char* datastruct, int dstart,
                             char* buffer, int bstart) const;
    virtual int decode(char* datastruct, int dstart,
                             char* buffer, int bstart, int bsize,
                             int byteOrder, int alignment) const;
    virtual int e_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int a_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int r_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int free_data(char* datastruct, int dstart,
                          char* buffer, int bstart, int bsize) const;
};

class IPCharFormat : public IPPrimFormat {
  public:
    IPCharFormat(IPFormatParser* p)
        : IPPrimFormat(p, CHAR_TRANS_SIMPLETYPE, CharFMT) {}

    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;
    virtual int encode(const char* datastruct, int dstart,
                             char* buffer, int bstart) const;
    virtual int decode(char* datastruct, int dstart,
                             char* buffer, int bstart, int bsize,
                             int byteOrder, int alignment) const;
    virtual int e_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int a_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int r_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
};

class IPShortFormat : public IPPrimFormat {
  public:
    IPShortFormat(IPFormatParser* p)
        : IPPrimFormat(p, SHORT_TRANS_SIMPLETYPE, ShortFMT) {}

    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;
    virtual int encode(const char* datastruct, int dstart,
                             char* buffer, int bstart) const;
    virtual int decode(char* datastruct, int dstart,
                             char* buffer, int bstart, int bsize,
                             int byteOrder, int alignment) const;
    virtual int e_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int a_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int r_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
};

class IPLongFormat : public IPPrimFormat {
  public:
    IPLongFormat(IPFormatParser* p)
        : IPPrimFormat(p, LONG_TRANS_SIMPLETYPE, LongFMT) {}

    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;
    virtual int encode(const char* datastruct, int dstart,
                             char* buffer, int bstart) const;
    virtual int decode(char* datastruct, int dstart,
                             char* buffer, int bstart, int bsize,
                             int byteOrder, int alignment) const;
    virtual int e_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int a_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int r_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
};

class IPIntFormat : public IPPrimFormat {
  public:
    IPIntFormat(IPFormatParser* p)
        : IPPrimFormat(p, INT_TRANS_SIMPLETYPE, IntFMT) {}

    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;
    virtual int encode(const char* datastruct, int dstart,
                             char* buffer, int bstart) const;
    virtual int decode(char* datastruct, int dstart,
                             char* buffer, int bstart, int bsize,
                             int byteOrder, int alignment) const;
    virtual int e_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int a_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int r_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
};

class IPFloatFormat : public IPPrimFormat {
  public:
    IPFloatFormat(IPFormatParser* p)
        : IPPrimFormat(p, FLOAT_TRANS_SIMPLETYPE, FloatFMT) {}

    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;
    virtual int encode(const char* datastruct, int dstart,
                             char* buffer, int bstart) const;
    virtual int decode(char* datastruct, int dstart,
                             char* buffer, int bstart, int bsize,
                             int byteOrder, int alignment) const;
    virtual int e_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int a_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int r_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
};

class IPDoubleFormat : public IPPrimFormat {
  public:
    IPDoubleFormat(IPFormatParser* p)
        : IPPrimFormat(p, DOUBLE_TRANS_SIMPLETYPE, DoubleFMT) {}

    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;
    virtual int encode(const char* datastruct, int dstart,
                             char* buffer, int bstart) const;
    virtual int decode(char* datastruct, int dstart,
                             char* buffer, int bstart, int bsize,
                             int byteOrder, int alignment) const;
    virtual int e_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int a_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int r_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
};

class IPBooleanFormat : public IPPrimFormat {
  public:
    IPBooleanFormat(IPFormatParser* p)
        : IPPrimFormat(p, BOOLEAN_TRANS_SIMPLETYPE, BooleanFMT) {}

    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;
    virtual int encode(const char* datastruct, int dstart,
                       char* buffer, int bstart) const;
    virtual int decode(char* datastruct, int dstart,
                       char* buffer, int bstart, int bsize,
                       int byteOrder, int alignment) const;
    virtual int e_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int a_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
    virtual int r_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const;
};

#endif
