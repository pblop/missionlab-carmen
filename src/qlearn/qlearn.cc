/**********************************************************************
 **                                                                  **
 **                         QLEARN.cc                                **
 **                                                                  **
 **  Written By:  Eric Martinson				     **
 **                                                                  **
 **********************************************************************/

/* $Id: qlearn.cc,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <string.h>
#include <time.h>
#include <iostream>

#include "qlearn.h"

using std::cout;
using std::endl;

/* Constructor for Qlearn
 * Parameters may be adjusted using accessor methods.
 *
 * numstates---the number of states the system could be in.
 * numactions--the number of actions or outputs to 
 *             select from.
 * criteria----should be DISCOUNTED or AVERAGE.
 * seedin------the seed.
 */

double ** array2d(int x, int y)
{
  double **temp;
  int i, j;

  // ENDO - gcc 3.4
  //temp = new (double *)[x];
  temp = new double *[x];

  for (i = 0; i < x; i++)
    {
      temp[i] = new double[y];
    }

  for (i = 0; i < x; i++)
    {
      for (j = 0; j < y; j++)
        {
          temp[i][j] = 0;
        }
    }

  return (temp);
}

//make sure this doesn't leak
void delarray2d(double **array, int x)
{
  int i;

  if (array == NULL)
    return;

  for (i = 0; i < x; i++)
    {
      if (array[i] != NULL)
        delete[]array[i];
    }

  delete[]array;
}

Qlearn::Qlearn (int numstatesin, int numactionsin, char *filename,
                int robot_id, double alphaIn, double alphaDecay,
                double randomRate, double rScenarioDecay,
                int criteriain, int seedin)
{
  int i, j;
  FILE *file;
  int highest;

  dataName = new char[128];
  sprintf (dataName, "%d%c", robot_id, '\0');
  dataName = strcat (dataName, filename);
  dataName = strcat (dataName, ".dat");

  cout << "passed it to Q... " << dataName << endl;

  dataName = new char[128];
  sprintf (dataName, "%d%c", robot_id, '\0');
  dataName = strcat (dataName, filename);
  dataName = strcat (dataName, ".dat");

  tableName = new char[128];
  sprintf (tableName, "%d%c", robot_id, '\0');
  tableName = strcat (tableName, filename);
  tableName = strcat (tableName, ".tab");


  AVERAGE = 0;
  DISCOUNTED = 1;

  numstates = numstatesin;
  numactions = numactionsin;

  cout << "STATEs.." << numstates << "  Actions.." << numactions << endl;

  if ((criteriain != DISCOUNTED) && (criteriain != AVERAGE))
    {
      cout << "Error: INVALID CRITERIA.  Setting to DISCOUNTED" << endl;
      criteria = DISCOUNTED;
    }
  else
    criteria = criteriain;

  seed = seedin;
  srand ((unsigned) time (NULL));

  q = array2d (numstates, numactions);

  profile = array2d (numstates, numactions);

  p = array2d (numstates, numactions);

  last_policy = new int[numstates];
  for (i = 0; i < numstates; i++)
    last_policy[i] = 0;


  //NOTE---MAKE MY OWN RANDOM NUMBER GENERATOR
  for (i = 0; i < numstates; i++)
    {
      highest = 0;
      for (j = 0; j < numactions; j++)
        {
          //      q[i][j] = ((double) rand()/(double) RAND_MAX) - 1;
          q[i][j] = (rand () / (RAND_MAX + 1.0)) + 1;
          //q[i][j] = 0;
          p[i][j] = 0;
          profile[i][j] = 0;
          if (q[i][highest] > q[i][j])
            highest = j;
        }
      last_policy[i] = j;
    }
  xn = an = 0;

//variable initialization
  changes = 0;
  changesAll = 0;
  queries = 0;
  total_reward = 0;
  first_of_trial = 1;
  gamma = 0.8;
  alpha = alphaIn;
  randomrate = randomRate;
  randomratedecay = rScenarioDecay;
  seed = seedin;
  debug = FALSE;


  //  Read the Qtable...
  //   replace all variables just initialized with 
  //   values from Qtable, if it exists

  if (dataName != NULL)
    {
      file = fopen (dataName, "r");
      if (file != NULL)
        {
          fclose (file);
          read ();
        }
    }

  alpha = alpha * alphaDecay;
  randomrate = randomrate * randomratedecay;
  first_of_trial = 1;

}

/*
 * Select an output based on the state and reward.
 *
 * curstate---the current state.
 * curreward--reward for the last output, positive
 *            numbers are "good."
 */

int Qlearn::query(int curstate, double curreward)
{
  int action;
  double randomnum;
  int highest;


  total_reward += curreward;
  queries++;

  //Check to see if the current state is out of range.
  if ((curstate > (this->numstates - 1)) || (curstate < 0))
    {
      return -1;
    }

  //Find approximate value of present state, and best action.
  //ie:  max q[yn][i] over all i, i is the best action.

  double Vn = -999999;          //very bad
  action = 0;
  for (int i = 0; i < numactions; i++)
    {
      if (q[curstate][i] > Vn)
        {
          Vn = q[curstate][i];
          action = i;
        }
    }

  //Now update according to Watkin's iteration:
  if (first_of_trial != 1)
    {

      q[xn][an] = q[xn][an] + alpha * (curreward + gamma * Vn - q[xn][an]);

      p[xn][an]++;              //count times in the last state/action
      profile[xn][an]++;        //count times for this trial
    }
  else
    first_of_trial = 0;

  for (int i = 0; i < numstates; i++)
    {
      highest = 0;
      for (int j = 1; j < numactions; j++)
        {
          if (q[i][highest] < q[i][j])
            highest = j;
        }

      if (highest != last_policy[i])
        {
          last_policy[i] = highest;
          changesAll++;

          if (i < numstates / 2)
            changes++;

        }
    }

  //Select random action, possibly
  //NOTE---MAKE MY OWN RANDOM NUMBER GENERATOR

  randomnum = rand () / (RAND_MAX + 1.0);
  if (randomnum <= randomrate)
    {
      action = ((int) (rand () / 10)) % numactions;
      if (action < 1)
        action = -1 * action;
    }

  printf ("%s LAST(STATE: %d, ACTION: %d) Action Chosen: %d reward: %f\n",
          dataName, xn, an, action, curreward);

  //Remember for next time
  xn = curstate;
  an = action;

  //  if (logging) CheckForChanges();

  return action;
}


/**
 * Called when the current trial ends.
 *
 * @param Vn     double, the value of the absorbing state.
 * @param reward double, the reward for the last output.
 */
void
Qlearn::endTrial (double Vn, double rn)
{
  total_reward += rn;

  if (criteria == DISCOUNTED)
    {
      // Watkins update rule:
      q[xn][an] = (1 - alpha) * q[xn][an] + alpha * (rn + gamma * Vn);
    }
  else                          // criteria == AVERAGE
    {
      // average update rule
      q[xn][an] = (p[xn][an] * q[xn][an] + rn) / (p[xn][an] + 2);
      // see update above in query() for explanation 
      // of this rule
    }

  p[xn][an] += 1;
  profile[xn][an] += 1;
}


/**
 * Called to initialize for a new trial.
 */
int
Qlearn::initTrial (int s)
{
  first_of_trial = 1;
  changes = 0;
  queries = 0;
  total_reward = 0;
  delarray2d (profile, numstates);
  profile = array2d (numstates, numactions);
  return (query (s, 0));
}

void
Qlearn::print ()
{
  int i, j;
  FILE *record;

  record = fopen (tableName, "a+");


  for (i = 0; i < numstates; i++)
    {
      for (j = 0; j < numactions; j++)
        {
          fprintf (record, "%g ", q[i][j]);
        }

    }
  fprintf (record, "\n");
  fclose (record);
}

//void Qlearn::save(char *filename)
void
Qlearn::save ()
{
  FILE *file;
  int i, j;

  file = fopen (dataName, "w");

  fwrite (&numstates, 1, sizeof (int), file);
  fwrite (&numactions, 1, sizeof (int), file);
  fwrite (&criteria, 1, sizeof (int), file);
//  fwrite(&changes, 1, sizeof(int), file);
  fwrite (&queries, 1, sizeof (int), file);
  fwrite (&first_of_trial, 1, sizeof (int), file);
  fwrite (&xn, 1, sizeof (int), file);
  fwrite (&an, 1, sizeof (int), file);
  fwrite (&debug, 1, sizeof (int), file);
  fwrite (&seed, 1, sizeof (long), file);

  fwrite (&total_reward, 1, sizeof (double), file);
  fwrite (&gamma, 1, sizeof (double), file);
  fwrite (&alpha, 1, sizeof (double), file);
  fwrite (&randomrate, 1, sizeof (double), file);
  fwrite (&randomratedecay, 1, sizeof (double), file);

  for (i = 0; i < numstates; i++)
    {
      for (j = 0; j < numactions; j++)
        {
          fwrite (&(q[i][j]), 1, sizeof (double), file);
          fwrite (&(p[i][j]), 1, sizeof (double), file);
          fwrite (&(profile[i][j]), 1, sizeof (double), file);
        }
      fwrite (&(last_policy[i]), 1, sizeof (int), file);
    }
//  readPolicy();

  fclose (file);

}

//void Qlearn::read(char *filename)
void
Qlearn::read ()
{
  FILE *file;
  int i, j;

  file = fopen (dataName, "r");

  fread (&numstates, 1, sizeof (int), file);
  fread (&numactions, 1, sizeof (int), file);
  fread (&criteria, 1, sizeof (int), file);
//  fread(&changes, 1, sizeof(int), file);
  fread (&queries, 1, sizeof (int), file);
  fread (&first_of_trial, 1, sizeof (int), file);
  fread (&xn, 1, sizeof (int), file);
  fread (&an, 1, sizeof (int), file);
  fread (&debug, 1, sizeof (int), file);
  fread (&seed, 1, sizeof (long), file);

  fread (&total_reward, 1, sizeof (double), file);
  fread (&gamma, 1, sizeof (double), file);
  fread (&alpha, 1, sizeof (double), file);
  fread (&randomrate, 1, sizeof (double), file);
  fread (&randomratedecay, 1, sizeof (double), file);

  for (i = 0; i < numstates; i++)
    {
      for (j = 0; j < numactions; j++)
        {
          fread (&(q[i][j]), 1, sizeof (double), file);
          fread (&(p[i][j]), 1, sizeof (double), file);
          fread (&(profile[i][j]), 1, sizeof (double), file);
        }
      fread (&(last_policy[i]), 1, sizeof (int), file);
    }

//  readPolicy();

  fclose (file);
}

void
Qlearn::readPolicy ()
{
  for (int i = 0; i < numstates; i++)
    {
      printf ("%d ", last_policy[i]);
    }
  printf ("\n");
}

/**********************************************************************
# $Log: qlearn.cc,v $
# Revision 1.1.1.1  2008/07/14 16:44:24  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.1.1.1  2006/07/20 17:17:51  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:38:00  endo
# MissionLab 7.0
#
# Revision 1.2  2006/05/14 07:39:09  endo
# gcc-3.4 upgrade
#
# Revision 1.1.1.1  2005/02/06 22:59:28  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.6  2003/04/06 11:47:01  endo
# gcc 3.1.1
#
# Revision 1.4  2002/01/13 03:03:05  endo
# *** empty log message ***
#
# Revision 1.3  2002/01/11 16:16:11  ebeowulf
# Fixed deallocation bug in "delarray2d"
#
 **********************************************************************/

