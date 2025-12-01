#include "pmouse.h"

PMouse::PMouse(void) {
	X = 0;
	Y = 0;
	Button_R = false;
	Button_L = false;
	Button_M = false;
}

PMouse::~PMouse(void) {
}

void PMouse::Set_XY(int x, int y) {
	X = x;
	Y = y;
}

void PMouse::Set_Button(int button, int state) {
	switch (button) {
		case RIGHT_BUTTON:
			Button_R = state;
			break;
		case LEFT_BUTTON:
			Button_L = state;
			break;
		case MIDDLE_BUTTON:
			Button_M = state;
			break;
	}
}

bool PMouse::Get_Button(int button) {
	switch (button) {
                case RIGHT_BUTTON:
			return Button_R;
                case LEFT_BUTTON:
                        return Button_L;
                case MIDDLE_BUTTON:
                        return Button_M;
                        break;
        }
	return false;
}

Mouse_Pos PMouse::GetSet_DiffXY(int x, int y) {
	Mouse_Pos rmp;

	rmp.X = X - x;
	rmp.Y = Y - y;
	
	X = x;
	Y = y;

	return rmp;
}

Mouse_Pos PMouse::Get_XY(void) {
	Mouse_Pos rmp;

	rmp.X = X;
	rmp.Y = Y;
	return rmp;
}


