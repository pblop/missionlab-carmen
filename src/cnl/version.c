/**********************************************************************
 **                                                                  **
 **                           version.c                              **
 **                                                                  **
 ** Compile the version and compile time strings separately so they  **
 ** can be updated efficiently everytime console is compiled.        **
 **                                                                  **
 ** By: Jonathan M. Cameron                                          **
 **                                                                  **
 ** Copyright 1995, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: version.c,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include <stdio.h>

char *version_str = "4.0";

char compile_time_str[40];


char *compile_time()
{
   sprintf(compile_time_str, "%s", DATE);
   return compile_time_str;
}



/**********************************************************************
 * $Log: version.c,v $
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
 * Revision 1.28  1996/02/25  16:06:24  doug
 * Version 4.0 executes the header block each time the procedure
 * is restarting after being blocked as well as the first initial pass
 *
 * Revision 1.27  1996/02/14  17:12:15  doug
 * 3.2 adds support for descriptions to help debugging cdl code
 *
 * Revision 1.26  1996/02/08  09:17:28  doug
 * 3.1 removes the need to run the preprocessor.
 * The cnl parser now skims through any include files to look
 * for CNL stuff.  It does not expand them.
 * The -p flag has now been flipped in logic so that it enables
 * the preprocessor since it is now off by default.
 *
 * Revision 1.25  1995/10/17  16:05:33  doug
 * version 3.0 will only execute nodes that are either void outputs
 * or were their outputs were consumed.
 * the procedure def consumes all inputs, the nprocedure def requires
 * the user to explicitly consume the inputs.
 *
 * Revision 1.24  1995/10/17  15:34:26  doug
 * version 2.2 support the new style procedures which do not automatically
 * consume the inputs.  The users code must mark those inputs consumed.
 * This allows not scheduling all nodes.  Only those whose outputs are
 * consumed will be executed each cycle.
 *
 * Revision 1.23  1995/08/22  15:24:45  doug
 * Modify to allow running from cfgedit easier
 *
 * Revision 1.22  1995/05/03  17:52:23  doug
 * switched to g++ version of getopt
 * Allows permuted arguments
 *
 * Revision 1.21  1995/05/02  19:20:29  doug
 * Support list of initilizers
 *
 * Revision 1.20  1995/04/21  22:38:22  doug
 * allow leading $ in names
 * allow numbers to have 1 pair of surrounding parens
 *
 * Revision 1.19  1995/04/20  19:35:21  doug
 * fixed a bug that only allowed a single procedure definition
 * per file if it had an extern prototype.  Never showed up since
 * libraries are usually one function per file anyway.
 *
 * Revision 1.18  1995/04/04  14:44:34  doug
 * Add support for -Dxxx to define xxx to the preprocessor.
 *
 * Revision 1.17  1995/03/31  21:12:40  jmc
 * Added copyright notice.
 *
 * Revision 1.16  1995/03/30  17:55:19  doug
 * use a prototype for malloc instead of including stdlib
 *
 * Revision 1.15  1995/03/30  15:28:19  doug
 * fixed problems caused by upgrade to newer gcc version
 *
 * Revision 1.14  1995/03/28  18:55:11  doug
 * 2.02 supports the -p option to not run the preprocessor
 * It is useful for debugging
 *
 * Revision 1.13  1995/03/23  20:06:42  doug
 * Version 2.01 addes the ability to specify multiple file names to the
 * CNL compiler.
 *
 * Revision 1.12  1995/03/14  22:11:53  doug
 * Converted to use plug & socket classes
 * Supports remote links using tcx
 *
 * Revision 1.11  1995/01/30  16:09:34  doug
 * In generated .cc file, make #define of NULL contingent on not already defined
 *
 * Revision 1.11  1995/01/30  15:55:58  doug
 * make definition of NULL in generated .cc file ifdef'd on not already defined
 *
 * Revision 1.10  1995/01/16  14:57:58  doug
 * max name size is expanded to 256 from 64 characters
 *
 * Revision 1.9  1994/11/18  19:57:23  doug
 * fixed bug declaring initializers - only worked for first node
 *
 * Revision 1.8  1994/11/16  21:05:32  doug
 * Call Attach_links even in case where didn't define any links
 * to catch the case where forgot to define all the links.
 *
 * Revision 1.7  1994/11/09  13:50:20  doug
 * Handle undefined procedures better
 *
 * Revision 1.6  1994/11/08  16:45:17  doug
 * Added test that input link assigned a value actually exists in the procedure definition
 *
 * Revision 1.5  1994/11/04  02:27:09  doug
 * Added condition wait at end of each node to synchronize the tree
 *
 * Revision 1.4  1994/11/02  18:28:00  doug
 * Added more meaningful error message for redeclaring a type
 *
 * Revision 1.3  1994/11/01  20:29:26  doug
 * Handle pragmas
 *
 * Revision 1.2  1994/10/24  19:06:40  doug
 * moved to version 1.1
 * Adding #line defines to get back to the generated file line numbers
 * for code I generate.
 *
 * Revision 1.1  1994/10/19  14:07:18  doug
 * Initial revision
 *
 * Revision 1.2  1994/09/09  21:40:16  jmc
 * Converted interface to compile time string to a function.
 *
 * Revision 1.1  1994/09/09  19:31:19  jmc
 * Initial revision
 **********************************************************************/
