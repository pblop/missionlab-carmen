/**********************************************************************
 **                                                                  **
 **                     GeneratePlan.cpp                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006  Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This class represents an internal map. The array        **
 ** "mapCost" is of primal importance. This array stores the cost    **
 ** associated with travel to a coordinate.                          **
 ** constructors.                                                    **
 **                                                                  **
 **********************************************************************/

/* $Id: GeneratePlan.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "ProjectDefinitions.h"
#include "CLoadMap.h"
#include "CPlan.h" 

const string FortBenningConfigurationName = "FortBenningConfig.pcf";
const string strVersionNumber = "1.0";
CPlan* aPlan = NULL;


void UsageInformation()
{
    cout << endl;
    cout << endl;
    cout << "<----------------------------------------------->" <<endl;
    cout << "< \t Internalized Planning Software \t>" <<endl;
    cout << "< \t\t Version "<<strVersionNumber<<" \t\t\t>" <<endl;
    cout << "< \t \t \t\t\t\t>"<<endl;
    cout << "<----------------------------------------------->" <<endl;
    cout << endl;
	cout << "This program edits and generates the files necessary" <<endl;
	cout << "to use the internalized planning software. The " <<endl;
    cout << "plan configuration file (.pcf) maintains information" <<endl;
    cout << "specific to the structure and usage of plan elements" <<endl;
    cout << "within a plan. Currently only two plan elements exist:" << endl;
    cout << "MOVETO and RECONBUILDING. The moveto element moves robots" << endl;
    cout << "into position prior to the executation. The reconbuilding" <<endl;
    cout << "element performs reconassaince of a building." <<endl;
    cout << endl;
    cout << "Given a plan configuration, the internalized plan generator" << endl;
    cout << "creates a the necessary data file for transfer to the robots" <<endl;
    cout << endl;
    cout << endl;
}

int CreateNewConfiguration()
{   
    cout << "Not implemented in this version" <<endl;
    return EXIT_SUCCESS;
}

int GenerateInternalizedPlan()
{
    if( aPlan == NULL )
    {
        CLoadMap clm;
        aPlan = new CPlan();

        if( clm.openPCF( FortBenningConfigurationName, aPlan ) != EXIT_SUCCESS )
        {
            cout << " An Error occurred when opening the file: "<< FortBenningConfigurationName << endl;
            return EXIT_FAILURE;
        }
    }

    if( aPlan->generateInternalizedPlanData() != EXIT_SUCCESS )
    {
        cout << "An Error occurred when generating the internalized plan data" <<endl;
        return EXIT_FAILURE;
    }

    string sFile;
    string strWriteType;
    long int size = (aPlan->GetContingencyMap()->getSizeX() * aPlan->GetContingencyMap()->getSizeY() * sizeof(float))/1000;
    size = size * (aPlan->GetNumberOfPlanSteps()/2)*7;  //approximate size
    cout << endl;
    cout << "Would you like to write the Iplan.dat file as text or float bytes?" <<endl;
    cout << "Float bytes will generate a dat file of approximate size: "<< size << " KB" <<endl;
    cout << "in seconds. Text will take longer a generate a smaller file."<<endl;
    cout << "Enter 1 for FLOAT bytes or 2 for TEXT." <<endl;
    cin >> sFile;

    if( sFile == "1" )
        strWriteType = "FLOAT";
    else
        strWriteType = "TEXT";
    if( aPlan->writeInternalizedPlanData( strWriteType ) != EXIT_SUCCESS )
    {
        cout << "An Error occurred when writing the internalized plan data file" <<endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int TestLoadIPlan()
{
    int rValue;
    
    cout <<endl;
    cout << "Testing loading Iplan.dat." << endl;
    if( aPlan == NULL )
        aPlan = new CPlan();

    rValue = aPlan->LoadPlanFromDataFile();
    if( rValue == EXIT_SUCCESS )
    {
        cout<< endl;
        cout << "The plan from the Iplan.dat loaded correctly" << endl;

        return EXIT_SUCCESS;
    }
    else 
    {   
        cout << "ERROR: "<<rValue << " Error loading plans" <<endl; 
        return EXIT_FAILURE;
    }
}

int PrintAStep()
{
    int rValue;
    
    cout <<endl;
    cout << "Testing loading Iplan.dat." << endl;
    if( aPlan == NULL )
        aPlan = new CPlan();

    rValue = aPlan->LoadPlanFromDataFile();
    if( rValue == EXIT_SUCCESS )
    {
        cout<< endl;
        cout << "The plan from the Iplan.dat loaded correctly" << endl;

        string sStepNumber;
        cout << endl;
        cout << "Which step would you like to examine?" <<endl;
        cin >> sStepNumber;

        int i;
        bool bNotFound=true;
        for(i=0;i<aPlan->GetNumberOfPlanSteps(); i++)
        {
            char numVal[10];
            sprintf(numVal,"%i",i+1);
            if( numVal == sStepNumber)
            {
                vector< CMap* > stepMaps = (aPlan->GetPlanStep(i))->GetMaps();
                for(int j=0; j<(signed)stepMaps.size(); j++)
                {
				  /*
				    if( j == 0 )
					  cout<< "Comms Map Left" <<endl;
					else if(j==1)
					  cout<< "Coverage Map Left" <<endl;
					else if(j==2)
					  cout<< "Comms Map Right" <<endl;
					else if(j==3)
					  cout<< "Coverage Map Right" <<endl;
					else if(j==4)
					  cout<< "leftFourPoint Map Left" <<endl;
					else if( j==5)
					  cout<< "rightFourPoint Map Left" <<endl;
				  */
					  
					stepMaps[j]->convertToVectorMap();
					stepMaps[j]->printMap();

                    cout <<endl;
                    cout <<endl;
                }
                bNotFound=false;
            }
        }

        if( bNotFound == true )
            cout << "ERROR: Plan "<<sStepNumber << " not found" <<endl; 

        return EXIT_SUCCESS;
    }
    else 
    {   
        cout << "ERROR: "<<rValue << " Error loading plans" <<endl; 
        return EXIT_FAILURE;
    }

}

int EditCurrentConfiguration(int iLineNumber)
{
    aPlan->editLineNumber( iLineNumber );
    aPlan->writePlanToConfigFile( FortBenningConfigurationName );
    return EXIT_SUCCESS;
}

int DisplayCurrentConfigurationFile()
{
    CLoadMap clm;
    aPlan = new CPlan();

    if( clm.openPCF( FortBenningConfigurationName, aPlan ) == EXIT_FAILURE )
        return EXIT_FAILURE;

    aPlan->print(true, 1);

	int iLineNumber = -1;
    string yesOrNo;
                       
    cout << endl;
    cout << "Enter the number of the line that you would like to change or ZERO for none: "<<endl;
    cin >> iLineNumber;
    
	if( iLineNumber != 0 )
	{
	    EditCurrentConfiguration( iLineNumber );
        cout << "Would You like to view the altered configuration file? (Type y or n)" <<endl;
        cin >> yesOrNo;
        if( yesOrNo == "y" )
	        DisplayCurrentConfigurationFile();
	}

    return EXIT_SUCCESS;
}

int GoWithFortBenning()
{
    const int SIZE =80;
	char buffer[SIZE];
    char* pEnd;

	cout << endl;
	cout << "Would you like to review and make changes to the Fort" <<endl;
	cout << "Benning configuration or generate a new configuration?" <<endl;
	cout << "Type 1 to edit or review the Fort Benning configuration" <<endl;
	cout << "Type 2 to create a new configuration" << endl;
    cout << "Type 3 to generate internalized plan data file" << endl; 
    cout << "Type 4 for usage information" << endl;
    cout << "Type 5 to test load a plan file" << endl;
    cout << "Type 6 to print the maps from a step" << endl;
    cout << "Type 7 to exit program"<<endl;
		
	cin.getline(buffer,SIZE);
	string stGoWithFortBenning = buffer;
    int iResult = strtol(stGoWithFortBenning.c_str(), &pEnd, 10);

    if( iResult == 1 )
        return DisplayCurrentConfigurationFile();
    else if( iResult == 2)
        return CreateNewConfiguration();
    else if( iResult == 3)
        return GenerateInternalizedPlan();
    else if( iResult == 4)
        UsageInformation();
    else if( iResult == 5 )
        return TestLoadIPlan();
    else if( iResult == 6 )
        return PrintAStep();
    else if( iResult == 7 )
        exit(1);
	else
	    return EXIT_SUCCESS;

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
	cout << "This application creates a Plan Configuration file"<< endl; 
    cout << "(.pcf) for the internal plan generator." << endl;
	cout << "To use this program enter the information queried.";
	cout << endl;
    
    while( 1 )
    {
	    GoWithFortBenning();
    }
    delete aPlan;
	return EXIT_SUCCESS;
}

/**********************************************************************
 * $Log: GeneratePlan.cpp,v $
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
