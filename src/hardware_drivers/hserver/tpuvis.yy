
%option  noyywrap 
%option prefix="tpuvis"

%{
#define YY_NO_UNPUT // ENDO - gcc 3.4
#define YY_DECL int tpuvislex(char **text,int *len)

#include "cognachrome.h"
#include "hserver.h"
#define YY_INPUT(buf,result,max_size) \
{ \
buf[0]=cognachrome->lexRead(); \
result = 1; \
}
%}       


%%

"Adding to training"				{*text=tpuvistext;*len=tpuvisleng;return(COG_LEX_ADDING);}
"Saving ..."					{*text=tpuvistext;*len=tpuvisleng;return(COG_LEX_SAVING);}
"tpuvis "\[(.)\]>				{*text=tpuvistext;*len=tpuvisleng;return(COG_LEX_PROMPT);}
"Retraining"					{*text=tpuvistext;*len=tpuvisleng;return(COG_LEX_RETRAINING);}
"New protocol header:"				{*text=tpuvistext;*len=tpuvisleng;return(COG_LEX_PROTOCOL_HEADER);}
"New protocol chan ".":"			{*text=tpuvistext;*len=tpuvisleng;return(COG_LEX_PROTOCOL_ABC);}
"M".." # of objects for channel "." ["."]"	{*text=tpuvistext;*len=tpuvisleng;return(COG_LEX_NUM_OBJECTS);}
\xff(a......)*(b......)*(c......)*			{*text=tpuvistext;*len=tpuvisleng;return(COG_LEX_TRACKING);}
\xfe.						{*text=tpuvistext;*len=tpuvisleng;return(COG_LEX_CHANNEL);}			     
.						{ }
