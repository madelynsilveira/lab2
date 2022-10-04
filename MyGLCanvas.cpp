#include "MyGLCanvas.h"

MyGLCanvas::MyGLCanvas(int x, int y, int w, int h, const char *l) : Fl_Gl_Window(x, y, w, h, l) {
	mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);
	wireframe = 0;
	filled = 1;
	silhouette = 0;
	showNormal = 0;
	frontvBackFace = 0;
	rotX = rotY = rotZ = 0;
	eyePosition = glm::vec3(0.0f, 0.0f, 2.0f);
	red = green = blue = 0.5f;
	myPLY = new ply();
}

MyGLCanvas::~MyGLCanvas() {
	delete myPLY;
}

void MyGLCanvas::draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!valid()) {  //this is called when the GL canvas is set up for the first time...
		puts("establishing GL context");

		glViewport(0, 0, w(), h());
		updateCamera(w(), h());

		glClearColor(0.1, 0.1, 0.1, 1.0);
		glShadeModel(GL_FLAT);

		GLfloat light_pos0[] = { 0.0f, 0.0f, 1.0f, 0.0f };
		GLfloat ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos0);

		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		/**********************************************/
		/*    Enable normalizing normal vectors       */
		/*    (e.g. normals not affected by glScalef) */
		/**********************************************/

		glEnable(GL_NORMALIZE);

		/****************************************/
		/*          Enable z-buferring          */
		/****************************************/

		glEnable(GL_DEPTH_TEST);
		glPolygonOffset(1, 1);
		glFrontFace(GL_CCW); //make sure that the ordering is counter-clock wise
	}

	GLfloat diffuse[] = { red, green, blue, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);


	// Clear the buffer of colors in each bit plane.
	// bit plane - A set of bits that are on or off (Think of a black and white image)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Set the mode so we are modifying our objects.
	glMatrixMode(GL_MODELVIEW);
	// Load the identify matrix which gives us a base for our object transformations
	// (i.e. this is the default state)
	glLoadIdentity();


	//now we need to update the lookVector	
	glm::mat4 rotXMat = glm::rotate(glm::mat4(1.0), glm::radians((float)-rotX), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotYMat = glm::rotate(glm::mat4(1.0), glm::radians((float)-rotY), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotZMat = glm::rotate(glm::mat4(1.0), glm::radians((float)-rotZ), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 curLookVector = glm::normalize(rotZMat * rotYMat * rotXMat * glm::vec4(-eyePosition, 0.0f));

	myPLY->computeFrontFace(curLookVector);

	//allow for user controlled rotation
	glRotatef(rotX, 1.0, 0.0, 0.0);
	glRotatef(rotY, 0.0, 1.0, 0.0);
	glRotatef(rotZ, 0.0, 0.0, 1.0);

	//draw the axes
	glLineWidth(1);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0, 0, 0); glVertex3f(1.0, 0, 0);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0, 0, 0); glVertex3f(0.0, 1.0, 0);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0, 0, 0); glVertex3f(0, 0, 1.0);
	glEnd();

	if (filled) {
		glEnable(GL_LIGHTING);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glColor3f(0.6, 0.6, 0.6);
		glPolygonMode(GL_FRONT, GL_FILL);
		myPLY->render(frontvBackFace);
	}

	if (wireframe) {
		glDisable(GL_LIGHTING);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glColor3f(1.0, 1.0, 0.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		myPLY->render();
		glEnable(GL_LIGHTING);
	}

	if (showNormal) {
		glDisable(GL_LIGHTING);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		myPLY->renderNormal();
		glEnable(GL_LIGHTING);
	}

	if (silhouette) {
		glDisable(GL_LIGHTING);
		glColor3f(1.0, 1.0, 1.0);
		glLineWidth(2);
		myPLY->renderSilhouette(curLookVector);
		glEnable(GL_LIGHTING);
	}
	//no need to call swap_buffer as it is automatically called
}

int MyGLCanvas::handle(int e) {
	//printf("Event was %s (%d)\n", fl_eventnames[e], e);
	switch (e) {
	case FL_ENTER: cursor(FL_CURSOR_HAND); break;
	case FL_LEAVE: cursor(FL_CURSOR_DEFAULT); break;
	}

	return Fl_Gl_Window::handle(e);
}

void MyGLCanvas::resize(int x, int y, int w, int h) {
	Fl_Gl_Window::resize(x, y, w, h);
	puts("resize called");
}

void MyGLCanvas::updateCamera(int width, int height) {
	float xy_aspect;
	xy_aspect = (float)width / (float)height;
	// Determine if we are modifying the camera(GL_PROJECITON) matrix(which is our viewing volume)
		// Otherwise we could modify the object transormations in our world with GL_MODELVIEW
	glMatrixMode(GL_PROJECTION);
	// Reset the Projection matrix to an identity matrix
	glLoadIdentity();
	gluPerspective(45.0f, xy_aspect, 0.1f, 10.0f);
	gluLookAt(eyePosition.x, eyePosition.y, eyePosition.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

