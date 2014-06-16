#ifndef __TEKST_H__
#define __TEKST_H__

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <gl/glu.h>
#include <gl/glut.h>

void wyswietlTekst(float x, float y, void *font, const char *string);
void wyswietlTekst(float x, float y, float z, void *font, const char *string);

#endif
