#include "tekst.h"

//funkcja wyswietlajaca ciag znakow korzystajac z 2 wspolrzednych (XY)
void wyswietlTekst(float x, float y, void *font, const char *string) {
    const char *c;
    glRasterPos2f(x, y);
    for (c=string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

//funkcja wyswietlajaca ciag znakow znakow korzystajac z 3 wspolrzednych (XYZ)
void wyswietlTekst(float x, float y, float z, void *font, const char *string) {
    const char *c;
    glRasterPos3f(x, y, z);
    for (c=string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}
