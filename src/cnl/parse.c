/*
 * Douglas C. MacKenzie
 *
 * parse.c ----  C routines used by gram.y for CNL ----
 *
 *    Copyright 1995, Georgia Tech Research Corporation
 *    Atlanta, Georgia  30332-0415
 *    ALL RIGHTS RESERVED, See file COPYRIGHT for details.
 *
 *    $Id: parse.c,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

#include "defs.h"
#include "cnl_gram.tab.h"
#include "version.h"

char outname[256] = "stdout";
char logname[256];
char include_paths[8][256];
char defines[8][256];
char *inname;
char **filenames;
int num_include_paths = 0;
int num_defines = 0;
FILE *outfile = 0;
FILE *logfile;
int pass;
int cnl_debug;
int compile;
int void_type = -1;
int had_error = FALSE;
int logging = FALSE;
int verbose = FALSE;
unsigned long our_pid;
int num_files;
int cur_file;

int generate_line_defines = TRUE;
int run_preprocessor = FALSE;
bool no_builtin_bool = false;



/*********************************************************************
*                                                                     *
*********************************************************************/

char *
strdupcat(char *a, const char *b)
{
   if (a == NULL)
      return strdup(b);

   int len1 = strlen(a);
   int len2 = strlen(b);
   char *out = (char *) malloc(len1 + len2 + 1);

   strcpy(out, a);
   strcpy(&out[len1], b);
   free(a);

   return out;
}

//*********************************************************************

void
set_file(int num)
{
   if (num > 0)
      fclose(yyin);

   char buf[2048];

   sprintf(buf, "/tmp/cnl_%ld_%d.cpp", our_pid, num);
   yyin = fopen(buf, "r");
   if (yyin == NULL)
   {
      char out[256];

      sprintf(out, "Fatal: Can't open temporary file %s\n", buf);
      perror(buf);
      exit(1);
   }
   inname = filenames[num];
   cur_file = num;

   if (verbose)
      fprintf(stderr, "Reading file %s\n", buf);
}

void
load_first_file(void)
{
   set_file(0);
}

/*************************************************************************
*                                                                        *
*                        main routine                                    *
*                                                                        *
*************************************************************************/

int
main(int argc, char **argv)
{
   int Loc;
   int len;
   int p1_yydebug = 0;
   int p2_yydebug = 0;
   int p1_cnldebug = 0;
   int p2_cnldebug = 0;
   char ccp_call[4096];

	 outfile = stdout;

   char *copyright_str =
#include "copyright"
    ;

   compile = 0;
   // modified by mjcramer
   int c;
   while( (c = getopt(argc, argv, "ABb12dD:l:o:cI:vpg") ) != EOF )
   {
      switch (c)
       {
       case '1':
	  p1_yydebug = 1;
	  break;

       case '2':
	  p2_yydebug = 1;
	  break;

       case 'I':
	  strcpy(include_paths[num_include_paths++], optarg);
	  break;

       case 'D':
	  strcpy(defines[num_defines++], optarg);
	  break;

       case 'A':
	  p1_cnldebug = 1;
	  break;

       case 'B':
	  p2_cnldebug = 1;
	  break;

       case 'b':
	  no_builtin_bool = true;
	  break;

       case 'c':
	  compile = 1;
	  break;

       case 'd':
	  p1_yydebug = 1;
	  p2_yydebug = 1;
	  break;

       case 'l':
	  logging = TRUE;
	  strcpy(logname, optarg);
	  logfile = fopen(logname, "w");
	  if (logfile == NULL)
	  {
	     fprintf(stderr, "Can't open %s\n", logname);
	     exit(1);
	  }
	  fclose(logfile);
	  freopen(logname, "w+", stderr);
	  break;

       case 'o':
	  strcpy(outname, optarg);
	  outfile = fopen(outname, "w");
	  if (outfile == NULL)
	  {
	     fprintf(stderr, "Can't open %s\n", outname);
	     exit(1);
	  }
	  break;

       case 'p':
	  run_preprocessor = TRUE;
	  break;

       case 'g':
	  generate_line_defines = FALSE;
	  break;

       case 'v':
	  fprintf(stderr, "\n");
	  fprintf(stderr, "cnl version %s\n", version_str);
	  fprintf(stderr, "compiled %s\n", compile_time());
	  fprintf(stderr, "\n%s\n\n", copyright_str);
	  verbose = TRUE;
	  break;
       }
   }

   our_pid = getpid();

   num_files = argc - optind;
   cur_file = optind;

   if (num_files <= 0)
   {
      fprintf(stderr, "No input file specified\n");
      exit(1);
   }

   if (outfile == stdout)
   {
      strcpy(outname, argv[optind]);

      /* see if input filename ends in ".cnl" */
      len = strlen(outname);
      if (len > 4 && strcmp(&outname[len - 4], ".cnl") == 0)
      {
	 /* delete the "nl" to make it end in ".cc" */
	 outname[len - 2] = 'c';
	 outname[len - 1] = '\0';
      }
      else
      {
	 /* add a ".cc" to the end */
	 outname[len++] = '.';
	 outname[len++] = 'c';
	 outname[len++] = 'c';
	 outname[len] = '\0';
      }

      outfile = fopen(outname, "w");
      if (outfile == NULL)
      {
	 fprintf(stderr, "Can't open %s\n", outname);
	 exit(1);
      }
   }

   /* print the header at the top of the output file */
   output_header(argc, argv);

   /* init code */
   for (Loc = 0; Loc < SymbolTableSize; Loc++)
   {
      SymbolTable[Loc].SymbolInfo = NULL;
   }

   AddType("int", "int");
   AddType("char", "char");
   AddType("short", "short");
   AddType("long", "int");
   AddType("float", "float");
   AddType("double", "double");
   AddType("unsigned", "int");
   AddType("signed", "int");
   void_type = AddType("void", "NONE");

/******************** preprocess **************************/

   filenames = &argv[optind];

   if (verbose)
      fprintf(stderr, "\nPreprocess Pass\n");

   for (int file_num = 0; file_num < num_files; file_num++)
   {
      char *inname = argv[optind + file_num];
      char outname[1024];

      sprintf(outname, "/tmp/cnl_%ld_%d.cpp", our_pid, file_num);

      // Check that file exists
      FILE *in = fopen(inname, "r");

      if (in == NULL)
      {
	 char buf[256];

	 sprintf(buf, "Can't open file %s\n", inname);
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
	    perror("CNL: Unable to create communication pipe");
	    exit(1);
	 }

	 if (fcntl(fd[0], F_SETFL, FNDELAY) == -1)
	 {
	    perror("CNL: Unable to set communication pipe to non-blocking");
	    exit(1);
	 }

	 if ((pid = fork()) == 0)
	 {
	    // Redirect stderr back to parent
	    if (dup2(fd[1], STDERR_FILENO) == -1)
	    {
	       perror("CNL: unable to redirect stderr back to caller");
	       exit(1);
	    }
	    if (fcntl(STDERR_FILENO, F_SETFD, 0) == -1)
	    {
	       perror("CNL: Unable to config communication pipe");
	       exit(1);
	    }
	    if (fcntl(fd[1], F_SETFD, 0) == -1)
	    {
	       perror("CNL: Unable to config communication pipe");
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

	       sprintf(msg, "Unable to exec %s", prog);
	       perror(msg);
	    }

	    // Kill this child
	    exit(2);
	 }

	 if (pid == -1)
	 {
	    perror("Unable to fork");
	    exit(1);
	 }
	 else
	 {
	    // wait for compile to finish
	    char buf[256];

	    bool done = false;
            bool empty_pipe = false;
	    while (!done || !empty_pipe)
	    {
	       // Try to read from the com pipe
	       int size = read(fd[0], buf, sizeof(buf));
	       if( size < 0 )
	       {
		  empty_pipe = true;
	       }
	       else if( size > 0 )
	       {
		  empty_pipe = false;

	          int pos = 0;

	          while (pos < size)
	          {
		     pos += write(STDERR_FILENO, &buf[pos], size - pos);

	          }
	       }

               if( !done )
	       {
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
			fprintf(stderr, "%s exited with status %d\n",
				prog, exit_status);
			exit(1);
		     }
		  }
		  else if (WIFSIGNALED(statusp))
		  {
		     // Error
		     fprintf(stderr, "%s died with signal: %d\n",
			     prog, WTERMSIG(statusp));
		     exit(1);
		  }
	       }
	       else if (rtn == -1)
	       {
		  char msg[256];

		  sprintf(msg, "Received signal from %s", prog);
		  perror(msg);
		  exit(1);
	       }
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
	    fprintf(stderr, "Unable to exec ccp %d: '%s'\n", (rtn & 0xff), ccp_call);
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
	 fprintf(stderr, "Preprocess step failed to create %s\n", outname);
	 exit(1);
      }
      fclose(test);
   }

/********************** pass 1 ****************************/

   pass = PASS_1;
   yydebug = p1_yydebug;
   cnl_debug = p1_cnldebug;

   if (verbose)
      fprintf(stderr, "\nPass 1\n");

   load_first_file();

   /* Process */
   yyparse();

/********************** reset input files ****************************/

   fclose(yyin);

/********************** pass 2 ****************************/

   if (!had_error)
   {
      if (verbose)
	 fprintf(stderr, "\nPass 2\n");

      load_first_file();

      pass = PASS_2;
      yydebug = p2_yydebug;
      cnl_debug = p2_cnldebug;

      file_reset();
      strcpy(filename, inname);

      /* Process standard input */
      yyparse();

      fclose(yyin);
   }

/********************** cleanup ****************************/

   if (outfile != stdout)
      fclose(outfile);

   if (!had_error && !p1_yydebug && !p2_yydebug && !p1_cnldebug && !p2_cnldebug && !verbose)
   {
      for (int file_num = 0; file_num < num_files; file_num++)
      {
	 char buf[2048];

	 sprintf(buf, "/tmp/cnl_%ld_%d.cpp", our_pid, file_num);

	 unlink(buf);
      }
   }

   /* Delete the output file incase of error (and if not debugging) */
   if (had_error && !p1_yydebug && !p2_yydebug && !p1_cnldebug && !p2_cnldebug && !verbose)
   {
      unlink(outname);
   }


   if (logging)
      fclose(logfile);

   if (!had_error)
      fprintf(stderr, "Writing C++ code to %s\n", outname);

   return (had_error);
}



/**********************************************************************
 * $Log: parse.c,v $
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
 * Revision 1.35  2003/04/06 11:42:44  endo
 * gcc 3.1.1
 *
 * Revision 1.34  1999/12/16 23:26:03  endo
 * rh-6.0 porting.
 *
 * Revision 1.33  1996/02/25  16:56:17  doug
 * made emitting linenumbers the default and added -g option to disable it.
 *
 * Revision 1.32  1996/02/09  01:49:36  doug
 * *** empty log message ***
 *
 * Revision 1.31  1996/02/08  09:18:53  doug
 * added support to skim include files without expanding them if
 * didn't run the preprocessor.
 *
 * Revision 1.30  1995/11/13  21:08:37  doug
 * fixed so compiles under linux
 *
 * Revision 1.29  1995/08/24  21:04:43  doug
 * *** empty log message ***
 *
 * Revision 1.28  1995/08/22  18:03:12  doug
 * print message at end saying done
 *
 * Revision 1.27  1995/08/22  15:25:09  doug
 * modified to use exec instead of system to run preprocessor
 *
 * Revision 1.26  1995/08/21  18:18:51  doug
 * changed some printf commands to fprintf(stderr
 *
 * Revision 1.25  1995/06/15  21:59:46  jmc
 * Added RCS ID and Log strings.
 **********************************************************************/
