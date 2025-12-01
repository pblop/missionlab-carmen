/**********************************************************************
 **                                                                  **
 **                          mission_design.h                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2002 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mission_design.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef MISSION_DESIGN_H
#define MISSION_DESIGN_H

#include <vector>
#include <string>

#include "gt_simulation.h"
#include "mission_expert.h"
#include "mlab_cbrclient.h"
#include "mission_design_types.h"
#include "CNPConstraint.h"
#include "symbol.hpp"
#include "cbrwizard.h"

using std::vector;
using std::string;

class MlabMissionDesign : public CBRWizard {

    typedef enum MMDMode_t {
        RUN_MODE,
        DESIGN_MODE,
        REPAIR_MODE
    };

    typedef enum MMDMouseButtonNum_t {
        NO_BTN,
        LEFT_BTN,
        MIDDLE_BTN,
        RIGHT_BTN
    };

    typedef enum ModViaPointPair_t {
        OLD_VIA_POINT,
        NEW_VIA_POINT,
        NUM_MOD_VIA_POINTS
    };

    typedef enum BackupStatus_t {
        BACKUP_FAILURE,
        BACKUP_SUCCESS,
        BACKUP_ORIGINAL_FILE
    };

    typedef enum PlaceTaskStatus_t {
        PLACETASK_FAILURE,
        PLACETASK_SUCCESS
    };

    typedef enum CursorIDs_t {
        CURSOR_LEFT_PTR,
        CURSOR_WATCH,
        CURSOR_PIRATE,
        CURSOR_HAND1,
        CURSOR_HAND2,
        CURSOR_POLYGON
    };

    typedef struct MMDPixmap_t {
        string name;
        Pixmap pixmap;
        Pixmap insensitivePixmap;
    };

    typedef struct MMDCursor_t {
        int id;
        Cursor cursor;
    };

    typedef struct MMDPolygon_t
    {
        Widget callerWidget;
        Widget editerWidget;
        vector<MExpPoint2D_t> posList;
        vector<MExpLatLon_t> geoList;
        MExpPoint2D_t lastMousePos;
        int featureType;
        int featureId;
        int number;
        int selectedPosIndex;
        bool dragging;
        bool drawn;
    };

    typedef struct DraggingCircle_t {
        MExpPoint2D_t lastPos;
        double radius;
        bool dragging;
        bool drawn;
    };

    typedef struct DeployedRuntimeCNPRobot_t {
        MExpMissionTask_t task;
        int robotID;
    };

    typedef struct MMDGlobalFeatureMenuCallbackData_t {
        int globalFeatureId;
        int optionType;
        float optionValue;
        MlabMissionDesign *mlabMissionDesignInstance;
        vector<void *> extraDataList;
    };

    typedef struct MMDRightBtnMenuCallbackData_t {
        int featureId;
        int optionType;
        float optionValue;
        MlabMissionDesign *mlabMissionDesignInstance;
    };

    typedef struct MMDNavRightBtnMenuCallbackData_t {
        int startFeatureId;
        int page;
        MlabMissionDesign *mlabMissionDesignInstance;
    };

    typedef struct MMDRobotConstraintMenuCallbackData_t {
        int robotId;
        int constraintId;
        int optionType;
        float optionValue;
        MlabMissionDesign *mlabMissionDesignInstance;
    };

    typedef struct MMDNavRobotConstraintMenuCallbackData_t {
        int targetRobotId;
        int startConstraindId;
        int page;
        MlabMissionDesign *mlabMissionDesignInstance;
    };

    typedef struct MMDToolBoxSetCurrentTaskCallbackData_t {
        int task;
        MlabMissionDesign *mlabMissionDesignInstance;
    };

    typedef struct MMDRepairWindowCallbackData_t {
        int mode;
        MlabMissionDesign *mlabMissionDesignInstance;
    };

protected:
    FILE *featureStatusFile_;
    Pixel MMDToolboxBtnBgColorSet_;
    Pixel MMDToolboxBtnBgColorUnset_;
    Widget MMDDrawingArea_;
    Widget Biohazard_btn_;
    Widget Waypoints_btn_;
    Widget Mine_btn_;
    Widget Other_btn_;
    Widget *task_btn_;
    Widget toolbox_w_;
    Widget repairwindow_w_;
    Widget globalFeatureMenu_w_;
    Widget **globalFeatureMenuOption_w_;
    Widget rightBtnMenu_w_;
    Widget robotConstraintMenu_w_;
    Widget **rightBtnMenuOption_w_;
    Widget **robotConstraintMenuOption_w_;
    Widget currentEditPolygonButton_;
    Cursor dragPolygonCursor_;
    MExpMissionTask_t rightBtnMenuUpTask_;
    MExpMissionTask_t liftedTask_;
    MlabCBRClient *mlabCBRClient_;
    DraggingCircle_t draggingCircle_;
    MMDPolygon_t specifyingPolygon_;
    vector<MExpRobotConstraints_t> upRobotConstraintsList_;
    vector<MExpMissionTask_t> specifiedTaskList_;
    vector<MMDPolygon_t> specifiedPolygonList_;
    vector<MMDPolygon_t> editingPolygonList_;
    vector<MMDPixmap_t> pixmapList_;
    vector<MMDCursor_t> cursorList_;
    vector<MExpPoint2D_t> viaPointList_;
    vector<DeployedRuntimeCNPRobot_t> deployedRuntimeCNPRobotList_;
    double drawingUnitLength_;
    double placeCircleDiameter_;
    double placePolygonPointDiameter_;
    double lastMouseClickedPointX_;
    double lastMouseClickedPointY_;
    int MMDMode_;
    int MMDCurrentTask_;
    int runtimeCNPMissionManagerTask_;
    int icarusTask_;
    int numVisibleTaskTypes_;
    int numFeatureTypes_;
    int lastMouseXRoot_;
    int lastMouseYRoot_;
    int numTasks_;
    int rightBtnMenuLineHeight_;
    int returnConfigureMission_;
    int placeCircleIndex_;
    int liftedViaPointIndex_;
    int upRuntimeCNPRobotID_;
    int selectedDeployedRuntimeCNPRobotID_;
    unsigned int MMDMissionAreaHeightPixels_;
    bool windowsCreated_;
    bool rightBtnMenuIsUp_;
    bool robotConstraintMenuIsUp_;
    bool taskLifted_;
    bool viaPointLifted_;
    bool allowModCNPRobots_;
    bool savedTasksLoaded_;
    bool disableColorButtons_;
    bool isSpecifyingPolygon_;
    bool isEditingPolygon_;

    static const int DEFAULT_PLACE_CIRCLE_DIAMETER_;
    static const int DEFAULT_PLACE_POLYGON_POINT_DIAMETER_;
    static const int DEFAULT_TOOLBOX_WIDTH_;
    static const int DEFAULT_TOOLBOX_XPOS_;
    static const int DEFAULT_TOOLBOX_YPOS_;
    static const int DEFAULT_TOOLBOX_ACTIONBUTTON_HEIGHT_;
    static const int DEFAULT_TOOLBOX_EXTRASPACE_HEIGHT_;
    static const int DEFAULT_TOOLBOX_BUTTON_HEIGHT_;
    static const int DEFAULT_TOOLBOX_BUTTON_WIDTH_;
    static const int DEFAULT_TOOLBOX_BUTTON_EXTRASPACE_HEIGHT_;
    static const int DEFAULT_TOOLBOX_NUM_COLUMNS_;
    static const int DEFAULT_REPAIRWINDOW_ACTIONAREA_HEIGHT_;
    static const int DEFAULT_REPAIRWINDOW_EXTRASPACE_HEIGHT_;
    static const int DEFAULT_REPAIRWINDOW_WIDTH_;
    static const int DEFAULT_REPAIRWINDOW_XPOS_;
    static const int DEFAULT_REPAIRWINDOW_YPOS_;
    static const int DEFAULT_RIGHTBTN_WIDTH_;
    static const int DEFAULT_RIGHTBTN_LINEHEIGHT_;
    static const int DEFAULT_RIGHTBTN_EDGEOFFSET_;
    static const int DEFAULT_RIGHTBTN_EXTRASPACE_HEIGHT_;
    static const int DEFAULT_ROBOTCONSTRAINT_WIDTH_;
    static const int DEFAULT_ROBOTCONSTRAINT_LINEHEIGHT_;
    static const int DEFAULT_ROBOTCONSTRAINT_EDGEOFFSET_;
    static const int DEFAULT_GLOBALFEATURE_WIDTH_;
    static const int DEFAULT_GLOBALFEATURE_LINEHEIGHT_;
    static const int DEFAULT_GLOBALFEATURE_XEDGEOFFSET_;
    static const int DEFAULT_GLOBALFEATURE_YPOS_;
    static const int DRAG_UPDATE_TIME_MSEC_;
    static const int MAX_CONSTRAINTS_DISPLAY_;
    static const int MAX_LOCAL_FEATURES_DISPLAY_;
    static const int TOOLBOX_EXTRASPACE_HEIGHT_RUNTIME_CNP_;
    static const int TOOLBOX_EXTRASPACE_HEIGHT_ICARUS_WIZARD_;
    static const int INVALID_TASK_ID_;
    static const int INVALID_TASK_NUMBER_;
    static const int INVALID_ROBOT_ID_;
    static const int INVALID_MOBILITY_TYPE_;
    static const string STRING_WAYPOINTS_TASK_;
    static const string STRING_RUNTIME_CNP_MISSION_MANAGER_TASK_;
    static const string STRING_RUNTIME_CNP_BIDDER_TASK_;
    static const string STRING_ICARUS_TASK_;
    static const string STRING_VEHICLE_TYPES_;
    static const string STRING_ROBOT_ID_;
    static const string STRING_ADD_;
    static const string STRING_ANY_;
    static const string STRING_DISABLED_;
    static const string STRING_DONE_;
    static const string STRING_EDIT_;
    static const string STRING_NA_;
    static const string TOOLBOX_OK_BUTTON_LABEL_;
    static const string TOOLBOX_CANCEL_BUTTON_LABEL_;
    static const string TOOLBOX_GLOBALSETTING_BUTTON_LABEL_;
    static const string TOOLBOX_ROBOTSETTING_BUTTON_LABEL_;
    static const string TOOLBOX_MISSIONSETTING_BUTTON_LABEL_;
    static const string REPAIRWINDOW_OK_BUTTON_LABEL_;
    static const string REPAIRWINDOW_CANCEL_BUTTON_LABEL_;
    static const string GLOBALFEATURE_TITLE_;
    static const string CONFIRM_EMPTY_MISSION_PHRASE_;
    static const string CONFIRM_NO_ROBOT_CONSTRAINT_PERMISSION_PHRASE_;
    static const string CONFIRM_NO_RC_PHRASE_;
    static const string CONFIRM_OFFENDING_PART_PHRASE_;
    static const string CONFIRM_PROCEED_PHRASE_;
    static const string CONFIRM_DELETE_PHRASE_;
    static const string CONFIRM_DEFAULT_INIT_INSTRUCTION_PHRASE_;
    static const string CONFIRM_RUNTIME_CNP_INIT_INSTRUCTION_PHRASE_;
    static const string CONFIRM_RUNTIME_CNP_NO_DEPLOYABLE_ROBOT_PHRASE_;
    static const string CONFIRM_RUNTIME_CNP_NO_REMOVE_MISSION_MANAGER_;
    static const string CONFIRM_ENABLE_ICARUS_WIZARD_INIT_INSTRUCTION_PHRASE_;
    static const string CONFIRM_ENABLE_ICARUS_WIZARD_NO_REMOVE_TASK_;
    static const string EXCEEDED_TASK_USAGE_PHRASE_;
    static const string ROBOTCONSTRAINT_TITLE_LABEL_;
    static const string ROBOTCONSTRAINT_TITLE_LABEL_RUNTIME_CNP_;
    static const string ROBOTCONSTRAINT_ROBOT_NAVIGATION_LABEL_;
    static const string ROBOTCONSTRAINT_CONSTRAINT_NAVIGATION_LABEL_;
    static const string ROBOTCONSTRAINT_PREV_BUTTON_LABEL_;
    static const string ROBOTCONSTRAINT_NEXT_BUTTON_LABEL_;
    static const string RIGHTBTN_TITLE_LABEL_;
    static const string RIGHTBTN_TITLE_LABEL_RUNTIME_CNP_;
    static const string RIGHTBTN_TITLE_LABEL_ICARUS_TASK_;
    static const string RIGHTBTN_PREFERNCE_NAVIGATION_LABEL_;
    static const string RIGHTBTN_PREV_BUTTON_LABEL_;
    static const string RIGHTBTN_NEXT_BUTTON_LABEL_;
    static const string RCTABLE_DISALLOW_MOD_CNP_ROBOTS_;
    static const string RCTABLE_DISABLE_COLOR_BUTTONS_;
    static const char POLYGON_ENCODER_KEY_XY_START_;
    static const char POLYGON_ENCODER_KEY_XY_END_;
    static const char POLYGON_ENCODER_KEY_LATLON_START_;
    static const char POLYGON_ENCODER_KEY_LATLON_END_;
    static const char POLYGON_ENCODER_KEY_POINT_SEPARATOR_;
    static const char POLYGON_ENCODER_KEY_POINTS_SEPARATOR_;

    MExpPoint2D_t askAndAddNewViaPoint_(string question);
    MExpPoint2D_t *askAndMoveViaPoint_(string question, double *closeRange);
    MExpPoint2D_t askAndDeleteViaPoint_(string question, double *closeRange);
    MExpRobotConstraints_t getRobotConstraints_(int robotId);
    MExpMissionTask_t createEmptyTask_(void);
    DeployedRuntimeCNPRobot_t getDeployedRuntimeCNPRobotFromRobotID_(int robotID);
    Pixmap getPixmapFromList_(string name, bool insensitive);
    Cursor getCursorFromList_(int cursorID);
    Pixel toolboxBtnBgColor_(int button);
    string generateTaskEntryKey_(MExpMissionTask_t task);
    string generateViaPointEntryKey_(int index);
    string generatePolygonEntryKey_(MMDPolygon_t poly);
    string encodePolygon_(MMDPolygon_t poly);
    float getTaskFeatureSelectedOption_(
        string featureName,
        MExpMissionTask_t task);
    int askRepairQuestions_(void);
    int getDefaultCurrentTask_(void);
    int getTaskID_(string taskName);
    int getSmallestDeployableRuntimeCNPRobotID_(void);
    int getDeployedRuntimeCNPRobotIDFromTaskNumber_(int taskNumber);
    int backupFile_(string filename);
    int addNewTask_(double taskPosX, double taskPosY, int taskType, MExpMissionTask_t *addedTask);
    bool loadData_(void);
    bool initCBRClient_(void);
    bool isInsideTask_(double x, double y, double unitLength, MExpMissionTask_t *task);
    bool isInsideViaPoint_(
        double x,
        double y,
        double unitLength,
        MExpPoint2D_t *viaPoint,
        int *index);
    bool isAtSpecifyingPolygonStartPoint_(
        double x,
        double y,
        double unitLength);
    bool isAtEditingPolygonPoint_(
        double x,
        double y,
        double unitLength,
        MMDPolygon_t *selectedPoly);
    bool isDeployedRuntimeCNPRobot_(int robotID);
    bool replaceTaskFeatureSelectedOption_(
        string featureName,
        float selectedOption,
        MExpMissionTask_t *task);
    bool replaceTaskFeatureSelectedOption_(
        int taskNumber,
        string featureName,
        float selectedOption,
        vector<MExpMissionTask_t> *taskList);
    bool decodePolygon_(string polygonString, MMDPolygon_t *poly);
    void saveData_(void);
    void createToolbox_(void);
    void updateToolboxBtn_(void);
    void placeTask_(double taskPosX, double taskPoxY, int taskType);
    void placePolygonPoint_(double x, double y);
    void removeTask_(MExpMissionTask_t task);
    void removeViaPoint_(int index);
    void removePolygon_(MMDPolygon_t poly);
    void liftTask_(MExpMissionTask_t task);
    void liftViaPoint_(int index);
    void liftPolygonVertex_(MMDPolygon_t poly);
    void unliftPolygonVertex_(double x, double y);
    void putdownLiftedTask_(double taskPosX, double taskPosY);
    void putdownLiftedViaPoint_(double x, double y);
    void placeCircle_(double x, double y);
    void createGlobalFeatureMenu_(void);
    void popUpRightBtnMenu_(MExpMissionTask_t task);
    void popUpRobotConstraintMenu_(void);
    void popUpRuntimeCNPMissionManagerMenu_(void);
    void popUpIcarusTaskMenu_(void);
    void createRightBtnMenu_(MExpMissionTask_t task, int startFeatureId, int page);
    void createRobotConstraintMenu_(int robotId, int startConstraintId, int page);
    void changeGlobalFeatureMenuValue_(Widget w, XtPointer client_data);
    void applyRightBtnMenu_(void);
    void applyRobotConstraintMenu_(void);
    void cancelRightBtnMenu_(void);
    void cancelRobotConstraintMenu_(void);
    void closeRightBtnMenu_(void);
    void closeRobotConstraintMenu_(void);
    void changeRightBtnMenuValue_(XtPointer client_data);
    void changeRobotConstraintMenuValue_(XtPointer client_data);
    void navigateRobotConstraintMenu_(XtPointer client_data);
    void navigateRightBtnMenu_(XtPointer client_data);
    void setCurrentTask_(int task);
    void setFeatureFileStatus_(void);
    void createPixmapList_(Pixel fg, Pixel bg, Widget parent);
    void createCursorList_(Pixel fg, Pixel bg, Widget parent);
    void askDiagnosis_(void);
    void repairMission_(void);
    void createRepairQuestionWindow_(void);
    void askRepairFollowUpQuestion_(void);
    void exit_(int exitMode);
    void placeViaPoint_(double x, double y);
    void dragCircle_(void);
    void dragPolygonPoint_(void);
    void dragPolygonVertex_(void);
    void clearViaPoints_(void);
    void clearLastViaPoint_(void);
    void placeRuntimeCNPMissionManager_(void);
    void placeIcarusTask_(void);
    void deployRuntimeCNPRobot_(int robotID, MExpMissionTask_t task);
    void undeployRuntimeCNPRobot_(int robotID);
    void specifyPolygon_(
        Widget callerWidget,
        Widget editerWidget,
        int featureType,
        int featureId,
        int number);
    void editPolygon_(
        Widget callerWidget,
        int featureType,
        int featureId,
        int number);
    void clearPolygon_(MMDPolygon_t *poly);
    void completeSpecifyingPolygon_(void);
    void cancelSpecifyingPolygon_(void);
    void finishEditingPolygon_(void);
    void drawSpecifiedTaskList_(double unitLength);
    void drawSpecifiedPolygons_(void);
    void eraseSpecifiedPolygons_(void);
    void resetEditingPolygons_(int featureType, int featureId);
    void setGlobalFeatureMissionTime_(string missionTimeString);

    // Callbacks
    static XtTimerCallbackProc cbDragCircle_(XtPointer clientData);
    static XtTimerCallbackProc cbDragPolygonPoint_(XtPointer clientData);
    static XtTimerCallbackProc cbDragPolygonVertex_(XtPointer clientData);
    static void cbSetCurrentTask_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbChangeGlobalFeatureMenuValue_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbApplyRightBtnMenu_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbApplyRobotConstraintMenu_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbCancelRightBtnMenu_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbCancelRobotConstraintMenu_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbChangeRightBtnMenuValue_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbChangeRobotConstraintMenuValue_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbNavigateConstraintMenu_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbNavigateRightBtnMenu_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbExit_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbToolboxGlobalSettingBtn_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbToolboxRobotSettingBtn_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbToolboxMissionSettingBtn_(Widget w, XtPointer client_data, XtPointer callback_data);
    static void cbRepair_(Widget w, XtPointer client_data, XtPointer callback_data);

public:
    MlabMissionDesign(
        Display *display,
        Widget parent,
        XtAppContext app,
        Widget drawing_area,
        const symbol_table<rc_chain> &rcTable,
        string mexpRCFileName,
        char *featureFileString,
        char *missionTimeString,
        bool enableCBRClient,
        char **defaultOverlayName,
        int cnpMode);
    
    ~MlabMissionDesign(void);
    vector<CNP_RobotConstraint> getCNPRobotConstraints(int robotId);
    int getMobilityTypeFromRobotConstraints(int robotId);
    void createWindows(void);
    void createRepairWindow(void);
    void mouseClick(double x, double y, XEvent *event, double unitLength);
    void mouseRelease(double x, double y, XEvent *event, double unitLength);
    void drawOverlayMeasures(double unitLength);
    void saveOverlayName(char *ovlFilename);
    void saveLogfile(string logfileName);
    bool windowsAlreadyCreated(void);
};

inline bool MlabMissionDesign::windowsAlreadyCreated(void){return windowsCreated_;}

extern MlabMissionDesign *gMMD;
extern bool gUseMlabMissionDesign;

#endif

/**********************************************************************
 * $Log: mission_design.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.8  2007/09/29 23:44:44  endo
 * Global feature can be now disabled.
 *
 * Revision 1.7  2007/09/18 22:37:00  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.6  2007/05/15 18:45:03  endo
 * BAMS Wizard implemented.
 *
 * Revision 1.5  2007/02/08 20:22:25  endo
 * Polygon's Edit option added.
 *
 * Revision 1.4  2007/01/28 21:00:54  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.3  2006/10/23 22:19:10  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.2  2006/09/13 19:06:53  endo
 * ICARUS Wizard implemented.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.9  2006/05/15 02:07:35  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.8  2006/05/02 04:19:59  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.7  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.6  2006/01/10 06:10:31  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.5  2005/08/09 19:14:00  endo
 * Things improved for the August demo.
 *
 * Revision 1.4  2005/07/27 20:38:10  endo
 * 3D visualization improved.
 *
 * Revision 1.3  2005/07/16 08:44:07  endo
 * CBR-CNP integration
 *
 * Revision 1.2  2005/02/07 23:12:36  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:11  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 * Revision 1.2  2003/04/06 09:03:48  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.1  2002/01/12 22:54:39  endo
 * Initial revision
 *
 **********************************************************************/
