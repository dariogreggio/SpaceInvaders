#include <windows.h>
#include "SpaceInvaders.h"
//https://www.youtube.com/watch?v=3WOwi2hikgc

extern struct MOB myMostri[5*11+1];			// 5 file da 11 alieni + esplosione :D
extern struct MOB myMob[20];			// astronave, missile, spaceship, 4 capanne, 10 bombe aliene
extern int AppXSize,AppYSize,AppXSizeR,AppYSizeR;
extern BYTE doubleSize,bSuoni;
extern int alienYPosInit;
extern int totShips[2];
extern int score[2];
extern BYTE mostriMovePhase;
extern BYTE missileCnt;
extern BYTE maxBombeNow;
extern DWORD subPlayModeTime;
extern WORD spaceshipScore[];
extern BYTE currQuadro;
extern HDC hCompDC;
extern HBRUSH hBrush,hBrush2;
extern HFONT hFont2;

int MobCreate(struct MOB *mp,int img1,int img2,WORD cx,WORD cy) {

	if(mp->hImg)
		DeleteObject(mp->hImg);
	if(mp->hImgAlt)
		DeleteObject(mp->hImgAlt);
	mp->hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(img1));
	if(img2)
		mp->hImgAlt=LoadBitmap(g_hinst,MAKEINTRESOURCE(img2));
	mp->s.cx=cx; mp->s.cy=cy;
	SetBitmapDimensionEx(mp->hImg,mp->s.cx,mp->s.cy,NULL);		// 
	mp->hImgOld=NULL;
	mp->bUseTransparency=0;
	return 1;
	}

int MobErase(HDC hDC,struct MOB *mp) {

//	mp->bVis=0;		// mah
	if(mp->hImgOld) {
		SelectObject(hCompDC,mp->hImgOld);
		return BitBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx*doubleSize,mp->s.cy*doubleSize,hCompDC,0,0,SRCCOPY);
		}
	else {
		SelectObject(hDC,hBrush);
	/*	if(doubleSize==2)
			return PatBlt(hDC,mp->x,mp->y,mp->s.cx*2,mp->s.cy*2,PATCOPY);
		else*/
		return PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
		}
	}

int MobDraw(HDC hDC,struct MOB *mp) {

	if(mp->bUseTransparency) {
		if(!mp->hImgOld)
			mp->hImgOld=CreateCompatibleBitmap(hDC,mp->s.cx*doubleSize,mp->s.cy*doubleSize);
		SelectObject(hCompDC,mp->hImgOld);
		BitBlt(hCompDC,0,0,mp->s.cx,mp->s.cy,hDC,mp->x.whole,mp->y.whole,SRCCOPY);
		}

//	SelectObject(hCompDC,mp->hImg); non è sempre così...
	if(doubleSize==2)
		return StretchBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx*2,mp->s.cy*2,hCompDC,0,0,mp->s.cx,mp->s.cy,SRCCOPY);
	else
		return BitBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,hCompDC,0,0,SRCCOPY);
	}

int MobDrawXY(HDC hDC,struct MOB *mp,WORD x,WORD y) {

	if(mp->bUseTransparency) {
		if(!mp->hImgOld)
			mp->hImgOld=CreateCompatibleBitmap(hDC,mp->s.cx*doubleSize,mp->s.cy*doubleSize);
		SelectObject(hCompDC,mp->hImgOld);
		BitBlt(hCompDC,0,0,mp->s.cx,mp->s.cy,hDC,mp->x.whole,mp->y.whole,SRCCOPY);
		}

	SelectObject(hCompDC,mp->hImg);
	if(doubleSize==2)
		return StretchBlt(hDC,x,y,mp->s.cx*2,mp->s.cy*2,hCompDC,0,0,mp->s.cx,mp->s.cy,SRCCOPY);
	else
		return BitBlt(hDC,x,y,mp->s.cx,mp->s.cy,hCompDC,0,0,SRCCOPY);
	}

int MobMove(HDC hDC,struct MOB *mp,SIZE s) {

	MobErase(hDC,mp);
	// qua NON uso fract... (2022)
	mp->x.whole += s.cx;
	mp->y.whole += s.cy;
	return MobDraw(hDC,mp);
	}

int MobCollisionRect(struct MOB *mp,RECT *rc) {
	RECT rc2;
	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy;

  return ! (rc2.left > rc->right || rc2.right < rc->left
        || rc2.top > rc->bottom || rc2.bottom < rc->top);

	}

int MobCollisionPoint(struct MOB *mp,POINT pt) {
	RECT rc2;
	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy;

  return ! (rc2.left > pt.x || rc2.right < pt.x
        || rc2.top > pt.y || rc2.bottom < pt.y);

	}


int loadMobs() {
	int i;

	for(i=0; i<11; i++) {
		MobCreate(&myMostri[i],IDB_ALIENO1,IDB_ALIENO12,/*ALIEN_X_SIZE*/11*doubleSize,ALIEN_Y_SIZE*doubleSize);
		myMostri[i].x.whole=10+i*(ALIEN_X_SIZE+ALIEN_X_SPACING)*doubleSize;
		myMostri[i].y.whole=alienYPosInit;
		myMostri[i].punti=30;
		myMostri[i].speed.x=2*doubleSize; myMostri[i].speed.y=0;
		myMostri[i].bVis=1;
		}
	for(i=11; i<33; i++) {
		MobCreate(&myMostri[i],IDB_ALIENO2,IDB_ALIENO22,/*ALIEN_X_SIZE*/12*doubleSize,ALIEN_Y_SIZE*doubleSize);
		myMostri[i].x.whole=10+(i % 11)*(ALIEN_X_SIZE+ALIEN_X_SPACING)*doubleSize;
		myMostri[i].y.whole=alienYPosInit+(i/11)*(ALIEN_Y_SIZE+ALIEN_X_SPACING)*doubleSize;
		myMostri[i].punti=20;
		myMostri[i].speed.x=2*doubleSize; myMostri[i].speed.y=0;
		myMostri[i].bVis=1;
		}
	for(i=33; i<55; i++) {
		MobCreate(&myMostri[i],IDB_ALIENO3,IDB_ALIENO32,ALIEN_X_SIZE*doubleSize,ALIEN_Y_SIZE*doubleSize);
		myMostri[i].x.whole=10+(i % 11)*(ALIEN_X_SIZE+ALIEN_X_SPACING)*doubleSize;
		myMostri[i].y.whole=alienYPosInit+(i/11)*(ALIEN_Y_SIZE+ALIEN_X_SPACING)*doubleSize;
		myMostri[i].punti=10;
		myMostri[i].speed.x=2*doubleSize; myMostri[i].speed.y=0;
		myMostri[i].bVis=1;
		}
	MobCreate(&myMostri[55],IDB_ALIENOBOOM,0,ALIEN_X_SIZE*doubleSize,ALIEN_Y_SIZE*doubleSize);
	myMostri[55].x.whole=0;
	myMostri[55].y.whole=0;
	myMostri[55].punti=0;
	myMostri[55].speed.x=0; myMostri[55].speed.y=0;
	myMostri[55].bVis=0;

	//astronave
	MobCreate(&myMob[0],IDB_ASTRONAVE,IDB_ASTRONAVEESPLOSA,16*doubleSize,14*doubleSize);
	myMob[0].x.whole=1;
	myMob[0].y.whole=AppYSizeR-36*doubleSize;
	myMob[0].speed.x=0; myMob[0].speed.y=0;
	myMob[0].bVis=1;

#pragma warning il MISSILE e singola linea!! non bomba
	MobCreate(&myMob[1],IDB_MISSILE,IDB_MISSILE2,3*doubleSize,10*doubleSize);
	myMob[1].x.whole=0;
	myMob[1].y.whole=150;
	myMob[1].speed.y=-4*doubleSize; myMob[1].speed.x=0;

	MobCreate(&myMob[2],IDB_SPACESHIP,IDB_SPACESHIP2,16*doubleSize,8*doubleSize);
	myMob[2].x.whole=0;
	myMob[2].y.whole=(SCORE_AREA+6)*doubleSize;
	myMob[2].speed.x=2*doubleSize; myMob[2].speed.y=0;

	//case
	for(i=4; i<8; i++) {			// 
		MobCreate(&myMob[i],IDB_CASA,IDB_CASA,22*doubleSize,16*doubleSize);
		// uso la Alt per distruggere :) v. sotto
		myMob[i].x.whole=(i-4)*(AppXSizeR/5)+40*doubleSize;
		myMob[i].y.whole=AppYSizeR-72*doubleSize;
		myMob[i].speed.x=myMob[i].speed.y=0;
		myMob[i].bVis=1;
		}

	//bombe
	for(i=10; i<20 /*10+MAX_BOMBE*/; i++) {			// 
#pragma warning usare 2 bombe diverse
		MobCreate(&myMob[i],IDB_BOMBA,IDB_BOMBA2,4*doubleSize,10*doubleSize);
		myMob[i].x.whole=0;
		myMob[i].y.whole=0;
		myMob[i].speed.y=0; myMob[i].speed.x=0;
		myMob[i].bVis=0;
		}

	return 1;
	}

int animateMobs(HWND hWnd) {
	struct MOB *mp;
	register int i,j;
	HDC hDC;
	static BYTE dividerUfo,dividerMissile;

	hDC=GetDC(hWnd);

	mp=&myMob[0];				// astronave
	if(mp->bVis) {
		if(mp->speed.x>0) {
			if(mp->x.whole > (AppXSizeR-mp->s.cx-4*doubleSize)) {
				mp->speed.x=0;
				}
			}
		else {
			if(mp->x.whole < (5*doubleSize)) {
				mp->speed.x=0;
				}
			}
		if(mp->speed.x) {
	//	if(mp->x != nauPos) {
		//				i=DrawIcon(hDC,xPos,100,hNave1);
			MobErase(hDC,mp);
			mp->x.whole += mp->speed.x;
			SelectObject(hCompDC,mp->hImg);
			MobDraw(hDC,mp);
			}
		}

	mp=&myMob[1];
	if(mp->bVis) {			// missile
		dividerMissile++;
		MobErase(hDC,mp);
		SelectObject(hCompDC,dividerMissile & 1 ? mp->hImgAlt : mp->hImg);
		mp->y.whole += mp->speed.y;
		if(mp->y.whole < SCORE_AREA*doubleSize) {
			mp->bVis=0;
			}
		else {

			for(i=4; i<4+4; i++) {			// 
				if(isMissileInCasa(&myMob[i])) {
					hitCasa(&myMob[i],mp,0);
					SelectObject(hCompDC,myMob[i].hImgAlt);
					MobDraw(hDC,&myMob[i]);
					mp->bVis=0;
					break;
					}
				}

			if(mp->bVis) {		// PRIMA PROVARE case e bombe!
				for(i=10; i<10+maxBombeNow; i++) {			// 
					if(myMob[i].bVis) {
						if(isMissileInArea(&myMob[i])) {
							mp->bVis=0;
							if((rand() % 20) > currQuadro) {		// in certi casi non scoppia :D
								myMob[i].bVis=0;
								MobErase(hDC,&myMob[i]);
	//							subPlayMode=SUBPLAY_BOMBABOOM; // in teoria ci sarebbe l'esplosioncina pure della bomba...
								}
							break;
							}
						}
					}
				}

			if(mp->bVis) {		// se non ha trovato bombe o case... provo mostri
				for(i=0; i<55; i++) {
					if(myMostri[i].bVis) {
						if(isMissileInArea(&myMostri[i])) {
							RECT scoreRect={ 0,0,AppXSize,40*doubleSize };
							mp->bVis=myMostri[i].bVis=0;
							MobErase(hDC,&myMostri[i]);
							myMostri[55].x.whole=myMostri[i].x.whole;  myMostri[55].y.whole=myMostri[i].y.whole-1*doubleSize;
							SelectObject(hCompDC,myMostri[55].hImg);
							if(doubleSize==2)
								j=StretchBlt(hDC,myMostri[55].x.whole,myMostri[55].y.whole,myMostri[55].s.cx*2,myMostri[55].s.cy*2,hCompDC,0,0,
									myMostri[55].s.cx,myMostri[55].s.cy,SRCCOPY);
							else
								j=BitBlt(hDC,myMostri[55].x.whole,myMostri[55].y.whole,myMostri[55].s.cx,myMostri[55].s.cy,hCompDC,0,0,SRCCOPY);
							subPlayMode=SUBPLAY_ALIENBOOM;
							subPlayModeTime=timeGetTime()+500;
							score[0]+=myMostri[i].punti;

							if(bSuoni)
								PlayResource(MAKEINTRESOURCE(IDR_WAVE_UFODESTROYED));
							InvalidateRect(hWnd,&scoreRect,TRUE);
							break;
							}
						}
					}
				}

			if(mp->bVis) {		// se non ha trovato mostri... provo astronave
				if(myMob[2].bVis) {
					if(isMissileInArea(&myMob[2])) {
						RECT scoreRect={ 0,0,AppXSize,40*doubleSize };
						char myBuf[16];
						HFONT hOldFont;

						mp->bVis=myMob[2].bVis=0;
						MobErase(hDC,&myMob[2]);
						subPlayMode=SUBPLAY_UFOBOOM;
						subPlayModeTime=timeGetTime()+1500;
						score[0]+=spaceshipScore[missileCnt % 21];

#pragma warning MISSILESCORE primo giro
						wsprintf(myBuf,"%u",spaceshipScore[missileCnt % 21]);
						SetTextColor(hDC,RGB(255,0,0));
						SetBkColor(hDC,RGB(0,0,0));
						hOldFont=SelectObject(hDC,hFont2);
						TextOut(hDC,myMob[2].x.whole,myMob[2].y.whole,myBuf,strlen(myBuf));
						SelectObject(hDC,hOldFont);
#pragma warning PULIRE PUNTI ASTRONAVE
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_UFODESTROYED));		// cambiare! o 2 di seguito, com'era??

						InvalidateRect(hWnd,&scoreRect,TRUE);
						}
					}
				}

			if(mp->bVis) 
				MobDraw(hDC,mp);

			}

		}

	mp=&myMob[2];			// Ufo
	if(mp->bVis) {
		MobErase(hDC,mp);
//		PatBlt(hDC,mp->x,mp->y,mp->s.cx,mp->s.cy,PATCOPY);
//		SelectObject(hCompDC,mp->hImg);
		mp->x.whole += mp->speed.x;
		if(mp->speed.x>0) {
			if(mp->x.whole>(AppXSizeR-mp->s.cx-2)) {
				mp->bVis=0;
				mp->speed.x=0;
				}
			else {
				SelectObject(hCompDC,dividerUfo & 1 ? mp->hImgAlt : mp->hImg);
				MobDraw(hDC,mp);
				}
			}
		else {
			if(mp->x.whole<(2)) {
				mp->bVis=0;
				mp->speed.x=0;
				}
			else {
				SelectObject(hCompDC,dividerUfo & 1 ? mp->hImgAlt : mp->hImg);
				MobDraw(hDC,mp);
				}
			}
		dividerUfo++;
		if(dividerUfo>4) {
			if(bSuoni)
				PlayResource(MAKEINTRESOURCE(IDR_WAVE_UFO));
			dividerUfo=0;
			}
		}

	for(i=10; i<10+maxBombeNow; i++) {			// 
		mp=&myMob[i];
		if(mp->bVis) {
			MobErase(hDC,mp);

			if(isBombaInArea(mp,&myMob[0])) {
#pragma warning se MOSTRI ULTIMA RIGA IN BASSO le bombe non colpiscono!!
				RECT shipAreaRect={ 0,AppYSizeR-LOWER_AREA*doubleSize,AppXSizeR,AppYSizeR};
				mp->bVis=0;
				mp->speed.y=0;
				subPlayMode=SUBPLAY_SPACESHIPBOOM;
				subPlayModeTime=timeGetTime()+2500;
				bPlayMode=PLAY_PAUSED;
				if(bSuoni)
					PlayResource(MAKEINTRESOURCE(IDR_WAVE_SHIPDESTROYED));
				totShips[0]--;

				InvalidateRect(hWnd,&shipAreaRect,TRUE);
				}


			for(j=4; j<4+4; j++) {			// 
				if(isBombaInCasa(mp,&myMob[j])) {
					hitCasa(&myMob[j],mp,1);
					SelectObject(hCompDC,myMob[j].hImgAlt);
					MobDraw(hDC,&myMob[j]);
					mp->bVis=0;
					mp->speed.y=0;
					}
				}

			if(mp->bVis) {
				SelectObject(hCompDC,rand() & 1 ? mp->hImgAlt : mp->hImg);
				mp->y.whole += mp->speed.y;
				if(mp->y.whole > (AppYSizeR-(LOWER_AREA+4)*doubleSize-mp->s.cy)) {
					mp->bVis=0;
					mp->speed.y=0;
					}
				else {
					MobDraw(hDC,mp);
					}
				}

			}
		}

	ReleaseDC(hWnd,hDC);

	return 1;
  }

int getLeftmostMostro() {
	int i,j;
	struct MOB *mp;

	j=AppXSizeR;
	for(i=0; i<55; i++) {
		mp=&myMostri[i];
		if(mp->bVis) {
			if(mp->x.whole < j) {
				j=mp->x.whole;
				}
			}
		}
	return j;
	}

int getRightmostMostro(void) {
	int i,j;
	struct MOB *mp;

	j=0;
	for(i=0; i<55; i++) {
		mp=&myMostri[i];
		if(mp->bVis) {
			if(mp->x.whole > j) {
				j=mp->x.whole+mp->s.cx;
				}
			}
		}
	return j;
	}

int getBottomestMostro(void) {			// :)
	int i,j;
	struct MOB *mp;

	j=0;
	for(i=0; i<55; i++) {
		mp=&myMostri[i];
		if(mp->bVis) {
			if(mp->y.whole > j) {
				j=mp->y.whole+mp->s.cy;
				}
			}
		}
	return j;
	}

int animateMostri(HWND hWnd) {
	struct MOB *mp;
	register int i,j;
	HDC hDC;

	hDC=GetDC(hWnd);

	for(i=0; i<55; i++) {
		mp=&myMostri[i];
		if(mp->bVis) {
//			PatBlt(hDC,mp->x,mp->y,mp->s.cx,mp->s.cy,PATCOPY);
			MobErase(hDC,mp);
//			mp->x+=mp->speed.x;
			}
		}

	if(myMostri[0].speed.x>0) {		// tanto la velocità è uguale x tutti!
		if(getRightmostMostro() > (AppXSizeR-13*doubleSize)) {
			for(i=0; i<55; i++) {
				mp=&myMostri[i];
	//bah			if(mp->bVis) {
					mp->speed.x=-mp->speed.x;
					mp->y.whole +=ALIEN_Y_SIZE+0;
	//				}
				}
			}
		}
	else {
		if(getLeftmostMostro() < 12*doubleSize) {
			for(i=0; i<55; i++) {
				mp=&myMostri[i];
	//bah			if(mp->bVis) {
					mp->speed.x=-mp->speed.x;
					mp->y.whole +=ALIEN_Y_SIZE+0;
	//				}
				}
			}
		}

#pragma warning MOSTRI contro CASE fare!

	for(i=0; i<55; i++) {
		mp=&myMostri[i];
		if(mp->bVis) {
		//				i=DrawIcon(hDC,xPos,100,hNave1);
			SelectObject(hCompDC,mostriMovePhase & 1 ? mp->hImgAlt : mp->hImg);
			mp->x.whole +=mp->speed.x;
			MobDraw(hDC,mp);
			}
//		if(!j)
//			MessageBeep(-1);
		}
#pragma warning EASTER EGG ultimo mostro ultima riga 500 pts fare!


	ReleaseDC(hWnd,hDC);

	switch(mostriMovePhase) {
		case 0:
			if(bSuoni)
				PlayResource(MAKEINTRESOURCE(IDR_WAVE_STEP1));
			break;
		case 1:
			if(bSuoni)
				PlayResource(MAKEINTRESOURCE(IDR_WAVE_STEP2));
			break;
		case 2:
			if(bSuoni)
				PlayResource(MAKEINTRESOURCE(IDR_WAVE_STEP3));
			break;
		case 3:
			if(bSuoni)
				PlayResource(MAKEINTRESOURCE(IDR_WAVE_STEP4));
			break;
		}
	mostriMovePhase++;
	mostriMovePhase &= 3;

	if(getBottomestMostro() > AppYSizeR-LOWER_AREA*doubleSize)			//INVASIONE!
		return 1;
	else
		return 0;
  }

BYTE isMissileInArea(struct MOB *mp) {		// v. MobCollisionPoint
	int x=myMob[1].x.whole+myMob[1].s.cx/2;
	int y=myMob[1].y.whole;			// centro, alto

	if(x>=(mp->x.whole) && x<(mp->x.whole+mp->s.cx) && 
		y>=(mp->y.whole) && y<(mp->y.whole+mp->s.cy)) {
		return 1;
		}
	return 0;
	}

BYTE isBombaInArea(struct MOB *m1,struct MOB *mp) {		// v. MobCollisionPoint
	int x=m1->x.whole+m1->s.cx/2;
	int y=m1->y.whole+m1->s.cy;			// centro, basso

	if(x>=(mp->x.whole) && x<(mp->x.whole+mp->s.cx) && 
		y>=(mp->y.whole) && y<(mp->y.whole+mp->s.cy)) {
		return 1;
		}
	return 0;
	}

int getNumBombe(void) {
	int i,n=0;

	for(i=10; i<10+maxBombeNow; i++) {			// 
		if(myMob[i].bVis)
			n++;
		}

	return n;
	}

int getSuitableBomb(int m,POINT *pos) {		// 0 se random o 1 se mirata a astronave
	int i,f;
	int t;

	f=0;
	if(m) {
		for(i=0; i<11; i++) {			// 
			if(myMostri[i].bVis) {			// almeno un mostro ci dev'essere sulla colonna!
				if(abs(myMostri[i].x.whole-myMob[0].x.whole) < 15*doubleSize)	{		// preferibilmente sparo sull'astronave...
					f=i+1;
					break;
					}
				}
			}
		}

	t=0;
	do {
		if(!f)
			f=(rand() % 11)+1;

		f--;
		for(i=44+f; i>0; i-=11) {			//	ora cerco il primo mostro della colonna scelta...
			if(myMostri[i].bVis) {
found:
				pos->x=myMostri[i].x.whole;
				pos->y=myMostri[i].y.whole;
				return 1;
				}
			}
		t++;
		} while(t<3);

	for(i=0; i<55; i++) {			// se non ne ho trovato nessuno dopo 3 tentativi, pesco il primo disponibile!
		if(myMostri[i].bVis) {
			goto found;
			}
		}

	return 0;
	}

BYTE isMissileInCasa(struct MOB *mp) {
	int x=myMob[1].x.whole+myMob[1].s.cx/2;
	int y=myMob[1].y.whole;			// centro, alto
//	BYTE buf[22/2 /* 4bpp */ * 16],c;		// sempre dimensioni originali qua!
	DWORD buf[22*16],c;		// 32bpp
	int x2,y2;

	if(x>=(mp->x.whole) && x<(mp->x.whole+mp->s.cx) && 
		y>=(mp->y.whole) && y<(mp->y.whole+mp->s.cy)) {

		int i=GetBitmapBits(mp->hImgAlt,sizeof(buf),buf);		// questa restituisce 32bpp, vecchia storia...
//		i=GetObject(mp->hImgAlt,sizeof(buf),buf);		// solo proprietà bitmap...
		x2=x-mp->x.whole; x2/=doubleSize;
		y2=y-mp->y.whole; y2/=doubleSize;
		c=buf[y2*mp->s.cx/doubleSize+x2];
		if(y2>0) {
			y2--;
			c|=buf[y2*mp->s.cx/doubleSize+x2];
			if(x2<(mp->s.cx/doubleSize-2))
				c|=buf[y2*mp->s.cx/doubleSize+x2+1];
			if(x2>0)
				c|=buf[y2*mp->s.cx/doubleSize+x2-1];
			}
		if(c)
			return 1;
		}
	return 0;
	}

BYTE isBombaInCasa(struct MOB *m1,struct MOB *mp) {
	int x=m1->x.whole+m1->s.cx/2;
	int y=m1->y.whole+m1->s.cy;			// centro, basso
//	BYTE buf[22/2 /* 4bpp */ * 16],c;		// sempre dimensioni originali qua!
	DWORD buf[22*16],c;		// 32bpp
	int x2,y2;

	if(x>=(mp->x.whole) && x<(mp->x.whole+mp->s.cx) && 
		y>=(mp->y.whole) && y<(mp->y.whole+mp->s.cy)) {

		int i=GetBitmapBits(mp->hImgAlt,sizeof(buf),buf);		// questa restituisce 32bpp, vecchia storia...
		x2=x-mp->x.whole; x2/=doubleSize;
		y2=y-mp->y.whole; y2/=doubleSize;
		c=buf[y2*mp->s.cx/doubleSize+x2];
		if(y2<(mp->s.cy/doubleSize-1)) {
			y2++;
			c|=buf[y2*mp->s.cx/doubleSize+x2];
			if(x2<(mp->s.cx/doubleSize-2))
				c|=buf[y2*mp->s.cx/doubleSize+x2+1];
			if(x2>0)
				c|=buf[y2*mp->s.cx/doubleSize+x2-1];
			}
		if(c)
			return 1;
		}
	return 0;
	}

BYTE hitCasa(struct MOB *m1,struct MOB *mp,int daDove) {
	int x=mp->x.whole+mp->s.cx/2;
	int y=mp->y.whole+ (daDove ? mp->s.cy : 0);			// centro, alto o basso
//	BYTE buf[22/2 /* 4bpp */ * 16],c;		// sempre dimensioni originali qua!
	DWORD buf[22*16],c;		// 32bpp
	int x2,y2,x3;

	GetBitmapBits(m1->hImgAlt,sizeof(buf),buf);

	x2=x-m1->x.whole; x2/=doubleSize;
	y2=y-m1->y.whole; y2/=doubleSize;
	if(!daDove) {
		if(y2<(m1->s.cy/doubleSize-1))
			y2+=1;
		}
	else {
		if(y2>=1)
			y2-=1;
		}
	if(x2<(m1->s.cx/doubleSize-2))
		x3=x2+1;
	else
		x3=x2;
	if(x2>0)
		x2--;
	for(y=0; y<4; y++) {
		for(x=x2; x<=x3; x++) {
//			c=buf[y2*m1->s.cx/doubleSize+x2+x];
			buf[y2*m1->s.cx/doubleSize+x] &= (rand() & 3) ? RGB(0,0,0) : RGB(0,255,0);
			}
		if(daDove) {
			if(y2<(m1->s.cy-2))
				y2++;
			else
				break;
			}
		else {
			if(y2>0)
				y2--;
			else 
				break;
			}
		}

	SetBitmapBits(m1->hImgAlt,sizeof(buf),buf);
	return 0;
	}


