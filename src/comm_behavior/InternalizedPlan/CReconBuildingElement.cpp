/*********************************************************************
 **                                                                  **
 **                     CReconBuildingElement.h                      **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Reconnaisance planning element... and a work in         **
 ** progress...                                                      **
 **                                                                  **
 **********************************************************************/

#include "stdafx.h"

#include "CReconBuildingElement.h"
#include "CMap.h"
#include "CCoordinate.h"
#include "CPlan.h"
#include "ProjectDefinitions.h"
#include "CCalculateMapCosts.h"
#include "CLoadMap.h"

#define GOTO_CONTINGENCY 15
#define COMMUNICATIONS_CUTOFF_VALUE 10
#define PAUSE_VALUE 1
#define MAX_TIMEOUT_VALUE 500
#define MAX_LOCAL_CONTIGENCY_VALUE 500

CReconBuildingElement::CReconBuildingElement():dGains(6,-1)
{
	CommunicationsMapLeft = new CMap();
    CoverageMapLeft = new CMap();
    CommunicationsMapRight = new CMap();
    CoverageMapRight = new CMap();
    leftFourRobotPoint = new CMap();
    rightFourRobotPoint = new CMap();

    sElementType = "RECONBUILDING";
    iLineNumbersForEdit = 0;

    iGoToCommsContingencyPlan = 0;
	iPauseBetweenSteps = 0;
	iProgressPoint = 0;
    bCommsWentDown = false;
	iTimeout = 0;
	bElementOverride=false;
}

CReconBuildingElement::~CReconBuildingElement()
{
    delete CommunicationsMapLeft;
    delete CoverageMapLeft;
    delete CommunicationsMapRight;
    delete CoverageMapRight;
    delete leftFourRobotPoint;
    delete rightFourRobotPoint;
}

void CReconBuildingElement::setLeftFourRobotPointGoal( CCoordinate lfGoal )
{
	leftFourRobotPoint->setGoal( lfGoal.getX(), lfGoal.getY() );
}

void CReconBuildingElement::setRightFourRobotPointGoal( CCoordinate rfGoal )
{
	rightFourRobotPoint->setGoal( rfGoal.getX(), rfGoal.getY() );
}

vector< CMap* > CReconBuildingElement::GetMaps() const
{
    vector< CMap* > rVector;

    rVector.push_back( CommunicationsMapLeft );
    rVector.push_back( CoverageMapLeft );
    rVector.push_back( CommunicationsMapRight );
    rVector.push_back( CoverageMapRight );
    rVector.push_back( leftFourRobotPoint );
    rVector.push_back( rightFourRobotPoint );
    
    return rVector;
}

bool CReconBuildingElement::IsWithinProgressBox(CCoordinate position, vector< CCoordinate > vProgressBox) const
{
  int iMinX;
  int iMaxX;
  int iMinY;
  int iMaxY;
  
  if( vProgressBox.size() == 2 )
  {
	  if(  vProgressBox[0].getX() < vProgressBox[1].getX() )
	  {	
		  iMinX = vProgressBox[0].getX();
		  iMaxX = vProgressBox[1].getX();
	  }
	  else
	  {
		  iMinX = vProgressBox[1].getX();
		  iMaxX = vProgressBox[0].getX();
	  }
  
	  if(  vProgressBox[0].getY() < vProgressBox[1].getY() )
	  {	
		  iMinY = vProgressBox[0].getY();
		  iMaxY = vProgressBox[1].getY();
	  }
	  else
	  {
		  iMinY = vProgressBox[1].getY();
		  iMaxY = vProgressBox[0].getY();
	  }

	  if ( position.getX() >= iMinX && position.getX() <= iMaxX )
	  {
		  if( position.getY() >= iMinY && position.getY() <= iMaxY )
		  {
			  return true;
		  }
	  }
  }
  
  return false;
}

VectorConfidenceAdvice CReconBuildingElement::getAdviceVector(CFeatureVector* cFeatureVector, int iRobot)
{
    VectorConfidenceAdvice vOutput;

	vector< CRobotLocationCommunication> communicationList = cFeatureVector->getCommsList();
	CCoordinate locationOfThisRobot = cFeatureVector->GetMyCurrentLocation();
	
	if( locationOfThisRobot.getX() != locationCounter.getX() || locationOfThisRobot.getY() != locationCounter.getY() )
	{ 
	   locationCounter = locationOfThisRobot;
	   iTimeout = 0;
	}
	else
	{
	  iTimeout++;
	}

	if( iTimeout > MAX_TIMEOUT_VALUE || iGoToCommsContingencyPlan > MAX_LOCAL_CONTIGENCY_VALUE )
	{
	  bElementOverride=true;
	}

	
	double dCommunicationMultplier = -1;
	double dCommunicationMultplier2 = -1;

	if( iNumberOfRobots == 2)
	{
		if( iRobot == 1)
		    dCommunicationMultplier = communicationList[1].dSignalStrength[2];
		else
			dCommunicationMultplier = communicationList[2].dSignalStrength[1];
	}
	else if( iNumberOfRobots == 3)
	{	
		if( iRobot == 1 )
			dCommunicationMultplier = communicationList[1].dSignalStrength[2];
		else if( iRobot == 3 )
			dCommunicationMultplier = communicationList[3].dSignalStrength[2];
		else 
		{
			dCommunicationMultplier = communicationList[2].dSignalStrength[1];	
			dCommunicationMultplier2 = communicationList[2].dSignalStrength[3];
		}
	}
	else if( iNumberOfRobots == 4)
	{
		if( iRobot == 1)
		{	
		    dCommunicationMultplier = communicationList[1].dSignalStrength[2];
		}
		else if(iRobot == 4)
			dCommunicationMultplier = communicationList[4].dSignalStrength[3];
		else if(iRobot == 2)
		{
			dCommunicationMultplier = communicationList[2].dSignalStrength[1];	
			dCommunicationMultplier2 = communicationList[2].dSignalStrength[3];
		}
		else if(iRobot == 3)
		{
			dCommunicationMultplier = communicationList[3].dSignalStrength[2];	
			dCommunicationMultplier2 = communicationList[3].dSignalStrength[4];	
		}
		 else
		 {
		    cout << "Never get here"<<endl;
		 }
	}
    else
	{
	  cout << "Never get here"<<endl;
	}
	

	
	//start with this and add later
	if( dCommunicationMultplier > 100)
		dCommunicationMultplier =100;

	if( dCommunicationMultplier2 > 100)
		dCommunicationMultplier2 =100;
	
	if( iNumberOfRobots == 4 )
	{
        CCoordinate robotOnesLocation = communicationList[1].cLocation;
        CCoordinate robotTwosLocation = communicationList[2].cLocation;
        CCoordinate robotThreesLocation = communicationList[3].cLocation;
        CCoordinate robotFoursLocation = communicationList[4].cLocation;		

        if( iProgressPoint == 0)
        {		  
		    if( iRobot == 1)//1
            {
			    if( IsWithinProgressBox( robotOnesLocation, vProgressBoxLeft) == true )
    	        {
				        if( (communicationList[1].dSignalStrength[4] > COMMUNICATIONS_CUTOFF_VALUE) &&
					    (communicationList[1].dSignalStrength[2] > COMMUNICATIONS_CUTOFF_VALUE))													 
                        {
					        iProgressPoint = 1;  //wait
					    }
				        else if( communicationList[1].dSignalStrength[4] > COMMUNICATIONS_CUTOFF_VALUE)
					    {
					        iProgressPoint = 1;   //wait
					    }
					    else if( communicationList[1].dSignalStrength[2] > COMMUNICATIONS_CUTOFF_VALUE)
				        {
					        ;
					    }
					    else
						    iGoToCommsContingencyPlan++;
			     }
			}
		    else if( iRobot == 2 || iRobot == 3 )//2 || 3
            {
                if( IsWithinProgressBox( robotOnesLocation, vProgressBoxLeft) == true && 
					IsWithinProgressBox( robotFoursLocation, vProgressBoxRight) == true )
				  {
				  	  iProgressPoint = 1;  
				  }	     	       
            }
		    else
   	        {
                if( IsWithinProgressBox( robotFoursLocation, vProgressBoxRight) == true ) 
                {			   
				    if( (communicationList[4].dSignalStrength[1] > COMMUNICATIONS_CUTOFF_VALUE) &&
					    (communicationList[4].dSignalStrength[3] > COMMUNICATIONS_CUTOFF_VALUE))													 
                    {
					  iProgressPoint = 1;  //wait
					}
				    else if( communicationList[4].dSignalStrength[1] > COMMUNICATIONS_CUTOFF_VALUE)
					{
					  iProgressPoint = 1;   //wait
					}
					else if( communicationList[4].dSignalStrength[3] > COMMUNICATIONS_CUTOFF_VALUE)
				    {
					  ;
					}
					else
						iGoToCommsContingencyPlan++;	
                 }
            }
        }
        else if( iProgressPoint == 1 )
        {
            if( iRobot == 1 )   //this may be fragile --- your trying to capture the robot moving through an area...
            {
                if( IsWithinProgressBox( robotTwosLocation, vProgressBoxLeft) == true )
                {
				  iProgressPoint = 2;
                }
            }
            else if(iRobot == 4)
            {
                if(  IsWithinProgressBox( robotThreesLocation, vProgressBoxRight) == true )
                {
                    iProgressPoint = 2;	
                }
            }
        }
   	}
    else if( iNumberOfRobots == 3)
    {
        CCoordinate robotOnesLocation = communicationList[1].cLocation;
		CCoordinate robotTwosLocation = communicationList[2].cLocation;
        CCoordinate robotThreesLocation = communicationList[3].cLocation;

        if( iProgressPoint == 0 )
		{
		    if( iRobot == 1)
			{
                if( IsWithinProgressBox( robotOnesLocation, vProgressBoxLeft) == true )
                { 
				  if( communicationList[1].dSignalStrength[1] > COMMUNICATIONS_CUTOFF_VALUE )
				    iProgressPoint = 1;
				  else
					iGoToCommsContingencyPlan++;
			    }
			}
		    else if( iRobot == 3 )
			{
                if(  IsWithinProgressBox( robotThreesLocation, vProgressBoxRight) == true )
                {
				    if( communicationList[3].dSignalStrength[1] > COMMUNICATIONS_CUTOFF_VALUE )
                      iProgressPoint = 1;  
                    else
                      iGoToCommsContingencyPlan++;
				}
			}
		    else
			{
			    if( (IsWithinProgressBox( robotOnesLocation, vProgressBoxLeft) == true) &&  
					(IsWithinProgressBox( robotThreesLocation, vProgressBoxRight) == true) )
                {
                    if( iPauseBetweenSteps < PAUSE_VALUE) 
					{
#ifdef MISSIONLAB_SIMULATION
					  sleep(1);   
#endif
					  iPauseBetweenSteps++;
					}
					else
					  iProgressPoint = 1;  
				}
			}
		}
		else if( iProgressPoint == 1 )
        {
            if( (iRobot == 1) || (iRobot == 3) )   //this may be fragile --- your trying to capture the robot moving through an area...
            {
			    if( (IsWithinProgressBox( robotTwosLocation, vProgressBoxLeft) == true) &&
					(IsWithinProgressBox( robotTwosLocation, vProgressBoxRight) == true) )
                {                    
                    iProgressPoint = 2;	
                }
            }
        }
    }
	
	if( (dCommunicationMultplier == 0) || (dCommunicationMultplier2 == 0) )
	{
	  iGoToCommsContingencyPlan++;
	}

    if( iGoToCommsContingencyPlan > GOTO_CONTINGENCY ) //Went to Contigency Plan b/c Comms Bad
    {
   	    goal = CommunicationsMapRight->getGoal();

        if((iNumberOfRobots == 2) && (iRobot == 1) &&  (dCommunicationMultplier < COMMUNICATIONS_CUTOFF_VALUE))
        {
            dGains[1] = 0;   //make one robot stop and wait for the other...
            dGains[2] = 0;
            dGains[3] = 0;
            dGains[4] = 0;
            dGains[0] = 0;			
        }
        else if ((iNumberOfRobots == 3) && (iRobot == 2) && (dCommunicationMultplier2 < COMMUNICATIONS_CUTOFF_VALUE))
        {
            dGains[1] = 0;   //make one robot stop and wait for the other...
            dGains[2] = 0;
            dGains[3] = 0;
            dGains[4] = 0;
            dGains[0] = 0;
        }
		else if ((iNumberOfRobots == 3) && (iRobot == 1) && (dCommunicationMultplier < COMMUNICATIONS_CUTOFF_VALUE))
        {
            dGains[1] = 0;   //make one robot stop and wait for the other...
            dGains[2] = 0;   //May need to be commented out when using tethered
            dGains[3] = 0;
            dGains[4] = 0;            
            dGains[0] = 0;
        }
		
        else if ((iNumberOfRobots == 4) && (iRobot == 1) && (dCommunicationMultplier < COMMUNICATIONS_CUTOFF_VALUE))
        {
            dGains[1] = 0;   //make one robot stop and wait for the other...
            dGains[2] = 0;
            dGains[3] = 0;
            dGains[4] = 0;
            dGains[0] = 0;
        }
        else if ((iNumberOfRobots == 4) && (iRobot == 2) && (dCommunicationMultplier2 < COMMUNICATIONS_CUTOFF_VALUE))
        {
            dGains[1] = 0;   //make one robot stop and wait for the other...
            dGains[2] = 0;
            dGains[3] = 0;
            dGains[4] = 0;
            dGains[0] = 0;
        } 
        else if ((iNumberOfRobots == 4) && (iRobot == 3) && (dCommunicationMultplier2 < COMMUNICATIONS_CUTOFF_VALUE))
        {
            dGains[1] = 0;   //make one robot stop and wait for the other...
            dGains[2] = 0;
            dGains[3] = 0;
            dGains[4] = 0;
            dGains[0] = 0;
        }
        else 
        {
            dGains[1] = 1;
            dGains[2] = 0;
            dGains[3] = 0;
            dGains[4] = 0;
            dGains[0] = 0;
        }
    }
	else   //Comms Good
    {
        if(iNumberOfRobots == 4) 
        {
            if(iRobot == 1 || iRobot == 4 )
            {
                if( iProgressPoint == 0 || iProgressPoint == 2)
                {
                    dGains[1] = 0;   //make one robot stop and wait for the other...
                    dGains[2] = 1;
                    dGains[3] = 0;
                    dGains[4] = 0;
                    dGains[0] = 0;
                }
                else
                {
                    dGains[1] = 0;
                    dGains[2] = 0;
                    dGains[3] = 0;
                    dGains[4] = 0;
                    dGains[0] = 0;
                }
            }
            else if( iRobot == 2 )
            {
                if( iProgressPoint == 0 )
                {
                    dGains[1] = 0;   //make one robot stop and wait for the other...
                    dGains[2] = 0;
                    dGains[3] = 0;
                    dGains[4] = 1;
                    dGains[0] = 0;
                } 
                else
				  {
                    dGains[1] = 0;   //make one robot stop and wait for the other...
                    dGains[2] = 1;
                    dGains[3] = 0;
                    dGains[4] = 0;
                    dGains[0] = 0;
                }
            }
            else if(iRobot == 3 )
            {
                if( iProgressPoint == 0 )
                {
                    dGains[1] = 0;   //make one robot stop and wait for the other...
                    dGains[2] = 0;
                    dGains[3] = 1;
                    dGains[4] = 0;
                    dGains[0] = 0;
                }
                else
                {
                    dGains[1] = 0;   //make one robot stop and wait for the other...
                    dGains[2] = 1;
                    dGains[3] = 0;
                    dGains[4] = 0;
                    dGains[0] = 0;
                }
            }
        }	    										//one robot-->this is a special case for robot 2 in a 3 team and robot 2 and 3 in a four team
		else if( iNumberOfRobots == 3)
        {
           if( iRobot == 1 || iRobot == 3)
           {
			  if( iProgressPoint == 0 || iProgressPoint == 2)
              {
				  dGains[1] = 0;   //comms are good
				  dGains[2] = 1;
				  dGains[3] = 0;
				  dGains[4] = 0;
				  dGains[0] = 0;
			  } 
			  else
              { 
				/*                  dGains[1] = 0;
                    dGains[2] = 0;
                    dGains[3] = 0;
                    dGains[4] = 0;
                    dGains[0] = 0;
				*/		
				    cout << "Some spot2: "<< iRobot<<" dComm: "<<dCommunicationMultplier<<endl;
              }
           }
           else if ( iRobot == 2)
           {
			 if( iProgressPoint == 0 )
             {
                    dGains[1] = 0;   //comms are good
                    dGains[2] = 0;
                    dGains[3] = 0;
                    dGains[4] = 0;
		            dGains[0] = 0;
			 }
			 else
			 {
			        dGains[1] = 0;   //comms are good
                    dGains[2] = 1;
                    dGains[3] = 0;
                    dGains[4] = 0;
		            dGains[0] = 0;
			 }
		   }
	    }
   	    else	//all two robot teams
	    {
		    if( dCommunicationMultplier > COMMUNICATIONS_CUTOFF_VALUE )
		    {
			    dGains[1] = 0;
			    dGains[2] = 1;
			    dGains[3] = 0;
			    dGains[4] = 0;
			    dGains[0] = 0;
		    }
		    else
		    {
			    dGains[1] = 1;
			    dGains[2] = 0;
			    dGains[3] = 0;
			    dGains[4] = 0;
			    dGains[0] = 0;
		    }
	    }   //end dCommunicationMultplier2 != -1
   	}   //end bContigency
	
	VectorConfidenceAdvice vCommunicationsVector; 
	VectorConfidenceAdvice vCoverageVector;
	
	if( iRobot == 1 )
	{
	  	vCoverageVector = CoverageMapLeft->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY());  
		vCommunicationsVector = CommunicationsMapLeft->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY()); 	
	}       //the leftmost robot
	else if( ((iRobot == 2) && (iNumberOfRobots==2)) ||
             ((iRobot == 3) && (iNumberOfRobots==3)) ||
             ((iRobot == 4) && (iNumberOfRobots==4))
           )
	{
	  	vCoverageVector = CoverageMapRight->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY()); 
		vCommunicationsVector = CommunicationsMapLeft->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY()); 
	}
	else if((iRobot == 2) && (iNumberOfRobots==3))
	{
	    vCoverageVector = CoverageMapLeft->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY());
		vCommunicationsVector = CommunicationsMapLeft->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY()); 
	}
	else if( (iRobot == 2) && (iNumberOfRobots==4))
	{
	  if( iProgressPoint == 1)
	  {	
		 dGains[1] = 0;
		 dGains[2] = 1;
		 dGains[3] = 0;
		 dGains[4] = 0;
		 dGains[0] = 0;
      }
      vCoverageVector = CoverageMapLeft->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY());
	  vCommunicationsVector =  CommunicationsMapLeft->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY());
	}
	else if( (iRobot == 3) && (iNumberOfRobots==4))
	{
	  if( iProgressPoint == 1)
	  {
		 dGains[1] = 0;
		 dGains[2] = 1;
		 dGains[3] = 0;
		 dGains[4] = 0;
		 dGains[0] = 0;
      }
   	  vCoverageVector = CoverageMapRight->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY());  	
	  vCommunicationsVector =  CommunicationsMapLeft->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY());
	}
	    
    VectorConfidenceAdvice vLeftFourRobotPoint = leftFourRobotPoint->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY()); 
    VectorConfidenceAdvice vRightFourRobotPoint = rightFourRobotPoint->getVector((double)locationOfThisRobot.getX(),(double)locationOfThisRobot.getY()); 

	vOutput.SetX( (dGains[1]*vCommunicationsVector.GetX() + dGains[2]*vCoverageVector.GetX() + 
		dGains[3]*vRightFourRobotPoint.GetX() +dGains[4]*vLeftFourRobotPoint.GetX()) );
	vOutput.SetY( (dGains[1]*vCommunicationsVector.GetY() + dGains[2]*vCoverageVector.GetY() +
		dGains[3]*vRightFourRobotPoint.GetY() +dGains[4]*vLeftFourRobotPoint.GetY()) );

	//Need to deal with problem of iGotToCommMap --> INFINITY
	if( iRobot == 4 && 0 )
    {
	    cout << "iGotToCommMap: "<< iGoToCommsContingencyPlan << endl;
 	    cout << "vCoverage(" << iRobot << "): <"<< vCoverageVector.GetX() << "," << vCoverageVector.GetY() << ">"<<endl;
	    cout << "vComms(" << iRobot << "): <"<< vCommunicationsVector.GetX() << "," << vCommunicationsVector.GetY() << ">"<<endl;
	    CCoordinate temp = CoverageMapLeft->getGoal();
		CCoordinate temp2 =  leftFourRobotPoint->getGoal();	
	    cout << "vCover Goal(" << iRobot << "): <"<< temp.getX() << "," << temp.getY() << ">"<<endl;
		cout << "vLeftFour Goal(" << iRobot << "): <"<< temp2.getX() << "," << temp2.getY() << ">"<<endl;
		cout << "vLeftFour (" << iRobot << "): <"<< vLeftFourRobotPoint.GetX() << "," << vLeftFourRobotPoint.GetY() << ">"<<endl;	
		cout << "Position (" << iRobot << "): <"<< (double)locationOfThisRobot.getX() << "," << (double)locationOfThisRobot.getY() << ">"<<endl;
		cout << "ProgressPoint: " << iProgressPoint << endl; 
		cout << "ProgressBoxRight0 <"<< vProgressBoxRight[0].getX() << "," <<   vProgressBoxRight[0].getY() << ">" << endl;
		cout << "ProgressBoxRight1 <"<< vProgressBoxRight[1].getX() << "," <<   vProgressBoxRight[1].getY() << ">" << endl;
		cout << "ProgressBoxLeft0 <"<< vProgressBoxLeft[0].getX() << "," <<   vProgressBoxLeft[0].getY() << ">" << endl;
		cout << "ProgressBoxLeft1 <"<< vProgressBoxLeft[1].getX() << "," <<   vProgressBoxLeft[1].getY() << ">" << endl;
		cout << "dGains: " <<  dGains[1]<<" "  << dGains[2]<< " " << dGains[3]<<" "<< dGains[4]<< " " << dGains[0] << endl;
		cout << "ADVICE(" << iRobot << "): <"<< vOutput.GetX()<< "," <<vOutput.GetY() << ">"<<endl; 
		cout << endl;
		cout << endl;
	}
	return vOutput;

}

void CReconBuildingElement::writePlanToConfigFile(ofstream& fStream, int stepNumber)
{
    fStream << this->sElementType << endl;
    fStream << "#Plan step number" << endl;
    fStream << stepNumber << endl;
    fStream << "#Goal Position" << endl;
    fStream << "<" << this->goal.getX() <<"," << this->goal.getY()<<">" << endl;
    fStream << "#Left Four Robot Goal Pos" << endl;
    fStream << "<" << this->leftFourRobotPoint->getGoal().getX() <<"," << this->leftFourRobotPoint->getGoal().getY()<<">" << endl;
    fStream << "#Right Four Robot Goal Pos" << endl;
    fStream << "<" << this->rightFourRobotPoint->getGoal().getX() <<"," << this->rightFourRobotPoint->getGoal().getY()<<">" << endl;
    fStream << "#Bounding Box" << endl;
    for(int i=0;i<4;i++)
        fStream << "<" << this->vBoundaryBox[i].getX() <<"," << this->vBoundaryBox[i].getY()<<">" << endl;
    fStream << "#Left Progress Box" << endl;
    fStream << "<" << this->vProgressBoxLeft[0].getX() <<"," << this->vProgressBoxLeft[0].getY()<<">" << endl;
    fStream << "<" << this->vProgressBoxLeft[1].getX() <<"," << this->vProgressBoxLeft[1].getY()<<">" << endl;
    fStream << "#Right Progress Box" << endl;
    fStream << "<" << this->vProgressBoxRight[0].getX() <<"," << this->vProgressBoxRight[0].getY()<<">" << endl;
    fStream << "<" << this->vProgressBoxRight[1].getX() <<"," << this->vProgressBoxRight[1].getY()<<">" << endl;
    fStream << "#Preprocessing Obstacle Left" << endl;
    fStream << "<" << this->preprocessStartLeft.getX() <<"," << this->preprocessStartLeft.getY()<<">" << endl;
    fStream << "<" << this->preprocessEndLeft.getX() <<"," << this->preprocessEndLeft.getY()<<">" << endl;
    fStream << "#Preprocessing Obstacle Right" << endl;
    fStream << "<" << this->preprocessStartRight.getX() <<"," << this->preprocessStartRight.getY()<<">" << endl;
    fStream << "<" << this->preprocessEndRight.getX() <<"," << this->preprocessEndRight.getY()<<">" << endl;
    fStream << "#Edge Growth Left" << endl;
    if( growObstacleLeft.size() == 0 )
        fStream << "NONE" <<endl;
    else
    {
        fStream << "<" << this->growObstacleLeft[0].getX() <<"," << this->growObstacleLeft[0].getY()<<">" << endl;
        fStream << "<" << this->growObstacleLeft[1].getX() <<"," << this->growObstacleLeft[1].getY()<<">" << endl;
    }
    fStream << "#Edge Growth Right" << endl;
    if( growObstacleRight.size() == 0 )
        fStream << "NONE" <<endl;
    else
    {
        fStream << "<" << this->growObstacleRight[0].getX() <<"," << this->growObstacleRight[0].getY()<<">" << endl;
        fStream << "<" << this->growObstacleRight[1].getX() <<"," << this->growObstacleRight[1].getY()<<">" << endl;
    }
}

int CReconBuildingElement::editLineNumbers( int iLineNumber )
{
 //   char buffer[255];
    int iValue;
//    bool bValue;
    int xValue;
    int yValue;

    string bTeth;
    string pType;

    size_t iTotalSize = 17 + growObstacleRight.size() + growObstacleLeft.size();
    size_t iStartField = iLineNumbersForEdit - iTotalSize + 1;
    size_t iField = iLineNumber - iStartField;

    switch(iField)
    {
    case 0:
        cout << "Edit line: "<<iLineNumber<<" Value: " <<this->getElementNumber() <<endl;
        cout << "Enter new value: "<<endl;
        cin >> iValue;
        this->setElementNumber( iValue );
        return 1;
    case 1:
        cout << "This line is not editable"<<endl;
        break;
    case 2:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->goal.getX() <<"," << this->goal.getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        goal.setXY( xValue, yValue );
        break;
    case 3:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->leftFourRobotPoint->getGoal().getX() <<"," << this->leftFourRobotPoint->getGoal().getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->leftFourRobotPoint->setGoal( xValue, yValue );
        break;
    case 4:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->rightFourRobotPoint->getGoal().getX() <<"," << this->rightFourRobotPoint->getGoal().getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->rightFourRobotPoint->setGoal( xValue, yValue );
        break;
    case 5:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->vBoundaryBox[0].getX() <<"," << this->vBoundaryBox[0].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->vBoundaryBox[0].setXY( xValue, yValue );
        break;
    case 6:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->vBoundaryBox[1].getX() <<"," << this->vBoundaryBox[1].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->vBoundaryBox[1].setXY( xValue, yValue );
        break;
    case 7:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->vBoundaryBox[2].getX() <<"," << this->vBoundaryBox[2].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->vBoundaryBox[2].setXY( xValue, yValue );
        break;
    case 8:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->vBoundaryBox[3].getX() <<"," << this->vBoundaryBox[3].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->vBoundaryBox[3].setXY( xValue, yValue );
        break;
    case 9:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->vProgressBoxLeft[0].getX() <<"," << this->vProgressBoxLeft[0].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->vProgressBoxLeft[0].setXY( xValue, yValue );
        break;
    case 10:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->vProgressBoxLeft[1].getX() <<"," << this->vProgressBoxLeft[1].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->vProgressBoxLeft[1].setXY( xValue, yValue );
        break;
    case 11:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->vProgressBoxRight[0].getX() <<"," << this->vProgressBoxRight[0].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->vProgressBoxRight[0].setXY( xValue, yValue );
        break;
    case 12:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->vProgressBoxRight[1].getX() <<"," << this->vProgressBoxRight[1].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->vProgressBoxRight[1].setXY( xValue, yValue );
        break;
    case 13:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->preprocessStartLeft.getX() <<"," << this->preprocessStartLeft.getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->preprocessStartLeft.setXY( xValue, yValue );
        break;
    case 14:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->preprocessEndLeft.getX() <<"," << this->preprocessEndLeft.getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->preprocessEndLeft.setXY( xValue, yValue );
        break;
    case 15:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->preprocessStartRight.getX() <<"," << this->preprocessStartRight.getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->preprocessStartRight.setXY( xValue, yValue );
        break;
    case 16:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->preprocessEndRight.getX() <<"," << this->preprocessEndRight.getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->preprocessEndRight.setXY( xValue, yValue );
        break;

   case 17:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->growObstacleLeft[0].getX() <<"," << this->growObstacleLeft[0].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->growObstacleLeft[0].setXY( xValue, yValue );
        break;
       
    case 18:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->growObstacleLeft[1].getX() <<"," << this->growObstacleLeft[1].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->growObstacleLeft[1].setXY( xValue, yValue );
        break;
   
    case 19:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->growObstacleRight[0].getX() <<"," << this->growObstacleRight[0].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->growObstacleRight[0].setXY( xValue, yValue );
        break;
       
    case 20:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->growObstacleRight[1].getX() <<"," << this->growObstacleRight[1].getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        this->growObstacleRight[1].setXY( xValue, yValue );
        break;

    default:
        cout << "Error" << endl;
    }
    return 0;
}

void CReconBuildingElement::print(bool bNumbered, int iOutput, int& iItemNumber)
{
    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Element Number:\t\t" << this->iElementNumber << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    string temp = this->sElementType;
    cout << "Element Type:\t\t" << this->sElementType << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Goal Location:\t\t" << "<" << this->goal.getX() <<"," << this->goal.getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Left Four Robot Goal Position:\t\t" << "<"<<this->leftFourRobotPoint->getGoal().getX()<<","<<this->leftFourRobotPoint->getGoal().getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Right Four Robot Goal Position:\t\t" << "<"<<this->rightFourRobotPoint->getGoal().getX()<<","<<this->leftFourRobotPoint->getGoal().getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Bounding Box 1:\t\t" << "<"<<this->vBoundaryBox[0].getX()<<","<<this->vBoundaryBox[0].getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Bounding Box 2:\t\t" << "<"<<this->vBoundaryBox[1].getX()<<","<<this->vBoundaryBox[1].getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Bounding Box 3:\t\t" << "<"<<this->vBoundaryBox[2].getX()<<","<<this->vBoundaryBox[2].getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Bounding Box 4:\t\t" << "<"<<this->vBoundaryBox[3].getX()<<","<<this->vBoundaryBox[3].getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Progress Box Left--Corner 1:\t\t" << "<"<<this->vProgressBoxLeft[0].getX()<<","<<this->vProgressBoxLeft[0].getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Progress Box Left--Corner 2:\t\t" << "<"<<this->vProgressBoxLeft[1].getX()<<","<<this->vProgressBoxLeft[1].getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Progress Box Right--Corner 1:\t\t" << "<"<<this->vProgressBoxRight[0].getX()<<","<<this->vProgressBoxRight[0].getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Progress Box Right--Corner 2:\t\t" << "<"<<this->vProgressBoxRight[1].getX()<<","<<this->vProgressBoxRight[1].getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Preprocess Start Left:\t\t" << "<"<<this->preprocessStartLeft.getX()<<","<<this->preprocessStartLeft.getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Preprocess End Left:\t\t" << "<"<<this->preprocessEndLeft.getX()<<","<<this->preprocessEndLeft.getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Preprocess Start Right:\t\t" << "<"<<this->preprocessStartRight.getX()<<","<<this->preprocessStartRight.getY()<<">" << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Preprocess End Right:\t\t" << "<"<<this->preprocessEndRight.getX()<<","<<this->preprocessEndRight.getY()<<">" << endl;

    if(growObstacleLeft.size() != 0) 
    {
        if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
        cout << "Grow Obstacle Left From:\t\t" << "<"<<this->growObstacleLeft[0].getX()<<","<<this->growObstacleLeft[0].getY()<<">" << endl;

        if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
        cout << "Grow Obstacle Left To:\t\t" << "<"<<this->growObstacleLeft[1].getX()<<","<<this->growObstacleLeft[1].getY()<<">" << endl;
    }

    if(growObstacleRight.size() != 0) 
    {
        if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
        cout << "Grow Obstacle Right From:\t\t" << "<"<<this->growObstacleRight[0].getX()<<","<<this->growObstacleRight[0].getY()<<">" << endl;

        if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
        cout << "Grow Obstacle Right To:\t\t" << "<"<<this->growObstacleRight[1].getX()<<","<<this->growObstacleRight[1].getY()<<">" << endl;
    }
    iLineNumbersForEdit = iItemNumber-1;
}

int CReconBuildingElement::generateInternalizedPlanData( string mapFileName)
{
    CLoadMap clm;
    
    cout << "Calculating Internalized Plan: \tElement Number " << this->getElementNumber() << "\tType: "<< this->getElementType() <<endl; 
    cout << flush;

    clm.openOVL( mapFileName, leftFourRobotPoint, vBoundaryBox);

    CCalculateMapCosts calMapCostLeftFourRobotPoint( leftFourRobotPoint );
    leftFourRobotPoint->setMapID( "temp" );
    if( calMapCostLeftFourRobotPoint.performCalculations() != EXIT_SUCCESS )
        return EXIT_FAILURE;

    clm.openOVL( mapFileName, rightFourRobotPoint, vBoundaryBox);
    rightFourRobotPoint->setMapID( "temp" );
    CCalculateMapCosts calMapCostRightFourRobotPoint( rightFourRobotPoint );
    if( calMapCostRightFourRobotPoint.performCalculations() != EXIT_SUCCESS )
        return EXIT_FAILURE;

    clm.openOVL( mapFileName, CommunicationsMapLeft, vBoundaryBox);
    CommunicationsMapLeft->setMapID( "temp" );
    CommunicationsMapLeft->setGoal( (double)goal.getX(), (double)goal.getY());
    CCalculateMapCosts calMapCostCommunicationsLeft( CommunicationsMapLeft );      //Left Map takes right side preprocessed obstacle
    if( calMapCostCommunicationsLeft.performProcessedCalculations(this->getPreprocessStartRight(), this->getPreprocessEndRight()) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    clm.openOVL( mapFileName, CommunicationsMapRight, vBoundaryBox);
    CommunicationsMapRight->setMapID( "temp" );
    CommunicationsMapRight->setGoal( (double)goal.getX(), (double)goal.getY());
    CCalculateMapCosts calMapCostCommunicationsRight( CommunicationsMapRight );   //Right Map takes left side preprocessed obstacle
    if( calMapCostCommunicationsRight.performProcessedCalculations(this->getPreprocessStartLeft(), this->getPreprocessEndLeft()) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    clm.openOVL( mapFileName, CoverageMapLeft, vBoundaryBox);
    CoverageMapLeft->setMapID( "temp" );
    CoverageMapLeft->setGoal( (double)goal.getX(), (double)goal.getY());
    CCalculateMapCosts calMapCostCoverageLeft( CoverageMapLeft );
    if( this->growObstacleLeft.size() != 0 )            //Growth should be on the same side
         calMapCostCoverageLeft.growObstacle(this->growObstacleLeft[0], this->growObstacleLeft[1]);//Left Map takes right side preprocessed obstacle
    if( calMapCostCoverageLeft.performProcessedCalculations(this->getPreprocessStartRight(), this->getPreprocessEndRight()) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    clm.openOVL( mapFileName, CoverageMapRight, vBoundaryBox);
    CoverageMapRight->setMapID( "temp" );
    CoverageMapRight->setGoal( (double)goal.getX(), (double)goal.getY());
    CCalculateMapCosts calMapCostCoverageRight( CoverageMapRight );
    if( this->growObstacleRight.size() != 0 )          //Growth should be on the same side
        calMapCostCoverageRight.growObstacle(this->growObstacleRight[0], this->growObstacleRight[1] );//Right Map takes left side preprocessed obstacle
    if( calMapCostCoverageRight.performProcessedCalculations(this->getPreprocessStartLeft(), this->getPreprocessEndLeft()) != EXIT_SUCCESS )
        return EXIT_FAILURE;


    return EXIT_SUCCESS;
    
}

int CReconBuildingElement::writeMaps(ofstream& fStream, string writeType)
{
    vector< CCoordinate > vProgBox;
    vProgBox = this->getProgressBoxLeft();

    unsigned int i,iCount;
    for(iCount=0;iCount<2; iCount++)
    {
        for(i=0; i<vProgBox.size();i++) 
        {
            fStream << "<" << vProgBox[i].getX() << "," << vProgBox[i].getY() << ">" <<endl;
        }
        vProgBox = this->getProgressBoxRight();
    }
    
    leftFourRobotPoint->writeMapToFile( fStream, writeType );
    rightFourRobotPoint->writeMapToFile( fStream, writeType );
    CommunicationsMapLeft->writeMapToFile( fStream, writeType  );
    CommunicationsMapRight->writeMapToFile( fStream, writeType  );
    CoverageMapLeft->writeMapToFile( fStream, writeType  );
    CoverageMapRight->writeMapToFile( fStream, writeType );

    return EXIT_SUCCESS;
}

int CReconBuildingElement::readMaps(ifstream& fStream, string writeType)
{
    CLoadMap clm;

    if( this->leftFourRobotPoint == NULL )
        leftFourRobotPoint = new CMap();

    clm.LoadMap( leftFourRobotPoint, fStream, writeType );

    if( this->rightFourRobotPoint == NULL )
        rightFourRobotPoint = new CMap();

    clm.LoadMap( rightFourRobotPoint, fStream, writeType );

    if( this->CommunicationsMapLeft == NULL )
        CommunicationsMapLeft = new CMap();

    clm.LoadMap( CommunicationsMapLeft, fStream, writeType );

    if( this->CommunicationsMapRight == NULL )
        CommunicationsMapRight = new CMap();

    clm.LoadMap( CommunicationsMapRight, fStream, writeType );

    if( this->CoverageMapLeft == NULL )
        CoverageMapLeft = new CMap();

    clm.LoadMap( CoverageMapLeft, fStream, writeType );

    if( this->CoverageMapRight == NULL )
        CoverageMapRight = new CMap();

    clm.LoadMap( CoverageMapRight, fStream, writeType );
   
    return EXIT_SUCCESS;
}

/**********************************************************************
 * $Log: CReconBuildingElement.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:42  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
