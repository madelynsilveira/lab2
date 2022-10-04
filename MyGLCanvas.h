#pragma once

#ifndef MYGLCANVAS_H
#define MYGLCANVAS_H

#include <FL/gl.h>
#include <FL/glut.h>
#include <FL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ply.h"


class MyGLCanvas : public Fl_Gl_Window {
public:
	int wireframe, filled, silhouette, showNormal, frontvBackFace;
	int rotX, rotY, rotZ;
	float red, green, blue;
	glm::vec3 eyePosition;

	/****************************************/
	/*         PLY Object                   */
	/****************************************/
	ply* myPLY = NULL;

	MyGLCanvas(int x, int y, int w, int h, const char *l = 0);
	~MyGLCanvas();

private:
	void draw();
	int handle(int);
	void resize(int x, int y, int w, int h);
	void updateCamera(int width, int height);
};


#endif // !MYGLCANVAS_H