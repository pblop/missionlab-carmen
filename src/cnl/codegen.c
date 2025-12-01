/*
 * Douglas C. MacKenzie codegen.c ---- C routines used for generating CNL
 *
 *    Copyright 1995 - 2005, Georgia Tech Research Corporation
 *    Atlanta, Georgia  30332-0415
 *    ALL RIGHTS RESERVED, See file COPYRIGHT for details.
 *
 *    $Id: codegen.c,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
 */

#include <stdlib.h>
#include "defs.h"
#include "cnl_gram.tab.h"
#include "version.h"

// externs from symtab.c
extern int InText;
extern int InData;
extern int LocalOffset;
extern int MaxLocalOffset;
extern int CurTemp;

// keep track of line number in generated file for #line directives
int outlinenumber = 1;
static char olbuf[ 2048 ];
char* module_name = NULL;

char* port_class_def =
#include "tcb_class_def.include"
;

char* port_class_extn =
#include "tcb_class_extern.include"
;

/**********************************************************************/
// outstring writes the string to the output file
void outstring( const char* str )
{
    const char* p = str;

    while ( ( p = strchr( p, '\n' ) ) != NULL )
    {
        outlinenumber++;
        p++;
    }
    fputs( str, outfile );
}

/**********************************************************************/
// outline writes one line of text to the output file followed by a newline
void outline( const char* str )
{
    outstring( str );
    outstring( "\n" );
}

/**********************************************************************/

void ResetLineNumber()
{
    if ( generate_line_defines )
    {
        sprintf( olbuf, "#line %d \"%s\"", lineno, filename );
        outline( olbuf );
    }
}

/**********************************************************************/

void DefaultLineNumber()
{
    if ( generate_line_defines )
    {
        // line number is one more since #line's give number of next line in file
        sprintf( olbuf, "#line %d \"%s\"", outlinenumber + 1, outname );
        outline( olbuf );
    }
}

/**********************************************************************/
// output_header outputs the top of the output file
void output_header( int argc, char** argv )
{
    // write a header to the output file
    outline( "/**************************************************" );
    outline( "*" );
    sprintf( olbuf, "* This file %s was created with the command", outname );
    outline( olbuf );

    strcpy( olbuf, "* \"" );
    for ( int i = 0; i < argc; i++ )
    {
        sprintf( &olbuf[ strlen( olbuf ) ], "%s%s", argv[ i ], ( i < ( argc - 1 ) ) ? " ":"\"" );
    }
    outline( olbuf );

    sprintf( olbuf, "* using the CNL compiler, version %s.", version_str );
    outline( olbuf );

    outline( "*" );
    outline( "**************************************************/" );
    outline( "\n" );
    outline( "extern \"C\"" );
    outline( "{" );
    outline( "#include <pthread.h>" );
    outline( "}" );
    outline( "#include \"../include/ipt/ipt.h\"" );
    outline( "#include <string.h>" );

    outline( "" );

    outline( "#ifndef NULL" );
    outline( "#define NULL 0" );
    outline( "#endif" );

    sprintf( olbuf, "#define SRC_TYPE_LOCAL_NODE (%d)", SRC_TYPE_LOCAL_NODE );
    outline( olbuf );
    sprintf( olbuf, "#define SRC_TYPE_REMOTE     (%d)", SRC_TYPE_REMOTE     );
    outline( olbuf );
    sprintf( olbuf, "#define SRC_TYPE_CONSTANT   (%d)", SRC_TYPE_CONSTANT   );
    outline( olbuf );

    outline( "#define require_input(name) parms->##name##require = true" );
    outline( "#define NAME2NAME_REQUIRE(name) name##_require" );
    outline( "" );

    if ( compile )
    {
        outline( "extern int             _num_nodes;"       );
        outline( "extern int             _done_count;"      );
        outline( "extern pthread_mutex_t         _done_count_lock;" );
        outline( "extern pthread_cond_t     _new_cycle;"       );
        outline( "extern char*           _prefix;"          );
        outline( "extern char*           _ipt_home;"        );
        outline( "extern IPCommunicator* _communicator;"    );
        outline( "extern IPConnection**  _ipt_modules;"     );
        outline( "extern int   	         _ipt_is_active;"   );
        outline( "extern double          _zero_time;"       );
    }
    else
    {
        outline( "struct { int argc; char** argv; } _p;" );

        outline( "int         _num_nodes = 0;   // total number of active nodes" );
        outline( "int         _done_count = 0;  // number of output nodes that finished current cycle" );
        outline( "pthread_mutex_t     _done_count_lock; // mutex lock to modify done_count" );
        outline( "pthread_cond_t _new_cycle;       // condition broadcast when last output node" );
        outline( "                              //   finishes the cycle. Each node waits after executing," );
        outline( "                              //   thereby syncronizing the entire dataflow tree" );
        outline( "char*           _prefix = \"\";" );
        outline( "char*           _ipt_home;" );
        outline( "IPCommunicator* _communicator;" );
        outline( "IPConnection**  _ipt_modules;" );
        outline( "int             _ipt_is_active = 0;" );

        // Since the timeval struct may or may not be defined, I need to use my own.
        // I always want a time of zero, so to be safe, I'll add an extra few bytes
        // of zeros and disregard the size of the two entries (longs probably).
        outline( "double          _zero_time = 0;" );
    }

    outline( "\n" );

    if ( no_builtin_bool )
    {
        outline( "typedef unsigned char bool;" );
        outline( "const bool false = 0;" );
        outline( "const bool true = 1;" );
        outline( "\n" );
    }

    // This will dump the file "tcb_class_extern.include" to the output
    outline( "\n" );
    outline( port_class_extn );
    outline( "\n" );

    // Generate a procedure to clear all task's status
    if ( compile )
    {
        outline( "void reset_all_tasks();" );
        outline( "void update_all_tasks();" );
        outline( "extern _list<tcb *> list_of_tasks;\n" );

    }
    else
    {
        // This will dump the file "tcb_class_def.include" to the output
        outline( "\n" );
        outline( port_class_def );
        outline( "\n" );

        outline( "_list<tcb *> list_of_tasks;\n" );
        outline( "void reset_all_tasks()" );
        outline( "{" );
        outline( "   void *ptr;" );
        outline( "   tcb  *task;" );
        outline( "   if ( (ptr = list_of_tasks.first(&task)) != NULL )" );
        outline( "   {" );
        outline( "      do" );
        outline( "      {" );
        outline( "         task->reset_status();" );
        outline( "      } while ( (ptr = list_of_tasks.next(&task,ptr)) != NULL );" );
        outline( "   }" );
        outline( "}" );

        outline( "void update_all_tasks()" );
        outline( "{" );
        outline( "   void *ptr;" );
        outline( "   tcb  *task;" );
        outline( "   if ( (ptr = list_of_tasks.first(&task)) != NULL )" );
        outline( "   {" );
        outline( "      do" );
        outline( "      {" );
        outline( "         task->update_run_status();" );
        outline( "      }while ( (ptr = list_of_tasks.next(&task,ptr)) != NULL );" );
        outline( "   }" );
        outline( "}" );
    }
}


//*********************************************************************
char* init_string( Symbol* type )
{
    const char* format = type->type_format_string;

    if ( ( strcmp( format, "byte"    ) == 0 ) ||
         ( strcmp( format, "short"   ) == 0 ) ||
         ( strcmp( format, "int"     ) == 0 ) ||
         ( strcmp( format, "float"   ) == 0 ) ||
         ( strcmp( format, "double"  ) == 0 ) ||
         ( strcmp( format, "boolean" ) == 0 ) )
    {
        return "0";
    }
    else if ( strcmp( format, "char" ) == 0 )
    {
        return "'\\0'";
    }
    else if ( strcmp( format, "{double, double, double}" ) == 0 )
    {
        return "{0, 0, 0}";
    }

    return NULL;
}


/**********************************************************************/
// emit_defs will generate the data structures for the procedures
void emit_defs( Symbol* nodes )
{
    char* l_name;
    char* typetext;

    if ( had_error )
    {
        return;
    }

    DefaultLineNumber();

    // generate the data structures for each node
    // declare pointers to the data blocks for each node
    Symbol* cur = nodes;
    int ncnt = 0;
    while ( cur )
    {
        if ( ncnt++ > 1000 )
        {
            fprintf( stderr, "Probable infinite loop in emit_defs:procs\n" );
            fclose( outfile );
            exit( 1 );
        }

        outline( "" );

        char* p_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];

        // generate storage for the parameters
        sprintf( olbuf, "struct T_%s : public tcb", p_name );
        outline( olbuf );
        outline( "{" );
        outline( "    char* node_name;" );
        Symbol* parm = cur->Parms;
        int pcnt = 0;
        while ( parm )
        {
            if ( pcnt++ > 1000 )
            {
                fprintf( stderr, "Probable infinite loop in emit_defs:parms\n" );
                fclose( outfile );
                exit( 1 );
            }

            typetext = &Symbols[ SymbolTable[ parm->Type_loc ].SymbolLocation ];
            l_name = &Symbols[ SymbolTable[ parm->SymbolLoc ].SymbolLocation ];

            if ( parm->IsList )
            {
                sprintf( olbuf, "    %s **%s;", typetext, l_name );
                outline( olbuf );
                sprintf( olbuf, "    int *%s_type;", l_name );
                outline( olbuf );
                sprintf( olbuf, "    int *%s_module;", l_name );
                outline( olbuf );
                sprintf( olbuf, "    char* *%s_src;", l_name );
                outline( olbuf );
                sprintf( olbuf, "    tcb **%s_tcb;", l_name );
                outline( olbuf );
                sprintf( olbuf, "    int %s_num; // number of entries in list", l_name );
                outline( olbuf );
                if ( cur->node_type & NODE_TYPE_NEW_STYLE_PROC )
                {
                    sprintf( olbuf, "    bool *%s_require;", l_name );
                    outline( olbuf );
                }
            }
            else
            {
                sprintf( olbuf, "    %s *%s;", typetext, l_name );
                outline( olbuf );
                sprintf( olbuf, "    int %s_type;", l_name );
                outline( olbuf );
                sprintf( olbuf, "    int %s_module;", l_name );
                outline( olbuf );
                sprintf( olbuf, "    char* %s_src;", l_name );
                outline( olbuf );
                sprintf( olbuf, "    tcb *%s_tcb;", l_name );
                outline( olbuf );
                if ( cur->node_type & NODE_TYPE_NEW_STYLE_PROC )
                {
                    sprintf( olbuf, "    bool %s_require;", l_name );
                    outline( olbuf );
                }
            }

            outline( "" );

            parm = parm->NextDef;
        }

        if (cur->Type_loc != void_type)
        {
            /* add the output parm */
            typetext = &Symbols[ SymbolTable[ cur->Type_loc ].SymbolLocation ];

            sprintf( olbuf, "    %s output;", typetext );
            outline( olbuf );
        }

        outline( "    void update_run_status()" );
        outline( "    {" );
        if ( cur->Type_loc != void_type )
        {
            outline( "        update_status( &output );" );
        }
        else
        {
            outline( "        update_status( NULL );" );
        }
        outline( "    }" );


        outline( "    bool will_use_port( void* port )" );
        outline( "    {" );

        // If this is a new style, then overload the will_use_port function.
        if ( cur->node_type & NODE_TYPE_NEW_STYLE_PROC )
        {
            outline( "        if ( status_ == STAT_UNKNOWN )" );
            outline( "        {" );
            if ( cur->Type_loc != void_type )
            {
                outline( "            update_status( &output );" );
            }
            else
            {
                outline( "            update_status( NULL );" );
            }
            outline( "        }" );

            outline( "        if ( status_ == STAT_BLOCKED )" );
            outline( "        {" );

            outline( "            return false;" );
            outline( "        }" );

            // This is ugly
            // The only way to figure out which input they are asking about
            // is to do a linear search which means generating each one in line.
            outline( "        bool will_use = false;" );
            outline( "        bool found_one = false;" );
            parm = cur->Parms;
            pcnt = 0;
            while ( parm )
            {
                l_name = &Symbols[ SymbolTable[ parm->SymbolLoc ].SymbolLocation ];

                if ( parm->IsList )
                {
                    sprintf( olbuf, "        for ( int i = 0; i < %s_num; i++ )", l_name );
                    outline( olbuf );
                    outline(        "        {" );
                    sprintf( olbuf, "            if ( port == (void*) %s[ i ] )", l_name );
                    outline( olbuf );
                    outline(        "            {" );
                    outline(        "                found_one = true;" );

                    sprintf( olbuf, "                will_use |= %s_require[ i ];", l_name );
                    outline( olbuf );
                    outline(        "            }" );
                    outline(        "        }" );
                }
                else
                {
                    sprintf( olbuf, "        if ( port == (void *)%s )", l_name );
                    outline( olbuf );
                    outline(        "        {" );
                    outline(        "             found_one = true;" );
                    sprintf( olbuf, "             will_use |= %s_require;", l_name );
                    outline( olbuf );
                    outline(        "        }" );
                }
                parm = parm->NextDef;
            }

            outline( "" );
            outline( "        if ( found_one )" );
            outline( "        {" );
            outline( "            return will_use;" );
            outline( "        }" );
            outline( "" );
            outline( "        printf( \"will_use_port didn't find a match\\n\" );" );
            outline( "        abort();" );
            outline( "        return false;" );
        }
        else
        {
            // Default handler for inquiries if we will use a particular input port
            // If we will run, then respond yes, we will use the input
            outline(        "       if ( status_ == STAT_UNKNOWN )" );
            outline(        "       {" );
            sprintf( olbuf, "           update_status(%s );",
                     cur->Type_loc == void_type ? "NULL" : "&output" );
            outline( olbuf );
            outline(        "       }" );
            outline(        "       return status_ == STAT_RUNNING;" );
        }
        outline( "   }" );

        if ( cur->node_type & NODE_TYPE_NEW_STYLE_PROC )
        {
            outline( "   void clear_requests()" );
            outline( "   {" );
            parm = cur->Parms;
            pcnt = 0;
            while ( parm )
            {
                l_name = &Symbols[ SymbolTable[ parm->SymbolLoc ].SymbolLocation ];

                if ( parm->IsList )
                {
                    sprintf( olbuf, "      for (int i=0; i<%s_num; i++)", l_name );
                    outline( olbuf );
                    sprintf( olbuf, "         %s_require[ i ] = false;", l_name );
                    outline( olbuf );
                }
                else
                {
                    sprintf( olbuf, "      %s_require = false;", l_name );
                    outline( olbuf );
                }
                parm = parm->NextDef;
            }
            outline( "   }" );
        }

        sprintf( olbuf,"   virtual ~T_%s();",p_name );
        outline( olbuf );
        // end the parm structure
        outline( "};" );

        cur = cur->NextDef;
    }

    // Generate tcx defines for each of the types
    if ( compile )
    {
        outline( "extern char* FRM_Query;" );
    }
    else
    {
        outline( "char* FRM_Query = \"int\";" );
    }

    cur = Type_Chain;
    ncnt = 0;
    while ( cur )
    {
        char* type_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];

        if ( ncnt++ > 1000 )
        {
            fprintf( stderr, "Probable infinite loop in Type_Chain\n" );
            fclose( outfile );
            exit( 1 );
        }

        if ( cur->SymbolLoc != void_type )
        {
            sprintf( olbuf, "typedef struct PCK_%s {int valid; %s val;} PCK_%s;",
                     type_name, type_name, type_name );
            outline( olbuf );

            if ( compile )
            {
                sprintf(olbuf, "extern char* FRM_%s;",type_name );
            }
            else
            {
                sprintf(olbuf, "char* FRM_%s = \"{int, %s}\";",
                        type_name, cur->type_format_string );
            }
            outline( olbuf );

            if ( compile )
            {
                sprintf( olbuf, "extern char* MESG_%s;",type_name );
            }
            else
            {
                sprintf( olbuf, "char* MESG_%s = \"%s\";", type_name, type_name );
            }
            outline( olbuf );
        }

        cur = cur->NextDef;
    }

    outline( "" );
    outline( "extern \"C\" {" );
    outline( "void *malloc(size_t );" );
    outline( "void exit(int );" );
    outline( "}" );
    outline( "" );
}


/**********************************************************************/
// print_proc will generate the code for a procedure
void print_proc( Symbol* item, int type )
{
    char* name;
    char* parm_name;
    char* typetext;
    char* parm_typetext;
    Symbol* parm;
    int is_void;
    int have_list = false;

    name = &Symbols[ SymbolTable[ item->SymbolLoc ].SymbolLocation ];

    is_void = item->Type_loc == void_type;

    // add the output parm
    typetext = &Symbols[ SymbolTable[ item->Type_loc ].SymbolLocation ];

    if ( type == LOCAL )
    {
        // Generate code for a virtual destructor for our task control block
        // This seems to be needed with the new egcs compiler to force it to
        // generate the virtual function table.  I think it is a bug in egcs.
        outline( "" );
        sprintf( olbuf, "T_%s::~T_%s() {};", name, name );
        outline( olbuf );
        outline( "" );

        if ( !is_void )
        {
            // Generate code for a ipt callback function
            sprintf( olbuf, "void _cb_%s( IPCommunicator* comm, IPMessage* msg, void* hndData )", name );
            outline( olbuf );
            outline( "{" );
            sprintf( olbuf, "    T_%s* parms = (T_%s*) hndData;", name, name );
            outline( olbuf );
            sprintf( olbuf, "    PCK_%s out;", typetext );
            outline( olbuf );
            outline( "" );
            outline( "    out.valid = parms->output_valid_;" );
            outline( "    out.val = parms->output;" );
            outline( "" );
            sprintf( olbuf, "    comm->Reply( msg, MESG_%s, &out );", typetext );
            outline( olbuf );
            outline( "}" );
            outline( "" );
        }
        outline( "" );

        // Generate code for the real procedure
        sprintf( olbuf, "void *%s( void *parm )", name );
        outline( olbuf );
        outline( "{" );

        if ( !is_void )
        {
            char* istringOutput = init_string(SymbolTable[ item->Type_loc ].SymbolInfo );
            if ( istringOutput )
            {
                sprintf( olbuf, "    %s output = (%s) %s;", typetext, typetext, istringOutput );
            }
            else
            {
            	sprintf( olbuf, "    %s output;", typetext );
            }
            outline( olbuf );
        }

        parm = item->Parms;
        if ( ( parm != NULL ) || !is_void )
        {
            sprintf( olbuf, "    T_%s* parms = (T_%s*) parm;", name, name );
            outline( olbuf );

            outline( "" );
            outline( "" );
        }
		else
		{
            outline( "" );
            outline( "    // node_name is the name of this node instance (AN_xxx) " );
            sprintf( olbuf, "    const char* node_name = \"%s\";", name );
            outline( olbuf );
            outline( "" );
		}

        // Emit the parameter shadows
        while ( parm != NULL )
        {
            parm_typetext = &Symbols[ SymbolTable[ parm->Type_loc ].SymbolLocation ];
            parm_name = &Symbols[ SymbolTable[ parm->SymbolLoc ].SymbolLocation ];
            if ( parm->IsList )
            {
                have_list = true;
                sprintf( olbuf, "    int %s_num = parms->%s_num;", parm_name, parm_name );
                outline( olbuf );

                sprintf( olbuf, "    %s* %s = (%s*) malloc( sizeof( *%s ) * %s_num );",
                         parm_typetext, parm_name, parm_typetext, parm_name, parm_name );
                outline( olbuf );
            }
            else
            {
                char* istring = init_string(SymbolTable[ parm->Type_loc ].SymbolInfo );
                if ( istring )
                {
                    sprintf( olbuf, "    %s %s = (%s) %s;", parm_typetext, parm_name, parm_typetext, istring );
                }
                else
                {
                    sprintf( olbuf, "    %s %s;", parm_typetext, parm_name );
                }
                outline( olbuf );
            }

            parm = parm->NextDef;
        }

        // add a loop variable used to copy the list arrays
        if ( have_list )
        {
            outline( "    int _i;" );
        }

        outline( "" );
        outline( "    //******* load any constants ********" );

        // Generate code to load any constants
        parm = item->Parms;
        while ( parm != NULL )
        {
            parm_name = &Symbols[ SymbolTable[ parm->SymbolLoc ].SymbolLocation ];

            if ( parm->IsList && ( parm->ThisInstance == 0 ) )
            {
                sprintf( olbuf, "    for ( _i = 0; _i < %s_num; _i++ )", parm_name );
                outline( olbuf );
                outline(        "    {" );
                sprintf( olbuf, "        if ( parms->%s_type[ _i ] == SRC_TYPE_CONSTANT )", parm_name );
                outline( olbuf );
                outline(        "        {" );
                sprintf( olbuf, "            %s[ _i ] = *parms->%s[ _i ];", parm_name, parm_name );
                outline( olbuf );
                outline(        "        }" );

                char* istringP = init_string(SymbolTable[ parm->Type_loc ].SymbolInfo );
                parm_typetext = &Symbols[ SymbolTable[ parm->Type_loc ].SymbolLocation ];
                if(istringP)
                {
                sprintf( olbuf, "        else");
                outline( olbuf );
                outline(        "        {" );
                sprintf( olbuf, "            %s[ _i ] = (%s) %s;", parm_name, parm_typetext, istringP);
                outline( olbuf );
                outline(        "        }" );
                outline(        "    }" );
                }
            }
            else
            {
                sprintf( olbuf, "    if ( parms->%s_type == SRC_TYPE_CONSTANT )", parm_name );
                outline( olbuf );
                outline(        "    {" );
                sprintf( olbuf, "        %s = *parms->%s;", parm_name, parm_name );
                outline( olbuf );
                outline(        "    }" );
                char* istringP = init_string(SymbolTable[ parm->Type_loc ].SymbolInfo );
                parm_typetext = &Symbols[ SymbolTable[ parm->Type_loc ].SymbolLocation ];
                if(istringP)
                {
                sprintf( olbuf, "    else");
                outline( olbuf );
                outline(        "    {" );
                sprintf( olbuf, "        %s = (%s) %s;", parm_name, parm_typetext, istringP);
                outline( olbuf );
                outline(        "    }" );
                }
            }

            parm = parm->NextDef;
        }

        // init the activeFlag to NULL to turn it off
        outline( "    parms->activeFlag = NULL;");

        // Generate the top of the once code
        outline( "" );
        outline( "    //******* start of once code ********" );

        ResetLineNumber();
        outline( "" );
    }
    else
    {
        sprintf( olbuf, "extern void *%s( void *parm );", name );
        outline( olbuf );
    }
}

/**********************************************************************/
void close_once_code( int type, bool new_style )
{
   if ( type == LOCAL )
   {
      outline( "        //******* End of once code ********" );

      // Generate the top of the main loop
      outline( "    while ( 1 )" );
      outline( "    {" );
      outline( "        // Loop here while we are blocked" );
      outline( "        do" );
      outline( "        {" );
      outline( "            parms->done_ = true;" );
      outline( "            pthread_mutex_lock( &_done_count_lock );" );
      outline( "            _done_count++;" );
      outline( "            if (_done_count >= _num_nodes) " );
      outline( "            {" );
      outline( "                extern void end_of_cycle(void );" );
      outline( "                _done_count = 0;" );
      outline( "                end_of_cycle();     // user function hook" );
      outline( "                reset_all_tasks();  // clear the status flags" );
      outline( "                update_all_tasks(); // reset the status flags" );
      outline( "                pthread_cond_broadcast( &_new_cycle );" );
      outline( "                pthread_mutex_unlock( &_done_count_lock );" );
      outline( "                continue;           // skip the condition wait" );
      outline( "            }" );
      outline( "            if( parms->activeFlag ) *parms->activeFlag = parms->blocked() ? '0' : '1';" );
      outline( "            pthread_cond_wait( &_new_cycle, &_done_count_lock );" );
      outline( "            pthread_mutex_unlock( &_done_count_lock );" );
      outline( "        } while ( parms->blocked() ); " );
	  outline( "" );
      outline( "        //******* start of user header ********" );

      ResetLineNumber();
      outline( "" );
   }
}

/**********************************************************************/
// start_body will emit the top of the loop
void start_body( Symbol* cur )
{
    //int  is_void   = cur->Type_loc == void_type;
    bool new_style = ( cur->node_type & NODE_TYPE_NEW_STYLE_PROC ) != 0;


    DefaultLineNumber();

    outline( "" );
    outline( "        //******* end of user header ********" );

    outline( "" );
    outline( "        while ( !parms->blocked() )" );
    outline( "        {" );
    outline( "            if( parms->activeFlag ) *parms->activeFlag = '1';" );

    outline( "            bool ok_to_run = true;\n" );

    outline( "            // if using ipt, run any waiting message handlers" );
    outline( "            if ( _ipt_is_active )" );
    outline( "            {" );
    outline( "                _communicator->Idle( _zero_time );" );
    outline( "            }" );
    outline( "" );

    Symbol* parm = cur->Parms;
    while ( parm != NULL )
    {
        char* parm_name = &Symbols[ SymbolTable[ parm->SymbolLoc ].SymbolLocation ];
        char* type_text = &Symbols[ SymbolTable[ parm->Type_loc ].SymbolLocation ];

        if ( parm->IsList && ( parm->ThisInstance == 0 ) )
        {
            sprintf( olbuf, "            for ( _i = 0; _i < %s_num; _i++ )", parm_name );
            outline( olbuf );
            outline(        "            {" );
            if ( new_style )
            {
                sprintf( olbuf, "                if ( !parms->%s_require[ _i ] )", parm_name );
                outline( olbuf );
                outline(        "                {" );
                outline(        "                    continue;" );
                outline(        "                }" );
            }

            sprintf( olbuf, "                if ( parms->%s_type[ _i ] == SRC_TYPE_LOCAL_NODE )", parm_name );
            outline( olbuf );
            outline(        "                {" );
            sprintf( olbuf, "                    while ( !parms->%s_tcb[ _i ]->done() )", parm_name );
            outline( olbuf );
            outline(        "                    {" );
            outline(        "                        usleep(5000);" );
            outline(        "                    }" );
            sprintf( olbuf, "                    if ( parms->%s_tcb[ _i ]->output_valid_ )",parm_name );
            outline( olbuf );
            outline(        "                    {" );
            sprintf( olbuf, "                        %s[ _i ] = *parms->%s[ _i ];", parm_name, parm_name );
            outline( olbuf );
            outline(        "                    }" );
            outline(        "                    else" );
            outline(        "                    {" );
            outline(        "                        ok_to_run = false;" );
            outline(        "                        if ( debug )" );
            outline(        "                        {" );
            sprintf( olbuf, "                            printf( \"%%s: Blocked because %s[ %%d ] is not running\\n\", ",
                     parm_name );
            outline( olbuf );
            outline(        "                                parms->task_name, _i );" );
            outline(        "                        }" );
            outline(        "                    }" );
            outline(        "                }" );

            sprintf( olbuf, "                else if ( parms->%s_type[ _i ] == SRC_TYPE_REMOTE)", parm_name );
            outline( olbuf );
            outline(        "                {" );
            outline(        "                    // Get the value from the remote node" );
            outline(        "                    int parm = 0;" );
            sprintf( olbuf, "                    PCK_%s* rcv = (PCK_%s*) _communicator->QueryFormatted( ",
                     type_text, type_text );
            outline( olbuf );
            sprintf( olbuf, "                        _ipt_modules[ parms->%s_module[ _i ] ], ", parm_name );
            outline( olbuf );
            sprintf( olbuf, "                        parms->%s_src[ _i ], &parm, MESG_%s );", parm_name, type_text );
            outline( olbuf );
            sprintf( olbuf, "                    %s[ _i ] = rcv->val;", parm_name );
            outline( olbuf );
            outline(		"					 delete rcv;");
            outline(        "                }" );

            outline(        "            }" );
        }
        else
        {
            if ( new_style )
            {
                sprintf( olbuf, "            if ( parms->%s_require )", parm_name );
                outline( olbuf );
            }
            outline(        "            {" );

            sprintf( olbuf, "                if ( parms->%s_type == SRC_TYPE_LOCAL_NODE)", parm_name );
            outline( olbuf );
            outline(        "                {" );
            sprintf( olbuf, "                    while ( !parms->%s_tcb->done() )", parm_name );
            outline( olbuf );
            outline(        "                    {" );
            outline(        "                        usleep(5000);" );
            outline(        "                    }" );
            sprintf( olbuf, "                    if ( parms->%s_tcb->output_valid_ )",parm_name );
            outline( olbuf );
            outline(        "                    {" );
            sprintf( olbuf, "                        %s = *parms->%s;", parm_name, parm_name );
            outline( olbuf );
            outline(        "                    }" );
            outline(        "                    else" );
            outline(        "                    {" );
            outline(        "                        ok_to_run = false;" );
            outline(        "                        if ( debug )" );
            outline(        "                        {" );
            sprintf( olbuf, "                            printf( \"%%s: Blocked because %s is not running\\n\", ",
                     parm_name );
            outline( olbuf );
            outline(        "                                parms->task_name );" );
            outline(        "                        }" );
            outline(        "                    }" );
            outline(        "                }" );
            sprintf( olbuf, "                else if ( parms->%s_type == SRC_TYPE_REMOTE)", parm_name );
            outline( olbuf );
            outline(        "                {" );
            outline(        "                    // Get the value from the remote node*/" );
            outline(        "                    int parm = 0;" );
            sprintf( olbuf, "                    PCK_%s* rcv = (PCK_%s*) _communicator->QueryFormatted( ",
                     type_text, type_text );
            outline( olbuf );
            sprintf( olbuf, "                        _ipt_modules[ parms->%s_module ], ", parm_name );
            outline( olbuf );
            sprintf( olbuf, "                        parms->%s_src, &parm, MESG_%s );", parm_name, type_text );
            outline( olbuf );
            sprintf( olbuf, "                    %s = rcv->val;", parm_name );
            outline( olbuf );
            outline(		"					 delete rcv;");
            outline(        "                }" );
            outline(        "            }" );
        }

        parm = parm->NextDef;
    }
    outline( "" );

    if ( !(cur->node_type & NODE_TYPE_NEW_STYLE_PROC) )
    {
        outline( "            if ( ok_to_run )" );
    }

    outline( "            {" );
    outline( "                if ( debug )" );
    outline( "                {" );
    outline( "                    printf(\"%s: Running\\n\",parms->task_name );" );
    outline( "                }" );
    outline( "" );
    outline( "                { //******* start of user body ********" );

    ResetLineNumber();
}

/**********************************************************************/
// end_procedure will generate the code for leaving the procedure
void end_procedure( int type, Symbol* Item )
{
   int is_void = Item->Type_loc == void_type;

   char* typetext = &Symbols[ SymbolTable[ Item->Type_loc ].SymbolLocation ];
   bool bIsTrigger = strcmp( typetext, "bool" ) == 0;

   if ( type == LOCAL )
   {
      DefaultLineNumber();
      outline( "" );
      outline( "                }/******** end of user code ********/" );
      outline( "" );

      if ( !is_void )
      {
         outline( "                parms->output = output;" );
         if ( bIsTrigger )
         {
             outline( "                parms->output &= !drawing_vector_field;" );
         }
         outline( "                parms->output_valid_ = true;" );
      }
      outline( "            }" );
      outline( "" );
      outline( "            parms->done_ = true;" );
      outline( "            pthread_mutex_lock( &_done_count_lock );" );
      outline( "            _done_count++;" );
      outline( "            if ( _done_count >= _num_nodes ) " );
      outline( "            {" );
      outline( "                extern void end_of_cycle(void );" );
      outline( "                _done_count = 0;" );
      outline( "                end_of_cycle();     // user function hook" );
      outline( "                reset_all_tasks();  // clear the status flags" );
      outline( "                update_all_tasks(); // reset the status flags" );
      outline( "                pthread_cond_broadcast( &_new_cycle );" );
      outline( "                pthread_mutex_unlock( &_done_count_lock );" );
      outline( "                continue;           // skip the condition wait" );
      outline( "            }" );
      outline( "            pthread_cond_wait( &_new_cycle, &_done_count_lock );" );
      outline( "            pthread_mutex_unlock( &_done_count_lock );" );
      outline( "        } // while not blocked" );
      outline( "    } // end of main while loop" );
      outline( "    return NULL;" );
      outline( "}" );
   }
}

/**********************************************************************/

void end_procs()
{
   outline( "" );
   outline( "/* End of procedures */" );
}

/**********************************************************************/

int module_name_to_index( char* name )
{
    int index = 0;
    Symbol* cur;

    // Find the requested module
    cur = Module_Chain;
    while ( cur )
    {
        if ( strcmp( name, &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ] ) == 0 )
        {
            return index;
        }

        index++;
        cur = cur->NextDef;
    }

    fprintf( stderr,"Internal Error: module_name_to_index(%s) ::: not found in module chain\n", name );
    exit( 1 );
    return 0;
}


/**********************************************************************/
// emit_nodes will generate the code for the node definitions
// It returns the number of threads required.
int emit_nodes( Symbol* procs, Symbol* nodes )
{
    char* n_name;
    char* p_name = NULL;
    char* l_name;
    char* src_name;
    char* typetext;
    Symbol* cur;
    Symbol* proc;
    Symbol* parm;
    Symbol* link;
    Symbol* t;
    int cnt;
    char initializer_name[ 2048 ];
    int num_threads;
    int num_nodes;
    int setup_ipt = false;

    if ( had_error )
    {
        return 0;
    }

    // attach the links for each node
    cur = nodes;
    while ( cur )
    {
        // fixup the list instances so the MaxInstances field is correct
        link = cur->Links;
        while ( link != NULL )
        {
            if ( link->IsList )
            {
                cnt = 0;

                // count
                t = cur->Links;
                while ( t != NULL )
                {
                    // if this link is the one we are fixing up, then count it
                    if ( link->SymbolLoc == t->SymbolLoc )
                    {
                        cnt++;
                    }

                    t = t->NextDef;
                }

                // fixup
                t = cur->Links;
                while ( t != NULL )
                {
                    // if this link is the one we are fixing up, then set it
                    if ( link->SymbolLoc == t->SymbolLoc )
                    {
                        t->MaxInstances = cnt;
                    }

                    t = t->NextDef;
                }
            }

            link = link->NextDef;
        }

        cur = cur->NextDef;
    }


    outline( "" );

    outline( "void cnl_shared_malloc(void **p, int size)" );
    outline( "{" );
    outline( "   static int total=0;" );
    outline( "" );
    outline( "   if ( (*p = malloc(size)) == NULL)" );
    outline( "   {" );
    outline( "      printf(\"Out of memory allocating %d bytes after %d bytes in cnl_shared_malloc\",size,total );" );
    outline( "      exit( 1 );" );
    outline( "   }" );
    outline( "   total += size;" );
    outline( "}" );
    outline( "" );


    outline( "void init_fnc()" );
    outline( "{" );
    outline( "    int size;" );
    outline( "    pthread_t t;" );
    outline( "    void init( int, char** );" );
    outline( "" );

    // declare pointers to the data blocks for each node
    cur = nodes;
    while ( cur )
    {
        proc = cur->Proc;

        n_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];

        if ( !( cur->node_type & NODE_TYPE_REMOTE ) )
        {

            if ( proc == NULL )
            {
                fprintf( stderr, "codegen:emit_nodes - Internal Error: invalid procedure pointer for node %s\n", n_name );
                fclose( outfile );
                abort();
                exit( 1 );
            }
            else
            {
                p_name = &Symbols[ SymbolTable[ proc->SymbolLoc ].SymbolLocation ];

                sprintf( olbuf, "    T_%s *N_%s;", p_name, n_name );
                outline( olbuf );
            }
        }

        cur = cur->NextDef;
    }

    outline( "" );
    outline( "" );




    // declare instance of any INITIALIZER data structures so load works
    cur = nodes;
    while ( cur )
    {
        // spin the input links
        link = cur->Links;
        while ( link != NULL )
        {
            if ( SymbolTable[ link->SrcNode ].SymbolType == INITIALIZER )
            {
                n_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];
                proc = cur->Proc;

                parm = proc->Parms;
                while ( ( parm != NULL ) && ( parm->SymbolLoc != link->SymbolLoc ) )
                {
                    parm = parm->NextDef;
                }

                if ( parm == NULL )
                {
                    fprintf( stderr, "Unable to match INITIALIZER link to parm name\n" );
                    abort();
                }

                typetext = &Symbols[ SymbolTable[ parm->Type_loc ].SymbolLocation ];
                l_name = &Symbols[ SymbolTable[ parm->SymbolLoc ].SymbolLocation ];

                if ( parm->IsList )
                {
                    sprintf( olbuf, "    %s %s_%s%d = %s;",
                             typetext, n_name, l_name,
                             link->ThisInstance,
                             &Symbols[ SymbolTable[ link->SrcNode ].SymbolLocation ] );
                }
                else
                {
                    sprintf( olbuf, "    %s %s_%s = %s;",
                             typetext, n_name, l_name,
                             &Symbols[ SymbolTable[ link->SrcNode ].SymbolLocation ] );
                }
                outline( olbuf );
            }
            link = link->NextDef;
        }
        cur = cur->NextDef;
    }



    outline( "" );
    outline( "" );

    // allocate memory for the data blocks for each node
    // Also, enqueue the tcb's into the list_of_tasks.
    cur = nodes;
    while ( cur )
    {
        if ( !( cur->node_type & NODE_TYPE_REMOTE ) )
        {

            proc = cur->Proc;

            n_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];
            p_name = &Symbols[ SymbolTable[ proc->SymbolLoc ].SymbolLocation ];

            sprintf( olbuf, "    N_%s = new T_%s;", n_name, p_name );
            outline( olbuf );
            sprintf( olbuf, "    list_of_tasks.append( N_%s );", n_name );
            outline( olbuf );
        }

        cur = cur->NextDef;
    }

    outline( "" );
    outline( "" );


    // allocate memory for list blocks for each node
    // and count the number of nodes

    cur = nodes;
    num_nodes = 0;
    while ( cur )
    {
        if ( !( cur->node_type & NODE_TYPE_REMOTE ) )
        {
            proc = cur->Proc;

            n_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];

            // Set the node's name.
            sprintf( olbuf, "   N_%s->node_name = \"%s\";", n_name, n_name );
            outline( olbuf );

            // generate storage for the parameters
            parm = proc->Parms;
            while ( parm )
            {
                if ( parm->IsList )
                {
                    typetext = &Symbols[ SymbolTable[ parm->Type_loc ].SymbolLocation ];
                    l_name = &Symbols[ SymbolTable[ parm->SymbolLoc ].SymbolLocation ];

                    sprintf( olbuf, "    size = sizeof( *N_%s->%s ) * %d;", n_name, l_name, parm->MaxInstances );
                    outline( olbuf );
                    sprintf( olbuf, "    cnl_shared_malloc( (void**) &N_%s->%s, size);", n_name, l_name );
                    outline( olbuf );


                    sprintf( olbuf, "   size = sizeof( tcb* ) * %d;", parm->MaxInstances );
                    outline( olbuf );
                    sprintf( olbuf, "   cnl_shared_malloc( (void**) &N_%s->%s_tcb, size);", n_name, l_name );
                    outline( olbuf );

                    sprintf( olbuf, "   size = sizeof( int ) * %d;", parm->MaxInstances );
                    outline( olbuf );
                    sprintf( olbuf, "   cnl_shared_malloc( (void**) &N_%s->%s_module, size);", n_name, l_name );
                    outline( olbuf );

                    sprintf( olbuf, "   size = sizeof( char* ) * %d;", parm->MaxInstances );
                    outline( olbuf );
                    sprintf( olbuf, "   cnl_shared_malloc( (void**) &N_%s->%s_src, size);", n_name, l_name );
                    outline( olbuf );

                    sprintf( olbuf, "   size = sizeof( int ) * %d;", parm->MaxInstances );
                    outline( olbuf );
                    sprintf( olbuf, "   cnl_shared_malloc( (void**) &N_%s->%s_type, size);", n_name, l_name );

                    outline( olbuf );


                    if ( proc->node_type & NODE_TYPE_NEW_STYLE_PROC )
                    {
                        sprintf( olbuf, "    size = sizeof( bool ) * %d;", parm->MaxInstances );
                        outline( olbuf );
                        sprintf( olbuf, "    cnl_shared_malloc( (void**) &N_%s->%s_require, size);",
                                 n_name, l_name );
                        outline( olbuf );
                    }

                    outline( "" );
                }

                parm = parm->NextDef;
            }

            // count the number of local nodes so can tell when a cycle completes
            num_nodes++;
        }
        cur = cur->NextDef;
    }

    outline( "" );
    sprintf( olbuf, "    _num_nodes = %d;", num_nodes );
    outline( olbuf );
    outline( "" );
    outline( "    pthread_cond_init( &_new_cycle, NULL);" );
    outline( "    pthread_mutex_init( &_done_count_lock, NULL);" );
    outline( "" );

    // Each public node has a query message which just sends an int
    // This gets ugly.  There may exist many remote nodes with the same
    // name in differerent modules.  Of course, they also may have the
    // same name as the local public nodes.  So, the easiest method to
    // prevent generating duplicate message definitions seems to be to
    // remember each name generated and check before generating a particular
    // message to see if it already exists.  Sigh.
    struct name_chain
    {
        char* name;
        name_chain* next;
    };
    name_chain* chain = NULL;

    cur = nodes;
    while ( cur )
    {
        if ( ( cur->node_type & NODE_TYPE_PUBLIC ) ||
             ( cur->node_type & NODE_TYPE_REMOTE ) )
        {
            char* name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];

            // Check if already exists
            name_chain* p = chain;
            int exists = false;
            while ( p )
            {
                if ( strcmp( p->name, name ) == 0 )
                {
                    exists = true;
                    break;
                }
                p = p->next;
            }

            if ( !exists )
            {
                sprintf( olbuf, "    char* MESG_%s = \"N_%s\";", name, name );
                outline( olbuf );

                // Add it to the chain
                p = new name_chain;
                p->next = chain;
                p->name = name;
                chain = p;
            }
        }

        cur = cur->NextDef;
    }

    // be nice and delete the allocated memory
    name_chain* p = chain;
    while ( p )
    {
        name_chain* next = p->next;
	    delete p;

	    p = next;
    }

    outline( "" );


    // attach the links for each node
    cur = nodes;
    while ( cur )
    {
        if ( !( cur->node_type & NODE_TYPE_REMOTE ) )
        {
            n_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];

            // spin the input links
            link = cur->Links;
            while ( link != NULL )
            {
                l_name = &Symbols[ SymbolTable[ link->SymbolLoc ].SymbolLocation ];
                src_name = &Symbols[ SymbolTable[ link->SrcNode ].SymbolLocation ];

                // set the number of items in the list
                if ( link->IsList && ( link->ThisInstance == 0 ) )
                {
                    sprintf( olbuf, "    N_%s->%s_num = %d;", n_name, l_name, link->MaxInstances );
                    outline( olbuf );
                }

                // Set the source name "src_name"
                if ( SymbolTable[ link->SrcNode ].SymbolType == INITIALIZER )
                {
                    if ( link->IsList )
                    {
                        sprintf( initializer_name, "%s_%s%d", n_name, l_name, link->ThisInstance );
                    }
                    else
                    {
                        sprintf( initializer_name, "%s_%s", n_name, l_name );
                    }
                    src_name = initializer_name;
                }

                // if it is a constant, then allocate memory for it and point the
                // input towards it.  Also set the cond ptr to null to say don't
                // need to wait.
                if ( SymbolTable[ link->SrcNode ].SymbolType == NUMBER ||
                     SymbolTable[ link->SrcNode ].SymbolType == CHARACTER ||
                     SymbolTable[ link->SrcNode ].SymbolType == INITIALIZER )
                {

                    if ( link->IsList )
                    {
                        sprintf( olbuf, "    size = sizeof( *( N_%s->%s[ %d ] ) );",
                                 n_name, l_name, link->MaxInstances - link->ThisInstance - 1 );
                        outline( olbuf );

                        sprintf( olbuf, "    cnl_shared_malloc( (void**) &N_%s->%s[ %d ], size);",
                                 n_name, l_name, link->MaxInstances - link->ThisInstance - 1 );
                    }
                    else
                    {
                        sprintf( olbuf, "    size = sizeof( *( N_%s->%s ) );", n_name, l_name );
                        outline( olbuf );

                        sprintf( olbuf, "    cnl_shared_malloc( (void**) &N_%s->%s, size);", n_name, l_name );
                    }
                    outline( olbuf );


                    if ( link->IsList )
                    {
                        sprintf( olbuf, "    N_%s->%s_type[ %d ] = SRC_TYPE_CONSTANT;",
                                 n_name, l_name, link->MaxInstances - link->ThisInstance - 1 );
                        outline( olbuf );
                        sprintf( olbuf, "   *N_%s->%s[ %d ] = %s;", n_name, l_name,
                                 link->MaxInstances - link->ThisInstance - 1,
                                 src_name );
                        outline( olbuf );
                    }
                    else
                    {
                        sprintf( olbuf, "    N_%s->%s_type = SRC_TYPE_CONSTANT;",
                                 n_name, l_name );
                        outline( olbuf );
                        sprintf( olbuf, "   *N_%s->%s = %s;", n_name, l_name, src_name );
                        outline( olbuf );
                    }
                }
                else
                {
                    Symbol* mod_rec = SymbolTable[ link->SrcNode ].SymbolInfo;

                    if ( link->IsList )
                    {
                        // reverse the order of links since get inserted into list backwards
                        int index = link->MaxInstances - link->ThisInstance - 1;

                        if ( mod_rec->node_type & NODE_TYPE_REMOTE )
                        {
                            char* module_name = &Symbols[ SymbolTable[ mod_rec->process_loc ].SymbolLocation ];
                            int module_num = module_name_to_index( module_name );

                            setup_ipt = true;
                            sprintf( olbuf, "    N_%s->%s_type[ %d ] = SRC_TYPE_REMOTE;", n_name, l_name, index );
                            outline( olbuf );

                            sprintf( olbuf, "    N_%s->%s_module[ %d ] = %d;", n_name, l_name, index, module_num );
                            outline( olbuf );

                            sprintf( olbuf, "    N_%s->%s_src[ %d ] = MESG_%s;", n_name, l_name, index, src_name );
                            outline( olbuf );
                        }
                        else
                        {
                            sprintf( olbuf, "    N_%s->%s[ %d ] = &N_%s->output;", n_name, l_name, index, src_name );
                            outline( olbuf );

                            sprintf( olbuf, "    N_%s->%s_type[ %d ] = SRC_TYPE_LOCAL_NODE;", n_name, l_name, index );
                            outline( olbuf );
                            sprintf( olbuf, "    N_%s->%s_tcb[ %d ] = N_%s;", n_name, l_name, index, src_name );
                            outline( olbuf );
                            sprintf( olbuf, "    N_%s->users_.append( N_%s );", src_name,n_name );
                            outline( olbuf );

                            if ( cur->Proc->node_type & NODE_TYPE_NEW_STYLE_PROC )
                            {
                                sprintf( olbuf, "    N_%s->%s_require[ %d ] = false;", n_name, l_name, index );
                                outline( olbuf );
                            }
                        }
                    }
                    else
                    {
                        if ( mod_rec->node_type & NODE_TYPE_REMOTE )
                        {
                            char* module_name = &Symbols[ SymbolTable[ mod_rec->process_loc ].SymbolLocation ];
                            int module_num = module_name_to_index( module_name );

                            sprintf( olbuf, "    N_%s->%s_type = SRC_TYPE_REMOTE;", n_name, l_name );
                            outline( olbuf );

                            sprintf( olbuf, "    N_%s->%s_module = %d;", n_name, l_name, module_num );
                            outline( olbuf );

                            sprintf( olbuf, "    N_%s->%s_src = MESG_%s;", n_name, l_name, src_name );
                            outline( olbuf );

                            setup_ipt = true;

                        }
                        else
                        {
                            sprintf( olbuf, "    N_%s->%s = &N_%s->output;", n_name, l_name, src_name );
                            outline( olbuf );
                            sprintf( olbuf, "    N_%s->%s_type = SRC_TYPE_LOCAL_NODE;", n_name, l_name );
                            outline( olbuf );
                            sprintf( olbuf, "    N_%s->%s_tcb = N_%s;", n_name, l_name, src_name );
                            outline( olbuf );
                            sprintf( olbuf, "    N_%s->users_.append( N_%s );", src_name,n_name );
                            outline( olbuf );

                            if ( cur->Proc->node_type & NODE_TYPE_NEW_STYLE_PROC )
                            {
                                sprintf( olbuf, "    N_%s->%s_require = false;", n_name, l_name );
                                outline( olbuf );
                            }
                        }
                    }
                }

                link = link->NextDef;
            }
        }
        cur = cur->NextDef;
    }

    outline( "" );
    outline( "" );


    outline( "    reset_all_tasks();     // initialize the status flags" );
    outline( "    update_all_tasks();    // initialize the status flags" );


    // emit the call to a users init fnc
    outline( "    // call an optional users init" );
    outline( "    init( _p.argc, _p.argv );" );
    outline( "" );
    outline( "" );

    if ( !setup_ipt )
    {
        // Hmm, no remote references, lets see if we have any public nodes
        cur = nodes;
        while ( cur )
        {
            if ( cur->node_type & NODE_TYPE_PUBLIC )
            {
                // Yes
                setup_ipt = true;
                break;
            }

            cur = cur->NextDef;
        }
    }

    if ( setup_ipt )
    {
        outline( "    // look in the environment for the tcx server host machine" );
        outline( "    _ipt_home = getenv( \"TCXHOST\" );" );
        outline( "" );
        outline( "    // if none specified, the current machine will be the default" );
        outline( "    if ( _ipt_home == NULL )" );
        outline( "    {" );
        outline( "        _ipt_home = getenv( \"HOST\" );" );
        outline( "    }" );
        outline( "" );
        outline( "    // if HOST wasn't set , use the hostname as the last resort" );
        outline( "    if ( _ipt_home == NULL )" );
        outline( "    {" );
        outline( "        char hostname[ 100 ];" );
        outline( "        if ( gethostname( hostname, 100 ) != 0 )" );
        outline( "        {" );
        outline( "            printf( \"Aborting: Unable to determine host for TCX server\n\" );" );
        outline( "            printf( \"Set environment variable TCXHOST\n\" );" );
        outline( "            exit( 1 );" );
        outline( "        }" );
        outline( "        _ipt_home = hostname;" );
        outline( "    }" );

        // Generate the tcx message array
        outline( "    IPMessageSpec messageArray[]={" );

        // Each public node has a query message which just sends an int
        cur = nodes;
        while ( cur )
        {
            if ( (cur->node_type & NODE_TYPE_PUBLIC) ||
                 (cur->node_type & NODE_TYPE_REMOTE))
            {
                n_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];

                sprintf( olbuf, "        { MESG_%s, FRM_Query },", n_name );
                outline( olbuf );
            }

            cur = cur->NextDef;
        }
        outline( "" );

        // Each data type has a cooresponding message
        cur = Type_Chain;
        while ( cur )
        {
            if ( cur->SymbolLoc != void_type )
            {
                char* type_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];

                sprintf( olbuf, "        { MESG_%s, FRM_%s },", type_name, type_name );
                outline( olbuf );
            }

            cur = cur->NextDef;
        }

        outline( "        { NULL, NULL }" );

        outline( "    };" );

        outline( "" );

        // Generate the tcx handler array
        outline( "    IPMsgHandlerSpec hndArray[] = {" );
        cur = nodes;
        while ( cur )
        {
            if ( cur->node_type & NODE_TYPE_PUBLIC )
            {
                proc = cur->Proc;

                n_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];
                p_name = &Symbols[ SymbolTable[ proc->SymbolLoc ].SymbolLocation ];

                sprintf( olbuf, "        { MESG_%s, _cb_%s, IPT_HNDL_ALL, (void*) N_%s }", n_name, p_name, n_name );
                outline( olbuf );
            }

            cur = cur->NextDef;
        }
        outline( "    };" );

        outline( "" );


        outline( "    char ipt_port[ 256 ];" );

        sprintf( olbuf,"    sprintf( ipt_port, \"%%s_CNL_%s\", _prefix );", ( module_name != NULL ) ? module_name : "" );
        outline( olbuf );
        outline( "    _communicator = IPCommunicator::Instance( strdup(ipt_port), _ipt_home );" );

        outline( "    _communicator->RegisterNamedFormatters( formatArray );" );
        outline( "    _communicator->RegisterMessages( messageArray );" );

        outline( "    for ( i = 0; i < (sizeof( hndArray) / sizeof( IPMsgHandlerSpec) ); i++)" );
        outline( "    {" );
        outline( "        _communicator->RegisterHandler( _communicator->LookupMessage( hndArray[ i ].msg_name ), " );
        outline( "                                        hndArray[ i ].callback, hndArray[ i ].data )" );
        outline( "    }" );

        // Allocate memory for the module pointers
        sprintf( olbuf, "    cnl_shared_malloc( (void**) &_ipt_modules, sizeof( IPConnection* ) * %d);",
                 num_modules );
        outline( olbuf );

        // Connect to the requested modules
        cur = Module_Chain;
        cnt = 0;
        while ( cur )
        {
            char* name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];

            sprintf( olbuf,"    sprintf( ipt_port, \"%%s_CNL_%s\", _prefix );", name );
            outline( olbuf );

            sprintf( olbuf, "    _ipt_modules[ %d ] = _communicator->Connect( ipt_port );", cnt );
            outline( olbuf );

            cnt++;
            cur = cur->NextDef;
        }

        outline( "    _ipt_is_active = 1;" );
    }

    outline( "" );

// emit the cthread forks for the nodes
    num_threads = 0;
    cur = nodes;
    while ( cur )
    {
        proc = cur->Proc;
        if ( !( cur->node_type & NODE_TYPE_REMOTE ) )
        {
            num_threads++;
	}
	cur = cur->NextDef;
    }

            sprintf( olbuf, "    pthread_t threads[%d];", num_threads);
            outline( olbuf );

    // emit the cthread forks for the nodes
    num_threads = 0;
    cur = nodes;
    while ( cur )
    {
        proc = cur->Proc;
        if ( !( cur->node_type & NODE_TYPE_REMOTE ) )
        {
            num_threads++;

            n_name = &Symbols[ SymbolTable[ cur->SymbolLoc ].SymbolLocation ];
            char* n_desc = cur->description;
            p_name = &Symbols[ SymbolTable[ proc->SymbolLoc ].SymbolLocation ];

            if ( n_desc != NULL )
            {
                sprintf( olbuf, "    N_%s->set_name( \"%s:%s\" );",n_name,n_desc,p_name );
            }
            else
            {
                sprintf( olbuf, "    N_%s->set_name( \"%s:%s\" );",n_name,n_name,p_name );
            }
            outline( olbuf );


            sprintf( olbuf, "    pthread_create( &(threads[%d]), NULL,  %s, N_%s);", num_threads-1 ,p_name, n_name );
            outline( olbuf );
            outline( "" );
        }

        cur = cur->NextDef;
    }

    outline("int i; void *value;");
    sprintf(olbuf,"for(i=0;i<%d;i++)",num_threads);
    outline(olbuf);
    outline("pthread_join(threads[i],&value);");

    outline( "} // End of init function" );

    return num_threads;
}



/**********************************************************************/

int end_nodes( int num_threads )
{
    if ( had_error )
    {
        return 0;
    }

    outline( "" );
    outline( "" );
    outline( "// main" );
    outline( "int main( int argc,char** argv )" );
    outline( "{" );
    outline( "    int rtn;" );
    //outline( "    configuration c;\n" );
    //outline( "    cthread_configure( &c, GET_CONFIG );" );
    //outline( "    c.memory_exponent = 16; // 64K" );

    // need to add an extra thread for the main cthreads thread
    //sprintf( olbuf, "    c.threads_per_proc = %d;", num_threads + 1 );
    //outline( olbuf );
    //outline( "    cthread_configure( &c, PUT_CONFIG );\n" );
    //outline( "    argc = cthread_parse_args( argc, argv );" );


    outline( "    _p.argc = argc;" );
    outline( "    _p.argv = argv;" );

    outline( "init_fnc();" );

    outline( "} // end of main" );

    return 0;
}



/**********************************************************************
 * $Log: codegen.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/05/15 06:23:09  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2005/02/07 23:45:47  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:38  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.58  2003/04/06 11:42:44  endo
 * gcc 3.1.1
 *
 * Revision 1.57  2002/07/02 20:24:32  blee
 * Boolean behaviors will now output only 'false' if mlab
 * is drawing a vector field.
 *
 * Revision 1.56  2001/12/22 16:06:08  endo
 * RH 7.1 porting.
 *
 * Revision 1.55  2000/04/13 20:40:10  endo
 * Check in for Doug.
 * This patch makes the name of the node available in CNL procedures
 * via a new builtin string constant called "node_name",
 * and corrects the execution semantics so that the HEADER block executes
 * once and only once each time the node gains scope.
 * The ONCE block continues to only execute once on program load.
 *
 * Revision 1.54  1999/12/16 23:30:01  endo
 * rh-6.0 porting.
 *
 * Revision 1.53  1996/06/01  21:53:55  doug
 * *** empty log message ***
 *
 * Revision 1.52  1996/05/05  17:59:22  doug
 * fixing warnings
 *
 * Revision 1.51  1996/05/03  17:48:33  doug
 * fixed for scoping problem
 *
 * Revision 1.50  1996/04/17  15:47:46  doug
 * Fixed while loop problem in generated code that ignored blocking
 *
 * Revision 1.49  1996/02/25  16:56:17  doug
 * made header of the procedure execute when ever the fnc becomes unblocked
 *
 * Revision 1.48  1996/02/14  17:12:53  doug
 * added support for descriptions
 * ./
 *
 * Revision 1.47  1996/02/09  01:49:36  doug
 * *** empty log message ***
 *
 * Revision 1.46  1995/11/08  16:49:34  doug
 * *** empty log message ***
 *
 * Revision 1.45  1995/11/04  18:57:01  doug
 * *** empty log message ***
 *
 * Revision 1.44  1995/11/02  23:30:14  doug
 * *** empty log message ***
 *
 * Revision 1.43  1995/11/01  23:00:28  doug
 * added the tcb class
 *
 * Revision 1.42  1995/10/27  20:30:09  doug
 * *** empty log message ***
 *
 * Revision 1.41  1995/10/18  18:20:49  doug
 * finalizing the skipping code
 *
 * Revision 1.40  1995/10/18  13:59:36  doug
 * added support for new style procs
 *
 * Revision 1.39  1995/06/15  21:57:52  jmc
 * Added RCS ID and Log strings.
 **********************************************************************/
