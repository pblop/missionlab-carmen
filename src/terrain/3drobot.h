#ifndef THREED_ROBOT_H
#define THREED_ROBOT_H

#include <string>

#include "model_3ds.h"

using std::string;

class Robot {

private:
    void Make_Verticies(void);

    string color_;
    float Robot_Vert[24][3];
    int Robot_Num;
    double X;
    double Y;
    double Z;
    double Theta;
    double Robot_Scale;
    int Robot_Type;
    float red, green, blue;

    int UncertaintyX;
    int UncertaintyY;
    int UncertaintyDx;
    int UncertaintyDy;
    bool Uncertainty;


    static const string STRING_RED_;
    static const string STRING_GREEN_;
    static const string STRING_PURPLE_;
    static const string STRING_BLACK_;
    static const string STRING_GRAY_;
    static const string STRING_ORANGE_;
    static const string STRING_YELLOW_;

public:
    Robot(
        int robotnum,
        double x,
        double y,
        double z,
        double heading,
        string color,
        int rtype);

    ~Robot(void);
    string Get_Color(void);
    void Update_Position(double x, double y, double z, double heading);
    void Get_Position(double &x_, double&y_, double &z_)
        {
            x_ = X; y_ = Y; z_ = Z;
        }

    void Update_Uncertainty(bool tog);
    void Update_Uncertainty(int x, int y, int dx, int dy);

    void Draw_Robot(float yoffsett, Model_3ds *model);
    void Set_RobotScale(double scale);
    void Set_Color(string color);
    void Toggle_Colors(string color1, string color2);
};

inline string Robot::Get_Color(void)
{
    return color_;
}

#endif
