/*
 * Douglas C. MacKenzie
 *
 * scan.c ----  C routines used by cnl_lex.l for CNL  ----
 *
 *    Copyright 1995, Georgia Tech Research Corporation
 *    Atlanta, Georgia  30332-0415
 *    ALL RIGHTS RESERVED, See file COPYRIGHT for details.
 *
 *    $Id: scan.c,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
 */

#include <stdlib.h>
#include "defs.h"
#include "cnl_gram.tab.h"

/* externs from lex */
extern int      lineno;

struct Symbol *Type_Chain = NULL;
struct Symbol *Module_Chain = NULL;
int num_modules = 0;

/************************************************************************
*                                                                       *
*                         int Hash(char *)                              *
*                                                                       *
************************************************************************/

/*
 * This routine will return an integer index into the SymbolTable. The
 * location that is returned must be then checked.  If it is EMPTY then the
 * symbol is not in the table an should be inserted in this position.  If
 * this is the symbol then it was found at this loc.
 */

static int
Hash(const char *Str,int parent)
{
   int             Val;
   int             Loc;
   int             Start;

   Val = 0;
   Loc = 0;
   while (Str[Loc] != 0)
   {
      Val += Str[Loc++];
   }

   Loc = (Val * 3) % SymbolTableSize;
   Start = Loc;

   while ((SymbolTable[Loc].SymbolType != EMPTY) &&
	  (strcmp(&Symbols[SymbolTable[Loc].SymbolLocation], Str) != 0 ||
	   parent != SymbolTable[Loc].Parent))
   {
      Loc = (Loc + 1) % SymbolTableSize;
      if (Loc == Start)
      {
	 /* Have gone through entire table and all locations are full! */
	 fprintf(stderr, "ERROR LINE %d : Symbol table overflow FATAL ERROR!\n", lineno);
	 exit(2);
      }
   }

   return (Loc);
}



/************************************************************************
*                                                                       *
*                      int AddString(char *,int TokenType)              *
*                                                                       *
************************************************************************/

/*
 * AddString will add a string to the symbol table if it isn't already there.
 * If it is found then a pointer to the existing entry is returned. If it is
 * added then a pointer to the new entry is returned. In either case an
 * index into the symbol table is returned.
 *
 * Returns an index into the symbol table to the location where the symbol is
 * located.
 */

int
AddString(const char *Str, int TokenType, int parent)
{
   int             Loc;
   int             StrLoc;
   int             RtnVal = 0;

   Loc = Hash(Str,parent);

   if (SymbolTable[Loc].SymbolType == EMPTY)
   {
      /* Not in table so add string */
      SymbolTable[Loc].SymbolLocation = NextSymbolLoc;
      SymbolTable[Loc].Parent = parent;
      StrLoc = 0;

      do
      {
	 Symbols[NextSymbolLoc] = Str[StrLoc];
	 NextSymbolLoc++;
      }
      while (Str[StrLoc++] && NextSymbolLoc < MaxSymbols);

      if (NextSymbolLoc >= MaxSymbols)
      {
	 fprintf(stderr, "ERROR LINE %d : Symbol array overflow FATAL ERROR!\n", lineno);
	 exit(1);
      }
      else
      {
	 SymbolTable[Loc].SymbolType = TokenType;
	 RtnVal = Loc;
      }

   }				/* end if EMPTY */
   else if (strcmp(&Symbols[SymbolTable[Loc].SymbolLocation], Str) == 0 &&
	    SymbolTable[Loc].Parent == parent)
   {
      /* already in table so return */
      RtnVal = Loc;
   }
   else
   {
      fprintf(stderr, "ERROR LINE %d : Hashing function failed on '%s' FATAL ERROR!\n", lineno, Str);
      exit(3);
   }

   return (RtnVal);
}


/************************************************************************
*                                                                       *
*                      int AddNumber(char *)                            *
*                                                                       *
************************************************************************/

/*
 * AddNumber will add the number to the symbol table if it isn't already
 * there.  If it is found then a pointer to the existing entry is returned.
 * If it is added then a pointer to the new entry is returned. In either
 * case an index into the symbol table is returned.
 *
 * Returns an index into the symbol table to the location where the symbol is
 * located.
 */

int
AddNumber(char *Str,int parent)
{
   int             RtnVal;
   int             Pntr;

   /* remove any leading 0's */
   Pntr = 0;
   while (Str[Pntr] == '0')
   {
      Pntr++;
      if (Str[Pntr] == '\0')	/* then number was 0 so put it back */
      {
	 Pntr--;
	 break;
      }
   }

   RtnVal = AddString(&Str[Pntr], NUMBER, parent);

   return (RtnVal);
}



/************************************************************************
*                                                                       *
*                      int AddChar(char *)                              *
*                                                                       *
************************************************************************/

/*
 * AddChar will add the character to the symbol table if it isn't already
 * there.  If it is found then a pointer to the existing entry is returned.
 * If it is added then a pointer to the new entry is returned. In either
 * case an index into the symbol table is returned.
 *
 * Returns an index into the symbol table to the location where the symbol is
 * located.
 */

int
AddChar(char *Str,int parent)
{
   int             RtnVal;

   RtnVal = AddString(Str, CHARACTER, parent);
   return (RtnVal);
}


/************************************************************************
*                                                                       *
*                      int AddInitializer(char *)                       *
*                                                                       *
************************************************************************/

/*
 * AddInitializer will add the init code to the symbol table
 *
 * Returns an index into the symbol table to the location where the text is
 * located.
 */

int
AddInitializer(char *Str,int parent)
{
   int             RtnVal;

   RtnVal = AddString(Str, INITIALIZER, parent);

   return (RtnVal);
}





/************************************************************************
*                                                                       *
*                      int AddName(char *)                              *
*                                                                       *
************************************************************************/

/*
 * AddName will add the name to the symbol table if it isn't already there.
 * If it is found then a pointer to the existing entry is returned. If it is
 * added then a pointer to the new entry is returned. In either case an index
 * into the symbol table is returned.
 *
 * Returns an index into the symbol table to the location where the symbol is
 * located.
 */

int
AddName(char *Str,int parent)
{
   int             RtnVal;

   if (strlen(Str) > MAX_NAME_LEN)
   {
      fprintf(stderr, "ERROR LINE %d : Name '%s' Truncated to ", lineno, Str);
      Str[MAX_NAME_LEN] = '\0';		/* truncate the string */
      fprintf(stderr, "'%s'\n", Str);
   }
   RtnVal = AddString(Str, NAME, parent);
   return (RtnVal);
}

/************************************************************************
*                                                                       *
*                      int LookupName(char *,int parent)                *
*                                                                       *
************************************************************************/

/*
 * Returns an index into the symbol table to the location where the symbol is
 * located or -1 if it is not found
 */

int
LookupName(char *Str,int parent)
{
   int             Loc;

   if (strlen(Str) > MAX_NAME_LEN)
   {
      fprintf(stderr, "ERROR LINE %d : Name '%s' Truncated to ", lineno, Str);
      Str[MAX_NAME_LEN] = '\0';		/* truncate the string */
      fprintf(stderr, "'%s'\n", Str);
   }

   Loc = Hash(Str,parent);

   if ( SymbolTable[Loc].SymbolType == EMPTY ||
        strcmp(&Symbols[SymbolTable[Loc].SymbolLocation], Str) != 0 ||
	SymbolTable[Loc].Parent != parent)
   {
      Loc = -1;
   }


   return Loc;
}

/************************************************************************
*                                                                       *
*                      int symbol_type(int)                             *
*                                                                       *
************************************************************************/

/*
 * symbol_type will return the type associated with the symbol
 */

int
symbol_type(int loc)
{
   if( loc >=0 && loc < SymbolTableSize )
      return SymbolTable[loc].SymbolType;
   else
      return None;
}

/************************************************************************
*                                                                       *
*                      int AddType(char *)                            *
*                                                                       *
************************************************************************/

/*
 * AddType will add a new type to the symbol table or change a NAME to a TYPE
 * if it is already there.
 *
 * Also allocates a symbol record for the name to hold the format string
 * The symbol field Type_loc is used to hold the location of the format
 * string in the Symbols array.
 * The symbol field process_loc is used to hold the location of the initializer
 * string in the Symbols array.  Set to -1 if no initializer.
 *
 * Returns an index into the symbol table to the location where the symbol is
 * located.
 */

int
AddType(const char *Str, const char *format)
{
   int             Loc;
   struct Symbol  *Cur;

   if (strlen(Str) > MAX_NAME_LEN)
   {
      fprintf(stderr, "ERROR LINE %d : Name '%s' Truncated to ", lineno, Str);
      //Str[MAX_NAME_LEN] = '\0';		/* truncate the string */
      fprintf(stderr, "'%s'\n", Str);
      exit(1);
   }
   Loc = AddString(Str, TYPE, NO_PARENT);

   if (SymbolTable[Loc].SymbolType == NAME)
   {
      SymbolTable[Loc].SymbolType = TYPE;
   }

   if (SymbolTable[Loc].SymbolType != TYPE )
   {
      fprintf(stderr, "ERROR LINE %d : Duplicate name %s\n", lineno,Str);
      exit(1);
   }

   /* get a new symbol record */
   Cur = New_symbol();

   Cur->type_format_string = format;

   Cur->SymbolLoc = Loc;
   Cur->NextDef = Type_Chain;
   Type_Chain = Cur;

   SymbolTable[Loc].SymbolInfo = Cur;


   if( cnl_debug )
   {
      printf("Adding %s as a typedef with format \"%s\"\n", Str, format);
   }

   return (Loc);
}


/************************************************************************
*                                                                       *
*                      int AddModule(char *)                            *
*                                                                       *
************************************************************************/

/*
 * AddModule will add a new module to the symbol table or change a NAME to
 * a MODULE if it is already there.
 *
 * Returns an index into the symbol table to the location where the symbol is
 * located.
 */

int
AddModule(char *Str)
{
   int             Loc;
   struct Symbol  *Cur;

   if (strlen(Str) > MAX_NAME_LEN)
   {
      fprintf(stderr, "ERROR LINE %d : Name '%s' Truncated to ", lineno, Str);
      Str[MAX_NAME_LEN] = '\0';		/* truncate the string */
      fprintf(stderr, "'%s'\n", Str);
   }
   Loc = AddString(Str, MODULE, NO_PARENT);

   if (SymbolTable[Loc].SymbolType == NAME)
   {
      SymbolTable[Loc].SymbolType = MODULE;
   }

   if (SymbolTable[Loc].SymbolType != MODULE )
   {
      fprintf(stderr, "ERROR LINE %d : Duplicate name %s\n", lineno,Str);
      exit(1);
   }

   /* get a new symbol record */
   Cur = New_symbol();

   Cur->SymbolLoc = Loc;
   Cur->NextDef = Module_Chain;
   Module_Chain = Cur;
   num_modules++;

   SymbolTable[Loc].SymbolInfo = Cur;


   if( cnl_debug )
      fprintf(stderr,"Adding %s as a moduledef\n", Str);

   return (Loc);
}

/************************************************************************
*                                                                       *
*                      int symbol_name(int)                             *
*                                                                       *
************************************************************************/

/*
 * symbol_name will return a pointer the the string
 */

char *
symbol_name(int Loc)
{
   static char null[]="";
   int offset;

   offset = SymbolTable[Loc].SymbolLocation;

   if( offset >=0 && offset < MaxSymbols )
      return &Symbols[offset];
   else
      return null;
}

/************************************************************************
*                                                                       *
************************************************************************/
char *
strip_quotes(char *in)
{
   char *out = in;
   int len;

   if( in == NULL )
      return NULL;

   if(*out == '\"' )
      out++;

   len = strlen(out);
   if( len > 0 && out[len-1] == '\"' )
      out[len-1] = '\0';

   return out;
}




/**********************************************************************
 * $Log: scan.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:38  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.13  1995/06/15  22:05:18  jmc
 * Added RCS ID and Log strings.
 **********************************************************************/
