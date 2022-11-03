//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#define STRICT

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#ifndef WIN32
#include <shell.h>
#endif

#include "afxres.h"
#include "resource.h"

extern HINSTANCE g_hinst;

union POS_FRACT {
	struct {
		unsigned int fract:2;
		unsigned int whole:14;
		};
	WORD pos;
	};
struct MOB {
	HANDLE hImg,hImgAlt;
	union POS_FRACT x,y;
	SIZE s;
	unsigned char bVis;
	BYTE mirrorX,mirrorY;
	POINT speed;
	unsigned int punti;
	int res1;          // usato per computare speed
	HANDLE hImgOld;
	BYTE bUseTransparency;		// evita flicker se non serve...
  };


#define ALIEN_X_SIZE 14		// circa 14(11+3spazi) x riga @240 pixel schermo, inter-spazio=4 ?
#define ALIEN_X_SPACING 3
#define ALIEN_Y_SIZE 14
#define ALIEN_Y_POS 72
#define SCORE_AREA 40
#define LOWER_AREA 14
#define MAX_SHIPS 5
#define MAX_BOMBE 5			// diciamo max 5

enum PLAY_STATE {
	PLAY_IDLE=0,
	PLAY_PAUSED=-1,
	PLAY_STARTING=1,
	PLAY_PLAY=2,
	PLAY_ENDING=3,
	PLAY_DEMO=99
	};
enum SUB_PLAY_STATE {
	SUBPLAY_NONE=0,
	SUBPLAY_ALIENBOOM,
	SUBPLAY_UFOBOOM,
	SUBPLAY_SPACESHIPBOOM
	};
extern enum PLAY_STATE bPlayMode;
extern enum SUB_PLAY_STATE subPlayMode;

ATOM MyRegisterClass(CONST WNDCLASS*);
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
BOOL CenterWindow (HWND, HWND);
int WritePrivateProfileInt(char *,char *,int,char *);
BOOL PlayResource(LPSTR lpName);

int animateMobs(HWND);
int animateMostri(HWND);
int loadMobs();
int MobErase(HDC,struct MOB *);
int MobDraw(HDC,struct MOB *);
int MobMove(HDC,struct MOB *,SIZE) ;
int MobDrawXY(HDC,struct MOB *,WORD,WORD);
int MobCollisionRect(struct MOB *,RECT *);
int MobCollisionPoint(struct MOB *,POINT);
BYTE isMissileInArea(struct MOB *);
BYTE isBombaInArea(struct MOB *,struct MOB *);
int getNumBombe(void);
int getSuitableBomb(int, POINT *);
BYTE isMissileInCasa(struct MOB *);
BYTE isBombaInCasa(struct MOB *,struct MOB *);
BYTE hitCasa(struct MOB *,struct MOB *,int);
