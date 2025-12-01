/**********************************************************************
 **                                                                  **
 **                       main.cc                                    **
 **                                                                  **
 **  main function for CDL compiler                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: main.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <fcntl.h>
#include <new>
#include <string>

#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

#include "load_cdl.h"
#include "load_rc.h"
#include "codegens.h"
#include "utilities.h"
#include "version.h"
#include "cfgedit_common.h"
#include "ExpandSubtrees.h"

using std::string;
using std::set_new_handler;

bool debug_load_cdl = false;
bool debug_save = false;

int cdl_debug;
bool verbose = false;

bool SHOW_SLIDER_VALUES = false;


char include_paths[8][256];
char defines[8][256];
char **filenames;
int num_include_paths = 0;
int num_defines = 0;
int compile;
unsigned long our_pid;
int num_files;
int cur_file;
int generate_line_defines = true;

// Declare the rc table
symbol_table<rc_chain> rc_table(47);


char *command_line = NULL;
int generated_cnl_files = 0;

void gSavePickedOverlayName(string overlayName) {} // Do nothing for cdl.

/*************************************************************************
*                                                                        *
*                        main routine                                    *
*                                                                        *
*************************************************************************/

const char *copyright_str =
#include "copyright"
 ;

int
main(int argc, char **argv)
{
    int run_preprocessor = true;
    char ccp_call[4096];

    set_new_handler(abort);

    for (int i = 0; i < argc; i++)
    {
        if (i > 0)
            command_line = strdupcat(command_line, " ");

        command_line = strdupcat(command_line, argv[i]);
    }

    cdl_debug = 0;
    debug_save = false;

    //************ parse the command line parameters ********************
        int option_char;
        char *out_prefix = NULL;

        const char *rc_filename = RC_FILENAME;
        bool no_path_search = false;

        //change by ellenber
        while ((option_char = getopt(argc, argv, "D:I:c:o:plgv")) != EOF)
        {
            switch (option_char)
            {
            case 'I':
                strcpy(include_paths[num_include_paths++], optarg);
                break;

            case 'D':
                strcpy(defines[num_defines++], optarg);
                break;

            case 'c':
                rc_filename = optarg;
                no_path_search = true;
                cerr << "Using configuration file " << rc_filename << '\n';
                break;

            case 'o':
                out_prefix = strdup(optarg);
                break;

            case 'p':
                run_preprocessor = false;
                generate_line_defines = false;
                break;

            case 'l':
                debug_load_cdl = true;
                cerr << "Debugging of cdl parser enabled\n";
                break;

            case 'g':
                debug_save = true;
                cerr << "Debugging of code generators is enabled\n";
                break;

            case 'v':
                fprintf(stderr, "\n%s\n\n", copyright_str);
                fprintf(stderr, "cdl version %s\n", version_str);
                fprintf(stderr, "compiled %s\n", compile_time());
                verbose = true;
                break;
            }
        }

        //*****************************************************************

            // Load the RC file
            int rtn = load_rc(rc_filename, &rc_table, no_path_search, verbose, false);
            if( rtn == 2 )
            {
                cerr << "Warning: Didn't find " << rc_filename << '\n';
            }

            // ************************************************************

            if (verbose)
            {
                fprintf(stderr, "\nRC List:\n");

                const char     *key;
                const rc_chain *val;

                bool valid = rc_table.first(&key,&val);
                while( valid )
                {
                    fprintf(stderr, "\t%s\t=\t", key);

                    char *str;
                    void *p = val->first(&str);
                    while( p )
                    {
                        fprintf(stderr, "%s", str);
                        p = val->next(&str, p);

                        if( p )
                            fprintf(stderr, ",");
                    }
                    fprintf(stderr, "\n");

                    valid = rc_table.next(&key,&val);
                };

                fprintf(stderr, "\n");
            }


            /* ------------------------------------------------------------- */

            // Init the parser
            if (init_cdl())
            {
                fprintf(stderr, "Error: unable to initialize the parser!\n");
                exit(1);
            }


            /* ------------------------------------------------------------- */
            // Load any libraries that were specified

            if( verbose )
                fprintf(stderr, "\nLoading libraries:\n");

            // Extract the library options
            const rc_chain *files = rc_table.get(CDL_LIBRARIES);
            if( files ) 
            {
                char *str;
                void *pos = files->first(&str);
                while( pos )
                {  
                    char *fullname = strdupcat(strdup(str),".gen");

                    FILE *chk = fopen(fullname,"r");
                    if( chk == NULL )
                    {
                        cerr << "Warning: Didn't find " << fullname << '\n';
                    }
                    else
                    {
                        fclose(chk);

                        if( verbose )
                            cerr << "Loading " << fullname << " ...";
                        const char *namelist[2];
                        namelist[0] = fullname;
                        namelist[1] = NULL;
                        bool errors;

                        load_cdl(namelist,errors,true,debug_load_cdl > 1);

                        if( verbose )
                            cerr << " Done\n";
                        free(fullname);

                        // Try to load any architecture specific versions of the library
                        Symbol *arch;
                        void *cur = defined_architectures.first(&arch);
                        while( cur )
                        {
                            char *fullname = strdupcat(strdup(str),".", arch->name);

                            FILE *chk = fopen(fullname,"r");
                            if( chk )
                            {
                                fclose(chk);

                                if( verbose )
                                    cerr << "Loading " << fullname << " ...";
                                const char *namelist[2];
                                namelist[0] = fullname;
                                namelist[1] = NULL;
                                bool errors;
                                load_cdl(namelist,errors,true,debug_load_cdl > 1);

                                if( verbose )
                                    cerr << " Done\n";

                                free(fullname);
                            }

                            cur = defined_architectures.next(&arch, cur);
                        }
                    }

                    pos = files->next(&str, pos);
                }
            }

            /* ------------------------------------------------------------- */
            // Mark that any new objects are at the user level
            set_user_class();

            /************** open the input and output files ********************/

            our_pid = getpid();

            num_files = argc - optind;
            cur_file = optind;

            if (num_files <= 0)
            {
                fprintf(stderr, "Error: No input files specified\n");
                exit(1);
            }

            /******************** preprocess **************************/

            filenames = &argv[optind];

            if (verbose)
                fprintf(stderr, "\nPreprocess Pass\n");

            for (int file_num = 0; file_num < num_files; file_num++)
            {
                char *inname = argv[optind + file_num];
                char outname[1024];

                sprintf(outname, "/tmp/cdl_%ld_%d.cpp", our_pid, file_num);

                // Check that file exists
                FILE *in = fopen(inname, "r");

                if (in == NULL)
                {
                    char buf[256];
                    sprintf(buf, "Error: Can't open file %s\n", inname);
                    perror(buf);
                    exit(1);
                }
                fclose(in);


                if (run_preprocessor)
                {
                    // Set executable name
                    char *prog = "gcc";

                    // Create a communications pipe to talk to the child processes
                    int pid;
                    int fd[2];

                    if (pipe(fd) < 0)
                    {
                        perror("Error: Unable to create communication pipe");
                        exit(1);
                    }

                    if (fcntl(fd[0], F_SETFL, FNDELAY) == -1)
                    {
                        perror("Error: Unable to set communication pipe to non-blocking");
                        exit(1);
                    }

                    if ((pid = fork()) == 0)
                    {
                        // Redirect stderr back to parent
                        if (dup2(fd[1], STDERR_FILENO) == -1)
                        {
                            perror("Error: unable to redirect stderr back to caller");
                            exit(1);
                        }
                        if (fcntl(STDERR_FILENO, F_SETFD, 0) == -1)
                        {
                            perror("Error: Unable to config communication pipe");
                            exit(1);
                        }
                        if (fcntl(fd[1], F_SETFD, 0) == -1)
                        {
                            perror("Error: Unable to config communication pipe");
                            exit(1);
                        }

                        char buf[2048];
                        char *argv[64];
                        int p = 0;
                        int i;

                        /* in child */

                        argv[p++] = prog;

                        argv[p++] = "-E";

                        for (i = 0; i < num_include_paths; i++)
                        {
                            sprintf(buf, "-I%s", include_paths[i]);
                            argv[p++] = strdup(buf);
                        }

                        for (i = 0; i < num_defines; i++)
                        {
                            sprintf(buf, "-D%s", defines[i]);
                            argv[p++] = strdup(buf);
                        }

                        argv[p++] = "-x";
                        argv[p++] = "c++";
                        argv[p++] = inname;
                        argv[p++] = "-o";
                        argv[p++] = outname;

                        // Mark end of parm list
                        argv[p] = NULL;

                        // Show the user
                        if (verbose)
                        {
                            char *cmd = strdup(prog);

                            for (i = 1; i < p; i++)
                            {
                                cmd = strdupcat(cmd, " ");
                                cmd = strdupcat(cmd, argv[i]);
                            }
                            fprintf(stderr, "%s\n", cmd);
                            free(cmd);
                        }

                        if (execvp(prog, argv))
                        {
                            char msg[256];

                            sprintf(msg, "Error: Unable to exec %s", prog);
                            perror(msg);
                        }

                        // Kill this child
                        exit(2);
                    }

                    if (pid == -1)
                    {
                        perror("Error: Unable to fork");
                        exit(1);
                    }
                    else
                    {
                        // wait for compile to finish
                        char buf[256];
                        bool done = false;

                        while (!done)
                        {
                            // Try to read from the com pipe
                            int size = read(fd[0], buf, sizeof(buf));

                            int pos = 0;

                            while (pos < size)
                            {
                                pos += write(STDERR_FILENO, &buf[pos], size - pos);

                            }

                            int statusp;
                            int options = WNOHANG | WUNTRACED;
                            int rtn = waitpid(pid, &statusp, options);

                            if (rtn == pid)
                            {
                                done = true;

                                if (WIFEXITED(statusp))
                                {
                                    int exit_status = WEXITSTATUS(statusp);

                                    if (exit_status != 0)
                                    {
                                        fprintf(stderr, "Error: %s exited with status %d\n",
                                                prog, exit_status);
                                        exit(1);
                                    }
                                }
                                else if (WIFSIGNALED(statusp))
                                {
                                    fprintf(stderr, "Error: %s died with signal: %d\n",
                                            prog, WTERMSIG(statusp));
                                    exit(1);
                                }
                            }
                            else if (rtn == -1)
                            {
                                char msg[256];
                                sprintf(msg, "Error: Received signal from %s", prog);
                                perror(msg);
                                exit(1);
                            }
                        }
                    }
                }
                else
                {
                    sprintf(ccp_call, "cp %s %s \n", inname, outname);
                    int rtn = system(ccp_call);

                    if (verbose)
                        fprintf(stderr, "%s", ccp_call);

                    if (rtn & 0xff)
                    {
                        fprintf(stderr, "Error: Unable to exec ccp %d: '%s'\n", (rtn & 0xff), ccp_call);
                        exit(1);
                    }

                    rtn = rtn >> 8;
                    if (rtn)
                    {
                        fprintf(stderr, "Error from ccp %d: '%s'\n", rtn, ccp_call);
                        exit(1);
                    }
                }

                // check that have the preprocessed file
                FILE *test = fopen(outname, "r");

                if (test == NULL)
                {
                    fprintf(stderr, "Error: Preprocess step failed to create %s\n", outname);
                    exit(1);
                }
                fclose(test);
            }

            /************** load the input files ********************/

            const char **filenames = new const char *[num_files + 1];

            int cnt;
            for (cnt = 0; cnt < num_files; cnt++)
            {
                char buf[2048];

                sprintf(buf, "/tmp/cdl_%ld_%d.cpp", our_pid, cnt);
                filenames[cnt] = strdup(buf);
            }
            filenames[cnt] = NULL;

            bool errors;
            Symbol *tree = load_cdl(filenames, errors, false, debug_load_cdl);

            // Expand any linked subtrees with pushed up parms
            ExpandSubtrees(tree);

            /************** generate the code ********************/

            bool ok = true;
            if (tree != NULL)
            {
                if (choosen_architecture == NULL)
                {
                    SemanticError("Didn't specify an architecture");
                }
                else
                {
                    if (strcmp(choosen_architecture->name, "UGV") == 0)
                    {
                        ok = SAUSAGES_codegen(tree, "plan.lisp");
                    }
                    else if ((strcmp(choosen_architecture->name, "AuRA") == 0) ||
                             (strcmp(choosen_architecture->name, "AuRA.urban") == 0) ||
                             (strcmp(choosen_architecture->name, "AuRA.naval") == 0))
                    {
                        ok = cnl_codegen(tree, out_prefix);
                    }
                    else
                    {
                        SemanticError("Unknown code generator");
                    }
                }
            }

            if (!ok)
            {
                cerr << "Error generating output\n";
                exit(1);
            }

            //********************** clean up *********************

                // Delete the temp files from the preprocessor
                if (tree && !verbose)
                {
                    for (int file_num = 0; file_num < num_files; file_num++)
                    {
                        char buf[2048];

                        sprintf(buf, "/tmp/cdl_%ld_%d.cpp", our_pid, file_num);

                        unlink(buf);
                    }
                }

                /************** done ********************/
                // Do not modify this sentence. CfgEdit expects this string
                // during the compilation of the robot(s).
                cerr << "CDL compiler has generated " << generated_cnl_files << " CNL file(s).\n";
                exit(0);
}




/**********************************************************************
 * $Log: main.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/09/07 23:17:55  endo
 * The overlay name is now remembered when the coordinates are picked from an overlay (CfgEdit only).
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/05/18 21:21:41  endo
 * AuRA.naval added.
 *
 * Revision 1.1.1.1  2005/02/06 22:59:28  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.53  2003/04/06 13:11:30  endo
 * gcc 3.1.1
 *
 * Revision 1.52  2000/10/16 19:36:57  endo
 * Modified due to the compiler upgrade.
 *
 * Revision 1.51  2000/03/20 00:28:53  endo
 * The bug of CfgEdit failing to compile multiple robots
 * was fixed.
 *
 * Revision 1.50  1999/12/16 22:03:30  endo
 * rh-6.0 porting.
 *
 * Revision 1.49  1999/09/03 21:07:54  endo
 * (strcmp(choosen_architecture->name, "AuRA.urban")
 *
 * Revision 1.48  1999/03/08 22:34:16  endo
 * *** empty log message ***
 *
 * Revision 1.47  1996/05/14 23:16:56  doug
 * *** empty log message ***
 *
 * Revision 1.46  1996/05/06  03:07:32  doug
 * fixing compiler warnings
 *
 * Revision 1.45  1996/04/11  04:07:58  doug
 * *** empty log message ***
 *
 * Revision 1.44  1996/04/06  23:54:56  doug
 * *** empty log message ***
 *
 * Revision 1.43  1996/03/13  01:52:36  doug
 * fixed error reporting
 *
 * Revision 1.42  1996/03/08  00:46:41  doug
 * *** empty log message ***
 *
 * Revision 1.41  1996/03/06  23:39:08  doug
 * *** empty log message ***
 *
 * Revision 1.40  1996/02/26  05:02:44  doug
 * *** empty log message ***
 *
 * Revision 1.39  1996/01/17  18:46:26  doug
 * *** empty log message ***
 *
 * Revision 1.38  1995/12/05  17:26:04  doug
 * upgrade to new style library names
 *
 * Revision 1.37  1995/11/14  15:44:46  doug
 * fixed so compiles under linux
 *
 * Revision 1.36  1995/10/31  19:24:23  doug
 * now uses the resource file
 *
 * Revision 1.35  1995/10/30  23:04:00  doug
 * *** empty log message ***
 *
 * Revision 1.34  1995/10/11  22:03:06  doug
 * *** empty log message ***
 *
 * Revision 1.33  1995/10/10  20:43:45  doug
 * *** empty log message ***
 *
 * Revision 1.32  1995/10/09  21:57:34  doug
 * *** empty log message ***
 *
 * Revision 1.31  1995/08/24  21:05:13  doug
 * *** empty log message ***
 *
 * Revision 1.30  1995/08/22  15:29:27  doug
 * Modified so is callable from cfgedit
 *
 * Revision 1.29  1995/08/21  16:18:12  doug
 * add support for -o flag so can set output file prefix
 *
 * Revision 1.28  1995/08/18  22:15:08  doug
 * making some printf's write to stderr
 *
 * Revision 1.27  1995/06/29  15:00:14  jmc
 * Added RCS log string.
 **********************************************************************/
