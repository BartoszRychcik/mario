#include<math.h>
#include<stdio.h>
#include<string.h>
#include<windows.h>
#ifdef __cplusplus
	extern "C"
#endif

extern "C" 
	{
	#include"./sdl/include/SDL.h"
	#include"./sdl/include/SDL_main.h"
	}
//****************************************STALE****************************************//
#define _USE_MATH_DEFINES
#define SCREEN_WIDTH	720
#define SCREEN_HEIGHT	480
#define MUREK_X 670
#define MUREK_Y 464
#define marioS 26
#define marioH 32
#define elementy 44
const int speed_mario = 170;
const int speed_enemy = 100;
//*********************************STRUKTURA_ZAPISU************************************//
struct game
	{
	int save_poziom;
	bool save_blokuj;
	bool save_blokuj2;
	bool save_pierwszy;
	bool save_spadaj;
	bool save_start;
	int save_LICZBA_ZYC;
	int save_distanceLR;
	int save_distanceUD;
	int save_przesuniecieP;
	double save_worldTime;
	double save_fpsTimer;
	double save_fps;
	int save_moveMARIOLR;
	int save_moveMARIOUD;
	bool save_prawo;
	bool save_skok;
	bool save_zderzenie;
	bool *save_zwrot;
	double *save_ruch;
	bool *save_stukanie;
	int save_monety;
	};
//*********************************STRUKTURA_PRZESZKOD*********************************//
struct klocki
	{
	int Xs, Ys, Xk, Yk;
	char type;
	};
//*********************************STRUKTURA_PRZECIWNIKOW******************************//
struct przeciwnik
	{
	klocki stworek;
	int XLD, XPD;
	bool zwrot;
	double ruch;
	};
//********************************STRUKTURA_GRAFIK*************************************//
struct grafika
	{
	SDL_Surface *obrazek;
	char filename[30]="\0";
	};
//*******************************STRUKTURA_ETAPU***************************************//
struct etap
	{
	int MARIO_X;
	int MARIO_Y;
	int TIME;
	int szerokosc_planszy;
	int wysokosc_planszy;
	int liczba_przeszkod;
	int liczba_przeciwnikow;
	};
//*************************************DEKLARACJE**************************************//
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset);  //narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y), charset to bitmapa 128x128 zawieraj¹ca znaki
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor); // rysowanie prostok¹ta o d³ugoœci boków l i k
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color); // rysowanie pojedynczego pixela
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color); // rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) b¹dŸ poziomie (gdy dx = 1, dy = 0)
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y); // narysowanie na ekranie screen powierzchni sprite w punkcie (x, y).(x, y) to punkt œrodka obrazka sprite na ekranie
void zwolnienie_powierzchni(SDL_Surface*,grafika*,SDL_Texture*,SDL_Renderer*,SDL_Window*);
void FPSY(double*,double*,int*,double);
void info_na_ekran(SDL_Surface*,SDL_Surface*,double,int,char*,etap,int,int);
void przesuniecie_ekranu(int,int*,etap);
void pokaz_przeszkody(SDL_Surface*, klocki*,int, grafika*,etap);
klocki *wczytaj_przeszkody(etap,FILE*);
przeciwnik *wczytaj_przeciwnikow(etap,FILE*);
grafika *wczytaj_obrazki(SDL_Surface*,SDL_Texture*,SDL_Renderer*,SDL_Window*);
void przesuniecie_postaci(int*,int*,int,int*,double,double,double,klocki*,bool*,bool,etap,bool,bool*,bool,int*,bool*);
void sklej(char*,char*);
bool koniec_etapu(int,int,etap,klocki*);
void wyczysc_nazwe(char*);
bool spadek_pod_mape(int,int,etap,klocki*);
void zapisz_gre(int,bool,bool,bool,bool,bool,int,int,int,int,double,double,double,game*,bool,bool,bool,int);
void wczytaj_gre(int*,bool*,bool*,bool*,bool*,bool*,int*,int*,int*,int*,double*,double*,double*,game,bool*,bool*,bool*,int*);
void pokaz_mario(SDL_Surface*,grafika*,etap,int,int,int,bool,bool,int,double);
void pokaz_przeciwnikow(SDL_Surface*,przeciwnik**,etap,grafika*,int,double,double);
bool zderzenie_z_przeciwnikiem(int,int,przeciwnik*,etap,int);
void zapisz_stadium_przeciwnikow(etap,przeciwnik*,game*);
void wczytaj_stadium_przeciwnikow(etap, przeciwnik**,game);
void zapisz_stukanie(etap, klocki*, game*);
void wczytaj_stukanie(etap, klocki**, game);
//***************************************MAIN*****************************************//
int main(int argc, char **argv) 
	{
	int t1, t2, t3 = 0, t4 = 0, t5 = 0, t6 = 0, t7 = 0, quit = 0, frames = 0, rc, LICZBA_ZYC = 3, moveMARIOLR = 0, moveMARIOUD = 0, poziom = 1, przesuniecieP = 0, distanceLR = 0, distanceUD = 0, MONETY = 0, X_monety=0, Y_monety=0,przesunieciemonety=0;
	double delta = 0, worldTime = 0, fpsTimer = 0, fps = 0, czasskoku = 0, czasspadku = 0, animacja = 0,czasanimacji = 0, czasdlamonety= 0;
	char text[128], filename[20]="\0";
	bool blokuj = false, blokuj2, start = true, pierwszy = false, spadaj = false, prawo = true, skok = false, zderzenie=false, monetka=false, animacjamonety=false;
	SDL_Event event;
	SDL_Surface *screen,*menu;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	grafika*obrazki=NULL; 
	klocki*przeszkody=NULL;
	przeciwnik *wrogowie = NULL;
	etap stage;
	static game save;
	FILE *fp;
	if(SDL_Init(SDL_INIT_EVERYTHING)!= 0){printf("SDL_Init error: %s\n", SDL_GetError());return 1;}
	//rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,&window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	if(rc != 0){SDL_Quit();printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());return 1;};
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0,0,0,255);
	SDL_SetWindowTitle(window,"SUPER MARIO BROS");
	SDL_ShowCursor(SDL_DISABLE); 
	screen=SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex=SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,SCREEN_WIDTH, SCREEN_HEIGHT);
	sklej(filename, "etapy/etap1.txt");
	if (fp = fopen(filename, "r"))
	{
		fscanf(fp, "%d %d %d", &stage.wysokosc_planszy, &stage.szerokosc_planszy, &stage.TIME);
		fscanf(fp, "%d %d %d", &stage.MARIO_X, &stage.MARIO_Y, &stage.liczba_przeszkod);
		obrazki = wczytaj_obrazki(screen, scrtex, renderer, window);
		przeszkody = wczytaj_przeszkody(stage, fp);
		fscanf(fp, "%d", &stage.liczba_przeciwnikow);
		wrogowie = wczytaj_przeciwnikow(stage, fp);
		fclose(fp);
	}
	else
	{
		printf("Blad ladowania pliku: %s\n", SDL_GetError());
		SDL_Quit();
		zwolnienie_powierzchni(screen, NULL, scrtex, renderer, window);
		return 0;
	}

	int kolor_tla = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	SDL_SetColorKey(obrazki[0].obrazek, true, 0x00);
	for (int i = 1; i < elementy; i++)
	{
		if (i != 26)
			SDL_SetColorKey(obrazki[i].obrazek, true, 0x1111CC);
	}
	t1 = SDL_GetTicks();

	while(!quit) 
		{
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		czasskoku = (t2 - t3) * 0.001;
		worldTime += delta;

		przesuniecie_postaci(&distanceLR, &distanceUD, moveMARIOLR, &moveMARIOUD, delta, worldTime, czasskoku, przeszkody, &blokuj2, blokuj,stage,spadaj,&skok,zderzenie,&MONETY,&monetka);
		przesuniecie_ekranu(distanceLR, &przesuniecieP,stage);

		if (spadek_pod_mape(stage.MARIO_X + distanceLR, stage.MARIO_Y + distanceUD + marioH/2, stage, przeszkody)&& !spadaj)
			{
			skok = true;
			t4 = SDL_GetTicks();
			moveMARIOUD = 2 * speed_mario;
			moveMARIOLR = 0;
			spadaj = true;
			}

		if(spadaj) //tryb spadania mario pod mape jest wlaczony
			{
			czasspadku = (t2 - t4)* 0.001;
			if(czasspadku>0.6)
				{
				spadaj = false;
				t1 = SDL_GetTicks();
				LICZBA_ZYC--;
				worldTime = fpsTimer = fps = moveMARIOLR = moveMARIOUD = 0;
				distanceLR = distanceUD = 0;
				przesuniecieP = 0;
				}
			}

		if (monetka)
			{
			t7 = SDL_GetTicks();
			animacjamonety = true;
			X_monety = stage.MARIO_X + distanceLR;
			Y_monety = stage.MARIO_Y + distanceUD-marioH;
			monetka = false;
			}

		if (koniec_etapu(stage.MARIO_X + distanceLR, stage.MARIO_Y + distanceUD, stage, przeszkody)||pierwszy)
			{
			pierwszy = false;
			wyczysc_nazwe(filename);
			sklej(filename, "etapy/etap");
			poziom++;
			char tab[2]="\0";
			wyczysc_nazwe(tab);
			tab[0] = poziom + '0';
			sklej(filename, tab);
			sklej(filename, ".txt");
			if (fp = fopen(filename, "r"))
				{
				fscanf(fp, "%d %d %d", &stage.wysokosc_planszy, &stage.szerokosc_planszy, &stage.TIME);
				fscanf(fp, "%d %d %d", &stage.MARIO_X, &stage.MARIO_Y, &stage.liczba_przeszkod);
				przeszkody = wczytaj_przeszkody(stage, fp);
				fscanf(fp, "%d", &stage.liczba_przeciwnikow);
				wrogowie = wczytaj_przeciwnikow(stage, fp);
				t1 = SDL_GetTicks();
				worldTime = fpsTimer = fps = moveMARIOLR = moveMARIOUD = 0;
				distanceLR = distanceUD = 0;
				przesuniecieP = 0;
				fclose(fp);
				}
			else
				start = true;
			}

		SDL_FillRect(screen, NULL, kolor_tla);
		DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, kolor_tla);
		DrawRectangle(screen, 1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2, NULL, kolor_tla);

		if (start) //jesli zaczynamy gre lub przeszlismy wszystkie mapki uruchamia siê tryb start
			{
			menu = SDL_LoadBMP("./grafiki/menu.bmp");
			SDL_FillRect(screen, NULL, czarny);
			DrawSurface(screen, menu, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
			sprintf(text, "Esc - wyjscie, n - nowa gra");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, obrazki[0].obrazek);
			}

		if (zderzenie_z_przeciwnikiem(stage.MARIO_X + distanceLR + przesuniecieP, stage.MARIO_Y + distanceUD, wrogowie, stage, przesuniecieP) && !zderzenie)
			{
			zderzenie = true;
			t6 = SDL_GetTicks();
			}

		animacja = (t2 - t5) * 0.001;
		if (animacja>0.45)t5 = SDL_GetTicks();

		if(LICZBA_ZYC>0&&!start)
			{
			FPSY(&fpsTimer, &fps, &frames, delta);
			info_na_ekran(screen, obrazki[0].obrazek, worldTime, LICZBA_ZYC, text,stage,poziom,MONETY);
			int zebymurkaniezabraklo = przesuniecieP%510; //zeby obrazek nie przesunal siê za swoj koniec
			DrawSurface(screen, obrazki[2].obrazek, MUREK_X + zebymurkaniezabraklo, MUREK_Y);
			pokaz_przeszkody(screen, przeszkody, przesuniecieP, obrazki,stage);
			pokaz_przeciwnikow(screen,&wrogowie,stage,obrazki,przesuniecieP,delta,animacja);
			if (stage.TIME - worldTime <= -2)
				{
				t1 = SDL_GetTicks();
				LICZBA_ZYC--;
				worldTime = fpsTimer = fps = moveMARIOLR = moveMARIOUD = 0;
				distanceLR = distanceUD = 0;
				przesuniecieP = 0;
				}
			else if (stage.TIME - worldTime < -1 && stage.TIME - worldTime > -2)
				{
				moveMARIOUD = 2*speed_mario;
				DrawSurface(screen, obrazki[4].obrazek, stage.MARIO_X + distanceLR + przesuniecieP, stage.MARIO_Y + distanceUD);
				}
			else if (stage.TIME - worldTime <= 0 && stage.TIME - worldTime >= -1)
				{
				distanceUD -= 7;
				DrawSurface(screen, obrazki[4].obrazek, stage.MARIO_X + distanceLR + przesuniecieP, stage.MARIO_Y + distanceUD);
				blokuj = true;
				}
			else if(!zderzenie)
				{ 
				pokaz_mario(screen, obrazki, stage, distanceLR, przesuniecieP, distanceUD, prawo,skok, moveMARIOLR, animacja);
				blokuj = false;
				}
			}
		else if(!start)
			{
			SDL_FillRect(screen, NULL, czarny);
			sprintf(text, "GAME OVER");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2, text, obrazki[0].obrazek);
			sprintf(text, "Esc - wyjscie, n - nowa gra");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, obrazki[0].obrazek);
			}

		if (animacjamonety)
			{
			czasdlamonety = (t2 - t7)* 0.001;
			if (czasdlamonety < 0.2)
				{
				przesunieciemonety -= 1;
				DrawSurface(screen, obrazki[43].obrazek,X_monety+przesuniecieP,Y_monety + przesunieciemonety);
				}
			else if (Y_monety+przesunieciemonety<SCREEN_HEIGHT)
				{
				przesunieciemonety += 15;
				DrawSurface(screen, obrazki[43].obrazek, X_monety + przesuniecieP, Y_monety + przesunieciemonety);
				}
			else
				{
				przesunieciemonety = 0;
				animacjamonety = false;
				}
			}

		if (zderzenie)
			{
			moveMARIOUD = 0;
			moveMARIOLR = 0;
			blokuj = true;
			czasanimacji = (t2 - t6)* 0.001;
			if (czasanimacji < 1)
				{
				distanceUD -= 1;
				DrawSurface(screen, obrazki[4].obrazek, stage.MARIO_X + distanceLR + przesuniecieP, stage.MARIO_Y + distanceUD);
				}
			else if (czasanimacji < 2)
				{
				moveMARIOUD = 2 * speed_mario;
				DrawSurface(screen, obrazki[4].obrazek, stage.MARIO_X + distanceLR + przesuniecieP, stage.MARIO_Y + distanceUD);
				}
			else if (czasanimacji > 2 )
				{
				zderzenie = false;
				t1 = SDL_GetTicks();
				LICZBA_ZYC--;
				worldTime = fpsTimer = fps = moveMARIOLR = moveMARIOUD = 0;
				distanceLR = distanceUD = 0;
				przesuniecieP = 0;
				}
			}

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		while(SDL_PollEvent(&event)) 
			{
			switch(event.type) 
				{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					if (event.key.keysym.sym == SDLK_UP) if ((distanceUD >= 0 && !spadaj) || (!blokuj2 && !spadaj)) { t3 = SDL_GetTicks();moveMARIOUD = -2 * speed_mario; skok = true; }
					if (event.key.keysym.sym == SDLK_RIGHT) if (!blokuj && !spadaj) { moveMARIOLR = speed_mario; if(!prawo)t5 = SDL_GetTicks(); prawo = true;}
					if (event.key.keysym.sym == SDLK_LEFT) if (!blokuj && !spadaj) { moveMARIOLR = -speed_mario; if(prawo)t5 = SDL_GetTicks(); prawo = false;}
					if (event.key.keysym.sym == SDLK_n)
						{
						if (start)
							{
							start = false;
							SDL_FreeSurface(menu);
							}
						MONETY = 0;
						poziom = 0; 
						prawo = true;
						pierwszy = true;
						zderzenie = false;
						t1=SDL_GetTicks(); 
						LICZBA_ZYC = 3;
						worldTime = fpsTimer = fps = moveMARIOLR = moveMARIOUD = 0;
						distanceLR = distanceUD = 0;
						przesuniecieP = 0;
						}
					else if (event.key.keysym.sym == SDLK_s)
						{
						zapisz_gre(poziom, blokuj, blokuj2, pierwszy, spadaj, start, LICZBA_ZYC, distanceLR, distanceUD, przesuniecieP, worldTime, fpsTimer, fps, &save, prawo, skok, zderzenie,MONETY);
						free(save.save_zwrot);
						free(save.save_ruch);
						free(save.save_stukanie);
						zapisz_stadium_przeciwnikow(stage,wrogowie,&save);
						zapisz_stukanie(stage, przeszkody, &save);
						}	
					else if (event.key.keysym.sym == SDLK_l)
						{
						if((save.save_LICZBA_ZYC))
							{
							wczytaj_gre(&poziom, &blokuj, &blokuj2, &pierwszy, &spadaj, &start, &LICZBA_ZYC, &distanceLR, &distanceUD, &przesuniecieP, &worldTime, &fpsTimer, &fps, save, &prawo, &skok, &zderzenie, &MONETY);
							wyczysc_nazwe(filename);
							sklej(filename, "etapy/etap");
							char tab[2] = "\0";
							wyczysc_nazwe(tab);
							tab[0] = poziom + '0';
							sklej(filename, tab);
							sklej(filename, ".txt");
							if (fp = fopen(filename, "r"))
								{
								fscanf(fp, "%d %d %d", &stage.wysokosc_planszy, &stage.szerokosc_planszy, &stage.TIME);
								fscanf(fp, "%d %d %d", &stage.MARIO_X, &stage.MARIO_Y, &stage.liczba_przeszkod);
								przeszkody = wczytaj_przeszkody(stage, fp);
								fscanf(fp, "%d", &stage.liczba_przeciwnikow);
								wrogowie = wczytaj_przeciwnikow(stage, fp);
								}
							wczytaj_stadium_przeciwnikow(stage, &wrogowie, save);
							wczytaj_stukanie(stage, &przeszkody, save);
							}
						}
					break;
				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_LEFT) moveMARIOLR = 0;
					if (event.key.keysym.sym == SDLK_UP) { moveMARIOUD = 2 * speed_mario;}
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		}

	zwolnienie_powierzchni(screen, obrazki,scrtex,renderer,window);
	free(przeszkody);
	free(obrazki);
	free(wrogowie);
	free(save.save_zwrot);
	free(save.save_ruch);
	free(save.save_stukanie);
	return 0;
	};
	
//*************************************DEFINICJE**************************************//
bool zderzenie_z_przeciwnikiem(int x,int y,przeciwnik *wrogowie, etap stage,int przesuniecieP)
	{
	for(int i =0;i<stage.liczba_przeciwnikow;i++)
		{
		int marioXs = x - marioS / 2;
		int marioXk = x + marioS / 2;
		int marioYs = y + marioH / 2;
		int marioYk = y - marioH / 2;
		int pXs = wrogowie[i].stworek.Xs + wrogowie[i].ruch + przesuniecieP;
		int pXk = wrogowie[i].stworek.Xk + wrogowie[i].ruch + przesuniecieP;
		int pYs = wrogowie[i].stworek.Ys;
		int pYk = wrogowie[i].stworek.Yk;
		if ((marioXk > pXs && marioXs < pXs && marioYs>=pYk)|| (marioXs <pXk && marioXk>pXk && marioYs>=pYk))
			return true;
		}
	return false;
	}

void pokaz_przeciwnikow(SDL_Surface *screen, przeciwnik **wrogowie, etap stage, grafika *obrazki, int przesuniecie,double delta,double animacja)
	{
		int numerek=0;
		for (int i = 0; i < stage.liczba_przeciwnikow; i++)
			{
			if(!(*wrogowie)[i].zwrot)
				(*wrogowie)[i].ruch += delta*speed_enemy;
			else
				(*wrogowie)[i].ruch -= delta*speed_enemy;

			if ((*wrogowie)[i].ruch + (*wrogowie)[i].stworek.Xk > (*wrogowie)[i].XPD)
				{
				(*wrogowie)[i].ruch = (*wrogowie)[i].XPD - (*wrogowie)[i].stworek.Xk;
				(*wrogowie)[i].zwrot = true;
				}
			else if ((*wrogowie)[i].ruch + (*wrogowie)[i].stworek.Xs < (*wrogowie)[i].XLD)
				{
				(*wrogowie)[i].ruch = (*wrogowie)[i].XLD - (*wrogowie)[i].stworek.Xs;
				(*wrogowie)[i].zwrot = false;
				}
		
			if ((*wrogowie)[i].stworek.type == 'a')
				numerek = 36;
			else
			if ((*wrogowie)[i].stworek.type == 'b'&& !(*wrogowie)[i].zwrot)
				numerek = 40;
			else 
				numerek = 38;

		if(animacja<0.2)
			DrawSurface(screen, obrazki[numerek].obrazek, ((*wrogowie)[i].stworek.Xk + (*wrogowie)[i].stworek.Xs) / 2 + przesuniecie + (*wrogowie)[i].ruch, ((*wrogowie)[i].stworek.Yk + (*wrogowie)[i].stworek.Ys) / 2);
		else
			DrawSurface(screen, obrazki[numerek+1].obrazek, ((*wrogowie)[i].stworek.Xk + (*wrogowie)[i].stworek.Xs) / 2 + przesuniecie + (*wrogowie)[i].ruch, ((*wrogowie)[i].stworek.Yk + (*wrogowie)[i].stworek.Ys) / 2);
			}
	}

void pokaz_mario(SDL_Surface *screen,grafika *obrazki,etap stage,int distanceLR,int przesuniecieP,int distanceUD,bool prawo, bool skok,int moveMARIOLR, double animacja)
	{
	int numerek = 0;
	if (prawo&&skok)
		numerek = 33; //marioskaczeprawo
	else if (prawo && !skok)
		numerek = 1; //mariostoiprawo
	else if (!prawo && skok)
		numerek = 32; //marioskaczelewo
	else if (!prawo && !skok)

		numerek = 27; //mariostoilewo
	if (moveMARIOLR > 0 && !skok && animacja<0.15)
		numerek = 29; //marioidzielewo
	else if (moveMARIOLR < 0 && !skok && animacja<0.15)
		numerek = 28; //marioidzieprawo
	else if (moveMARIOLR > 0 && !skok && animacja<0.3)
		numerek = 31; //marioidzielewo2
	else if (moveMARIOLR < 0 && !skok && animacja<0.3)
		numerek = 30; //marioidzieprawo2
	else if (moveMARIOLR > 0 && !skok && animacja<0.45)
		numerek = 35; //marioidzielewo3
	else if (moveMARIOLR < 0 && !skok && animacja<0.45)
		numerek = 34; //marioidzieprawo3

	DrawSurface(screen, obrazki[numerek].obrazek, stage.MARIO_X + distanceLR + przesuniecieP, stage.MARIO_Y + distanceUD);
	}

void wczytaj_stukanie(etap stage, klocki **przeszkody, game save)
{
	for (int i = 0; i < stage.liczba_przeszkod; i++)
		if (save.save_stukanie[i]==true)
			(*przeszkody)[i].type = 's';
}

void wczytaj_stadium_przeciwnikow(etap stage, przeciwnik **wrogowie, game save)
{
	for (int i = 0; i < stage.liczba_przeciwnikow; i++)
		{
		(*wrogowie)[i].zwrot = save.save_zwrot[i];
		(*wrogowie)[i].ruch = save.save_ruch[i];
		}
}

void wczytaj_gre(int *poziom, bool *blokuj, bool *blokuj2, bool *pierwszy, bool *spadaj, bool *start, int *LICZBA_ZYC, int *distanceLR, int *distanceUD, int *przesuniecieP, double *worldTime, double *fpsTimer, double *fps, game save, bool *prawo, bool *skok,bool *zderzenie,int *monety)
	{
	*poziom = save.save_poziom;
	*blokuj = save.save_blokuj;
	*blokuj2 = save.save_blokuj2;
	*pierwszy = save.save_pierwszy;
	*spadaj = save.save_spadaj;
	*start = save.save_start;
	*LICZBA_ZYC = save.save_LICZBA_ZYC;
	*distanceLR = save.save_distanceLR;
	*distanceUD = save.save_distanceUD;
	*przesuniecieP = save.save_przesuniecieP;
	*worldTime = save.save_worldTime;
	*fpsTimer = save.save_fpsTimer;
	*fps = save.save_fps;
	*prawo = save.save_prawo;
	*skok = save.save_skok;
	*zderzenie = save.save_zderzenie;
	*monety = save.save_monety;
	}

void zapisz_stukanie(etap stage, klocki *przeszkody, game *save)
	{

	(*save).save_stukanie = (bool*)malloc(stage.liczba_przeszkod * sizeof(*(*save).save_stukanie));

	for (int i = 0; i <stage.liczba_przeszkod ; i++)
		{
		if (przeszkody[i].type == 'q')
			(*save).save_stukanie[i] = false;
		else if(przeszkody[i].type =='s')
			(*save).save_stukanie[i] = true;
		else
			(*save).save_stukanie[i] = false;
		}
	}

void zapisz_stadium_przeciwnikow(etap stage, przeciwnik *wrogowie, game*save)
{
	(*save).save_zwrot = (bool*)malloc(stage.liczba_przeciwnikow * sizeof(*(*save).save_zwrot));
	(*save).save_ruch = (double*)malloc(stage.liczba_przeciwnikow * sizeof(*(*save).save_ruch));

	for (int i = 0; i < stage.liczba_przeciwnikow; i++)
		{
		(*save).save_zwrot[i] = wrogowie[i].zwrot;
		(*save).save_ruch[i] = wrogowie[i].ruch;
		}
}

void zapisz_gre(int poziom, bool blokuj, bool blokuj2, bool pierwszy, bool spadaj, bool start, int LICZBA_ZYC, int distanceLR, int distanceUD, int przesuniecieP, double worldTime, double fpsTimer, double fps, game *save, bool prawo, bool skok,bool zderzenie,int monety)
	{
	save->save_poziom = poziom;
	(*save).save_blokuj = blokuj;
	(*save).save_blokuj2 = blokuj2;
	(*save).save_pierwszy = pierwszy;
	(*save).save_spadaj = spadaj;
	(*save).save_start = start;
	(*save).save_LICZBA_ZYC = LICZBA_ZYC;
	(*save).save_distanceLR = distanceLR;
	(*save).save_distanceUD = distanceUD;
	(*save).save_przesuniecieP = przesuniecieP;
	(*save).save_worldTime = worldTime;
	(*save).save_fpsTimer = fpsTimer;
	(*save).save_fps = fps;
	(*save).save_prawo = prawo;
	(*save).save_skok = skok;
	(*save).save_zderzenie = zderzenie;
	(*save).save_monety = monety;
	}

bool spadek_pod_mape(int x, int y, etap stage, klocki *przeszkody)
	{
	for (int i = 0; i < stage.liczba_przeszkod; i++)
		if (przeszkody[i].type == 'x') //type x to klocek reprezentujacy nieciaglosc mapy
			if (x> przeszkody[i].Xs && x < przeszkody[i].Xk && y >= przeszkody[i].Yk-5)
				return true;
	return false;
	}

void wyczysc_nazwe(char *a)
	{
		int i = 0;
		while (a[i] != '\0')
		{
			a[i] = '\0';
			i++;
		}
	}

bool koniec_etapu(int x, int y, etap stage, klocki *przeszkody)
	{
	for (int i = 0; i < stage.liczba_przeszkod; i++)
		if (przeszkody[i].type == 'c' || przeszkody[i].type == 'z') //type c lub z to klocek ktory jest wejsciem do zamku
			if ( x> przeszkody[i].Xs && x < przeszkody[i].Xk && y < przeszkody[i].Ys && y> przeszkody[i].Yk)
				return true;
	return false;
	}

void przesuniecie_postaci(int *distanceLR,int *distanceUD,int moveMARIOLR,int *moveMARIOUD,double delta,double worldTime,double czasskoku,klocki *przeszkody,bool *blokuj2, bool blokuj, etap stage,bool spadaj, bool *skok,bool zderzenie,int *monety,bool *monetka)
	{
	*blokuj2 = true;
	int copyUD = *distanceUD;
	int copyLR = *distanceLR;
	if(!blokuj)
		{
		if ((*distanceLR + moveMARIOLR*delta)>1)
			{
			if (*distanceLR + stage.MARIO_X<stage.szerokosc_planszy - marioS / 2 - 10)
				*distanceLR += moveMARIOLR * delta;
			else
				if (moveMARIOLR<0)
					*distanceLR +=moveMARIOLR * delta;
			}
		}
	if (czasskoku<0.32 && *moveMARIOUD < 0) *distanceUD += *moveMARIOUD * delta;
	if (*moveMARIOUD > 0) *distanceUD += *moveMARIOUD * delta;
	if (czasskoku > 0.4) { *moveMARIOUD = 2 * speed_mario; }
	if (*distanceUD > 0 && stage.TIME - worldTime > 0 && !spadaj && !zderzenie) { *distanceUD = 0; *skok = false; }

	//podwojne zbadanie zderzen dla przypadkow przesuniec jednoczesnych w LR i UD. \\Solve Bug 1.
	for(int i=0;i<2;i++)
		{
		int Xsmario = *distanceLR + stage.MARIO_X - marioS / 2;
		int Xkmario = *distanceLR + stage.MARIO_X + marioS / 2;
		int Ykmario = stage.MARIO_Y + *distanceUD - marioH / 2;
		int Ysmario = stage.MARIO_Y + *distanceUD + marioH / 2;
		int copydistanceUD = *distanceUD;
		int copydistanceLR = *distanceLR;

		if(stage.TIME-worldTime>0&&!zderzenie)
			{
			for (int i = 0; i < stage.liczba_przeszkod; i++)
				{
				if(przeszkody[i].type=='p'|| przeszkody[i].type == 't'|| przeszkody[i].type == 'u'|| przeszkody[i].type == 'v'|| przeszkody[i].type == 'q' || przeszkody[i].type == 'y') //type p,t,u,v,q,y to klocki ktore sa przeszkodami, reszta to tekstury
					{
					if ((Xkmario > przeszkody[i].Xs && Xsmario < przeszkody[i].Xk && Ykmario<przeszkody[i].Ys)||(Xsmario < przeszkody[i].Xk && Xkmario > przeszkody[i].Xs && Ykmario<przeszkody[i].Ys))
						{
						if(copyUD + stage.MARIO_Y + marioH/2>przeszkody[i].Yk)
							{
							if(moveMARIOLR>0)
								*distanceLR = przeszkody[i].Xs - stage.MARIO_X - marioS / 2;
							else if(moveMARIOLR<0)
								*distanceLR = przeszkody[i].Xk - stage.MARIO_X + marioS / 2;
							*distanceUD = copydistanceUD;
						*blokuj2 = true;
							}
						else
							if (*distanceUD + stage.MARIO_Y + marioH / 2 > przeszkody[i].Yk)
								{
								*distanceUD = (-stage.MARIO_Y+przeszkody[i].Yk - marioH / 2);
								*blokuj2 = false;
								*skok = false;
								}
						}
					//gdy w poprzednim momencie mario jest pod klockiem a w obecnym przecina go gorna krawedzia \\Solve Bug 2.
					if (copyLR + stage.MARIO_X + marioS / 2 > przeszkody[i].Xs && copyLR + stage.MARIO_X - marioS / 2 < przeszkody[i].Xk && stage.MARIO_Y + copyUD - marioH / 2 >= przeszkody[i].Ys && stage.MARIO_Y + *distanceUD + marioH / 2 >= przeszkody[i].Ys)
						{
						if (Ykmario < przeszkody[i].Ys)
							{
							if (przeszkody[i].type == 'q') przeszkody[i].type = 's';
							else if (przeszkody[i].type == 'y') { przeszkody[i].type = 's'; (*monety)++; *monetka = true; }
							*distanceLR = copydistanceLR;
							*distanceUD = (-stage.MARIO_Y + przeszkody[i].Ys + marioH / 2);
							*moveMARIOUD = 2 * speed_mario;
							*blokuj2 = true;
							}
						}
					}
				}
			}
		}
	}

void przesuniecie_ekranu(int distanceLR, int *przesuniecieP, etap stage)
{
	double przesuniecieL = SCREEN_WIDTH / 1.5 - (stage.MARIO_X + distanceLR + *przesuniecieP);
	if (stage.MARIO_X + distanceLR > SCREEN_WIDTH / 1.5 && stage.MARIO_X + distanceLR < stage.szerokosc_planszy - (SCREEN_WIDTH - SCREEN_WIDTH / 1.5) && przesuniecieL < 0)
		*przesuniecieP = SCREEN_WIDTH / 1.5 - (stage.MARIO_X + distanceLR);
	else
		if (przesuniecieL>SCREEN_WIDTH / 1.5 - SCREEN_WIDTH / 4 && *przesuniecieP<0)
			*przesuniecieP += przesuniecieL - (SCREEN_WIDTH / 1.5 - SCREEN_WIDTH / 4);
}

przeciwnik *wczytaj_przeciwnikow(etap stage, FILE *fp)
	{
	przeciwnik *tablica = (przeciwnik*)malloc(stage.liczba_przeciwnikow * sizeof(*tablica));
	for (int i = 0; i < stage.liczba_przeciwnikow; i++)
		fscanf(fp, "%d %d %d %d %d %d %c", &tablica[i].stworek.Xs, &tablica[i].stworek.Xk, &tablica[i].stworek.Ys, &tablica[i].stworek.Yk, &tablica[i].XLD, &tablica[i].XPD, &tablica[i].stworek.type);
	return tablica;
	}

klocki *wczytaj_przeszkody(etap stage, FILE *fp)
	{
	klocki *tablica=(klocki*)malloc(stage.liczba_przeszkod*sizeof(*tablica));
	for (int i = 0; i < stage.liczba_przeszkod; i++)
		fscanf(fp,"%d %d %d %d %c", &tablica[i].Xs, &tablica[i].Xk, &tablica[i].Ys, &tablica[i].Yk, &tablica[i].type);
	return tablica;
	}

void pokaz_przeszkody(SDL_Surface *screen, klocki *przeszkody, int przesuniecieP,grafika *obrazki, etap stage)
	{
	int numerek;
	for (int i = 0; i < stage.liczba_przeszkod; i++)
		{
		switch (przeszkody[i].type)
			{
			case 'p':numerek = 3; break;
			case 'q':numerek = 5; break;
			case 'b':numerek = 5; break;
			case 'c':numerek = 6; break;
			case 'z':numerek = 7; break;
			case 'a':numerek = 8; break;
			case 'd':numerek = 9; break;
			case 'e':numerek = 10; break;
			case 'f':numerek = 11; break;
			case 'g':numerek = 12; break;
			case 'h':numerek = 13; break;
			case 'i':numerek = 14; break;
			case 'j':numerek = 15; break;
			case 'k':numerek = 16; break;
			case 'l':numerek = 17; break;
			case 'm':numerek = 18; break;
			case 'n':numerek = 19; break;
			case 'o':numerek = 20; break;
			case 's':numerek = 26; break;
			case 't':numerek = 22; break;
			case 'r':numerek = 21; break;
			case 'u':numerek = 23; break;
			case 'v':numerek = 24; break;
			case 'w':numerek = 25; break;
			case 'x':numerek = 26; break;
			case 'y':numerek = 42; break;
			}
		DrawSurface(screen, obrazki[numerek].obrazek, (przeszkody[i].Xk + przeszkody[i].Xs) / 2 + przesuniecieP, (przeszkody[i].Yk + przeszkody[i].Ys) / 2);
		}
	}

void info_na_ekran(SDL_Surface *screen, SDL_Surface *charset, double worldTime, int LICZBA_ZYC, char *text, etap stage,int poziom,int monety)
	{
	sprintf(text, "ZYCIA - %d", LICZBA_ZYC);
	DrawString(screen, 20, 10, text, charset);
	sprintf(text, "POZIOM - %d",poziom);
	DrawString(screen, 20, 36, text, charset);
	if (stage.TIME - worldTime > 0)
		sprintf(text, "CZAS - %.0lf", stage.TIME - worldTime);
	else
		sprintf(text, "CZAS - 0");
	DrawString(screen, SCREEN_WIDTH / 1.2, 10, text, charset);
	sprintf(text, "MONETY - %d", monety);
	DrawString(screen, SCREEN_WIDTH / 1.2, 36, text, charset);
	sprintf(text, "Esc - wyjscie, n - nowa gra, \030 skok, \032 lewo, \033 prawo");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	}

void FPSY(double *fpsTimer,double *fps,int *frames,double delta)
	{
	*fpsTimer += delta;
	if (*fpsTimer > 0.5) 
		{
		*fps = *frames * 2;
		*frames = 0;
		*fpsTimer -= 0.5;
		};
	}

void zwolnienie_powierzchni(SDL_Surface *screen,grafika *obrazki,SDL_Texture *scrtex,SDL_Renderer *renderer,SDL_Window *window)
	{
	for(int i=0;i<elementy;i++)
		SDL_FreeSurface(obrazki[i].obrazek);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	}

void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset)
	{
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) 
		{
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};

void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor)
	{
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) 
	{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};

void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) 
	{
	for (int i = 0; i < l; i++) 
		{
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};

void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y)
	{
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};

void sklej(char *a, char *b)
{
	int i = 0, k = 0;
	while (a[i] != '\0')i++;
	while (b[k] != '\0')
	{
		a[i] = b[k];
		i++;
		k++;
	}
	a[i] = '\0';
}

grafika *wczytaj_obrazki(SDL_Surface *screen, SDL_Texture *scrtex, SDL_Renderer *renderer, SDL_Window *window)
{
	grafika *tablica = (grafika*)malloc(elementy*sizeof(*tablica));
	tablica[0].obrazek = SDL_LoadBMP("./cs8x8.bmp"); //szablon
	sklej(tablica[0].filename, "charset");
	tablica[1].obrazek = SDL_LoadBMP("./grafiki/mariostoiprawo.bmp"); //stan_postaci
	sklej(tablica[1].filename, "mariostoiprawo");
	tablica[2].obrazek = SDL_LoadBMP("./grafiki/murek.bmp"); //dol_planszy
	sklej(tablica[2].filename, "murek");
	tablica[3].obrazek = SDL_LoadBMP("./grafiki/klocek.bmp"); //p
	sklej(tablica[3].filename, "klocek");
	tablica[4].obrazek = SDL_LoadBMP("./grafiki/marioumiera.bmp"); //stan_postaci
	sklej(tablica[4].filename, "marioumiera");
	tablica[5].obrazek = SDL_LoadBMP("./grafiki/brick.bmp"); //b //q jesli mozna go popsuc
	sklej(tablica[5].filename, "brick");
	tablica[6].obrazek = SDL_LoadBMP("./grafiki/czarny.bmp"); //c
	sklej(tablica[6].filename, "czarny");
	tablica[7].obrazek = SDL_LoadBMP("./grafiki/zaokraglony_czarny.bmp"); //z
	sklej(tablica[7].filename, "zaokraglony_czarny");
	tablica[8].obrazek = SDL_LoadBMP("./grafiki/tower.bmp"); //a
	sklej(tablica[8].filename, "tower");
	tablica[9].obrazek = SDL_LoadBMP("./grafiki/tower_wolny.bmp"); //d
	sklej(tablica[9].filename, "tower_wolny");
	tablica[10].obrazek = SDL_LoadBMP("./grafiki/tower_lewy.bmp"); //e
	sklej(tablica[10].filename, "tower_lewy");
	tablica[11].obrazek = SDL_LoadBMP("./grafiki/tower_prawy.bmp"); //f
	sklej(tablica[11].filename, "tower_prawy");
	tablica[12].obrazek = SDL_LoadBMP("./grafiki/flaga.bmp"); //g
	sklej(tablica[12].filename, "flaga");
	tablica[13].obrazek = SDL_LoadBMP("./grafiki/cgl.bmp"); //h
	sklej(tablica[13].filename, "chmura_gora_lewa");
	tablica[14].obrazek = SDL_LoadBMP("./grafiki/cgs.bmp"); //i
	sklej(tablica[14].filename, "chmura_gora_srodek");
	tablica[15].obrazek = SDL_LoadBMP("./grafiki/cgp.bmp"); //j
	sklej(tablica[15].filename, "chmura_gora_prawa");
	tablica[16].obrazek = SDL_LoadBMP("./grafiki/cdl.bmp"); //k
	sklej(tablica[16].filename, "chmura_dol_lewa");
	tablica[17].obrazek = SDL_LoadBMP("./grafiki/cds.bmp"); //l
	sklej(tablica[17].filename, "chmura_dol_srodek");
	tablica[18].obrazek = SDL_LoadBMP("./grafiki/cdp.bmp"); //m
	sklej(tablica[18].filename, "chmura_dol_prawa");
	tablica[19].obrazek = SDL_LoadBMP("./grafiki/tl.bmp"); //n
	sklej(tablica[19].filename, "trawa_lewa");
	tablica[20].obrazek = SDL_LoadBMP("./grafiki/ts.bmp"); //o
	sklej(tablica[20].filename, "trawa_srodek");
	tablica[21].obrazek = SDL_LoadBMP("./grafiki/tr.bmp"); //r
	sklej(tablica[21].filename, "trawa_prawa");
	tablica[22].obrazek = SDL_LoadBMP("./grafiki/pl.bmp"); //t
	sklej(tablica[22].filename, "platforma_lewa");
	tablica[23].obrazek = SDL_LoadBMP("./grafiki/ps.bmp"); //u
	sklej(tablica[23].filename, "platforma_srodek");
	tablica[24].obrazek = SDL_LoadBMP("./grafiki/pp.bmp"); //v
	sklej(tablica[24].filename, "platforma_prawa");
	tablica[25].obrazek = SDL_LoadBMP("./grafiki/pk.bmp"); //w
	sklej(tablica[25].filename, "platforma_korzen");
	tablica[26].obrazek = SDL_LoadBMP("./grafiki/dziura.bmp"); //x
	sklej(tablica[26].filename, "przerwa_w_platformie");
	tablica[27].obrazek = SDL_LoadBMP("./grafiki/mariostoilewo.bmp"); //stan_postaci
	sklej(tablica[27].filename, "mariostoilewo");
	tablica[28].obrazek = SDL_LoadBMP("./grafiki/marioidzielewo.bmp"); //stan_postaci
	sklej(tablica[28].filename, "marioidzielewo");
	tablica[29].obrazek = SDL_LoadBMP("./grafiki/marioidzieprawo.bmp"); //stan_postaci
	sklej(tablica[29].filename, "marioidzieprawo");
	tablica[30].obrazek = SDL_LoadBMP("./grafiki/marioidzielewo2.bmp"); //stan_postaci
	sklej(tablica[30].filename, "marioidzielewo2");
	tablica[31].obrazek = SDL_LoadBMP("./grafiki/marioidzieprawo2.bmp"); //stan_postaci
	sklej(tablica[31].filename, "marioidzieprawo2");
	tablica[32].obrazek = SDL_LoadBMP("./grafiki/marioskaczelewo.bmp"); //stan_postaci
	sklej(tablica[32].filename, "marioskaczelewo");
	tablica[33].obrazek = SDL_LoadBMP("./grafiki/marioskaczeprawo.bmp"); //stan_postaci
	sklej(tablica[33].filename, "marioskaczeprawo");
	tablica[34].obrazek = SDL_LoadBMP("./grafiki/marioidzielewo3.bmp"); //stan_postaci
	sklej(tablica[34].filename, "marioidzielewo3");
	tablica[35].obrazek = SDL_LoadBMP("./grafiki/marioidzieprawo3.bmp"); //stan_postaci
	sklej(tablica[35].filename, "marioidzieprawo3");
	tablica[36].obrazek = SDL_LoadBMP("./grafiki/goombas1.bmp"); //stan1przeciwnik1
	sklej(tablica[36].filename, "goombas1");
	tablica[37].obrazek = SDL_LoadBMP("./grafiki/goombas2.bmp"); //stan2przeciwnik1
	sklej(tablica[37].filename, "goombas2");
	tablica[38].obrazek = SDL_LoadBMP("./grafiki/koopa1.bmp"); //stan1przeciwnik2
	sklej(tablica[38].filename, "zolwio-ptasior1");
	tablica[39].obrazek = SDL_LoadBMP("./grafiki/koopa2.bmp"); //stan2przeciwnik2
	sklej(tablica[39].filename, "zolwio-ptasior2");
	tablica[40].obrazek = SDL_LoadBMP("./grafiki/koopa3.bmp"); //stan1przeciwnik3
	sklej(tablica[40].filename, "zolwio-ptasior3");
	tablica[41].obrazek = SDL_LoadBMP("./grafiki/koopa4.bmp"); //stan2przeciwnik4
	sklej(tablica[41].filename, "zolwio-ptasior4");
	tablica[42].obrazek = SDL_LoadBMP("./grafiki/moneta_blok.bmp"); //moneta_blok
	sklej(tablica[42].filename, "moneta_blok");
	tablica[43].obrazek = SDL_LoadBMP("./grafiki/moneta.bmp"); //moneta
	sklej(tablica[43].filename, "moneta");
	int i = 0;
	while (tablica[i].obrazek != NULL &&i <elementy)i++;
	
	if (i != elementy)
		{
		printf("Blad ladowania obrazka: %s %s\n",tablica[i].filename, SDL_GetError());
		zwolnienie_powierzchni(screen, tablica, scrtex, renderer, window);
		}
	return tablica;
}