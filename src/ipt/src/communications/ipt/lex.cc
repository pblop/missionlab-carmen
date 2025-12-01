///////////////////////////////////////////////////////////////////////////////
//
//                                 lex.cc
//
// This file implements classes that convert format string tokens into 
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <ipt/lex.h>

/*  ungetToken() - backs up one position in the string */
void IPTokenReader::ungetToken(IPToken* token)
{
    _unget_token = token;
}


/******************************************************************************
*
*  FUNCTION:  IPToken* readString(s)
*
*  DESCRIPTION:  reads a string into the current token's value
*
*  INPUTS: char string
*
*  OUTPUTS: returns the token w/ the string in token->value.str
*
*  EXCEPTIONS: none
*
*  DESIGN: read string of characters not include delimiters i.e. {}[]<>\0
*
*  NOTES: none
*
******************************************************************************/
IPToken* IPTokenReader::readString(const char* s)
{
    const char *t2;
    IPToken* tmp;
    int i, amount;

    for (i=_current_location;
         (s[i] != ',' && s[i] != '}' && s[i] != '{' && s[i] != '[' 
          && s[i] != ']' && s[i] != ':' && s[i] != ' ' && s[i] != '>'
          && s[i] != '<' && s[i] != '\0');
         i++);

    amount = i - _current_location;
    tmp = new IPToken(IPToken::STR, _current_location);
    tmp->value.str = new char[amount+1];
    t2 = s + _current_location;
    strncpy(tmp->value.str, t2, amount);
    tmp->value.str[amount] = '\0';

    _current_location += amount;

    return tmp;
}


/******************************************************************************
 *
 *  FUNCTION:  IPToken* readInt(s)
 *
 *  DESCRIPTION:  reads the integers from a string s
 *
 *  INPUTS: char string s
 *
 *  OUTPUTS: returns the token w/ the integer in token->value.num
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN: read string of characters not include delimiters i.s. {}[]<>\0
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPToken* IPTokenReader::readInt(const char *s)
{
    char *t;
    const char *t2;
    IPToken* tmp;
    int i, amount;

    for (i = _current_location;
         (isdigit(s[i]) && s[i] != ',' && s[i] != '}' && s[i] != '{' 
          && s[i] != '[' && s[i] != ']' && s[i] != ':' && s[i] != ' ' 
          && s[i] != '>' && s[i] != '<' && s[i] != '\0');
         i++);

    amount = i - _current_location;
    tmp = new IPToken( IPToken::INT, _current_location);
    t = new char[amount+1];
    t2 = s + _current_location;
    strncpy(t, t2, amount);
    t[amount] = '\0';

#ifndef VXWORKS
    tmp->value.num = atoi(t);
#else
    int num;
    sscanf(t, "%d", &num);
    tmp->value.num = num;
#endif

    // ENDO - MEM LEAK
    //delete t;
    delete [] t;
    t = NULL; // Added by ENDO

    _current_location += amount;

    return tmp;
}    


/******************************************************************************
 *
 *  FUNCTION: void freeToken (tok)
 *
 *  DESCRIPTION: Takes a token and frees the memory allocated for it.
 *
 *  INPUTS: token of type IPToken*
 *
 *  OUTPUTS: none
 * 
 *  EXCEPTIONS: does nothing if input in NULL
 *
 *  DESIGN: Basically free(token) unless it has a string for value tok->value.str
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPToken::~IPToken()
{
    if (this) {
        if (type == STR)
        {
            // ENDO - MEM LEAK
            //delete value.str;
            delete [] value.str;
        }
    }
}


/******************************************************************************
 *
 *  FUNCTION: IPToken* nextToken()
 *
 *  DESCRIPTION: reads a character and returns the tokenized result
 *
 *  INPUTS: none
 *
 *  OUTPUTS: IPToken* token w/ the token->type updated
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN: checks each character against token table.  Then creates token
 *          by using macro.
 *
 *  NOTES: Assumes that currentString is defined.
 *
 *****************************************************************************/
IPToken* IPTokenReader::nextToken()
{
    IPToken* token;

    if (_unget_token) {
        token = _unget_token;
        _unget_token = NULL;
        return token;
    }

    if (!_current_string) {
        return new IPToken(IPToken::EOSTRING, _current_location);
    }

  Start:
    switch(_current_string[_current_location]) {
      case ' ':
      case '\t':
      case '\f':
        _current_location++;
        goto Start;
      case '{':
        token = new IPToken( IPToken::LBRACE, _current_location);
        _current_location++;
        break;
      case '}':
        token = new IPToken( IPToken::RBRACE, _current_location);
        _current_location++;
        break;
      case '[':
        token = new IPToken( IPToken::LBRACK, _current_location);
        _current_location++;
        break;
      case ']':
        token = new IPToken( IPToken::RBRACK, _current_location);
        _current_location++;
        break;
      case '*':
        token = new IPToken( IPToken::STAR, _current_location);
        _current_location++;
        break;
      case '!':
        token = new IPToken( IPToken::BANG, _current_location);
        _current_location++;
        break;
      case ',':
        token = new IPToken( IPToken::COMMA, _current_location);
        _current_location++;
        break;
      case '<':
        token = new IPToken( IPToken::LT, _current_location);
        _current_location++;
        break;
      case '>':
        token = new IPToken( IPToken::GT, _current_location);
        _current_location++;
        break;
      case ':':
        token = new IPToken( IPToken::COLON, _current_location);
        _current_location++;
        break;
      case '\0':
        token = new IPToken( IPToken::EOSTRING, _current_location);
        _current_location++;
        break;
      default:
        if (isdigit(_current_string[_current_location]))
            token = readInt(_current_string);
        else 
            token = readString(_current_string);
    }

    return token;
}


/******************************************************************************
 *
 *  FUNCTION: void printToken(token)
 *
 *  DESCRIPTION: takes a token and displays its type to stdout
 *
 *  INPUTS: IPToken* token
 *
 *  OUTPUTS: displays token->type to stdout
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN:  checks token->type against token type and prints the type
 *
 *  NOTES: none
 *
 *****************************************************************************/
void IPToken::Print()
{
    fprintf(stderr, "loc: %d: type: ", loc);

    switch(type) {
      case LBRACE:
        fprintf(stderr, "LBRACE\n");
        break;
      case RBRACE:
        fprintf(stderr, "RBRACE\n");
        break;
      case LBRACK:
        fprintf(stderr, "LBRACK\n");
        break;
      case RBRACK:
        fprintf(stderr, "RBRACK\n");
        break;
      case STAR:
        fprintf(stderr, "STAR\n");
        break;
      case BANG:
        fprintf(stderr, "BANG\n");
        break;
      case COMMA:
        fprintf(stderr, "COMMA\n");
        break;
      case LT:
        fprintf(stderr, "LT\n");
        break;
      case GT:
        fprintf(stderr, "GT\n");
        break;
      case COLON:
        fprintf(stderr, "COLON\n");
        break;
      case EOSTRING:
        fprintf(stderr, "EOSTRING\n");
        break;
      case INT:
        fprintf(stderr, "INT: %d\n", value.num);
        break;
      case STR:
        fprintf(stderr, "STR: %s\n", value.str);
        break;
      case PRIMITIVE:
        fprintf(stderr, "PRIMITIVE: %s\n", value.str);
        break;
      default:
        fprintf(stderr, "Token of Unknown type: %d\n", type);
        break;
    }
}


/******************************************************************************
 *
 *  FUNCTION: void initLex(s)
 *
 *  DESCRIPTION: Initializes the lexer the read first element in char string
 *
 *  INPUTS: char string s
 *
 *  OUTPUTS: none
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN: Sets the currentLocation to 0. That is the index for the string.
 *          Sets the currentString to s.
 *
 *  NOTES: This should be called before you lex a string
 *
 *****************************************************************************/
void IPTokenReader::Init(const char* s)
{
    _current_string = s;
    _current_location = 0;
    _unget_token = NULL;
}

/* create a token reader */
IPTokenReader::IPTokenReader()
{
    Init(NULL);
}
