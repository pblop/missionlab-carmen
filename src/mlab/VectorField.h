/**********************************************************************
 **                                                                  **
 **                            VectorField.h                         **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: VectorField.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $*/

#ifndef _VECTORFIELD_H
#define _VECTORFIELD_H

#include <list>

#include "gt_std.h"

using std::list;

struct SuMovement
{
    Vector suFrom, suDisplacement;
};

class ClVectorField
{
public:
    ClVectorField();
    ~ClVectorField();

    int GetRobotID();
    void SetRobotID( int iRobotID );

    double GetEdgeDistance();
    void SetEdgeDistance( double dEdgeDistance );

    double GetResolution();
    void SetResolution( double dResolution );

    void SetMissionArea( double dWidth, double dHeight );

    void SetStopDrawing( bool bStopDrawing );

    void DrawVectorField();
    void DrawVectorField( int iRobotID, double dEdgeDistance, double dResolution );

    void DrawVector( const Vector& suFrom, const Vector& suDisplacement );
    void EraseVectors();

protected:
    int m_iRobotID;
    double m_dEdgeDistance;
    double m_dResolution;
    list<SuMovement> m_objVectorList;

    double m_dMissionAreaWidth;
    double m_dMissionAreaHeight;

    bool m_bStopDrawing;
};

#endif // ndef _VECTORFIELD_H

/**********************************************************************
 * $Log: VectorField.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:12  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 * Revision 1.1  2002/07/02 20:47:19  blee
 * Initial revision
 *
 *
 **********************************************************************/
