///////////////////////////////////////////////////////////////////////////////
//
//                                 lex.h
//
// This header file defines classes that convert format string tokens into 
// tokens that can be manipulated by IPFormatParser
//
// Classes defined for export:
//    IPToken, IPTokenReader
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_lex_h
#define ip_lex_h

/* This structure is what token string specifiers get converted into for easier
   analysis */
struct IPToken {
    /* types of tokens */
    enum {
        LBRACE, RBRACE, COMMA, LT, GT,
        STAR, BANG, COLON, LBRACK, RBRACK,
        INT,
        STR,
        PRIMITIVE,
        EOSTRING
    };
    
    IPToken(int t, int l) { type = t; loc = l; }
    ~IPToken();
    void Print();

    int loc;          // index into string where token is found
    int type;         // type of token
    union {           // value of the token
        char *str;    // either a string if is a STR type token
        int num;      // or a number if it is a INT type token
    } value;
};

/* Class which gets initialized with a string by the Init member function, and
   which then nextToken is called on repeatedly to process the tokens as they
   are gotten from the string.  There is a one token undo buffer which can
   be invoked using ungetToken */
class IPTokenReader {
  public:
    IPTokenReader();

    void Init(const char*);
    IPToken* nextToken();
    void ungetToken(IPToken*);

  private:
    IPToken* readString(const char* s);
    IPToken* readInt(const char* s);

  private:
    IPToken* _unget_token;       // the one token undo buffer
    int _current_location;       // index into the parse string
    const char* _current_string; // the parse string
};

#endif
