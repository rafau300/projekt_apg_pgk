#include <windows.h>	//WinApi
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#include <process.h>	//do wykorzystania watkow z WinApi

#include <Cg/cg.h>		//Biblioteki
#include <Cg/cgGL.h>	//do obslugi shaderow

#include "mgla.h"	//plik naglowkowy z funkcja generujaca mgle
#include "ziemia.h" //plik naglowkowy z funkcja generujaca podloze
#include "enable.h" //plik naglowkowy z funkcja wlaczajaca glownie oswietlenie oraz wywolanie funkcji glEnable()
#include "tekst.h"	//plik naglowowy z funkcja pozwalajaca na pisanie tekstu

#define ILOSC_CZASTECZEK_OGNIA 10000		//ilosc czasteczek, z ktorych sklada sie ogien
#define ILOSC_CZASTECZEK_DYMU 10000			//ilosc czasteczek, z ktorych skada sie dym
#define CZAS_ZYCIA 400						//"czas zycia" czasteczki
#define CZAS_ZYCIA_DYMU 500					//"czas zycia" czasteczki
float GRAWITACJA = - 0.00004;				//grawitacja dzialajaca na kazda z czasteczek
#define PROCENT_POWSTAJACYCH_CZASTECZEK 10	//szansa na powstanie czasteczce po jej zniknieciu

#define FPS 60	//docelowa ilosc klatek na sekunde

using namespace std;

//pozycja obserwatora i obrot
float xpos = 0, ypos = -2.0, zpos = 0, xrot = 0, yrot = 0, angle=0.0;

float odleglosc = 0.0;
int petla;

//zmienne do obslugi myszy - wspolrzedne
float lastx, lasty;

//positions of the cubes
float positionz[10];
float positionx[10];

//zmienna przydatna przy falowaniu ognia
float czas = 0;

//zmienna do wlaczania/wylaczania sortowania - potrzebne po to, by blending dzialal jak nalezy
bool sortowanieWlaczone = true;

//Wlaczenie/wylaczenie shaderow
bool shaderyWlaczone = true;

bool wygladzanie = false;

bool wyswietleniePomocy = false;

float mnoznikPrzezroczystosci = 1.0;

//pojedynczy punkt
struct punkt {
	float x,y,z;
};

struct punkt wspolrzedneSrodkaPlomienia;

//uchwyt do mutexa
HANDLE hMutex, hMutex2;	

//zmienne do obslugi shadera w jezyku CG
CGcontext	cgContext, cgContextDym;
CGprogram	cgProgram, cgDym;
CGprofile	cgVertexProfile, cgVertexProfileDym;
CGparameter	modelViewMatrix, position, color, wielkosc, modelViewMatrixDym, positionDym, colorDym, wielkoscDym;

//struktura w ktorej przechowane sa dane czasteczek ognia oraz dymu
struct czasteczka_ognia {
	float x,y,z;
	int czas_zycia;
	int zycie;
	bool czy_aktywna;
	float grawitacja;
	float przesuniecie_x,przesuniecie_y, przesuniecie_z;
};

struct czasteczka_ognia ogien[ILOSC_CZASTECZEK_OGNIA];
struct czasteczka_ognia dym[ILOSC_CZASTECZEK_DYMU];


//Prototypy funkcji
void inicjalizujCzasteczkiOgnia();
void inicjalizujCzasteczkiDymu();
void inicjalizujCzasteczkiOgnia(int ktora_czasteczka);
int inicjalizujCzasteczkiDymu(int ktora_czasteczka);
void aktualizujCzasteczkiOgnia();
void aktualizujCzasteczkiDymu();
void cubepositions (void);
void sortujTablice(struct czasteczka_ognia punkty[ILOSC_CZASTECZEK_OGNIA]);
void cube (void);
void init (void);
void __cdecl ThreadProc( void * Args );
void __cdecl ThreadProc2( void * Args );
void camera (void);
void rysujHUD();
void display (void);
void reshape (int w, int h);
void mouseMovement(int x, int y);
void keyboard (unsigned char key, int x, int y);
void klawiszeSpecjalne( int key, int x, int y );
void zegar(int val);
int main (int argc, char **argv);


//-----------------wstepne losowanie danych poszczegolnych czasteczek------------------
void inicjalizujCzasteczkiOgnia() {
	//wspolrzedne, w ktorych znajduje sie szescian, z ktorego wylatuje ogien
		wspolrzedneSrodkaPlomienia.x = 2.0;
		wspolrzedneSrodkaPlomienia.y = -0.5;
		wspolrzedneSrodkaPlomienia.z = 1.0;
	
	for (int i=0;i<ILOSC_CZASTECZEK_OGNIA;i++) {
		float miejsce1, miejsce2;
		//losowanie w zakresie 0..1, zeby wszystkie czastki nie startowaly z jednego miejsca
		miejsce1 = (float)(rand()%100)/100.0;
		miejsce2 = (float)(rand()%100)/100.0;
		ogien[i].x = wspolrzedneSrodkaPlomienia.x + miejsce1;
		ogien[i].y = wspolrzedneSrodkaPlomienia.y;
		ogien[i].z = wspolrzedneSrodkaPlomienia.z + miejsce2;

		//losowanie dlugosci zycia czasteczki w zakresie: 0..CZAS_ZYCIA
		ogien[i].czas_zycia = rand()%CZAS_ZYCIA;
		if (miejsce1 <= 0.3 && miejsce1 >= 0.7) ogien[i].czas_zycia *= 0.9;
		if (miejsce2 <= 0.3 && miejsce2 >= 0.7) ogien[i].czas_zycia *= 0.9;
		if (miejsce1 <= 0.1 && miejsce1 >= 0.9) ogien[i].czas_zycia *= 0.5;
		if (miejsce2 <= 0.1 && miejsce2 >= 0.9) ogien[i].czas_zycia *= 0.5;
		if (miejsce1 >= 0.3 && miejsce1 <= 0.7 && ogien[i].czas_zycia < CZAS_ZYCIA/2) ogien[i].czas_zycia *= 1.5;
		if (miejsce2 >= 0.3 && miejsce2 <= 0.7 && ogien[i].czas_zycia < CZAS_ZYCIA/2) ogien[i].czas_zycia *= 1.5;
		ogien[i].zycie = 0;
		ogien[i].czy_aktywna = false;

		int random = rand()%100;
		if (random < PROCENT_POWSTAJACYCH_CZASTECZEK) ogien[i].czy_aktywna = true;

		//wartosci o jakie czasteczki sa przesuwane w kazdej petli
		ogien[i].przesuniecie_x = -(float)(rand()%10)/1000 + 0.005;
		ogien[i].przesuniecie_y = -(float)(rand()%10)/800;
		ogien[i].przesuniecie_z = -(float)(rand()%10)/1000 + 0.005;

		ogien[i].grawitacja = GRAWITACJA;
	}
}

//-----------------wstepne losowanie danych poszczegolnych czasteczek------------------
void inicjalizujCzasteczkiDymu() {
	//zerowanie czasteczek dymu, bo dym powstaje z czasteczek ognia, dlatego wartosci beda przypisywane pozniej
	for (int i=0;i<ILOSC_CZASTECZEK_DYMU;i++) {		
		dym[i].czas_zycia = rand()%CZAS_ZYCIA_DYMU;
		dym[i].zycie = 0;
		dym[i].czy_aktywna = false;
		dym[i].grawitacja = GRAWITACJA;
	}
}

//-----------------losowanie danych konkretnej czasteczki------------------------------
void inicjalizujCzasteczkiOgnia(int ktora_czasteczka) {
		float miejsce1, miejsce2;
		//losowanie w zakresie 0..1, zeby wszystkie czastki nie startowaly z jednego miejsca
		miejsce1 = (float)(rand()%100)/100.0;
		miejsce2 = (float)(rand()%100)/100.0;
		ogien[ktora_czasteczka].x = wspolrzedneSrodkaPlomienia.x + miejsce1;
		ogien[ktora_czasteczka].y = wspolrzedneSrodkaPlomienia.y;
		ogien[ktora_czasteczka].z = wspolrzedneSrodkaPlomienia.z + miejsce2;
		
		//losowanie dlugosci zycia czasteczki w zakresie: 0..CZAS_ZYCIA
		ogien[ktora_czasteczka].czas_zycia = rand()%CZAS_ZYCIA;
		if (miejsce1 <= 0.3 && miejsce1 >= 0.7) ogien[ktora_czasteczka].czas_zycia *= 0.9;
		if (miejsce2 <= 0.3 && miejsce2 >= 0.7) ogien[ktora_czasteczka].czas_zycia *= 0.9;
		if (miejsce1 <= 0.1 && miejsce1 >= 0.9) ogien[ktora_czasteczka].czas_zycia *= 0.5;
		if (miejsce2 <= 0.1 && miejsce2 >= 0.9) ogien[ktora_czasteczka].czas_zycia *= 0.5;
		if (miejsce1 >= 0.3 && miejsce1 <= 0.7 && ogien[ktora_czasteczka].czas_zycia < CZAS_ZYCIA/2) ogien[ktora_czasteczka].czas_zycia *= 1.5;
		if (miejsce2 >= 0.3 && miejsce2 <= 0.7 && ogien[ktora_czasteczka].czas_zycia < CZAS_ZYCIA/2) ogien[ktora_czasteczka].czas_zycia *= 1.5;
		ogien[ktora_czasteczka].zycie = 0;
		
		//int random = rand()%100;
		//if (random < PROCENT_POWSTAJACYCH_CZASTECZEK) ogien[ktora_czasteczka].czy_aktywna = true;

		//wartosci o jakie czasteczki sa przesuwane w kazdej petli
		ogien[ktora_czasteczka].przesuniecie_x = -((float)(rand()%10)/1000 + 0.005) * (sin(czas)/3.5);
		ogien[ktora_czasteczka].przesuniecie_y = -(float)(rand()%10)/800;
		ogien[ktora_czasteczka].przesuniecie_z = -((float)(rand()%10)/1000 + 0.005) * (sin(czas)/3.5);

		ogien[ktora_czasteczka].grawitacja = GRAWITACJA;
}

//-----------------losowanie danych konkretnej czasteczki------------------------------
int inicjalizujCzasteczkiDymu(int ktora_czasteczka) {
	//if (dym[ktora_czasteczka].czy_aktywna == false) {
		dym[ktora_czasteczka].x = ogien[ktora_czasteczka].x; 
		dym[ktora_czasteczka].y = ogien[ktora_czasteczka].y - 1.5;
		dym[ktora_czasteczka].z = ogien[ktora_czasteczka].z;
		
		//losowanie dlugosci zycia czasteczki w zakresie: 0..CZAS_ZYCIA_DYMU
		dym[ktora_czasteczka].czas_zycia = rand()%CZAS_ZYCIA_DYMU;
		dym[ktora_czasteczka].zycie = 0;

		//wartosci o jakie czasteczki sa przesuwane w kazdej petli
		dym[ktora_czasteczka].przesuniecie_x = -((float)(rand()%10)/1000 + 0.005) * (sin(czas)/3.5);
		dym[ktora_czasteczka].przesuniecie_y = -(float)(rand()%10)/1000;
		dym[ktora_czasteczka].przesuniecie_z = -((float)(rand()%10)/1000 + 0.005) * (sin(czas)/3.5);

		dym[ktora_czasteczka].grawitacja = GRAWITACJA;
		dym[ktora_czasteczka].czy_aktywna = true;
	//}

	return 0;
}

//-----------------aktualizacja czasteczek ognia w kazdej z petli----------------------
void aktualizujCzasteczkiOgnia() {
	for (int i=0;i<ILOSC_CZASTECZEK_OGNIA;i++) {
		//przesuwanie czasteczki o dane wspolrzedne
		ogien[i].x += ogien[i].przesuniecie_x;
		ogien[i].przesuniecie_y += ogien[i].grawitacja;
		ogien[i].y += ogien[i].przesuniecie_y;// + ogien[i].grawitacja;
		ogien[i].z += ogien[i].przesuniecie_z;

		ogien[i].zycie++;

		//losowe ruchy czasteczek
		int random = rand()%300;
		if (random == 0) ogien[i].przesuniecie_x *= -0.1;
		if (random == 1) ogien[i].przesuniecie_z *= -0.1;
		if (random == 2) ogien[i].przesuniecie_x *= 1.1;
		if (random == 3) ogien[i].przesuniecie_z *= 1.1;
		if (random == 4) ogien[i].przesuniecie_x *= 0.9;
		if (random == 5) ogien[i].przesuniecie_z *= 0.9;

		//Jesli czasteczka "umarla" - stworzenie czasteczki dymu i "zresetowanie" czasteczki ognia
		if (ogien[i].zycie >= ogien[i].czas_zycia) {
			ogien[i].czy_aktywna = false;
			inicjalizujCzasteczkiDymu(i);
			inicjalizujCzasteczkiOgnia(i);
		}
		//Jezeli czasteczka przekroczyla polowe "czasu zycia" to bardzo powoli zwalnia
		else if ((float)ogien[i].zycie/(float)ogien[i].czas_zycia > 0.5)
			ogien[i].przesuniecie_y *= 0.99;
	}
}

//-----------------aktualizacja czasteczek dymu w kazdej z petli-----------------------
void aktualizujCzasteczkiDymu() {
	for (int i=0;i<ILOSC_CZASTECZEK_DYMU;i++) {
		//dodatkowe sprawdzenie czy czasteczka dymu jest aktywna, zeby nie wylatywala znikad
		if (dym[i].czy_aktywna) {
		//przesuwanie czasteczki o dane wspolrzedne
		dym[i].x += dym[i].przesuniecie_x;
		dym[i].przesuniecie_y += dym[i].grawitacja;
		dym[i].y += dym[i].przesuniecie_y;
		dym[i].z += dym[i].przesuniecie_z;

		dym[i].zycie++;

		//losowe ruchy czasteczek
		int random = rand()%300;
		if (random == 0) dym[i].przesuniecie_x *= -0.1;
		if (random == 1) dym[i].przesuniecie_z *= -0.1;
		if (random == 2) dym[i].przesuniecie_x *= 1.1;
		if (random == 3) dym[i].przesuniecie_z *= 1.1;
		if (random == 4) dym[i].przesuniecie_x *= 0.9;
		if (random == 5) dym[i].przesuniecie_z *= 0.9;

		//Jezeli czasteczka "umarla"
		if (dym[i].zycie >= dym[i].czas_zycia) {
			ogien[i].czy_aktywna = false;
		}
		/*else if ((float)dym[i].zycie/(float)dym[i].czas_zycia > 0.5)
			dym[i].przesuniecie_y *= 0.99;*/
		}
	}
}

//-----------------ustawienie pozycji szescianow---------------------------------------
void cubepositions (void) { 

    for (int i=0;i<10;i++) {
		positionz[i] = rand()%5 + 5;
		positionx[i] = rand()%5 + 5;
    }
}

//-----------------rysowanie szescianow------------------------------------------------
void cube (void) {
    for (int i=0;i<9;i++) {
		glPushMatrix();
		glColor3f(1.0,1.0,1.0);
		glTranslated(-positionx[i + 1] * 10, 0, -positionz[i + 1] *10); //translate the cube
		glutSolidCube(1); //draw the cube
		glPopMatrix();
    }
}

//-----------------inicjalizacja programu - glownie shadera----------------------------
void init (void) {
	//wylaczenie widocznosci kursora myszy
	ShowCursor(false);
	cubepositions();

	//glutGameModeString("1366x768:32@60");
	//if (glutGameModeGet (GLUT_GAME_MODE_POSSIBLE)) {
	//	glutEnterGameMode();
	//}

	//wstepna inicjalizacja czasteczek
	inicjalizujCzasteczkiOgnia();
	inicjalizujCzasteczkiDymu();  
	
	//Odtad zaczyna sie inicjalizacja shaderow w jezyku cG

	//inicjalizacja shadera ogien.cg
	cgContext = cgCreateContext();	
	if(cgContext==0){
	  printf("Nie można utworzyć kontekstu Cg!");
	  exit(1);
	}
	
	cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	if(cgVertexProfile == CG_PROFILE_UNKNOWN){
	  printf("Nieznany profil!");
	  exit(1);
	}
	
	cgGLSetOptimalOptions(cgVertexProfile);	
	
	cgProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, "ogien.cg", cgVertexProfile, "main", 0);
	
	if (cgProgram == 0)
	{
		// We Need To Determine What Went Wrong
		CGerror Error = cgGetError();

		// Show A Message Box Explaining What Went Wrong
		fprintf(stderr,"%s \n",cgGetErrorString(Error));

		  const char* errorString = cgGetErrorString(cgGetError());

		  printf("Cg error: %s", errorString);

		getchar();
		exit(-1);													// We Cannot Continue
	}
	
	cgGLLoadProgram(cgProgram);
	position		= cgGetNamedParameter(cgProgram, "IN.position");
	color			= cgGetNamedParameter(cgProgram, "IN.color");
	modelViewMatrix		= cgGetNamedParameter(cgProgram, "ModelViewProj");
	wielkosc		= cgGetNamedParameter(cgProgram, "IN.wielkosc");

	//////////////////////////////////////////////////////////////////////////////
	//inicjalizacja shadera dym.cg
	cgContextDym = cgCreateContext();	
	if(cgContextDym==0){
	  printf("Nie można utworzyć kontekstu Cg!");
	  exit(1);
	}
	
	cgVertexProfileDym = cgGLGetLatestProfile(CG_GL_VERTEX);
	if(cgVertexProfileDym == CG_PROFILE_UNKNOWN){
	  printf("Nieznany profil!");
	  exit(1);
	}

	cgGLSetOptimalOptions(cgVertexProfileDym);	
	
	cgDym = cgCreateProgramFromFile(cgContextDym, CG_SOURCE, "dym.cg", cgVertexProfileDym, "dym", 0);
	
	if (cgDym == 0)
	{
		// We Need To Determine What Went Wrong
		CGerror Error = cgGetError();

		// Show A Message Box Explaining What Went Wrong
		fprintf(stderr,"%s \n",cgGetErrorString(Error));

		  const char* errorString = cgGetErrorString(cgGetError());

		  printf("Cg error: %s", errorString);

		getchar();
		exit(-1);													// We Cannot Continue
	}
	
	cgGLLoadProgram(cgDym);
	positionDym		= cgGetNamedParameter(cgDym, "IN.positionDym");
	colorDym			= cgGetNamedParameter(cgDym, "IN.colorDym");
	wielkosc		= cgGetNamedParameter(cgProgram, "IN.wielkosc");
	modelViewMatrixDym		= cgGetNamedParameter(cgDym, "ModelViewProjDym");


	//uruchomienie watkow, zeby uplynnic i przyspieszyc dzialanie programu
	HANDLE hThread =( HANDLE ) _beginthread( ThreadProc, 0, NULL );
	HANDLE hThread2 =( HANDLE ) _beginthread( ThreadProc2, 0, NULL );
	//ThreadProc(NULL);
}

//-----------------watek, w ktorym wykonuje się sortowanie czasteczek ognia, zeby blending dzialal prawidlowo
void __cdecl ThreadProc( void * Args ) {
	while(true) {
	//stworzenie mutexa
	hMutex = CreateMutex( NULL, FALSE, "Mutex" );
	if (sortowanieWlaczone) {
		//sortowanie czasteczek ognia
		sortujTablice(ogien);
		//Sortowanie(ogien,0,ILOSC_CZASTECZEK_OGNIA);
	}
	//deaktywowanie mutexa, zeby program glowny mogl dzialac
	CloseHandle(hMutex);
	//wywolanie Sleep(), zeby watek nie zawlaszczyl calego watku procesora dla siebie
	Sleep(50);
	}
}

//-----------------watek, w ktorym wykonuje się sortowanie czasteczek ognia, zeby blending dzialal prawidlowo
void __cdecl ThreadProc2( void * Args ) {
	while(true) {
	//stworzenie mutexa
	hMutex2 = CreateMutex( NULL, FALSE, "Mutex2" );
	if (sortowanieWlaczone) {
		//sortowanie czasteczek ognia
		sortujTablice(dym);
		//Sortowanie(dym,0,ILOSC_CZASTECZEK_DYMU);
	}
	//deaktywowanie mutexa, zeby program glowny mogl dzialac
	CloseHandle(hMutex2);
	//wywolanie Sleep(), zeby watek nie zawlaszczyl calego watku procesora dla siebie
	Sleep(50);
	}
}

//-----------------Funkcja obracajaca kamere-------------------------------------------
void camera (void) {
    glRotatef(xrot,1.0,0.0,0.0);  //rotate our camera on the x-axis (left and right)
    glRotatef(yrot,0.0,1.0,0.0);  //rotate our camera on the y-axis (up and down)
    glTranslated(-xpos,-ypos,-zpos); //translate the screen to the position of our camera
}

//-----------------obliczenie odleglosci czasteczki od obserwatora---------------------
float obliczOdleglosc(struct punkt wspolrzedne) {
	//niestety trzeba obliczac odleglosc od kazdej czasteczki osobno, bo obserwator moze sie swobodnie przemieszczac
	float odlegloscX, odlegloscZ, odleglosc;
	odlegloscX = wspolrzedne.x - xpos;
	odlegloscZ = wspolrzedne.z - zpos;
	odlegloscX *= odlegloscX;
	odlegloscZ *= odlegloscZ;
	//odleglosc = sqrt(odlegloscX + odlegloscZ);
	odleglosc = odlegloscX + odlegloscZ;
	return odleglosc;
}

//-----------------Sortowanie tablicy ze wspolrzednymi czasteczek----------------------
void sortujTablice(struct czasteczka_ognia punkty[ILOSC_CZASTECZEK_OGNIA]) {
	//To jest najbardziej obciazajaca procesor funkcja, dlatego jest wykonywana w watkach
	//Trzeba wykonywac sortowanie, by blending dzialal prawidlowo, trzeba rysowac
	//czasteczki od najdalszych do najbliższych
	bool zmiana;
	for (int i=0;i<ILOSC_CZASTECZEK_OGNIA;i++) {
		if ((float)punkty[i].zycie/(float)punkty[i].czas_zycia < 0.2) {
		zmiana = false;
		for (int j=0;j<ILOSC_CZASTECZEK_OGNIA-i;j++) {
			//wspolrzedne dwoch czasteczek
			struct punkt pktI, pktJ;
			pktI.x = punkty[i].x;
			pktI.z = punkty[i].z;
			pktJ.x = punkty[j].x;
			pktJ.z = punkty[j].z;
			//Jezeli czasteczka[i] lezy dalej od czasteczki[j]; wykonuj sortowanie
			if (obliczOdleglosc(pktI) > obliczOdleglosc(pktJ)) {
				struct czasteczka_ognia pom;
				pom = punkty[i];
				punkty[i] = punkty[j];
				punkty[j] = pom;
				zmiana = true;
			}
		}
		if (!zmiana) break;
		}
	}
}

//-----------------Rysowanie obrazu 2D nad obrazem 3D----------------------------------
void rysujHUD() {
	//przelaczenie sie na 2D
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glMatrixMode(GL_PROJECTION);					// Select Projection
	//glPushMatrix();							// Push The Matrix
	glLoadIdentity();						// Reset The Matrix
	glOrtho( 0, 800 , 600 , 0, -1, 1 );				// Select Ortho Mode
	glMatrixMode(GL_MODELVIEW);					// Select Modelview Matrix
	//glPushMatrix();							// Push The Matrix
	glLoadIdentity();

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Rysowanie HUDa
    glBegin(GL_QUADS);
        glColor3f(0.2,0.2,0.2);
        glVertex2d(0,0);
        glVertex2d(0,30);
        glVertex2d(800,30);
        glVertex2d(800,0);
    glEnd();

	//Wypisywanie, ktore funkcje sa wlaczone
	if (shaderyWlaczone) glColor4f(0.0,1.0,0.0,1.0);
	else glColor4f(0.5,0.5,0.5,1.0);
    wyswietlTekst(1.0,21.0,GLUT_BITMAP_TIMES_ROMAN_24,"Shader");

	if (sortowanieWlaczone) glColor4f(0.0,1.0,0.0,1.0);
	else glColor4f(0.5,0.5,0.5,1.0);
    wyswietlTekst(100.0,21.0,GLUT_BITMAP_TIMES_ROMAN_24,"Sortowanie");

	if (wygladzanie) glColor4f(0.0,1.0,0.0,1.0);
	else glColor4f(0.5,0.5,0.5,1.0);
    wyswietlTekst(250.0,21.0,GLUT_BITMAP_TIMES_ROMAN_24,"Wygladzanie");

	glColor4f(1.0,1.0,1.0,1.0);
	wyswietlTekst(500.0,21.0,GLUT_BITMAP_TIMES_ROMAN_24,"F1 - sterowanie");

	//Wyswietlenie pomocy do sterowania
	if (wyswietleniePomocy) {
		glEnable(GL_BLEND);
		glBegin(GL_QUADS);
			glColor4f(0.2,0.2,0.2,0.5);
			glVertex2d(500,30);
			glVertex2d(500,600);
			glVertex2d(800,600);
			glVertex2d(800,30);
		glEnd();

		glColor4f(1.0,1.0,1.0,1.0);
		wyswietlTekst(500.0,100.0,GLUT_BITMAP_TIMES_ROMAN_24,"W,S,A,D:");
		wyswietlTekst(500.0,130.0,GLUT_BITMAP_TIMES_ROMAN_24,"Chodzenie");

		wyswietlTekst(500.0,180.0,GLUT_BITMAP_TIMES_ROMAN_24,"MYSZ:");
		wyswietlTekst(500.0,210.0,GLUT_BITMAP_TIMES_ROMAN_24,"Obracanie");

		wyswietlTekst(500.0,260.0,GLUT_BITMAP_TIMES_ROMAN_24,"SPACJA:");
		wyswietlTekst(500.0,290.0,GLUT_BITMAP_TIMES_ROMAN_24,"WL/WYL sortowanie czastek");

		wyswietlTekst(500.0,340.0,GLUT_BITMAP_TIMES_ROMAN_24,"ENTER");
		wyswietlTekst(500.0,370.0,GLUT_BITMAP_TIMES_ROMAN_24,"WL/WYL shadery");

		wyswietlTekst(500.0,420.0,GLUT_BITMAP_TIMES_ROMAN_24,"Backspace:");
		wyswietlTekst(500.0,450.0,GLUT_BITMAP_TIMES_ROMAN_24,"WL/WYL wygladzanie czastek");

		wyswietlTekst(500.0,500.0,GLUT_BITMAP_TIMES_ROMAN_24,"+/-:");
		wyswietlTekst(500.0,530.0,GLUT_BITMAP_TIMES_ROMAN_24,"Zwieksz/zmniejsz grawitacje");
	
		glDisable(GL_BLEND);
	}
}

//-----------------Glowna funkcja, w ktorej rysowana jest scena------------------------
void display (void) {
	//obliczanie czasu, zeby uzyskac efekt falowania ognia
	czas+=0.01;
	petla++;

	//przelaczenie sie na widok 3D
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glMatrixMode( GL_PROJECTION );					// Select Projection
    glLoadIdentity();
	//glPopMatrix();
	gluPerspective(60, (double)800/600, 1.0, 1000.0);
	glMatrixMode( GL_MODELVIEW );					// Select Modelview
	//glPopMatrix();
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //czysczenie ekranu
	glClearColor (0.2,0.2,0.5,1.0); 
   // glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer and the depth buffer
   // glLoadIdentity();

    camera();
    //enable();
    cube(); //call the cube drawing function

	glPushMatrix();
		glColor3f(0.0,0.0,0.0);
		glTranslatef(2.5,0.0,1.5);
		glutSolidCube(1);
	glPopMatrix();

	rysujZiemie();
	rysujMgle();

	//aktualizujCzasteczkiOgnia();
	//aktualizujCzasteczkiDymu();


	//wlaczenie blendingu
	glEnable(GL_BLEND); 
	//Nakladajace sie czasteczki beda wywolywac efekt swiecenia ognia; 
	//im wiecej czasteczek nalozy sie na siebie, tym jasniejszy bedzie plomien
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	//ustawienie shadera
	if (shaderyWlaczone) {
		cgGLSetStateMatrixParameter(modelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);	
		cgGLEnableProfile(cgVertexProfile);	
		cgGLBindProgram(cgProgram);
	}

	float odlegloscX, odlegloscY, odlegloscZ;

	float przezroczystosc;

	//rownanie sfery - obliczenie odleglosci od srodka plomienia za pomoca wzoru r^2 = (x-x0)^2 + (y-y0)^2 + (z-z0)^2
	odlegloscX = wspolrzedneSrodkaPlomienia.x - xpos;
	odlegloscY = wspolrzedneSrodkaPlomienia.y - ypos;
	odlegloscZ = wspolrzedneSrodkaPlomienia.z - zpos;
	odlegloscX *= odlegloscX;
	odlegloscY *= odlegloscY;
	odlegloscZ *= odlegloscZ;

	odleglosc = sqrt (odlegloscX + odlegloscY + odlegloscZ);
	
	//ustawienie wielkosci punktu w zaleznosci od odleglosci
	float wielkoscCzasteczki = 10.0 - sqrt(odleglosc);
	if (odleglosc < 10.0) wielkoscCzasteczki *= 1.2;
	if (odleglosc < 5.0) wielkoscCzasteczki *= 1.3;
	if (odleglosc < 3.0) wielkoscCzasteczki *= 1.4;
	glPointSize(wielkoscCzasteczki);

	//stworzenie mutexa, bedzie to mozliwe wtedy, gdy watek sortujacy usunie swoj mutex
	//dzieki temu unika sie zakleszczen
	hMutex = CreateMutex( NULL, FALSE, "Mutex" );

	//Rysowanie plomienia
	glPushMatrix();
	glBegin(GL_POINTS);
	for (int i=0;i<ILOSC_CZASTECZEK_OGNIA;i++) {

		float kolor = 1.0 - (float)ogien[i].zycie/(float)ogien[i].czas_zycia;
		if (kolor >= 0.2) przezroczystosc = 0.2;
		else przezroczystosc = kolor * mnoznikPrzezroczystosci;
		if (przezroczystosc < 0.1) przezroczystosc *= 1.5;
		if (wygladzanie) glColor4f(kolor,kolor,kolor,przezroczystosc/3);
		else glColor4f(kolor,kolor,kolor,przezroczystosc);
		//rysujSzescian(ogien[i].x,ogien[i].y,ogien[i].z);
		glVertex3f(ogien[i].x,ogien[i].y,ogien[i].z);
		if (wygladzanie) {
		glVertex3f(ogien[i].x,ogien[i].y,ogien[i].z-(0.05 * przezroczystosc));
		glVertex3f(ogien[i].x,ogien[i].y,ogien[i].z+(0.05 * przezroczystosc));
		glVertex3f(ogien[i].x,ogien[i].y-(0.05 * przezroczystosc),ogien[i].z);
		glVertex3f(ogien[i].x,ogien[i].y+(0.05 * przezroczystosc),ogien[i].z);
		glVertex3f(ogien[i].x-(0.05 * przezroczystosc),ogien[i].y,ogien[i].z);
		glVertex3f(ogien[i].x+(0.05 * przezroczystosc),ogien[i].y,ogien[i].z);
		}
		cgSetParameterValuefc( wielkosc, 1, &wielkoscCzasteczki ); 
	}
	glEnd();
	glPopMatrix();

	//usuniecie mutexa
	CloseHandle(hMutex);

	//wylaczenie shadera
	if (shaderyWlaczone) cgGLDisableProfile(cgVertexProfile);

	//ustawienie blendingu dla dymu - odwrotny efekt niz w przypadku ognia - nakladajace sie
	//czasteczki staja sie coraz ciemniejsze
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//wlaczenie shadera dla dymu
	if (shaderyWlaczone) {
		cgGLSetStateMatrixParameter(modelViewMatrixDym, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);	
		cgGLEnableProfile(cgVertexProfileDym);	
		cgGLBindProgram(cgDym);
	}

	//ustawienie wielkosci punktu w zaleznosci od odleglosci od obserwatora
	glPointSize(wielkoscCzasteczki + 2.0);
	//glPointSize(50);

	//utworzenie mutexa
	hMutex2 = CreateMutex( NULL, FALSE, "Mutex2" );

	//rysowanie dymu
	glPushMatrix();
	glBegin(GL_POINTS);
	for (int i=0;i<ILOSC_CZASTECZEK_DYMU;i++) {
		float kolor = 1.0 - (float)dym[i].zycie/(float)dym[i].czas_zycia;
		if (kolor >= 0.2) przezroczystosc = 0.2;
		else przezroczystosc = kolor * mnoznikPrzezroczystosci;
		glColor4f(0.0,0.0,0.0,przezroczystosc/5.0);
		if (wygladzanie) glColor4f(0.0,0.0,0.0,przezroczystosc/6.0);
		//rysujSzescian(ogien[i].x,ogien[i].y,ogien[i].z);
		glVertex3f(dym[i].x,dym[i].y,dym[i].z);
		if (wygladzanie) {
			float o = 0.03;
			glVertex3f(dym[i].x+o,dym[i].y,dym[i].z);
			glVertex3f(dym[i].x-o,dym[i].y,dym[i].z);
			glVertex3f(dym[i].x,dym[i].y+o,dym[i].z);
			glVertex3f(dym[i].x,dym[i].y-o,dym[i].z);
			glVertex3f(dym[i].x,dym[i].y,dym[i].z+o);
			glVertex3f(dym[i].x,dym[i].y,dym[i].z-o);
		}
		cgSetParameterValuefc( wielkoscDym, 1, &wielkoscCzasteczki ); 
	}
	glEnd();
	glPopMatrix();

	//usuniecie mutexa
	CloseHandle(hMutex2);

	//wylaczenie shadera dymu
	if (shaderyWlaczone) cgGLDisableProfile(cgVertexProfileDym);

	//wylacznie blendingu, zeby reszta sceny byl arysowana normalnie
	glDisable(GL_BLEND);

	//glutSwapBuffers(); //swap the buffers
    angle++; //increase the angle

	rysujHUD();
	glFlush ();
	glutSwapBuffers();
	glutPostRedisplay();
}

//-----------------Funkcja wywolywana przy zmianie rozmiaru okna-----------------------
void reshape (int w, int h) {
    glViewport (0, 0, (GLsizei)w, (GLsizei)h); //set the viewport to the current window specifications
    glMatrixMode (GL_PROJECTION); //set the matrix to projection

    glLoadIdentity ();
    gluPerspective (60, (GLfloat)w / (GLfloat)h, 1.0, 1000.0); //set the perspective (angle of sight, width, height, ,depth)
    glMatrixMode (GL_MODELVIEW); //set the matrix back to model

}

//-----------------obrot ekranu przy ruchu mysza---------------------------------------
void mouseMovement(int x, int y) {
    int diffx=x-lastx; //check the difference between the current x and the last x position
    int diffy=y-lasty; //check the difference between the current y and the last y position
    lastx=x; //set lastx to the current x position
    lasty=y; //set lasty to the current y position
    xrot += (float) diffy; //set the xrot to xrot with the addition of the difference in the y position
    yrot -= (float) diffx;// set the xrot to yrot with the addition of the difference in the x position
}

//-----------------Obsluga klawiatury--------------------------------------------------
void keyboard (unsigned char key, int x, int y) {
    if (key=='s') {
		float xrotrad, yrotrad;
		yrotrad = (yrot / 180 * 3.141592654f);
		xrotrad = (xrot / 180 * 3.141592654f);
		xpos += float(sin(yrotrad)) ;
		zpos -= float(cos(yrotrad)) ;
		//ypos -= float(sin(xrotrad)) ;
    }

    if (key=='w') {
		float xrotrad, yrotrad;
		yrotrad = (yrot / 180 * 3.141592654f);
		xrotrad = (xrot / 180 * 3.141592654f);
		xpos -= float(sin(yrotrad));
		zpos += float(cos(yrotrad)) ;
		//ypos += float(sin(xrotrad));
    }

    if (key=='d') {
        float yrotrad;
        yrotrad = (yrot / 180 * 3.141592654f);
        xpos += float(cos(yrotrad));// * 0.2;
        zpos += float(sin(yrotrad));// * 0.2;
    }

    if (key=='a') {
        float yrotrad;
        yrotrad = (yrot / 180 * 3.141592654f);
        xpos -= float(cos(yrotrad));// * 0.2;
        zpos -= float(sin(yrotrad));// * 0.2;
    }

	if (key=='+') {
		GRAWITACJA += 0.00001;
		printf("Sila grawitacji= %f\n",GRAWITACJA);
	}

	if (key == '-') {
		GRAWITACJA -= 0.00001;
		printf("Sila grawitacji= %f\n",GRAWITACJA);
	}

	if (key == '*') {
		mnoznikPrzezroczystosci += 0.1;
	}

	if (key == '/' && mnoznikPrzezroczystosci > 0.1) {
		mnoznikPrzezroczystosci -= 0.1;
	}

	if (key == ' ') {
		if (sortowanieWlaczone) sortowanieWlaczone = false;
		else sortowanieWlaczone = true;
	}

	if (key==8) {//Backspace
		if (wygladzanie) wygladzanie = false;
		else wygladzanie = true;
	}

    if (key==27) {//ESC
        exit(0);
    }

	if (key==13) {//ENTER
		if (shaderyWlaczone) shaderyWlaczone = false;
		else shaderyWlaczone = true;
	}
}

//-----------------Obsluga klawiszy specjalnych----------------------------------------
void klawiszeSpecjalne( int key, int x, int y ) {
    switch( key ) {
    //przycisk F1
    case GLUT_KEY_F1:
        if (wyswietleniePomocy) wyswietleniePomocy = false;
        else wyswietleniePomocy = true;
    break;

    }
}

//-----------------Zegar---------------------------------------------------------------
void zegar(int val) {
	aktualizujCzasteczkiOgnia();
	aktualizujCzasteczkiDymu();
	glutPostRedisplay();
	glutTimerFunc(1000/FPS, zegar, 0);
}

//-----------------Main----------------------------------------------------------------
int main (int argc, char **argv) {
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH |GLUT_RGBA); 
    glutInitWindowSize (800, 600); 
    glutInitWindowPosition (100, 100); //set the position of the window
    glutCreateWindow ("Projekt z PGK i APG - Rafal Bebenek i Jacek Kominek, grupa 313A");
    //inicjalizacja
	init (); 
    //funkcja wyswietlajaca obraz
	glutDisplayFunc (display);
    //Dzieki temu gdy komputer jest wystarczajaco szybki - obraz jest wysiwetany czesciej niz wynosci wartosc FPS
	glutIdleFunc (display);
    glutReshapeFunc (reshape);

    //Funckaj przechwytujaca ruchy mysza
	glutPassiveMotionFunc(mouseMovement);
	glutTimerFunc(1000/FPS, zegar, 0);

	srand(time(0));

    glutKeyboardFunc (keyboard); 
	glutSpecialFunc (klawiszeSpecjalne);
    glutMainLoop (); 
    return 0;
}
