///////////////////////////////////////////////////////////////////////////////
//
//                                 parseformat.h
//
// This header file defines a class that takes a format specifying string and
// parses it into the equivalent IPFormat subclasses
//
// Classes defined for export:
//    IPFormatParser, IPNamedFormatter
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_parseformat_h
#define ip_parseformat_h

#define MAXFORMATTERS 30

#define UNKNOWN_FMT 0
#define INT_FMT     1
#define BOOLEAN_FMT 2
#define FLOAT_FMT   3
#define DOUBLE_FMT  4
#define STR_FMT     7
#define CHAR_FMT    16
#define SHORT_FMT   17
#define LONG_FMT    18

class IPFormat;
class IPTokenReader;
class IPToken;
class IPNamedFormatter;
class IPHashTable;

struct IPNamedFormatter {
  char *definition;
  IPFormat* format;
  int parsed;
};

/* The format parsing class */
class IPFormatParser {
  public:
    IPFormatParser();
    ~IPFormatParser();

    IPFormat* Parse(const char*);
    IPNamedFormatter* Lookup(const char*);
    void addFormatToTable(const char *name, const IPFormat* format);

  private:
    void parserError(IPToken*, char*);
    IPFormat* structFormat();
    IPFormat* fixedArrayFormat();
    IPFormat* varArrayFormat();
    IPFormat* selfPtrFormat();
    IPFormat* ptrFormat();
    IPFormat* lengthFormat(IPToken*);
    IPFormat* namedFormat(IPToken*);
    IPFormat* primitiveFormat(IPToken*);
    IPFormat* parse(int);

    void addFormatStringToTable(char *name, char *formatString);

  private:
    IPTokenReader* _reader;    // the lexical analyzer for format tokens
    const char* _parse_string; // the current string we are working on
    IPHashTable* _format_table;
};

#endif
