/**********************************************************************
 **                                                                  **
 **                           memory_manager.h                       **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

/* $Id: memory_manager.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <vector>
#include <string>

#include "cbrplanner_protocol.h"
#include "cbrplanner_domain.h"

class MemoryManager {

public:
    typedef enum FormatType_t
    {
        FORMAT_STRING,
        FORMAT_INT,
        FORMAT_DOUBLE,
        FORMAT_BOOLEAN,
        FORMAT_NA
    };

    typedef struct Feature_t
    {
        string name;
        string value;
        bool nonIndex;
        double weight;
        int formatType;
    };

    typedef vector<Feature_t> Features_t;

    typedef struct Constraint_t
    {
        string name;
        string value;
        int id;
        int formatType;
    };

    typedef struct Constraints_t
    {
        int id;
        string name;
        vector<Constraint_t> constraints;
    };

    typedef Constraints_t RobotConstraints_t;

    typedef Constraints_t TaskConstraints_t;

    typedef enum DecisionTreeDataStatus_t
    {
        NEUTRAL_DATA,
        POSITIVE_DATA,
        NEGATIVE_DATA
    };

    typedef struct DecisionTreeData_t
    {
        vector<int> indexes;
        vector<int> statuses;
        Features_t otherFeatures;
    };

    typedef struct DecisionTreeEdge_t
    {
        string parentName;
        string value;
        void *nextNode;
    };

    typedef struct DecisionTreeNode_t
    {
        string name;
        bool isLeafNode;
        vector<DecisionTreeEdge_t> edges;
        DecisionTreeData_t data;
    };

    typedef DecisionTreeNode_t DecisionTree_t;

protected:
    MemoryManager **self_;
    DecisionTree_t *decisionTree_;
    DecisionTreeNode_t *currentNode_;
    bool memoryChanged_;
    
    static const string EMPTY_STRING_;
    static const string STRING_PERIOD_;

    DecisionTreeNode_t *getCurrentNode_(void);
    DecisionTreeNode_t *getCurrentNode_(string chosenEdge);
    vector<int> getSavedData_(
        DecisionTree_t *decisionTree,
        Features_t features,
        int maxRating,
        vector<int> *ratings,
        bool disableBacktrack);
    string findValueOfFeature_(Features_t features, string featureName);
    void addNewDataToDecisionTree_(
        DecisionTree_t **decisionTree,
        Features_t features,
        int dataIndex);
    void addNewNodeToDecisionTree_(
        DecisionTree_t *decisionTree,
        string nodeName,
        string edgeValue,
        DecisionTreeData_t data);
    void removeFeatureFromList_(Features_t *features, string featureName);
    void dumpDecisionTree_(DecisionTree_t *decisionTree);
    void giveFeedback_(DecisionTreeNode_t *node, int index, int status);
    void resetCurrentNode_(void);

    virtual DecisionTree_t *loadDecisionTree_(FILE *in);
    virtual DecisionTreeData_t loadNodeData_(FILE *in);
    virtual DecisionTreeNode_t *loadNode_(FILE *in);
    virtual void writeDecisionTree_(FILE *out, DecisionTree_t *decisionTree);
    virtual void writeNode_(FILE *out, DecisionTreeNode_t *node);
    virtual void writeNodeData_(FILE *out, DecisionTreeData_t data);

public:
    MemoryManager(void);
    MemoryManager(MemoryManager **self);
    DecisionTreeNode_t *getCurrentNode(void);
    DecisionTreeNode_t *getCurrentNode(string chosenEdge);
    bool memoryChanged(void);
    void givePositiveFeedback(vector<int> dataIndexList);
    void giveNegativeFeedback(vector<int> dataIndexList);
    void resetCurrentNode(void);
    
    virtual ~MemoryManager(void);
};

inline MemoryManager::DecisionTreeNode_t *MemoryManager::getCurrentNode_(void)
{
    return (currentNode_);
}

inline MemoryManager::DecisionTreeNode_t *MemoryManager::getCurrentNode(void)
{
    return (getCurrentNode_());
}

inline MemoryManager::DecisionTreeNode_t *MemoryManager::getCurrentNode(
    string chosenEdge)
{
    return getCurrentNode_(chosenEdge);
}

inline bool MemoryManager::memoryChanged(void)
{
    return (memoryChanged_);
}

inline void MemoryManager::resetCurrentNode_(void)
{
    currentNode_ = (DecisionTreeNode_t *)decisionTree_;
}

inline void MemoryManager::resetCurrentNode(void)
{
    resetCurrentNode_();
}

//-----------------------------------------------------------------------
// Virtuals
//-----------------------------------------------------------------------
inline MemoryManager::DecisionTree_t *MemoryManager::loadDecisionTree_(FILE *in)
{
    return NULL;
}

inline MemoryManager::DecisionTreeData_t MemoryManager::loadNodeData_(FILE *in)
{
    const DecisionTreeData_t EMPTY_DATA = {vector<int>(), vector<int>(), Features_t()};
    return EMPTY_DATA;
}

inline MemoryManager::DecisionTreeNode_t *MemoryManager::loadNode_(FILE *in)
{
    return NULL;
}

inline void MemoryManager::writeDecisionTree_(FILE *out, DecisionTree_t *decisionTree) {}
inline void MemoryManager::writeNode_(FILE *out, DecisionTreeNode_t *node) {}
inline void MemoryManager::writeNodeData_(FILE *out, DecisionTreeData_t data) {}

#endif
/**********************************************************************
 * $Log: memory_manager.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/09/29 23:45:36  endo
 * Global feature can be now disabled.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.5  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.4  2005/07/16 08:49:21  endo
 * CBR-CNP integration
 *
 * Revision 1.3  2005/06/23 22:09:04  endo
 * Adding the mechanism to talk to CNP.
 *
 * Revision 1.2  2005/02/07 19:53:44  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
