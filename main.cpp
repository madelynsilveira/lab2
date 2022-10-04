/*  =================== File Information =================
    File Name: main.cpp
    Description:
    Author: Michael Shah

    Purpose: Driver for 3D program to load .ply models
    Usage:
    ===================================================== */

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Window.H>
#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/glut.h>
#include <FL/names.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>

#include "MyGLCanvas.h"

using namespace std;


class MyAppWindow : public Fl_Window {
public:
    // slider widgets for rotation
    Fl_Slider *rotXSlider;
    Fl_Slider *rotYSlider;
    Fl_Slider *rotZSlider;

    // slider widgets for color
    Fl_Slider *redSlider;
    Fl_Slider *greenSlider;
    Fl_Slider *blueSlider;

    Fl_Button  *wireButton;
    Fl_Button  *fillButton;
    Fl_Button  *normalButton;
    Fl_Button  *debugFaceButton;
    Fl_Button  *silhouetteButton;
    Fl_Button  *openFileButton;
    MyGLCanvas *canvas;

public:
    // APP WINDOW CONSTRUCTOR
    MyAppWindow(int W, int H, const char *L = 0);

    static void idleCB(void *data) {
        MyAppWindow *win = (MyAppWindow *)data;
        win->canvas->redraw();
    }

private:
    // Someone changed one of the sliders
    static void rotateCB(Fl_Widget *w, void *userdata) {
        int value          = ((Fl_Slider *)w)->value();
        *((int *)userdata) = value;
    }

    static void buttonIntCB(Fl_Widget *w, void *userdata) {
        int value          = ((Fl_Button *)w)->value();
        *((int *)userdata) = value;
    }

    static void loadFileCB(Fl_Widget *w, void *data) {
        MyAppWindow    *win = (MyAppWindow *)data;
        Fl_File_Chooser G_chooser("", "", Fl_File_Chooser::MULTI, "");
        G_chooser.show();
        // Block until user picks something.
        //     (The other way to do this is to use a callback())
        //
        G_chooser.directory("./data");
        while (G_chooser.shown()) {
            Fl::wait();
        }

        // Print the results
        if (G_chooser.value() == NULL) {
            printf("User cancelled file chooser\n");
            return;
        }

        cout << "Loading new ply file from: " << G_chooser.value() << endl;
        // Reload our model
        win->canvas->myPLY->reload(G_chooser.value());
        // Print out the attributes
        win->canvas->myPLY->printAttributes();

        win->canvas->redraw();
    }
};


MyAppWindow::MyAppWindow(int W, int H, const char *L) : Fl_Window(W, H, L) {
    begin();
    // OpenGL window

    canvas = new MyGLCanvas(10, 10, w() - 110, h() - 20);

    Fl_Pack *overallPack = new Fl_Pack(w() - 100, 30, 100, h(), "");
    overallPack->box(FL_DOWN_FRAME);
    overallPack->labelfont(1);
    overallPack->type(Fl_Pack::VERTICAL);
    overallPack->spacing(30);
    overallPack->begin();


    Fl_Pack *pack = new Fl_Pack(w() - 100, 30, 100, h(), "Control Panel");
    pack->box(FL_DOWN_FRAME);
    pack->labelfont(1);
    pack->type(Fl_Pack::VERTICAL);
    pack->spacing(0);
    pack->begin();

    openFileButton = new Fl_Button(0, 100, pack->w() - 20, 20, "Load File");
    openFileButton->callback(loadFileCB, (void *)this);

    wireButton = new Fl_Check_Button(0, 100, pack->w() - 20, 20, "Wireframe");
    wireButton->callback(buttonIntCB, (void *)(&canvas->wireframe));
    wireButton->value(canvas->wireframe);

    fillButton = new Fl_Check_Button(0, 100, pack->w() - 20, 20, "Fill");
    fillButton->callback(buttonIntCB, (void *)(&canvas->filled));
    fillButton->value(canvas->filled);

    normalButton =
        new Fl_Check_Button(0, 100, pack->w() - 20, 20, "Draw Normal");
    normalButton->callback(buttonIntCB, (void *)(&canvas->showNormal));
    normalButton->value(canvas->showNormal);

    debugFaceButton =
        new Fl_Check_Button(0, 100, pack->w() - 20, 20, "Front v. Back Face");
    debugFaceButton->callback(buttonIntCB, (void *)(&canvas->frontvBackFace));
    debugFaceButton->value(canvas->frontvBackFace);

    silhouetteButton =
        new Fl_Check_Button(0, 100, pack->w() - 20, 20, "Silhouette");
    silhouetteButton->callback(buttonIntCB, (void *)(&canvas->silhouette));
    silhouetteButton->value(canvas->silhouette);


    // slider for controlling rotation
    Fl_Box *rotXTextbox = new Fl_Box(0, 0, pack->w() - 20, 20, "RotateX");
    rotXSlider          = new Fl_Value_Slider(0, 0, pack->w() - 20, 20, "");
    rotXSlider->align(FL_ALIGN_TOP);
    rotXSlider->type(FL_HOR_SLIDER);
    rotXSlider->bounds(-359, 359);
    rotXSlider->step(1);
    rotXSlider->value(canvas->rotX);
    rotXSlider->callback(rotateCB, (void *)(&(canvas->rotX)));

    Fl_Box *rotYTextbox = new Fl_Box(0, 0, pack->w() - 20, 20, "RotateY");
    rotYSlider          = new Fl_Value_Slider(0, 0, pack->w() - 20, 20, "");
    rotYSlider->align(FL_ALIGN_TOP);
    rotYSlider->type(FL_HOR_SLIDER);
    rotYSlider->bounds(-359, 359);
    rotYSlider->step(1);
    rotYSlider->value(canvas->rotY);
    rotYSlider->callback(rotateCB, (void *)(&(canvas->rotY)));

    Fl_Box *rotZTextbox = new Fl_Box(0, 0, pack->w() - 20, 20, "RotateZ");
    rotZSlider          = new Fl_Value_Slider(0, 0, pack->w() - 20, 20, "");
    rotZSlider->align(FL_ALIGN_TOP);
    rotZSlider->type(FL_HOR_SLIDER);
    rotZSlider->bounds(-359, 359);
    rotZSlider->step(1);
    rotZSlider->value(canvas->rotZ);
    rotZSlider->callback(rotateCB, (void *)(&(canvas->rotZ)));

    pack->end();


    // Fl_Pack* colorPack = new Fl_Pack(w() - 100, 30, 100, h(), "Color Panel");
    // colorPack->box(FL_DOWN_FRAME);
    // colorPack->labelfont(1);
    // colorPack->type(Fl_Pack::VERTICAL);
    // colorPack->spacing(0);
    // colorPack->begin();
    ////color control
    // Fl_Box *redTextbox = new Fl_Box(0, 0, pack->w() - 20, 20, "Red");
    // redSlider = new Fl_Value_Slider(0, 0, pack->w() - 20, 20, "");
    // redSlider->align(FL_ALIGN_TOP);
    // redSlider->type(FL_HOR_SLIDER);
    // redSlider->bounds(0, 1);
    // redSlider->value(canvas->red);
    // redSlider->callback(colorCB, (void*)(&(canvas->red)));

    // Fl_Box *greenTextbox = new Fl_Box(0, 0, pack->w() - 20, 20, "Green");
    // greenSlider = new Fl_Value_Slider(0, 0, pack->w() - 20, 20, "");
    // greenSlider->align(FL_ALIGN_TOP);
    // greenSlider->type(FL_HOR_SLIDER);
    // greenSlider->bounds(0, 1);
    // greenSlider->value(canvas->green);
    // greenSlider->callback(colorCB, (void*)(&(canvas->green)));

    // Fl_Box *blueTextbox = new Fl_Box(0, 0, pack->w() - 20, 20, "Blue");
    // blueSlider = new Fl_Value_Slider(0, 0, pack->w() - 20, 20, "");
    // blueSlider->align(FL_ALIGN_TOP);
    // blueSlider->type(FL_HOR_SLIDER);
    // blueSlider->bounds(0, 1);
    // blueSlider->value(canvas->blue);
    // blueSlider->callback(colorCB, (void*)(&(canvas->blue)));


    // colorPack->end();
    overallPack->end();

    end();

    resizable(this);
    Fl::add_idle((Fl_Idle_Handler)(void *)idleCB, (void *)this);
}


/**************************************** main() ********************/
int main(int argc, char **argv) {
    MyAppWindow win(600, 500, "User Interface");
    win.show();
    return (Fl::run());
}