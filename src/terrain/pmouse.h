
#define BUTTON_PRESS	true
#define BUTTON_RELEASE  false
#define RIGHT_BUTTON	0
#define LEFT_BUTTON	2
#define MIDDLE_BUTTON	1

struct Mouse_Pos{
	int X;
	int Y;
};

class PMouse {
public:
	PMouse(void);
	~PMouse(void);
	void Set_XY(int x, int y);
	void Set_Button(int button, int state);
	bool Get_Button(int button);
	Mouse_Pos GetSet_DiffXY(int x, int y);
	Mouse_Pos Get_XY(void);
private:
	int X;
	int Y;
	bool Button_R;
	bool Button_L;
	bool Button_M;
};

