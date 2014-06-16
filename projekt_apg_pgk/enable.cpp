#include "enable.h"

void enable (void) {

	glEnable (GL_DEPTH_TEST ); //enable the depth testing
	glShadeModel (GL_SMOOTH); //set the shader to smooth shader


	/* Parametry œwiat³a i materia³ów */
	GLfloat lightAmb[] = {0.1, 0.1, 0.1, 1.0};
	GLfloat lightDif[] = {0.7, 0.7, 0.7, 1.0};

	GLfloat lightPos[] = {0,-300,30,5.0};
	GLfloat lightSpec[] = {1, 1, 1, 1};


    //W³¹czenie œwiat³a
    glEnable(GL_LIGHTING);
    /* Natê¿enie œwiat³a otoczenia (AMBIENT) */
	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmb);
	/* Natê¿enie œwiat³a rozpraszaj¹cego (DIFFUSE) */
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDif);
    /* Œwiat³o nr 0 umieszczone nad scen¹ z prawej strony */
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
	/* Natê¿enie odb³ysków */
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpec);
	/* W³¹czenie œwiat³a nr 0 */
	glEnable(GL_LIGHT1);

	/* Ustawienie odb³ysku materia³ów */
	glMaterialfv(GL_FRONT, GL_SPECULAR, lightSpec);
	/* Skupienie i jasnoœæ plamy œwiat³a */
	glMateriali(GL_FRONT, GL_SHININESS, 64);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}