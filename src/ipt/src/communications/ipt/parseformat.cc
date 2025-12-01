///////////////////////////////////////////////////////////////////////////////
//
//                                 parseformat.cc
//
// This file implements a class that takes a format specifying string and
// parses it into the equivalent IPFormat subclasses
//
// Classes defined for export:
//    IPFormatParser
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

#include <ipt/list.h>
#include <ipt/hash.h>
#include <ipt/format.h>
#include <ipt/lex.h>
#include <ipt/primformatters.h>
#include <ipt/parseformat.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Used as an iterator to delete all primitive formats when deleting the 
   primitive format hash table */
static int delete_primitive_formats(char*, char* data)
{
    IPNamedFormatter* named_formatter = (IPNamedFormatter*) data;

    if(named_formatter->format) {
        IPResource::unref(named_formatter->format);
        named_formatter->format = NULL;
    }
	
    delete named_formatter;
    named_formatter = NULL;

    return 1;
}

/******************************************************************************
*
*  FUNCTION: void parserError
*
*  DESCRIPTION:  displays the format parsing error and exits program
*
*  INPUTS: TOKEN_PTR badToken, char *expecting
*
*  OUTPUTS: error message
*
*  EXCEPTIONS: none
*
*  DESIGN: prints the bad token and the string it was expecting
*
*  NOTES: none
*
******************************************************************************/
void IPFormatParser::parserError(IPToken* badToken, char* expecting)
{ 
    int i;

    fprintf(stderr, "Format Parsing Error: Expected %s.\n%s\n", 
            expecting, _parse_string);
    for (i=0; i<badToken->loc; i++) 
        fprintf(stderr, " ");
    fprintf(stderr, "\n");

    exit(1);
}

/******************************************************************************
 *
 *  FUNCTION: IPFormat* structFormat()
 *
 *  DESCRIPTION:  creates a list of items with the structure
 *
 *  INPUTS: none
 *
 *  OUTPUTS: completed list of formats
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN: append each token to format list, except for commas
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPFormat* IPFormatParser::structFormat()
{ 
    IPStructFormat* form;
    IPList* formatList;
    int  numFormats, i;
    IPToken* token;

    formatList = IPList::Create();

    formatList->Prepend((char*) parse(TRUE));
    numFormats = 1;

    token = _reader->nextToken();
    while (token->type == IPToken::COMMA) {
        delete token;
        formatList->Prepend((char*) parse(TRUE));
        token = _reader->nextToken();
        numFormats++;
    }

    _reader->ungetToken(token);

    form = new IPStructFormat(this, numFormats);

    /* Index from high to low since "formatList" 
       has formatters in reverse order */

    for(i=numFormats-1;i>=0;i--) 
        form->elements()[i] = ((IPFormat*) formatList->Pop()); 

    IPList::Delete(formatList);

    return form;
}


/******************************************************************************
 *
 *  FUNCTION: IPFormat* fixedArrayFormat
 *
 *  DESCRIPTION:  creates a format list for items in fixed array
 *
 *  INPUTS: none
 *
 *  OUTPUTS: format list
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN: for each token, append to format list...except commas
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPFormat* IPFormatParser::fixedArrayFormat()
{ 
    IPFixedArrayFormat* form;
    IPFormat* nextFormat;
    IPToken* token;
    IPToken* tmp;
    IPList* argList;
    int numberOfIndexes, goAhead, arrayIndex;

    nextFormat = parse(FALSE); /* Formatter */

    token = _reader->nextToken();
    if (token->type != IPToken::COLON)
        parserError(token, "':'");
    else delete token;

    argList = IPList::Create();
    tmp = _reader->nextToken();
    numberOfIndexes = 0;
    goAhead = TRUE;
    do {
        if (tmp->type != IPToken::INT) 
            parserError(tmp, "an integer value");
        else {
            numberOfIndexes++;
            argList->Append((char*) tmp);

            tmp = _reader->nextToken();

            if (tmp->type == IPToken::COMMA) {
                delete tmp;
                tmp = _reader->nextToken();
                goAhead = TRUE;
            } else if (tmp->type == IPToken::RBRACK) {
                delete tmp;
                goAhead = FALSE;
            }
            else 
                parserError(tmp, "a ',' or ']'");
        }
    } while (goAhead);

    form = new IPFixedArrayFormat(this, nextFormat, numberOfIndexes);

    /* this time munch tokens */
    token = (IPToken*) argList->First();

    for (arrayIndex=0; arrayIndex < numberOfIndexes; arrayIndex++) {
        form->dimensions()[arrayIndex] = token->value.num;
        delete token;
        token = (IPToken*) argList->Next();
    }

    IPList::Delete(argList);

    return form;
}


/******************************************************************************
 *
 *  FUNCTION: IPFormat* varArrayFormat()
 *
 *  DESCRIPTION:  makes list of formats for items in variable length array
 *
 *  INPUTS: none
 *
 *  OUTPUTS: format list
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN: for each token in array, put format in format list, except commas
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPFormat* IPFormatParser::varArrayFormat()
{ 
    IPVarArrayFormat* form;
    IPFormat* nextFormat;
    IPToken* token;
    IPToken* tmp;
    IPList* argList;
    int numberOfIndexes, goAhead, arrayIndex;

    nextFormat = parse(FALSE); /* Formatter */

    token = _reader->nextToken();
    if (token->type != IPToken::COLON)
        parserError(token, "':'");
    else delete token;

    argList = IPList::Create();
    tmp = _reader->nextToken();
    numberOfIndexes = 0;
    goAhead = TRUE;
    do {
        if (tmp->type != IPToken::INT) 
            parserError(tmp, "an integer value");
        else {
            numberOfIndexes++;
            argList->Append((char*) tmp);

            tmp = _reader->nextToken();

            if (tmp->type == IPToken::COMMA) {
                delete tmp;
                tmp = _reader->nextToken();
                goAhead = TRUE;
            } else if (tmp->type == IPToken::GT) {
                delete tmp;
                goAhead = FALSE;
            }
            else 
                parserError(tmp, "a ',' or '>'");
        }
    } while (goAhead);

    form = new IPVarArrayFormat(this, nextFormat, numberOfIndexes);

    /* this time munch tokens */
    token = (IPToken*) argList->First();

    for (arrayIndex=0; arrayIndex < numberOfIndexes; arrayIndex++) {
        form->indices()[arrayIndex] = token->value.num;
        delete token;
        token = (IPToken*) argList->Next();
    }

    IPList::Delete(argList);

    return form;
}


/******************************************************************************
 *
 *  FUNCTION: IPFormat* selfPtrFormat()
 *
 *  DESCRIPTION:  returns a format pointer that signifies it is a self pointer
 *
 *  INPUTS: none
 *
 *  OUTPUTS: new format
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN:
 *
 *  NOTES:  none
 *
 *****************************************************************************/
IPFormat* IPFormatParser::selfPtrFormat()
{ 
    return new IPPointerFormat(this, NULL);
}


/******************************************************************************
 *
 *  FUNCTION: IPFormat* prtFormat()
 *
 *  DESCRIPTION:  returns a format pointer that signifies it is a pointer
 *                within a structure
 *
 *  INPUTS: none
 *
 *  OUTPUTS: new format
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN:
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPFormat* IPFormatParser::ptrFormat()
{ 
    return new IPPointerFormat(this, parse(FALSE));
}


/******************************************************************************
 *
 *  FUNCTION: lengthFormat(token)
 *
 *  DESCRIPTION:  returns an integer format for a tokens value
 *
 *  INPUTS: IPToken* token
 *
 *  OUTPUTS: new integer format
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN:
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPFormat* IPFormatParser::lengthFormat(IPToken* token)
{
    return new IPLengthFormat(this, token->value.num);
}


/******************************************************************************
 *
 *  FUNCTION: namedFormat(token)
 *
 *  DESCRIPTION:  return the format for a token
 *
 *  INPUTS: token
 *
 *  OUTPUTS: format
 *
 *  EXCEPTIONS: exits if format is not registered
 *
 *  DESIGN: do a hash lookup on token name, if not there then generate error
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPFormat* IPFormatParser::namedFormat(IPToken* token)
{ 
    IPNamedFormatter* named;
    IPFormatParser tmp_parse;

    named = Lookup(token->value.str);

    if (!named)
    {
        fprintf(
            stderr,
            "Error: IPFormatParser::namedFormat()[%s]: Format %s is not registered\n%s\n",
            __FILE__,
            token->value.str,
            _parse_string);
        exit(1);
    }

	delete token;
    // ENDO - MEM LEAK
    token = NULL;

    if (named->parsed)
    {
        return named->format;
    }

    return tmp_parse.Parse(named->definition);
}


/******************************************************************************
 *
 *  FUNCTION: IPFormat* primitiveFormat(token)
 *
 *  DESCRIPTION:  returns the format of a primitive (i.e. int, char...)
 *
 *  INPUTS: IPToken* token
 *
 *  OUTPUTS: format
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN: makes a call to namedFormat
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPFormat* IPFormatParser::primitiveFormat(IPToken* token)
{ 
    return namedFormat(token);
}



/******************************************************************************
 *
 *  FUNCTION: parseFormatString(formString)
 *
 *  DESCRIPTION:  parse an input string and returns the format of string
 *
 *  INPUTS: char *formString
 *
 *  OUTPUTS: returns formatterp
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN: lex the tokens and parse each one on the fly
 *
 *  NOTES: none
 *
 *****************************************************************************/
IPFormat* IPFormatParser::Parse(const char* formString)
{
    if (!formString)
        return NULL;

    IPFormat* formatter;

    _parse_string = formString;
    _reader->Init(formString);

    formatter = parse(FALSE);
    return formatter;
}


/******************************************************************************
 *
 *  FUNCTION: IPFormat* parse
 *
 *  DESCRIPTION:  Parse the current token 
 *
 *  INPUTS: int withinStructFlag (see below)
 *
 *  OUTPUTS: return the format
 *
 *  EXCEPTIONS: none
 *
 *  DESIGN: switch(token->type) and get new formats for string recursively
 *
 *  NOTES:  "withinStructFlag" is TRUE if the enclosing format is "{...}" 
 *
 *****************************************************************************/
IPFormat* IPFormatParser::parse(int withinStructFlag) 
{ 
    IPToken* token;
    IPToken* tmp;
    IPFormat* returnForm = NULL;

    token = _reader->nextToken();

    switch(token->type) {
      case IPToken::LBRACE:
        delete token;
        returnForm = structFormat();
        token = _reader->nextToken();
        if (token->type != IPToken::RBRACE) parserError(token, "'}'");
        else delete token;
        break;
      case IPToken::LBRACK:
        delete token;
        returnForm = fixedArrayFormat();
        break;
      case IPToken::LT:
        if (!withinStructFlag)
            parserError(token,
                        "var array format '<..>' embedded within a structure '{..}'");
        else
            returnForm = varArrayFormat();
        delete token;
        break;
      case IPToken::STAR:
        delete token;
        tmp = _reader->nextToken();
        if (tmp->type == IPToken::BANG) {
            delete tmp;
            if (!withinStructFlag) 
                parserError(token, 
                            "self pointer '*!' embedded within a structure '{..}'");
            else {
                returnForm = selfPtrFormat();
            }
        }
        else {
            _reader->ungetToken(tmp);
            returnForm = ptrFormat();
        }
        break;
      case IPToken::INT:
        returnForm = lengthFormat(token);
        break;
      case IPToken::STR:
        returnForm = namedFormat(token);
        break;
      case IPToken::PRIMITIVE:
        returnForm = primitiveFormat(token);
        break;
      case IPToken::EOSTRING:
        if (withinStructFlag)
            parserError(token, 
                        "additional tokens; premature end of string encountered");
        else
            returnForm = NULL;
        delete token;
        break;
      default:
        parserError(token, "a different token type");
        delete token;
    }

    IPResource::ref(returnForm);
    return returnForm;
}

static int formatterHashFN(const char *key)
{ 
    int i, sum;
  
    for(i=0, sum=0; key[i] != '\0'; i++) 
        sum += key[i];
    return sum;
}

static int formatterEqFN(const char *key1, const char *key2)
{ 
  return(!strcmp(key1, key2));
}

/* Create a format parser */
IPFormatParser::IPFormatParser()
{
    _format_table = new IPHashTable(50, formatterHashFN, formatterEqFN);

    addFormatToTable("string", new IPStringFormat(this));
    addFormatToTable("char", new IPCharFormat(this));
    addFormatToTable("short", new IPShortFormat(this));
    addFormatToTable("long", new IPLongFormat(this));
    addFormatToTable("int", new IPIntFormat(this));
    addFormatToTable("float", new IPFloatFormat(this));
    addFormatToTable("boolean", new IPBooleanFormat(this));
    addFormatToTable("double", new IPDoubleFormat(this));
    
    _reader = new IPTokenReader();
}

/* KeyLength is calculated assuming "name" is a null-terminated string */
void IPFormatParser::addFormatToTable(const char *name, const IPFormat* format)
{ 
    IPNamedFormatter* namedFormatter;
  
    namedFormatter = new IPNamedFormatter;
    namedFormatter->definition = (char *)name;
    namedFormatter->format = (IPFormat*)format;
    namedFormatter->parsed = 1;

    IPResource::ref(format);

    _format_table->Insert(name, strlen(name)+1, (char*) namedFormatter);
}

/* Only parse named formatter when it is needed to be used 
   (see parseFmtters.c) */ 
void IPFormatParser::addFormatStringToTable(char *name, char *formatString)
{ 
    IPNamedFormatter* namedFormatter;
  
    namedFormatter = new IPNamedFormatter;
    namedFormatter->definition = formatString;
    namedFormatter->format = NULL;
    namedFormatter->parsed = 0;

    _format_table->Insert(name, 1+strlen(name), (char *)namedFormatter);
}

/* Static member function to lookup a format by name */
IPNamedFormatter* IPFormatParser::Lookup(const char* name)
{
    return (IPNamedFormatter*) _format_table->Find(name);
}

/* delete a format parser */
IPFormatParser::~IPFormatParser()
{
    delete _reader;

    _format_table->Iterate(delete_primitive_formats);
    delete _format_table;
}
