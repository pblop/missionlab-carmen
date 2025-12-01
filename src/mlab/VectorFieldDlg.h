/**********************************************************************
 **                                                                  **
 **                          VectorFieldDlg.h                        **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: VectorFieldDlg.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $*/

#ifndef _VECTORFIELDDLG_H
#define _VECTORFIELDDLG_H

#include <Xm/Xm.h>

#include "VectorField.h"

class ClVectorFieldDlg
{
public:
    ClVectorFieldDlg( Widget widgetParent, ClVectorFieldDlg** ppobjThis, 
                      ClVectorField* pobjVectorField );
    ~ClVectorFieldDlg();

    bool Create();
    void Destroy();

    Widget GetParent();
    void Raise();

    void SetStopDrawing( bool bStopDrawing );

protected:
    int m_iRobotID;
    Widget m_widgetParent;
    Widget m_widgetDlgShell;
    Widget m_widgetDistanceSlider;
    Widget m_widgetResolutionSlider;
    Widget m_widgetRobotIdLabel;
    Widget m_widgetDrawFieldBtn;
    Widget m_widgetStopDrawingBtn;
    bool   m_bStopDrawing;
    ClVectorFieldDlg** m_ppobjThis;

    ClVectorField* m_pobjVectorField;

    void DrawVectorField();
    void GetNextRobotID();
    void UpdateControlsForDrawing( bool bDrawing );

    static void NextRobotCallback( Widget widgetNextBtn, XtPointer client_data, XtPointer call_data );
    static void DrawFieldCallback( Widget widgetDrawBtn, XtPointer client_data, XtPointer call_data );
    static void StopDrawingCallback( Widget widgetDrawBtn, XtPointer client_data, XtPointer call_data );
    static void CloseCallback( Widget widgetCloseBtn, XtPointer client_data, XtPointer call_data );
};

#endif // ndef _VECTORFIELDDLG_H

/**********************************************************************
 * $Log: VectorFieldDlg.h,v $
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
 * Revision 1.1  2002/07/02 20:47:36  blee
 * Initial revision
 *
 *
 **********************************************************************/
