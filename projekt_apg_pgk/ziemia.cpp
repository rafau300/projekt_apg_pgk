#include "ziemia.h"

void rysujZiemie() {
	glBegin(GL_QUADS);
		glColor3f(1.0,1.0,0);
		glVertex3f(-150,1,-150);

		glColor3f(1.0,1.0,0);
		glVertex3f(-150,1,150);

		glColor3f(1.0,1.0,1.0);
		glVertex3f(150,1,150);

		glColor3f(1.0,1.0,0);
		glVertex3f(150,1,-150);
	glEnd();
}