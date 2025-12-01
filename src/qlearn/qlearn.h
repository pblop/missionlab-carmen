#ifndef QLEARN_H
#define QLEARN_H

#define FALSE 0
#define TRUE 1

#include <stdio.h>
#include <stdlib.h>

class Qlearn
{
public:
    
    int AVERAGE;     //Used to indicate the learner uses average rewards.
    int DISCOUNTED;  //Used to indicate the learner uses discounted rewards.
    
    Qlearn(int numstatesin, int numactionsin, char *filename = NULL, 
	   int robot_id = 0, double alpha = 1.0, double alphaDecay = 0.8,
	   double randomRate = 0.1, double rScenarioDecay = 1.0, 
	   int criteriain = 1, int seedin = 0 );
    void print();
    int query(int yn, double rn);
    void endTrial(double Vn, double rn);
    int initTrial(int s);
    void save();
    void read();
    
    void readPolicy();
    
private:
    
    //*********************************************************************
    int     numstates;         // the number of perceptual states we can be in
    int     numactions;        // the number of actions we have to select from
    int	    criteria;          // assume discounted rewards
    int     changes;           // used to count changes in policy per trial
    int	    changesAll;	       // records all changes, and not just half
    int	    queries;	       // queries per trial
    int     first_of_trial;    // indicates if first time
    int     xn;                // last state
    int     an;                // last action
    int     debug;
    double  total_reward;      // reward over trial
    double  gamma;             // discount rate
    double  alpha;             // learning rate
    double  randomrate;        // frequency of random actions
    double  randomratedecay;   // decay rate of random actions 
    long    seed;              // random number seed
    double  **q;               // the q-values
    double  **p;               // count of times in each state/action
    double  **profile;         // count of times in each 
                               // state/action for this trial
    int     *last_policy;      // used to count changes in policy
    char    *dataName;         // the name of the data file
    char    *tableName;	       // the name of the ascii text file
};

#endif
#ifndef QLEARN_H
#define QLEARN_H

#define FALSE 0
#define TRUE 1

#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

class Qlearn
{
public:
    
    int AVERAGE;     //Used to indicate the learner uses average rewards.
    int DISCOUNTED;  //Used to indicate the learner uses discounted rewards.
    
    Qlearn(int numstatesin, int numactionsin, char *filename = NULL, 
	   int robot_id = 0, double alpha = 1.0, double alphaDecay = 0.8,
	   double randomRate = 0.1, double rScenarioDecay = 1.0, 
	   int criteriain = 1, int seedin = 0 );
    void print();
    int query(int yn, double rn);
    void endTrial(double Vn, double rn);
    int initTrial(int s);
    void save();
    void read();
    
    void readPolicy();
    
private:
    
    //*********************************************************************
    int     numstates;         // the number of perceptual states we can be in
    int     numactions;        // the number of actions we have to select from
    int	    criteria;          // assume discounted rewards
    int     changes;           // used to count changes in policy per trial
    int	    changesAll;	       // records all changes, and not just half
    int	    queries;	       // queries per trial
    int     first_of_trial;    // indicates if first time
    int     xn;                // last state
    int     an;                // last action
    int     debug;
    double  total_reward;      // reward over trial
    double  gamma;             // discount rate
    double  alpha;             // learning rate
    double  randomrate;        // frequency of random actions
    double  randomratedecay;   // decay rate of random actions 
    long    seed;              // random number seed
    double  **q;               // the q-values
    double  **p;               // count of times in each state/action
    double  **profile;         // count of times in each 
                               // state/action for this trial
    int     *last_policy;      // used to count changes in policy
    char    *dataName;         // the name of the data file
    char    *tableName;	       // the name of the ascii text file
};

#endif
