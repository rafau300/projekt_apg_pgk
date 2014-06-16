#include "mgla.h"

void rysujMgle() {
	glEnable(GL_FOG);
    GLfloat fogColor[4] = {0.1, 0.1, 0.1, 0.5};
	glFogfv (GL_FOG_COLOR, fogColor);
    glFogf (GL_FOG_DENSITY, 0.5);
    glFogi (GL_FOG_MODE, GL_LINEAR);
    glHint (GL_FOG_HINT, GL_NICEST);
    //glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf (GL_FOG_START, 20.0f);
    glFogf (GL_FOG_END, 100.0f);
}