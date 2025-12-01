/**********************************************************************
 **                                                                  **
 **                           memory_manager.cc                      **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This modules deals with storing and retrieving of missions.     **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: memory_manager.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <sys/types.h>

#include "cbrplanner_domain.h"
#include "memory_manager.h"
#include "windows.h"
#include "debugger.h"

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
extern void gQuitProgram(int exitStatus); // Defined in main.cc

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const string MemoryManager::EMPTY_STRING_ = "";
const string MemoryManager::STRING_PERIOD_ = ".";

//-----------------------------------------------------------------------
// Constructor for MemoryManager class.
//-----------------------------------------------------------------------
MemoryManager::MemoryManager(MemoryManager **memoryManagerInstance) :
    self_(memoryManagerInstance)
{
    if (memoryManagerInstance != NULL)
    {
        *memoryManagerInstance = this;
    }

    decisionTree_ = NULL;
    memoryChanged_ = false;
}

//-----------------------------------------------------------------------
// Distructor for MemoryManager class.
//-----------------------------------------------------------------------
MemoryManager::~MemoryManager(void)
{
    if (decisionTree_ != NULL)
    {
        delete decisionTree_;
        decisionTree_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function will save a new mission plan into its memory (decision
// tree).
//-----------------------------------------------------------------------
void MemoryManager::addNewDataToDecisionTree_(
    DecisionTree_t **decisionTree,
    Features_t features,
    int dataIndex)
{
    Feature_t feature;
    DecisionTree_t *localDecisionTree = NULL;
    DecisionTreeNode_t *node = NULL, *childNode = NULL;
    vector<DecisionTreeNode_t *>nodeStack;
    DecisionTreeData_t data;
    string value;
    int i, numEdges;
    bool dataSaved = false;
    const bool DEBUG = true;

    localDecisionTree = *decisionTree;

    if (localDecisionTree == NULL)
    {
        localDecisionTree = new DecisionTree_t;

        // Get the feature of the highest priority.
        feature = features.front();

        // Update the list (i.e., remove the feature from the list).
        removeFeatureFromList_(&features, feature.name);

        // Construct the data for the leaf node.
        data.indexes.push_back(dataIndex);
        data.statuses.push_back(NEUTRAL_DATA);
        data.otherFeatures = features;

        // Create a new node.
        addNewNodeToDecisionTree_(localDecisionTree, feature.name, feature.value, data);
    }
    else
    {
        nodeStack.push_back(localDecisionTree);
        dataSaved = false;
	
        while (!dataSaved)
        {
            // Get the node from the stack.
            node = nodeStack.back();
            nodeStack.pop_back();

            // Get the value of the feature.
            value = findValueOfFeature_(features, node->name);

            // Update the list (i.e., remove the feature from the list).
            removeFeatureFromList_(&features, node->name);

            numEdges = node->edges.size();

            // Look for edges to see if the value exists.
            for (i = 0; i < numEdges; i++)
            {
                if (value == node->edges[i].value)
                {
                    childNode = (DecisionTreeNode_t *)(node->edges[i].nextNode);

                    if (childNode->isLeafNode)
                    {
                        // The child is a leaf node. It means that the child contains data.
                        if ((int)(childNode->data.otherFeatures.size()) > 0)
                        {
                            // Other feature(s) is still available to expand the tree.
                            // Create a new grandchild node, using the next priority of the
                            // feature. Copy the data to the grandchild as well.
                            feature = childNode->data.otherFeatures.front();
                            removeFeatureFromList_(&(childNode->data.otherFeatures), feature.name);

                            addNewNodeToDecisionTree_(
                                childNode,
                                feature.name,
                                feature.value,
                                childNode->data);
                        }
                        else
                        {
                            // There is no other feature to distinguish this data
                            // and the existing one. Append this data in the child node.
                            childNode->data.indexes.push_back(dataIndex);
                            childNode->data.statuses.push_back(NEUTRAL_DATA);
                            dataSaved = true;
                            break;
                        }
                    }
                    else
                    {
                        // The child is not a leaf node. It means there is a grandchild node
                        // being already created. Just rememeber it to explore it later.
                    }

                    nodeStack.push_back(childNode);
                    break;
                }
            }

            if (((int)(nodeStack.size()) == 0) && (!dataSaved))
            {
                // It is a new value. Add new edge and node to the tree.
                // First, construct the data for the leaf node.
                data.indexes.push_back(dataIndex);
                data.statuses.push_back(NEUTRAL_DATA);
                data.otherFeatures = features;

                // Create a new node.
                addNewNodeToDecisionTree_(node, node->name, value, data);
                dataSaved = true;
            }
        }
    }

    if (DEBUG)
    {
        dumpDecisionTree_(localDecisionTree);
    }

    *decisionTree = localDecisionTree;
}

//-----------------------------------------------------------------------
// This function creates a new node to the decision tree.
//-----------------------------------------------------------------------
void MemoryManager::addNewNodeToDecisionTree_(
    DecisionTree_t *decisionTree,
    string nodeName,
    string edgeValue,
    DecisionTreeData_t data)
{
    DecisionTreeEdge_t edge;
    DecisionTreeNode_t *childNode = NULL;

    // Copy the node name.
    decisionTree->name = nodeName;

    // This is no longer a leaf node.
    decisionTree->isLeafNode = false;

    // Construct the child node.
    childNode = new DecisionTreeNode_t;
    childNode->isLeafNode = true;
    childNode->data = data;

    // Construct an edge based on the feature value, and connect
    // the parent and child nodes with it.
    edge.parentName = nodeName;
    edge.value = edgeValue;
    edge.nextNode = (void *)childNode;
    decisionTree->edges.push_back(edge);
}

//-----------------------------------------------------------------------
// This function removes a feature from the list.
//-----------------------------------------------------------------------
void MemoryManager::removeFeatureFromList_(Features_t *features, string featureName)
{
    int i, j, listSize;

    listSize = features->size();

    for (i = 0; i < listSize; i++)
    {
        if ((*features)[i].name == featureName)
        {
            for (j = i; j < (listSize-1); j++)
            {
                (*features)[j] = (*features)[j+1];
            }

            features->pop_back();

            return;
        }
    }
}

//-----------------------------------------------------------------------
// This function finds a value of the feature from the list.
//-----------------------------------------------------------------------
string MemoryManager::findValueOfFeature_(
    Features_t features,
    string featureName)
{
    string value = EMPTY_STRING_;
    int i, listSize;

    listSize = features.size();

    for (i = 0; i < listSize; i++)
    {
        if (features[i].name == featureName)
        {
            value = features[i].value;
            break;
        }
    }

    return value;
}

//-----------------------------------------------------------------------
// This function will get an saved mission plan into its memory (decision
// tree).
//-----------------------------------------------------------------------
vector<int> MemoryManager::getSavedData_(
    DecisionTree_t *decisionTree,
    Features_t features,
    int maxRating,
    vector<int> *ratings,
    bool disableBacktrack)
{
    DecisionTreeNode_t *node, *childNode;
    vector<DecisionTreeNode_t *> evaluationNodeStack, backtrackNodeStack;
    vector<string> visitedNodeNames;
    vector<int> indexes;
    string value;
    int i, j, index, numEdges, rating, dataRating;
    bool savedBefore = false;
    const bool DEBUG = true;
    //const int MIN_RATING = 2; // ENDO 20040406 -- Undid on 20040408

    rating = maxRating;

    if (decisionTree == NULL)
    {
        // No tree, no solution.
        return indexes;
    }
    else
    {
        if (DEBUG)
        {
            dumpDecisionTree_(decisionTree);
        }

        evaluationNodeStack.push_back(decisionTree);
    }

    while(((int)(evaluationNodeStack.size())) > 0)
    {
        // Get the node in the stack.
        node = evaluationNodeStack.back();
        evaluationNodeStack.pop_back();

        // Remember the node for backtracking.
        backtrackNodeStack.push_back(node);

        // Get the value of the feature.
        value = findValueOfFeature_(features, node->name);

        numEdges = node->edges.size();

        // Look for edges to see if the value exits.
        for (i = 0; i < numEdges; i++)
        {
            if (value == node->edges[i].value)
            {
                childNode = (DecisionTreeNode_t *)(node->edges[i].nextNode);

                if (childNode->isLeafNode)
                {
                    for (j = 0; j < ((int)(childNode->data.indexes.size())); j++)
                    {
                        // This child node is a leaf node. It means that it contains the
                        // data.
                        index = childNode->data.indexes[j];
                        dataRating = rating;
                        
                        // If it had a negative feedback rate it less.
                        // If it had a positive feedback rate it more.
                        switch (childNode->data.statuses[j]) {

                        case POSITIVE_DATA:
                            dataRating++;
                            break;

                        case NEGATIVE_DATA:
                            dataRating--;
                            break;
                        }

                        if (dataRating > 0)
                        {
                            indexes.push_back(index);
                            ratings->push_back(dataRating);
                        }
                    }
                    return indexes;
                }
                else
                {
                    // If the child is not a leaf node. Remember this node to explore
                    // it later.
                    evaluationNodeStack.push_back(childNode);
                }
            }
        }
    }

    if (disableBacktrack)
    {
        return indexes;
    }

    // The perfect match not found. Backtrack to see if compromised version exists.
    while (((int)(backtrackNodeStack.size())) > 0)
    {
        if (rating <= 1)
        {
            //break;
        }
        else
        {
            rating--;
        }

        node = backtrackNodeStack.back();
        backtrackNodeStack.pop_back();

        visitedNodeNames.push_back(node->name);

        numEdges = node->edges.size();

        // Look for all the edges to see if data exits.
        for (i = 0; i < numEdges; i++)
        {
            childNode = (DecisionTreeNode_t *)(node->edges[i].nextNode);
	    
            if (childNode->isLeafNode)
            {
                for (j = 0; j < ((int)(childNode->data.indexes.size())); j++)
                {
                    // This child node is a leaf node. It means that it contains the
                    // data.
                    index = childNode->data.indexes[j];
                    dataRating = rating;

                    // If it had a negative feedback rate it less.
                    // If it had a positive feedback rate it more.
                    switch (childNode->data.statuses[j]) {
			
                    case POSITIVE_DATA:
                        dataRating++;
                        break;
			
                    case NEGATIVE_DATA:
                        dataRating--;
                        break;
                    }

                    
                    if (dataRating > 0)
                    //if (dataRating >= MIN_RATING) // ENDO 20040406 -- Undid on 20040408
                    {
                        indexes.push_back(index);
                        ratings->push_back(dataRating);
                    }
                }
            }
            else
            {
                // If this node is saved before, save it for further backtracking.
                savedBefore = false;
                for (j = 0; j < ((int)(visitedNodeNames.size())); j++)
                {
                    if (visitedNodeNames[j] == childNode->name)
                    {
                        savedBefore = true;
                        break;
                    }
                }

                if (!savedBefore)
                {
                    backtrackNodeStack.push_back(childNode);
                }
            }
        }
    }

    return indexes;
}

//-----------------------------------------------------------------------
// This function prints out the entire decision tree.
//-----------------------------------------------------------------------
void MemoryManager::dumpDecisionTree_(DecisionTree_t *decisionTree)
{
    int i, numEdges;
    DecisionTreeNode_t *node;
    DecisionTreeEdge_t edge;
    vector<DecisionTreeEdge_t> edgeStack;

    gDebugger->printfLine();
    gDebugger->printfDebug("ROOT NODE:\n");
    gDebugger->printfDebug("NAME %s\n", decisionTree->name.c_str());
    gDebugger->printfDebug("LEAF %d\n", decisionTree->isLeafNode);
    gDebugger->printfDebug("EDGES %d\n", decisionTree->edges.size());

    edgeStack = decisionTree->edges;

    while(!(edgeStack.empty()))
    {
        edge = edgeStack.back();
        edgeStack.pop_back();
        gDebugger->printfLine();
        gDebugger->printfDebug("EDGE PARENT %s\n", edge.parentName.c_str());
        gDebugger->printfDebug("EDGE VALUE %s\n", edge.value.c_str());

        node = (DecisionTreeNode_t *)edge.nextNode;
        gDebugger->printfDebug("NEW NODE:\n");
        gDebugger->printfDebug("NAME %s\n", node->name.c_str());
        gDebugger->printfDebug("LEAF %d\n", node->isLeafNode);
        gDebugger->printfDebug("EDGES %d\n", node->edges.size());

        if (node->isLeafNode)
        {
            for (i = 0; i < ((int)(node->data.indexes.size())); i++)
            {
                gDebugger->printfDebug("DATA %d\n", node->data.indexes[i]);
                gDebugger->printfDebug("STATUS %d\n", node->data.statuses[i]);
            }
        }

        numEdges = node->edges.size();
        for (i = 0; i < numEdges; i++)
        {
            edgeStack.push_back(node->edges[i]);
        }
    }
}

//-----------------------------------------------------------------------
// This function deals with the user's positive feedback.
//-----------------------------------------------------------------------
void MemoryManager::givePositiveFeedback(vector<int> dataIndexList)
{
    int i;

    for (i = 0; i < (int)(dataIndexList.size()); i++)
    {
        giveFeedback_(decisionTree_, dataIndexList[i], POSITIVE_DATA);

        // Report the status.
        gWindows->printfPlannerWindow("Positive feedback recorded (ID %d).\n");
    }
}

//-----------------------------------------------------------------------
// This function deals with the user's negative feedback.
//-----------------------------------------------------------------------
void MemoryManager::giveNegativeFeedback(vector<int> dataIndexList)
{
    int i;

    for (i = 0; i < (int)(dataIndexList.size()); i++)
    {
        giveFeedback_(decisionTree_, dataIndexList[i], NEGATIVE_DATA);

        // Report the status.
        gWindows->printfPlannerWindow("Negative feedback recorded (ID %d).\n");
    }
}

//-----------------------------------------------------------------------
// This function deals with the user's positive feedback.
//-----------------------------------------------------------------------
void MemoryManager::giveFeedback_(DecisionTreeNode_t *node, int index, int status)
{
    int i;
    DecisionTreeEdge_t edge;
    DecisionTreeNode_t *nextNode = NULL;

    if (node->isLeafNode)
    {
        for (i = 0; i < (int)(node->data.indexes.size()); i++)
        {
            if (node->data.indexes[i] == index)
            {
                node->data.statuses[i] = status;
                memoryChanged_ = true;
                return;
            }
        }
        return;
    }

    for (i = 0; i < (int)(node->edges.size()); i ++)
    {
        edge = node->edges[i];

        nextNode = (DecisionTreeNode_t *)(edge.nextNode);

        giveFeedback_(nextNode, index, status);
    }
}

//-----------------------------------------------------------------------
// This function descends the current node (decision tree) based on the
// chosen edge, and return the new node. If the chosen Edge is not found,
// it just returns the root of the decision tree.
//-----------------------------------------------------------------------
MemoryManager::DecisionTreeNode_t *MemoryManager::getCurrentNode_(string chosenEdge)
{
    int i;

    if (chosenEdge == EMPTY_STRING_)
    {
        resetCurrentNode_();
        return currentNode_;
    }

    if (currentNode_ == NULL)
    {
        gWindows->printfPlannerWindow(
            "Warning: Error in memory.\n");
        resetCurrentNode_();
        return currentNode_;
    }

    for (i = 0; i < (int)(currentNode_->edges.size()); i++)
    {
        if (currentNode_->edges[i].value == chosenEdge)
        {
            if (currentNode_->edges[i].nextNode == NULL)
            {
                gWindows->printfPlannerWindow(
                    "Warning: Error in memory.\n");
                resetCurrentNode_();
                return currentNode_;
            }

            currentNode_ = (DecisionTreeNode_t *)(currentNode_->edges[i].nextNode);
            return currentNode_;
        }
    }

    gWindows->printfPlannerWindow(
        "Warning: Error in memory.\n");
    resetCurrentNode_();
    return currentNode_;
}

/**********************************************************************
 * $Log: memory_manager.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.3  2007/09/29 23:45:36  endo
 * Global feature can be now disabled.
 *
 * Revision 1.2  2007/08/04 23:53:59  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
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
