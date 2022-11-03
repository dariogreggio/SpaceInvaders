// http://banzies.blogspot.com/2007/04/lastronave-da-300-punti-di-space_622.html

#define APPNAME "Space Invaders"

// Windows Header Files:
#include <windows.h>
#include <string.h>

// Local Header Files
#include "SpaceInvaders.h"
#include "resource.h"

// Makes it easier to determine appropriate code paths:
#if defined (WIN32)
	#define IS_WIN32 TRUE
#else
	#define IS_WIN32 FALSE
#endif
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32

// Global Variables:
HINSTANCE g_hinst;
char szAppName[] = APPNAME; // The name of this application
char INIFile[] = APPNAME".ini";
char szTitle[]   = APPNAME; // The title bar text

#define APP_XSIZE (240+10)
#define APP_YSIZE (320+60)
int AppXSize=APP_XSIZE,AppYSize=APP_YSIZE,AppXSizeR,AppYSizeR;
BYTE doubleSize=1,bSuoni=1;
int alienYPosInit;
HWND ghWnd,hStatusWnd;
HBRUSH hBrush,hBrush2;
HPEN hPen1,hPen2;
HFONT hFont,hFont2,hTitleFont;
DWORD playTime;		// conto cmq il tempo giocato :)
BYTE currQuadro;
int score[2],hiScore,totShips[2],credit;
BYTE missileCnt=0;			// per punti astronave :)
BYTE maxBombeNow;
WORD spaceshipScore[21]={ 300, 50,50,50,100,50,50,50,150,50,200,50,50,100,50,100,50,50,50,150,50 /*,300 */};
BYTE mostriMovePhase=0,velocitaMostri=55/3;
int nauLChar=VK_LEFT,nauRChar=VK_RIGHT,nauFChar=VK_SPACE;
struct MOB myMostri[5*11+1];
struct MOB myMob[20];
//BYTE bombs[MAX_BOMBE];
HDC hCompDC;
UINT hTimer;
WORD demoTime;

enum PLAY_STATE bPlayMode;
enum SUB_PLAY_STATE subPlayMode;
DWORD subPlayModeTime;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	HANDLE hAccelTable;

	if(!hPrevInstance) {
		if(!InitApplication(hInstance)) {
			return (FALSE);
		  }
	  }

	if(!InitInstance(hInstance, nCmdShow)) {
		return (FALSE);
  	}

	if(*lpCmdLine) {
		PostMessage(ghWnd,WM_USER+1,0,(LPARAM)lpCmdLine);
		}
	hAccelTable = LoadAccelerators (hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));
	while(GetMessage(&msg, NULL, 0, 0)) {
		if(!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

  return (msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId,wmEvent;
	PAINTSTRUCT ps;
	HDC hDC;
 	POINT pnt;
	HMENU hMenu;
 	BOOL bGotHelp;
	int i,j,k;
	long l;
	char myBuf[128];
	LOGBRUSH br;
	RECT rc;
	SIZE mySize;
	static int TimerState=-1,TimerCnt;
	HFONT hOldFont;
	HPEN hOldPen;
	struct MOB *mp;

	switch(message) { 
		case WM_COMMAND:
			wmId    = LOWORD(wParam); // Remember, these are...
			wmEvent = HIWORD(wParam); // ...different for Win32!

			switch(wmId) {
				case ID_APP_ABOUT:
					DialogBox(g_hinst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,(DLGPROC)About);
					break;

				case ID_APP_EXIT:
					PostMessage(hWnd,WM_CLOSE,0,0l);
					break;

				case ID_FILE_NEW:
					if(credit>0)			// disattivare se credit=0??
						credit--;
					playTime=0;
					totShips[0]=3;
					score[0]=0;
					TimerCnt=19;			// forza redraw iniziale!
					bPlayMode=PLAY_STARTING;
					alienYPosInit=ALIEN_Y_POS*doubleSize;
					currQuadro=1;
					maxBombeNow=5;
#pragma warning fare NUOVOQUADRO QUA?
					mostriMovePhase=0;

					loadMobs();
					for(i=0; i<55; i++) {
						myMostri[i].bVis=1;
						}
					velocitaMostri=55/3;		//v.sotto
					missileCnt=0;

					RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE | RDW_ERASE);
					// continua a venir cancellata DOPO...
#pragma warning NON RESETTARE pos astronave su quadronuovo!
					mp=&myMob[0];
					mp->x.whole=20*doubleSize;
					hDC=GetDC(hWnd);
//					SelectObject(hDC,hBrush);
//					PatBlt(hDC,mp->x,mp->y,mp->s.cx,mp->s.cy,PATCOPY);
					MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
					ReleaseDC(hWnd,hDC);

updPlay:
					TimerState=0;
					break;

				case ID_FILE_CLOSE:
					currQuadro=0;		// tanto per
					InvalidateRect(hWnd,NULL,TRUE);
					bPlayMode=PLAY_IDLE;
					goto updPlay;
					break;

				case ID_FILE_UPDATE:
					if(bPlayMode==PLAY_PLAY) {
						bPlayMode=PLAY_PAUSED;
						SetWindowText(hWnd,APPNAME" (in pausa)");
						}
					else {
						bPlayMode=PLAY_PLAY;
						SetWindowText(hWnd,APPNAME);
						}
					break;

				case ID_OPZIONI_HEX:
					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case ID_OPZIONI_CREDITO:
					{
					RECT creditRect={ AppXSize/2,AppYSizeR-16*doubleSize,AppXSizeR,AppYSizeR };
					credit++;
					InvalidateRect(hWnd,&creditRect,TRUE);
					}
					break;

				case ID_OPZIONI_DIMENSIONEDOPPIA:
					doubleSize= doubleSize==1 ? 2 : 1;
					InvalidateRect(hWnd,NULL,TRUE);
          MessageBox(GetFocus(),"Riavviare il gioco!",szAppName,MB_OK|MB_ICONEXCLAMATION);
					break;

				case ID_OPZIONI_SUONI:
					bSuoni=!bSuoni;
					break;

				case ID_EDIT_PASTE:
					break;

        case ID_HELP: // Only called in Windows 95
          bGotHelp = WinHelp(hWnd, APPNAME".HLP", HELP_FINDER,(DWORD)0);
          if(!bGotHelp) {
            MessageBox(GetFocus(),"Unable to activate help",
              szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_INDEX: // Not called in Windows 95
          bGotHelp = WinHelp(hWnd, APPNAME".HLP", HELP_CONTENTS,(DWORD)0);
		      if(!bGotHelp) {
            MessageBox(GetFocus(),"Unable to activate help",
              szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_FINDER: // Not called in Windows 95
          if(!WinHelp(hWnd, APPNAME".HLP", HELP_PARTIALKEY,	(DWORD)(LPSTR)"")) {
						MessageBox(GetFocus(),"Unable to activate help",
							szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_USING: // Not called in Windows 95
					if(!WinHelp(hWnd, (LPSTR)NULL, HELP_HELPONHELP, 0)) {
						MessageBox(GetFocus(),"Unable to activate help",
							szAppName, MB_OK|MB_ICONHAND);
					  }
					break;

				default:
					return (DefWindowProc(hWnd, message, wParam, lParam));
					break;
				}
			break;

		case WM_NCRBUTTONUP: // RightClick on windows non-client area...
			if(IS_WIN95 && SendMessage(hWnd, WM_NCHITTEST, 0, lParam) == HTSYSMENU) {
				// The user has clicked the right button on the applications
				// 'System Menu'. Here is where you would alter the default
				// system menu to reflect your application. Notice how the
				// explorer deals with this. For this app, we aren't doing
				// anything
				return DefWindowProc(hWnd, message, wParam, lParam);
			  }
			else {
				// Nothing we are interested in, allow default handling...
				return DefWindowProc(hWnd, message, wParam, lParam);
			  }
      break;

    case WM_RBUTTONDOWN: // RightClick in windows client area...
      pnt.x = LOWORD(lParam);
      pnt.y = HIWORD(lParam);
      ClientToScreen(hWnd, (LPPOINT)&pnt);
      hMenu = GetSubMenu(GetMenu(hWnd),2);
      if(hMenu) {
        TrackPopupMenu(hMenu, 0, pnt.x, pnt.y, 0, hWnd, NULL);
        }
      break;

		case WM_PAINT:
			hDC=BeginPaint(hWnd,&ps);
			hOldPen=SelectObject(hDC,hPen1);
			hOldFont=SelectObject(hDC,hFont2);
			switch(bPlayMode) {
				case PLAY_IDLE:
					SetTextColor(hDC,RGB(255,255,255));
					SetBkColor(hDC,RGB(0,0,0));
					goto plot_credit;
					break;
				case PLAY_STARTING:
				case PLAY_PLAY:
				case PLAY_PAUSED:
				case PLAY_DEMO:
				case PLAY_ENDING:
					hOldPen=SelectObject(hDC,hPen2);
					MoveToEx(hDC,0,AppYSizeR-(LOWER_AREA+2)*doubleSize,NULL);
					LineTo(hDC,AppXSizeR,AppYSizeR-(LOWER_AREA+2)*doubleSize);
					SetTextColor(hDC,RGB(255,255,255));
					SetBkColor(hDC,RGB(0,0,0));
					hOldFont=SelectObject(hDC,hFont2);
					TextOut(hDC,20,8*doubleSize,"SCORE<1>",8);
					wsprintf(myBuf,"%05u",score[0]);
					TextOut(hDC,36,20*doubleSize,myBuf,_tcslen(myBuf));
					TextOut(hDC,AppXSizeR/2-30*doubleSize,8*doubleSize,"HI-SCORE",8);
					wsprintf(myBuf,"%05u",hiScore);
					TextOut(hDC,AppXSizeR/2-18*doubleSize,20*doubleSize,myBuf,strlen(myBuf));
					TextOut(hDC,AppXSizeR-68*doubleSize,8*doubleSize,"SCORE<2>",8);
					wsprintf(myBuf,"%05u",score[1]);
					TextOut(hDC,AppXSizeR-58*doubleSize,20*doubleSize,myBuf,strlen(myBuf));
					wsprintf(myBuf,"%u",totShips[0]);
					TextOut(hDC,10*doubleSize,AppYSizeR-14*doubleSize,myBuf,strlen(myBuf));
					for(i=0; i<MAX_SHIPS; i++) {		// astronavi rimaste
						mp=&myMob[0];
						SelectObject(hDC,hBrush);
						PatBlt(hDC,30*doubleSize+(mp->s.cx+4)*i,AppYSizeR-12*doubleSize,mp->s.cx,mp->s.cy,PATCOPY);
						if(i<(totShips[0]-1)) {
							MobDrawXY(hDC,mp,30*doubleSize+(mp->s.cx+4)*i,AppYSizeR-12*doubleSize);
							}
						}
plot_credit:
					wsprintf(myBuf,"CREDIT %02u",credit);
					TextOut(hDC,AppXSizeR-78*doubleSize,AppYSizeR-14*doubleSize,myBuf,_tcslen(myBuf));
					break;
				}
			SelectObject(hDC,hOldFont);
			SelectObject(hDC,hOldPen);
			EndPaint(hWnd,&ps);
			break;        

		case WM_SIZE:
			GetClientRect(hWnd,&rc);
			MoveWindow(hStatusWnd,0,rc.bottom-16,rc.right,16,1);
			break;        

		case WM_KEYDOWN:
			if(bPlayMode==PLAY_PLAY) {
				i=(TCHAR)wParam;
				if(i==nauLChar) {
					myMob[0].speed.x=-3*doubleSize;
					}
				else if(i==nauRChar) {
					myMob[0].speed.x=+3*doubleSize;
					}
				else if(i==nauFChar) {
					mp=&myMob[1];
					if(!mp->bVis) {
						mp->bVis=1;
						mp->x.whole=myMob[0].x.whole+(myMob[0].s.cx/2-6);
						mp->y.whole=myMob[0].y.whole-mp->s.cy+0*doubleSize;
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_MISSILE));
						missileCnt++;
#ifdef _DEBUG
						wsprintf(myBuf,"MissileCnt=%u",missileCnt);
						SetWindowText(hStatusWnd,myBuf);
#endif
						}
					}
				}
			break;        
		case WM_KEYUP:
			i=(TCHAR)wParam;
			if(i==nauLChar || i==nauRChar) {
				myMob[0].speed.x=0;
				}
			break;        

		case WM_CREATE:
//			bInFront=GetPrivateProfileInt(APPNAME,"SempreInPrimoPiano",0,INIFile);

			srand( (unsigned)time( NULL ) );  

			doubleSize=GetPrivateProfileInt(APPNAME,"DoubleSize",1,INIFile);
			bSuoni=GetPrivateProfileInt(APPNAME,"Suoni",1,INIFile);
			AppXSize=APP_XSIZE*doubleSize;
			AppYSize=APP_YSIZE*doubleSize;

			hFont=CreateFont(8*doubleSize,4*doubleSize,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_MODERN, (LPSTR)"Courier New");
			hFont2=CreateFont(14*doubleSize,7*doubleSize,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, (LPSTR)"Free pixel regular" /*"Arial"*/);
			hTitleFont=CreateFont(18*doubleSize,8*doubleSize,0,0,FW_NORMAL,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, (LPSTR)"Free pixel regular" /*"Arial"*/);


			GetWindowRect(hWnd,&rc);
			rc.right=rc.left+AppXSize;
			rc.bottom=rc.top+AppYSize;
			MoveWindow(hWnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);

			GetClientRect(hWnd,&rc);
			hStatusWnd = CreateWindow("static","",
				WS_BORDER | SS_LEFT | WS_CHILD,
				0,rc.bottom-16,AppXSize-GetSystemMetrics(SM_CXVSCROLL)-2*GetSystemMetrics(SM_CXSIZEFRAME),16,
				hWnd,1001,g_hinst,NULL);
			ShowWindow(hStatusWnd, SW_SHOW);
			GetClientRect(hWnd,&rc);
			AppXSizeR=rc.right-rc.left;
			AppYSizeR=rc.bottom-rc.top-16;
			SendMessage(hStatusWnd,WM_SETFONT,(WPARAM)hFont,0);
			hPen1=CreatePen(PS_SOLID,1,RGB(255,255,255));
			hPen2=CreatePen(PS_SOLID,1,RGB(0,255,0));
			br.lbStyle=BS_SOLID;
			br.lbColor=0x000000;
			br.lbHatch=0;
			hBrush=CreateBrushIndirect(&br);
			br.lbStyle=BS_SOLID;
			br.lbColor=GetSysColor(COLOR_MENU);
			br.lbHatch=0;
			hBrush2=CreateBrushIndirect(&br);
			loadMobs();
			hDC=GetDC(hWnd);
			hCompDC=CreateCompatibleDC(hDC);
			ReleaseDC(hWnd,hDC);

			hiScore=GetPrivateProfileInt(APPNAME,"HiScore",0,INIFile);

			alienYPosInit=ALIEN_Y_POS*doubleSize;
			credit=0;

			hTimer=SetTimer(hWnd,1,40,NULL);

#ifdef _DEBUG
			SetWindowText(hStatusWnd,"<debugmode>");
#endif
			break;

		case WM_TIMER:
			TimerCnt++;
			switch(bPlayMode) {
				static BYTE introScreen=0;

				case PLAY_STARTING:
					hDC=GetDC(hWnd);
					SetTextColor(hDC,RGB(255,255,255));
					SetBkColor(hDC,RGB(0,0,0));
					hOldFont=SelectObject(hDC,hFont2);

					loadMobs();
					for(i=0; i<4; i++) {		// case
						mp=&myMob[i+4];
						if(mp->bVis) {
							SelectObject(hDC,hBrush);
							PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
							MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
							}
						}
					ReleaseDC(hWnd,hDC);
					bPlayMode=PLAY_PLAY;
					break;

				case PLAY_PLAY:
				  animateMobs(hWnd);
					if(!(TimerCnt % velocitaMostri)) {
						i=animateMostri(hWnd);
						if(i) {
							subPlayMode=SUBPLAY_SPACESHIPBOOM;
							subPlayModeTime=timeGetTime()+3000;		// qua di +!!
							bPlayMode=PLAY_PAUSED;
							totShips[0]=0;
							}
						}

					j=0;
					for(i=0; i<55; i++) {
						if(myMostri[i].bVis) {
							j++;
							}
						}
					if(!j) {
nuovoQuadro:
						currQuadro++;

						alienYPosInit+=ALIEN_Y_SIZE*doubleSize;
						maxBombeNow=min(10,5+currQuadro/2);

						// if FINITO! ...
						loadMobs();
/*						for(i=0; i<11; i++) {
							myMostri[i].bVis=1;
							myMostri[i].x=10+i*(ALIEN_X_SIZE+ALIEN_X_SPACING);
							myMostri[i].y=alienYPosInit;
							}
						for(i=11; i<33; i++) {
							myMostri[i].bVis=1;
							myMostri[i].x=10+(i % 11)*(ALIEN_X_SIZE+ALIEN_X_SPACING);
							myMostri[i].y=alienYPosInit+ALIEN_Y_SIZE+ALIEN_X_SPACING;
							}
						for(i=33; i<55; i++) {
							myMostri[i].bVis=1;
							myMostri[i].x=10+(i % 11)*(ALIEN_X_SIZE+ALIEN_X_SPACING);
							myMostri[i].y=alienYPosInit+ALIEN_Y_SIZE*2+ALIEN_X_SPACING*2;
							}*/

						mostriMovePhase=0;

						}
					velocitaMostri=max(2,j/3);			// circa ok 3/7/2020

					if(!(TimerCnt % 20)) {
						playTime++;

						if(!myMob[2].bVis && rand() > 30000) {
							myMob[2].bVis=1;
							if(rand() & 1) {
								myMob[2].x.whole=0;
								myMob[2].y.whole=(SCORE_AREA+6)*doubleSize;
								myMob[2].speed.x=2*doubleSize;
								}
							else {
								myMob[2].x.whole=AppXSizeR-myMob[2].s.cx;
								myMob[2].y.whole=(SCORE_AREA+6)*doubleSize;
								myMob[2].speed.x=-2*doubleSize;
								}
							}

						if(getNumBombe()<maxBombeNow && rand() > 20000) {
							POINT pos;
							if(getSuitableBomb(rand() & 1,&pos)) {		// 50% probabile contro astronave! aumentare coni quadri?
								for(i=10; i<10+maxBombeNow; i++) {			// 
									if(!myMob[i].bVis) {
										myMob[i].x.whole=pos.x+ALIEN_X_SIZE/2-1;
										myMob[i].y.whole=pos.y+ALIEN_Y_SIZE;
										myMob[i].bVis=1;
										myMob[i].speed.y=3+3*(currQuadro/3);
										break;
										}
									}
								}
							}

//						if(!totShips[0]) {
//							bPlayMode=PLAY_ENDING;
//							}

						}

					switch(subPlayMode) {
						case SUBPLAY_NONE:
							break;
						case SUBPLAY_ALIENBOOM:
							if(timeGetTime() > subPlayModeTime) {
								hDC=GetDC(hWnd);
								MobErase(hDC,&myMostri[55]);
								ReleaseDC(hWnd,hDC);
								subPlayMode=SUBPLAY_NONE;
								}
							break;
						case SUBPLAY_UFOBOOM:
							if(timeGetTime() > subPlayModeTime) {
								hDC=GetDC(hWnd);
								MobErase(hDC,&myMob[2]);
								SelectObject(hDC,hBrush);
								PatBlt(hDC,myMob[2].x.whole,myMob[2].y.whole,myMob[2].s.cx*2,myMob[2].s.cy*2,PATCOPY);		// pulisco bene "punti"
								ReleaseDC(hWnd,hDC);
								subPlayMode=SUBPLAY_NONE;
								}
							break;
						case SUBPLAY_SPACESHIPBOOM:
							break;
						}


					break;

				case PLAY_PAUSED:
					switch(subPlayMode) {
						case SUBPLAY_NONE:
							break;
						case SUBPLAY_ALIENBOOM:
							break;
						case SUBPLAY_UFOBOOM:
							break;
						case SUBPLAY_SPACESHIPBOOM:
							if(timeGetTime() > subPlayModeTime) {
								hDC=GetDC(hWnd);
								MobErase(hDC,&myMob[0]);
								myMob[0].x.whole=20*doubleSize;
								subPlayMode=SUBPLAY_NONE;
								if(totShips[0]>0) {
									SelectObject(hCompDC,myMob[0].hImg);
									MobDraw(hDC,&myMob[0]);
									bPlayMode=PLAY_PLAY;
									}
								else
									bPlayMode=PLAY_ENDING;
								ReleaseDC(hWnd,hDC);
								}
							else {
								hDC=GetDC(hWnd);
								MobErase(hDC,&myMob[0]);
								SelectObject(hCompDC,timeGetTime() & 2 ? myMob[0].hImg : myMob[0].hImgAlt);
								MobDraw(hDC,&myMob[0]);
								ReleaseDC(hWnd,hDC);
								}
							break;
						}
//					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case PLAY_ENDING:
					if(score[0] > hiScore)
						hiScore=score[0];
					PostMessage(hWnd,WM_COMMAND,ID_FILE_CLOSE,0);
					demoTime=600;
					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case PLAY_IDLE:
					if(!(TimerCnt % 15)) {
						TimerState++;
						switch(TimerState) {
							HBITMAP hImg;
							case 0:
//								InvalidateRect(hWnd,NULL,TRUE);
								hDC=GetDC(hWnd);
								hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_SPACEINVADERS));
								SelectObject(hCompDC,hImg);
								StretchBlt(hDC,0,0,AppXSizeR,AppYSizeR,hCompDC,0,0,226 /*240*/,320,SRCCOPY);
								ReleaseDC(hWnd,hDC);
								break;
							case 1:
							case 2:
							case 3:
							case 4:
							case 5:
							case 6:
								LoadString(g_hinst,IDS_OPENSTRING1-1+TimerState,myBuf,64);
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(255,255,255));
								SetBkColor(hDC,RGB(0,0,0));
								hOldFont=SelectObject(hDC,hTitleFont);
								switch(TimerState) {
									struct MOB *mp;

									case 1:
									case 6:
										GetTextExtentPoint32(hDC,myBuf,strlen(myBuf),&mySize);
										TextOut(hDC,(AppXSize-mySize.cx)/2,(15+TimerState*35)*doubleSize,myBuf,strlen(myBuf));
										break;
									case 2:
										mp=&myMob[2];
										goto case4;
									case 3:
										mp=&myMostri[0];
										goto case4;
									case 4:
										mp=&myMostri[11];
										goto case4;
									case 5:
										mp=&myMostri[33];
										goto case4;
case4:
										MobDrawXY(hDC,mp,AppXSizeR/2-mp->s.cx-40,(40+TimerState*25)*doubleSize+10);
										TextOut(hDC,AppXSizeR/2-20,(40+TimerState*25)*doubleSize,myBuf,strlen(myBuf));
										break;
									}
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 7:
								introScreen++;
								if(introScreen>3) {
									bPlayMode=PLAY_DEMO;
									InvalidateRect(hWnd,NULL,TRUE);
									loadMobs();
									myMob[0].x.whole=AppXSizeR/2;
									demoTime=600;
									}
								TimerState=-1;
								break;
							}
						}
					break;
				case PLAY_DEMO:
					velocitaMostri=10;
				  animateMobs(hWnd);
					if(!(TimerCnt % velocitaMostri)) {
						animateMostri(hWnd);
						hDC=GetDC(hWnd);
						for(i=0; i<4; i++) {		// case
							mp=&myMob[i+4];
							SelectObject(hDC,hBrush);
							PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
							MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
							}
						ReleaseDC(hWnd,hDC);
						}
					hDC=GetDC(hWnd);
					if(!(TimerCnt % 15)) {
						mp=&myMob[0];
						SelectObject(hDC,hBrush);
						PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
						if(rand() > 20000) {
							if(mp->x.whole < (AppXSizeR - 20*doubleSize))
								mp->speed.x=3;
							else
								mp->speed.x=0;
							}
						else if(rand() > 10000) {
							if(mp->x.whole > 14*doubleSize)
								mp->speed.x=-3;
							else
								mp->speed.x=0;
							}
						MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
						}
					ReleaseDC(hWnd,hDC);

					demoTime--;
					if(!demoTime) {
						introScreen=0;
						bPlayMode=PLAY_IDLE;
						InvalidateRect(hWnd,NULL,TRUE);
						}
					break;
				}
			break;

		case WM_CLOSE:
			if(bPlayMode==PLAY_PLAY || bPlayMode==PLAY_PAUSED || bPlayMode==PLAY_STARTING || bPlayMode==PLAY_ENDING) {
				if(MessageBox(hWnd,"Terminare la partita in corso?",APPNAME,MB_YESNO | MB_DEFBUTTON2)==IDYES)
					DestroyWindow(hWnd);
			  }
			else
				DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			WritePrivateProfileInt(APPNAME,"HiScore",hiScore,INIFile);
			WritePrivateProfileInt(APPNAME,"DoubleSize",doubleSize,INIFile);
			WritePrivateProfileInt(APPNAME,"Suoni",bSuoni,INIFile);
//			WritePrivateProfileInt(APPNAME,"SempreInPrimoPiano",bInFront,INIFile);
			// Tell WinHelp we don't need it any more...
			KillTimer(hWnd,hTimer);
	    WinHelp(hWnd,APPNAME".HLP",HELP_QUIT,(DWORD)0);
			DeleteDC(hCompDC);
			DeleteObject(hBrush);
			DeleteObject(hBrush2);
			DeleteObject(hPen1);
			DeleteObject(hPen2);
			DeleteObject(hFont);
			DeleteObject(hFont2);
			DeleteObject(hTitleFont);
			PostQuitMessage(0);
			break;

   	case WM_INITMENU:
   	  EnableMenuItem((HMENU)wParam,ID_FILE_NEW,(bPlayMode==PLAY_IDLE || bPlayMode==PLAY_DEMO) ? MF_ENABLED : MF_GRAYED);
   	  EnableMenuItem((HMENU)wParam,ID_FILE_UPDATE,(bPlayMode==PLAY_PLAY || bPlayMode==PLAY_PAUSED) ? MF_ENABLED : MF_GRAYED);
   	  EnableMenuItem((HMENU)wParam,ID_FILE_CLOSE,(bPlayMode==PLAY_PLAY || bPlayMode==PLAY_PAUSED) ? MF_ENABLED : MF_GRAYED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_DIMENSIONEDOPPIA,doubleSize==2 ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_SUONI,bSuoni ? MF_CHECKED : MF_UNCHECKED);
			break;

		case WM_CTLCOLORSTATIC:
			SetTextColor((HDC)wParam,GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor((HDC)wParam,GetSysColor(COLOR_MENU));
			return (long)hBrush2;
			break;

		default:
			return (DefWindowProc(hWnd, message, wParam, lParam));
		}
	return (0);
	}



ATOM MyRegisterClass(CONST WNDCLASS *lpwc) {
	HANDLE  hMod;
	FARPROC proc;
	WNDCLASSEX wcex;

	hMod=GetModuleHandle("USER32");
	if(hMod != NULL) {

#if defined (UNICODE)
		proc = GetProcAddress (hMod, "RegisterClassExW");
#else
		proc = GetProcAddress (hMod, "RegisterClassExA");
#endif

		if(proc != NULL) {
			wcex.style         = lpwc->style;
			wcex.lpfnWndProc   = lpwc->lpfnWndProc;
			wcex.cbClsExtra    = lpwc->cbClsExtra;
			wcex.cbWndExtra    = lpwc->cbWndExtra;
			wcex.hInstance     = lpwc->hInstance;
			wcex.hIcon         = lpwc->hIcon;
			wcex.hCursor       = lpwc->hCursor;
			wcex.hbrBackground = lpwc->hbrBackground;
    	wcex.lpszMenuName  = lpwc->lpszMenuName;
			wcex.lpszClassName = lpwc->lpszClassName;

			// Added elements for Windows 95:
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.hIconSm = LoadIcon(wcex.hInstance, "SMALL");
			
			return (*proc)(&wcex);//return RegisterClassEx(&wcex);
		}
	}
return (RegisterClass(lpwc));
}


BOOL InitApplication(HINSTANCE hInstance) {
  WNDCLASS  wc;
  HWND      hwnd;

  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = (WNDPROC)WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APP32));
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(GetStockObject(BLACK_BRUSH));

        // Since Windows95 has a slightly different recommended
        // format for the 'Help' menu, lets put this in the alternate menu like this:
  if(IS_WIN95) {
		wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    } else {
	  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    }
  wc.lpszClassName = szAppName;

  if(IS_WIN95) {
	  if(!MyRegisterClass(&wc))
			return 0;
    }
	else {
	  if(!RegisterClass(&wc))
	  	return 0;
    }


  }

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	
	g_hinst=hInstance;

	ghWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX /*| WS_MAXIMIZEBOX */ | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, AppXSize,AppYSize,
		NULL, NULL, hInstance, NULL);

	if(!ghWnd) {
		return (FALSE);
	  }

	ShowWindow(ghWnd, nCmdShow);
	UpdateWindow(ghWnd);

	return (TRUE);
  }

//
//  FUNCTION: About(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for "About" dialog box
// 		This version allows greater flexibility over the contents of the 'About' box,
// 		by pulling out values from the 'Version' resource.
//
//  MESSAGES:
//
//	WM_INITDIALOG - initialize dialog box
//	WM_COMMAND    - Input received
//
//
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static  HFONT hfontDlg;		// Font for dialog text
	static	HFONT hFinePrint;	// Font for 'fine print' in dialog
	DWORD   dwVerInfoSize;		// Size of version information block
	LPSTR   lpVersion;			// String pointer to 'version' text
	DWORD   dwVerHnd=0;			// An 'ignored' parameter, always '0'
	UINT    uVersionLen;
	WORD    wRootLen;
	BOOL    bRetCode;
	int     i;
	char    szFullPath[256];
	char    szResult[256];
	char    szGetName[256];
	DWORD	dwVersion;
	char	szVersion[40];
	DWORD	dwResult;

	switch (message) {
    case WM_INITDIALOG:
//			ShowWindow(hDlg, SW_HIDE);
			hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				VARIABLE_PITCH | FF_SWISS, "");
			hFinePrint = CreateFont(11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				VARIABLE_PITCH | FF_SWISS, "");
//			CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
			GetModuleFileName(g_hinst, szFullPath, sizeof(szFullPath));

			// Now lets dive in and pull out the version information:
			dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
			if(dwVerInfoSize) {
				LPSTR   lpstrVffInfo;
				HANDLE  hMem;
				hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
				lpstrVffInfo  = GlobalLock(hMem);
				GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
				// The below 'hex' value looks a little confusing, but
				// essentially what it is, is the hexidecimal representation
				// of a couple different values that represent the language
				// and character set that we are wanting string values for.
				// 040904E4 is a very common one, because it means:
				//   US English, Windows MultiLingual characterset
				// Or to pull it all apart:
				// 04------        = SUBLANG_ENGLISH_USA
				// --09----        = LANG_ENGLISH
				// ----04E4 = 1252 = Codepage for Windows:Multilingual
				lstrcpy(szGetName, "\\StringFileInfo\\040904E4\\");	 
				wRootLen = lstrlen(szGetName); // Save this position
			
				// Set the title of the dialog:
				lstrcat (szGetName, "ProductName");
				bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
					(LPSTR)szGetName,
					(LPVOID)&lpVersion,
					(UINT *)&uVersionLen);
//				lstrcpy(szResult, "About ");
//				lstrcat(szResult, lpVersion);
//				SetWindowText (hDlg, szResult);

				// Walk through the dialog items that we want to replace:
				for(i=DLG_VERFIRST; i <= DLG_VERLAST; i++) {
					GetDlgItemText(hDlg, i, szResult, sizeof(szResult));
					szGetName[wRootLen] = (char)0;
					lstrcat (szGetName, szResult);
					uVersionLen   = 0;
					lpVersion     = NULL;
					bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
						(LPSTR)szGetName,
						(LPVOID)&lpVersion,
						(UINT *)&uVersionLen);

					if(bRetCode && uVersionLen && lpVersion) {
					// Replace dialog item text with version info
						lstrcpy(szResult, lpVersion);
						SetDlgItemText(hDlg, i, szResult);
					  }
					else {
						dwResult = GetLastError();
						wsprintf (szResult, "Error %lu", dwResult);
						SetDlgItemText (hDlg, i, szResult);
					  }
					SendMessage (GetDlgItem (hDlg, i), WM_SETFONT, 
						(UINT)((i==DLG_VERLAST)?hFinePrint:hfontDlg),TRUE);
				  } // for


				GlobalUnlock(hMem);
				GlobalFree(hMem);

			}
		else {
				// No version information available.
			} // if (dwVerInfoSize)

    SendMessage(GetDlgItem (hDlg, IDC_LABEL), WM_SETFONT,
			(WPARAM)hfontDlg,(LPARAM)TRUE);

			// We are  using GetVersion rather then GetVersionEx
			// because earlier versions of Windows NT and Win32s
			// didn't include GetVersionEx:
			dwVersion = GetVersion();

			if(dwVersion < 0x80000000) {
				// Windows NT
				wsprintf (szVersion, "Microsoft Windows NT %u.%u (Build: %u)",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
          (DWORD)(HIWORD(dwVersion)) );
				}
			else
				if(LOBYTE(LOWORD(dwVersion))<4) {
					// Win32s
				wsprintf (szVersion, "Microsoft Win32s %u.%u (Build: %u)",
  				(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
					(DWORD)(HIWORD(dwVersion) & ~0x8000) );
				}
			else {
					// Windows 95
				wsprintf(szVersion,"Microsoft Windows 95 %u.%u",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))) );
				}

			SetWindowText(GetDlgItem(hDlg, IDC_OSVERSION), szVersion);
//			SetWindowPos(hDlg,NULL,GetSystemMetrics(SM_CXSCREEN)/2,GetSystemMetrics(SM_CYSCREEN)/2,0,0,SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER);
//			ShowWindow(hDlg, SW_SHOW);
			return (TRUE);

		case WM_COMMAND:
			if(wParam==IDOK || wParam==IDCANCEL) {
  		  EndDialog(hDlg,0);
			  return (TRUE);
			  }
			else if(wParam==3) {
				MessageBox(hDlg,"Se trovate utile questo programma, mandate un contributo!!\nLunt rd. 121 - L205EZ Liverpool (England)\n[Dario Greggio]","ADPM Synthesis sas",MB_OK);
			  return (TRUE);
			  }
			break;
		}

	return FALSE;
	}


BOOL PlayResource(LPSTR lpName) { 
  BOOL bRtn; 
  LPSTR lpRes; 
  HANDLE hResInfo, hRes; 

  // Find the WAVE resource.  
  hResInfo = FindResource(g_hinst, lpName, "WAVE"); 
  if(hResInfo == NULL) 
    return FALSE; 

  // Load the WAVE resource. 
  hRes = LoadResource(g_hinst, hResInfo); 
  if(hRes == NULL) 
    return FALSE; 

  // Lock the WAVE resource and play it. 
  lpRes = LockResource(hRes); 
  if(lpRes != NULL) { 
    bRtn = sndPlaySound(lpRes, SND_MEMORY | SND_ASYNC | SND_NODEFAULT | SND_NOSTOP); 
    UnlockResource(hRes); 
		} 
  else 
    bRtn = 0; 
 
  // Free the WAVE resource and return success or failure. 
 
  FreeResource(hRes); 
  return bRtn; 
	}

int WritePrivateProfileInt(char *s, char *s1, int n, char *f) {
//  int i;
  char myBuf[16];
  
  wsprintf(myBuf,"%d",n);
  return WritePrivateProfileString(s,s1,myBuf,f);
  }

int ShowMe(void) {
	int i;
	char buffer[16];

	buffer[0]='A'^ 0x17;
	buffer[1]='D'^ 0x17;
	buffer[2]='P'^ 0x17;
	buffer[3]='M'^ 0x17;
	buffer[4]='-'^ 0x17;
	buffer[5]='G'^ 0x17;
	buffer[6]='.'^ 0x17;
	buffer[7]='D'^ 0x17;
	buffer[8]='a'^ 0x17;
	buffer[9]='r'^ 0x17;
	buffer[10]=' '^ 0x17;
	buffer[11]='2'^ 0x17;
	buffer[12]='0' ^ 0x17;
	buffer[13]=0;
	for(i=0; i<13; i++) buffer[i]^=0x17;
	return MessageBox(GetDesktopWindow(),buffer,APPNAME,MB_OK | MB_ICONEXCLAMATION);
	}


