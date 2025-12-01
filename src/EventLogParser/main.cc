/**********************************************************************
 **                                                                  **
 **                            main.cc                               **
 **                                                                  **
 **  Main entry point for the event log parser                       **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Reseaelh Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: main.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#define sqr(x) ((x)*(x))

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
//#include <iomanip.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <iomanip>

#include "utilities.h"
#include "version.h"
#include "el_gram.tab.h"
#include "defs.h"
#include "list.hpp" // ENDO - gcc 3.4

using std::cout;
using std::setprecision;
using std::ios;

const char *filename;

const char *copyright_str =
#include "copyright"
 ;               

bool verbose = false;
bool table_mode;
bool graph_mode;
bool dist_mode;
bool stat_mode;
double y_offset = 0.0;
char *event_name;
char *event_names[] = {
	"PlaceState", 
	"AddTransition",
	"PickAgent",
	"PickTrigger",
	"ModAgentParms",
	"ModTriggerParms",
	NULL};



// Last time read from the file
double last_time = 0.0;

// Stack of events that haven't closed yet
event_list<events *> open_stack;
      
// List of events in the time line.
event_list<events *> time_line;

// Times for the graph mode
const int NUM_DURATIONS = 1024;
double durations[NUM_DURATIONS];
int    num_durations;
double  resolution = 0.5;

// min and max times for each subject
const int NUM_SUBJECTS = 24;
double min_times[NUM_SUBJECTS];
double max_times[NUM_SUBJECTS];
bool min_loaded[NUM_SUBJECTS];
bool max_loaded[NUM_SUBJECTS];

/*********************************************************************
*                                                                     *
*********************************************************************/ 

// Statistics
int num_sessions;
double mean_session_time;

// Session stats
char *subject = "Unknown";
char *task = "Unknown";
double duration = 0;
double end_of_session = 0;
int num_makes = 0;
double total_make_time = 0;
int num_runs = 0;
double total_run_time = 0;
double end_of_last_run = 0;

// ENDO - gcc 3.4
//--------------------------------------------------------------------
// Alphabetize the contents of the list in place
// This assumes that the data is a pointer to an events record 
//--------------------------------------------------------------------
void alphabetize_event_list(event_list<events *> *el)
{
    // Do a bubble sort on the list
    bool made_change;
    do
    {
        made_change = false;
        event_list<events *>::SLink *p = el->last_ptr;
        while( p && p->next != el->last_ptr)
        {
            p = p->next;

            if( ((events *)p->data)->start > ((events *)p->next->data)->start )
            {
                events *data = p->data;
                p->data = p->next->data;
                p->next->data = data;
                made_change = true;
            }
        }
    } while(made_change);
}


/*********************************************************************
*                                                                     *
*********************************************************************/ 

void
reset()
{
   open_stack.clear();
   time_line.clear();


   // Session stats
   subject = "Unknown";
   task = "Unknown";
   duration = 0;
   end_of_session = 0;
   num_makes = 0;
   total_make_time = 0;
   num_runs = 0;
   total_run_time = 0;
   end_of_last_run = 0;
}

/*********************************************************************
*                                                                     *
*********************************************************************/ 

void
dump_session_stuff()
{
   double ending_pause = 0;
   if( num_runs > 0 )
      ending_pause = end_of_session - end_of_last_run;

   if( table_mode )
   {
      cout << subject << "\t& " << task;
   }
   else if( graph_mode )
   {
      static bool first_time = true;
      if( first_time )
      {
         cout << "# Subject: " << subject << '\n';
	 first_time = false;
      }
   }
   else if( !dist_mode && !stat_mode )
   {
      cout << "Subject " << subject << " Task " << task << ":\n";
      cout << "   Total Time:      " << duration << '\n';
      cout << "   Number of Makes: " << num_makes << '\n';
      cout << "   Total Make time: " << total_make_time << '\n';
      cout << "   Number of Runs:  " << num_runs << '\n';
      cout << "   Total Run time:  " << total_run_time << '\n';
      cout << "   Ending Pause:    " << ending_pause << '\n';
      cout << "   Edit Time:       " << 
		duration - total_make_time - total_run_time - ending_pause 
		<< '\n';
      cout << '\n';
   }
}

/*********************************************************************
*                                                                     *
*********************************************************************/ 

void
process_time_lines()
{
//   cerr << "Processing the time lines\n";

   // define local accumulators
   double total_session_time = 0;

   // Clear global accumulators
   num_sessions = 0;
   bool in_session = false;

   void   *cur;
   events *p;
   if( (cur = time_line.first(&p)) )
   {
      do
      {
//cerr << "Event: " << p->event_name() << '\n';
	 if( strcmp(p->event_name(), "Session") == 0 )
	 {
	    if( in_session )
	    {
	       dump_session_stuff();
	    }
            in_session = true;

            subject = "Unknown";
            task = "Unknown";
            duration = p->duration();
            num_makes = 0;
            total_make_time = 0;
            num_runs = 0;
            total_run_time = 0;
	    end_of_session = p->end;


	    num_sessions++;
	    total_session_time += p->duration();

	    // Figure out which subject it was
	    events *stat = p->check_for_su_match(STATUS, "Subject");
	    if( stat )
	    {
	       subject = stat->message();
	    }

	    // Figure out which task it was
	    stat = p->check_for_su_match(STATUS, "Task");
	    if( stat )
	    {
	       task = stat->message();
	    }
         }
	 else if( strcmp(p->event_name(), "Make") == 0 )
	 {
	    total_make_time += p->duration();
	    num_makes ++;
	 }    
	 else if( strcmp(p->event_name(), "Run") == 0 )
	 {
            end_of_last_run = p->end;
	    total_run_time += p->duration();
	    num_runs ++;
	 }    
      } while( (cur = time_line.next(&p,cur)) );
   }

   if( in_session )
   {
      dump_session_stuff();
   }
}

/*********************************************************************
*                                                                     *
*********************************************************************/ 

void
summerize_event(char *event_name)
{
#if defined(linux)
   cout << setprecision(1)
	<< setiosflags(ios::fixed);
#endif

      void   *cur;
      events *p;
      double  duration;
      double  sum_of_event_durations = 0;
      int     number_of_events = 0;
      int     number_of_cancels = 0;
      if( (cur = time_line.first(&p)) )
      {
         do
         {
	    if( strcmp(p->event_name(), event_name) == 0 )
	    {
	       if( p->canceled )
	       {
	          number_of_cancels ++;
	       }
	       else
	       {
		  if( table_mode )
		  {
                     cout << "\t& " << p->duration();
		  }
		  else if( graph_mode || stat_mode )
		  {
		     // Save them and print them later.
		     double lng = p->duration();
                     durations[num_durations++] = lng;
		     assert(num_durations < NUM_DURATIONS);

		     // remember min & max values
		     int id = atoi(subject);
		     assert(id < NUM_SUBJECTS);

		     if( !min_loaded[id] || lng < min_times[id] )
		     {
			min_loaded[id] = true;
                        min_times[id] = lng;
		     }
		     if( !max_loaded[id] || lng > max_times[id] )
		     {
			max_loaded[id] = true;
                        max_times[id] = lng;
		     }
		  }
		  else if( dist_mode )
		  {
		     // Save them and print them later.
		     double lng = p->duration();
		     int i = (int)(lng / resolution + 0.5);
		     assert(i < NUM_DURATIONS);
                     durations[i] ++;
		     if( i > num_durations )
			num_durations = i;

		     // remember min & max values
		     int id = atoi(subject);
		     assert(id < NUM_SUBJECTS);

		     if( !min_loaded[id] || lng < min_times[id] )
		     {
			min_loaded[id] = true;
                        min_times[id] = lng;
		     }
		     if( !max_loaded[id] || lng > max_times[id] )
		     {
			max_loaded[id] = true;
                        max_times[id] = lng;
		     }
		  }

                  sum_of_event_durations += p->duration();
                  number_of_events ++;
	       }
	    }    
         } while( (cur = time_line.next(&p,cur)) );
      }

      // Catch divide by zero
      duration = 0;
      if( number_of_events > 0 )
      {
         duration = sum_of_event_durations / number_of_events;
      }

      if( !table_mode && !graph_mode && !dist_mode && !stat_mode )
      {
         cout << number_of_events 
	      << " " 
	      << event_name
	      << " events averaging " 
	      << duration
	      << " Seconds per event\n";
   
         if( number_of_cancels > 0 )
         {
            cout << number_of_cancels 
	      << " " 
	      << event_name
	      << " events were canceled\n";
         }
      }

}

/*********************************************************************
*                                                                     *
*********************************************************************/ 

void
print_summaries()
{
   if( table_mode )
   {
      summerize_event(event_name);
      cout << "\\\\\n"; 
   }
   else if( graph_mode || dist_mode || stat_mode )
   {
      summerize_event(event_name);
   }
   else
   {
#if defined(linux)
      cout << setprecision(2)
	   << setiosflags(ios::fixed);
#endif

      for(int event_index=0; event_names[event_index] != NULL; event_index++)
      {
         summerize_event(event_names[event_index]);
         cout << "\n"; 
      }
   }
}

/***************************************************************
*                                                              *
***************************************************************/ 

// qsort comparison function for doubles

static int 
double_compare(const void *A, const void *B)
{
   if( *(double *)A == *(double *)B )
      return 0;
   else if( *(double *)A < *(double *)B )
      return -1;
   return 1;
}

/***************************************************************
*                                                              *
***************************************************************/ 

int
main(int argc, char *argv[])
{

   //************ parse the command line parameters ********************
  //   GetOpt getopt(argc, argv, "lvT:G:y:D:S:R:");
   int option_char;

   table_mode = false;
   graph_mode = false;
   dist_mode = false;
   stat_mode = false;
   event_name = NULL;
   resolution = 0.5;
   while ((option_char = getopt(argc, argv, "lvT:G:y:D:S:R:") ) != EOF)
   { 
      switch (option_char)
      {
         case 'R':
            resolution = atof(optarg);
            break;

         case 'l':
            fprintf(stderr, "YACC debugging enabled\n");
            el_debug = true;
            break;

         case 'v':
            fprintf(stderr, "\n%s\n\n", copyright_str);
            fprintf(stderr, "EventLogParser version %s\n", version_str);
            fprintf(stderr, "compiled %s\n", compile_time());
            verbose = true;
            break;

	    // Generate output suitable for a latex table
         case 'T':
	    graph_mode = false;
	    dist_mode = false;
	    event_name = optarg;
            for(int i=0; event_names[i] != NULL; i++)
	    {
	       if( strcmp(event_name,event_names[i]) == 0 )
	       {
		  table_mode = true;
		  break;
	       }
	    }
	    if( !table_mode )
	    {
	       cerr << "Unknown event name '" 
		    << event_name 
		    << "' in Table mode\n";
	       cerr << "Valid names:\n";
               for(int i=0; event_names[i] != NULL; i++)
	          cerr << '\t' << event_names[i] << '\n';
	       cerr << '\n';
	       exit(1);
	    }
            break;

	    // Generate output suitable for gnuplot
         case 'G':
	    table_mode = false;
	    dist_mode = false;
	    event_name = optarg;
            for(int i=0; event_names[i] != NULL; i++)
	    {
	       if( strcmp(event_name,event_names[i]) == 0 )
	       {
		  graph_mode = true;
		  break;
	       }
	    }
	    if( !graph_mode )
	    {
	       cerr << "Unknown event name '" 
		    << event_name 
		    << "' in Graph mode\n";
	       cerr << "Valid names:\n";
               for(int i=0; event_names[i] != NULL; i++)
	          cerr << '\t' << event_names[i] << '\n';
	       cerr << '\n';
	       exit(1);
	    }
            break;

	    // Generate output suitable for gnuplot to show the distribution
         case 'D':
	    table_mode = false;
	    graph_mode = false;
	    dist_mode = false;
	    event_name = optarg;
            for(int i=0; event_names[i] != NULL; i++)
	    {
	       if( strcmp(event_name,event_names[i]) == 0 )
	       {
		  dist_mode = true;
		  break;
	       }
	    }
	    if( !dist_mode )
	    {
	       cerr << "Unknown event name '" 
		    << event_name 
		    << "' in Distribution mode\n";
	       cerr << "Valid names:\n";
               for(int i=0; event_names[i] != NULL; i++)
	          cerr << '\t' << event_names[i] << '\n';
	       cerr << '\n';
	       exit(1);
	    }
            break;

	    // Generate statistics for output 
         case 'S':
	    event_name = optarg;
            for(int i=0; event_names[i] != NULL; i++)
	    {
	       if( strcmp(event_name,event_names[i]) == 0 )
	       {
	          stat_mode = true;
		  break;
	       }
	    }
	    if( !stat_mode )
	    {
	       cerr << "Unknown event name '" 
		    << event_name 
		    << "' in Statistics mode\n";
	       cerr << "Valid names:\n";
               for(int i=0; event_names[i] != NULL; i++)
	          cerr << '\t' << event_names[i] << '\n';
	       cerr << '\n';
	       exit(1);
	    }
            break;

	    // Add the y offset to graph outputs
         case 'y':
	    y_offset = atof(optarg);
	    break;
      }
   }

   if (argc - optind <= 0)
   {
      fprintf(stderr, "No input files specified\n"); 
      exit(1);
   }        

   had_error = false;
   num_durations = 0;
   for(int i=0; i<NUM_DURATIONS; i++)
      durations[i] = 0;

   for(int i=0; i<NUM_SUBJECTS; i++)
   {
      min_loaded[i] = false;
      max_loaded[i] = false;
   }

   /******************** process the event logs **************************/
   for (int file_num = optind; file_num < argc; file_num++)
   {              
      reset();
      char *filename = argv[file_num];
                  
      el_in = fopen(filename, "r");
      if( el_in == NULL )
         return(2);

      /************** parse the log file ********************/

      el_parse();

      /************** done ********************/

      fclose(el_in);

/*
      if( had_error )
         return 1;
*/

      if( !open_stack.isempty() )
      {
	 semantic_error("Didn't close all the events in input file %s", 
		filename);

         while( !open_stack.isempty() )
         {
            events *p = open_stack.pop();
   
            // mark it as ending at the end of the file
            p->finish(last_time);

            // Add us to the time line
            time_line.append(p);
         }
      }

      /******************** print info **************************/
      //time_line.alphabetize();
      alphabetize_event_list(&time_line);
      process_time_lines();
      print_summaries();
   }

   if( graph_mode )
   {
#if defined(linux)
      cout << setprecision(2);
#endif

      for(int i=0; i<num_durations; i++)
      {
#if 0
         double x;

	 if( (num_durations - 1) == 0 )
	    x = 0.5;
         else
	    x = (double)i / (num_durations - 1);

         cout << x << "\t" << (durations[i] + y_offset) << '\n';
#endif
         cout << i << "\t" << (durations[i] + y_offset) << '\n';
      }
      cout << '\n'; 
      cerr << "number of " << event_name << " events = " << num_durations << '\n';
   }
   else if( dist_mode )
   {
#if defined(linux)
      cout << setprecision(2);
#endif

#if 0
      // Remove the high and low value for each subject from consideration
      for(int i=0; i<NUM_SUBJECTS; i++)
      {
	 if( min_loaded[i] )
	 {
            int pos = (int)(min_times[i] * 2 + 0.5);
	    assert(pos < NUM_DURATIONS);
            durations[pos] --;
	    if( pos == num_durations )
	    {
	       while( durations[num_durations] == 0 && num_durations > 0)
	          num_durations --;
	    }
	 }
	 if( max_loaded[i] )
	 {
            int pos = (int)(max_times[i] * 2 + 0.5);
	    assert(pos < NUM_DURATIONS);
            durations[pos] --;
	    if( pos == num_durations )
	    {
	       while( durations[num_durations] == 0 && num_durations > 0)
	          num_durations --;
	    }
	 }
      }
#endif

      for(int i=0; i<=num_durations; i++)
      {
         cout << (double)i * resolution << "\t" << (int)(durations[i] + y_offset) << '\n';
      }
      cout << '\n'; 
   }
   else if( stat_mode )
   {
#if defined(linux)
      cout << setprecision(2);
#endif

      // Sort the durations so can compute the median
      qsort(durations, num_durations, sizeof(durations[0]), 
	    double_compare);

      double median = 0;
      int center = num_durations / 2;
      if( (center % 2) == 0 )
      {
	 median = (durations[center] + durations[center + 1]) / 2;
      }
      else
	 median = durations[center];

//      cout << "Median Value for " << event_name << " is " << median << '\n';

      // Compute the sum of the durations
      double sum = 0;
      for(int i=0; i<num_durations; i++)
	 sum += durations[i];

      int num_remaining = num_durations;
#if 0
      // Remove the high and low value for each subject 
      for(int i=0; i<NUM_SUBJECTS; i++)
      {
	 if( min_loaded[i] )
	 {
            sum -= min_times[i];
            num_remaining --;
	 }
	 if( max_loaded[i] )
	 {
            sum -= max_times[i];
            num_remaining --;
	 }
      }
#endif

      // Compute the average value
      double x_bar = (double)sum / num_remaining;

//      cout << "Average Value is " << x_bar << '\n';
      cout << "Average Value for " << event_name << " is " << x_bar << '\n';

      // Compute the variance
      sum = 0;
      for(int i=0; i<num_durations; i++)
	 sum += sqr(durations[i] - x_bar);

#if 0
      // Remove the high and low value for each subject 
      for(int i=0; i<NUM_SUBJECTS; i++)
      {
	 if( min_loaded[i] )
            sum -= sqr(min_times[i] - x_bar);
	 if( max_loaded[i] )
            sum -= sqr(max_times[i] - x_bar);
      }
#endif
      double variance = sum / (num_remaining - 1);

//      cout << "Variance is " << variance << '\n';

      double std_deviation = sqrt(variance);
      cout << "Standard Deviation is " << std_deviation << '\n';
      cout << "Number of data points are " << num_remaining << '\n';
      cout << '\n'; 
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////
// $Log: main.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:14  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:53  endo
// MissionLab 7.0
//
// Revision 1.2  2006/05/12 22:51:56  endo
// g++-3.4 upgrade.
//
// Revision 1.1.1.1  2005/02/06 22:59:28  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.9  2003/04/06 11:39:04  endo
// gcc 3.1.1
//
// Revision 1.8  1999/12/16 23:07:47  endo
// rh-6.0 porting.
//
// Revision 1.7  1996/10/08  19:42:35  doug
// setpresion doesn't seem to work yet
//
// Revision 1.6  1996/10/04  20:44:34  doug
// Revised for dissertation work
//
// Revision 1.7  1996/09/03 16:05:56  doug
// uploaded
//
// Revision 1.6  1996/08/07 15:27:48  doug
// Added distribution mode
//
// Revision 1.5  1996/08/07 15:06:32  doug
// *** empty log message ***
//
// Revision 1.4  1996/06/01  21:54:20  doug
// *** empty log message ***
//
// Revision 1.3  1996/04/18  00:20:37  doug
// *** empty log message ***
//
// Revision 1.2  1996/04/13  21:54:42  doug
// *** empty log message ***
//
// Revision 1.1  1996/03/05  22:24:18  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
