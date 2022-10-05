/*  =================== File Information =================
  File Name: ply.cpp
  Description: Loads a .ply file and renders it on screen.
		New to this version: also renders the silhouette!
  Author: Paul Nixon
  ===================================================== */
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <FL/gl.h>
#include "ply.h"
#include "geometry.h"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>


using namespace std;

/*  ===============================================
	  Desc: Default constructor for a ply object
	  Precondition: _filePath is set to a valid filesystem location
			which contains a valid .ply file (triangles only)
	  Postcondition: vertexList, faceList are filled in
	=============================================== */
ply::ply() {
	vertexList = NULL;
	faceList = NULL;
	edgeList = NULL;
	properties = 0;
	vertexCount = 0;
	faceCount = 0;
	edgeCount = 0;
	// Call helper function to load geometry
	//loadGeometry();
}

/*  ===============================================
	  Desc: Destructor for a ply object
	  Precondition: Memory has been already allocated
	  =============================================== */
ply::~ply() {
	deconstruct();
}

void ply::deconstruct() {
	int i, j;

	for (i = 0; i < vertexCount; i++) {
		delete vertexList[i];
	}
	if (vertexList)
		delete[] vertexList;
	// Delete the allocated arrays

	for (i = 0; i < faceCount; i++) {
		delete faceList[i];
	}
	if (faceList)
		delete[] faceList;

	for (i = 0; i < edgeCount; i++) {
		delete edgeList[i];
	}
	if (edgeList)
		delete[] edgeList;

	// Set pointers to NULL
	vertexList = NULL;
	faceList = NULL;
	edgeList = NULL;
	properties = 0;
	vertexCount = 0;
	faceCount = 0;
	edgeCount = 0;
}

/*  ===============================================
	  Desc: reloads the geometry for a 3D object
			(or loads a different file)
	=============================================== */
void ply::reload(string _filePath) {

	filePath = _filePath;
	deconstruct();
	// Call our function again to load new vertex and face information.
	loadGeometry();
}
/*  ===============================================
	  Desc: Loads the data structures (look at geometry.h and ply.h)
	  Precondition: filePath is something valid, arrays are NULL
	  Postcondition: data structures are filled
		  (including edgeList, this calls scaleAndCenter and findEdges)
	  =============================================== */
void ply::loadGeometry() {

	/* You will implement this section of code
		  1. Parse the header
		  2.) Update any private or helper variables in the ply.h private section
		  3.) allocate memory for the vertexList
			  3a.) Populate vertices
		  4.) allocate memory for the faceList
			  4a.) Populate faceList
	*/


	ifstream myfile(filePath.c_str()); // load the file
	if (myfile.is_open()) { // if the file is accessable
		properties = -2; // set the properties because there are extras labeled

		string line;
		char * token_pointer;
		char * lineCopy = new char[256];
		int count;
		bool reading_header = true;
		// loop for reading the header 
		while (reading_header && getline(myfile, line)) {

			// get the first token in the line, this will determine which
			// action to take. 
			strcpy(lineCopy, line.c_str());
			token_pointer = strtok(lineCopy, " \r");
			// case when the element label is spotted:
			if (strcmp(token_pointer, "element") == 0) {
				token_pointer = strtok(NULL, " \r");

				// When the vertex token is spotted read in the next token
				// and use it to set the vertexCount and initialize vertexList
				if (strcmp(token_pointer, "vertex") == 0) {
					token_pointer = strtok(NULL, " \r");
					vertexCount = atoi(token_pointer);
					vertexList = new vertex*[vertexCount];
				}

				// When the face label is spotted read in the next token and 
				// use it to set the faceCount and initialize faceList.
				if (strcmp(token_pointer, "face") == 0) {
					token_pointer = strtok(NULL, " \r");
					faceCount = atoi(token_pointer);
					faceList = new face*[faceCount];
				}
			}
			// if property label increment the number of properties.
			if (strcmp(token_pointer, "property") == 0) { properties++; }
			// if end_header break the header loop and move to reading vertices.
			if (strcmp(token_pointer, "end_header") == 0) { reading_header = false; }
		}

		// Read in exactly vertexCount number of lines after reading the header
		// and set the appropriate vertex in the vertexList.
		for (int i = 0; i < vertexCount; i++) {
			vertexList[i] = new vertex();

			getline(myfile, line);
			strcpy(lineCopy, line.c_str());

			// depending on how many properties there are set that number of 
			// elements (x, y, z, confidence, intensity, r, g, b) (max 7) with
			// the input given
			if (properties >= 0) {
				vertexList[i]->position.x = atof(strtok(lineCopy, " \r"));
			}
			if (properties >= 1) {
				vertexList[i]->position.y = atof(strtok(NULL, " \r"));
			}
			if (properties >= 2) {
				vertexList[i]->position.z = atof(strtok(NULL, " \r"));
			}
		}

		// Read in the faces (exactly faceCount number of lines) and set the 
		// appropriate face in the faceList
		for (int i = 0; i < faceCount; i++) {
			getline(myfile, line);

			strcpy(lineCopy, line.c_str());
			count = atoi(strtok(lineCopy, " \r"));

			faceList[i] = new face();
			//faceList[i]->vertexCount = count; // number of vertices stored 
			//faceList[i]->vertexList = new int[count]; // initialize the vertices

			// set the vertices from the input, reading only the number of 
			// vertices that are specified
			for (int j = 0; j < 3; j++) {
				faceList[i]->vertexList[j] = atoi(strtok(NULL, " \r"));
			}

		}
		delete(lineCopy);
	}
	// if the path is invalid, report then exit.
	else {
		cout << "cannot open file " << filePath.c_str() << "\n";
		exit(1);
	}

	myfile.close();
	scaleAndCenter();
	computeFaceNormals();
	findEdges();
};

void ply::computeFaceNormals() {
	int i;
	for (i = 0; i < faceCount; i++) {
		glm::vec3 v0Pos = vertexList[faceList[i]->vertexList[0]]->position;
		glm::vec3 v1Pos = vertexList[faceList[i]->vertexList[1]]->position;
		glm::vec3 v2Pos = vertexList[faceList[i]->vertexList[2]]->position;

		glm::vec3 v1v0 = glm::normalize(v1Pos - v0Pos);
		glm::vec3 v2v0 = glm::normalize(v2Pos - v0Pos);

		glm::vec3 normal = glm::normalize(glm::cross(v1v0, v2v0));

		faceList[i]->faceNormal = normal;
	}
}

/*  ===============================================
Desc: Moves all the geometry so that the object is centered at 0, 0, 0 and scaled to be between 0.5 and -0.5
Precondition: after all the vetices and faces have been loaded in
Postcondition: points have reasonable values
=============================================== */
void ply::scaleAndCenter() {
    glm::vec3 avrg(0.0f, 0.0f, 0.0f);
    float max = 0.0;
    int i, j;

    //loop through each vertex in the given image
    for (i = 0; i < vertexCount; i++) {
        // obtain the total for each property of the vertex
        avrg = avrg + vertexList[i]->position;
    }
    
    // compute the average for each property
    avrg = avrg / (float)vertexCount;
    
    // obtain the max dimension to find the furthest point from 0,0
    for (i = 0; i < vertexCount; i++) {
        vertexList[i]->position = vertexList[i]->position - avrg;
        for (j = 0; j < 3; j++) {
            if (max < fabs(vertexList[i]->position[j]))
            max = fabs(vertexList[i]->position[j]);
        }
    }

    max *= 2.0f;

    // center and scale each vertex 
    for (i = 0; i < vertexCount; i++) {
        vertexList[i]->position = (vertexList[i]->position) / max;
    }
}

/*  ===============================================
	  Desc: Draws a filled 3D object
	  Precondition: arrays are EITHER valid data OR NULL
	  Postcondition: no changes to data
	  Error Condition: If we haven't allocated memory for our
	  faceList or vertexList then do not attempt to render.
	=============================================== */
void ply::render(int frontvBackFace) {
	int i;
	if (vertexList == NULL || faceList == NULL) {
		return;
	}

	glPushMatrix();
	// For each of our faces
	glBegin(GL_TRIANGLES);
	for (i = 0; i < faceCount; i++) {
		int isFrontFace = faceList[i]->frontFace;
		glm::vec3 faceNormal = faceList[i]->faceNormal;

		glNormal3fv(glm::value_ptr(faceNormal));

		if (frontvBackFace == 1) {
			if (isFrontFace == 1) {
				glColor3f(0.0f, 1.0f, 0.0f);
			}
			else {
				glColor3f(1.0f, 0.0f, 0.0f);
			}
		}

		for (int j = 0; j < 3; j++) {
			// Get each vertices x,y,z and draw them
			int index = faceList[i]->vertexList[j];
			glVertex3fv(glm::value_ptr(vertexList[index]->position));
		}
	}
	glEnd();
}

void ply::renderNormal() {
	int i;
	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	for (i = 0; i < faceCount; i++) {
		glm::vec3 centroid(0.0f, 0.0f, 0.0f);

		for (int j = 0; j < 3; j++) {
			centroid = centroid + vertexList[faceList[i]->vertexList[j]]->position;
		}
		centroid = centroid / 3.0f;

		glm::vec3 faceNormal = faceList[i]->faceNormal;

		glm::vec3 lineEnd = centroid + faceNormal * 0.05f;

		glVertex3fv(glm::value_ptr(centroid));
		glVertex3fv(glm::value_ptr(lineEnd));
	}
	glEnd();

	glPopMatrix();
}

void ply::computeFrontFace(glm::vec3 lookVector) {
	//TODO: given the input lookVector, figure out which of the faces is front facing (fronFace == 1)    
    float dot_product;
    
    for (int i = 0; i < faceCount; i++) {
		dot_product = glm::dot(lookVector, faceList[i]->faceNormal);
        
        if (dot_product < 0) {faceList[i]->frontFace = 1;}
        else {faceList[i]->frontFace = 0;}
	}
}


//loads data structures so edges are known
void ply::findEdges() {
	//edges, if you want to use this data structure
	//TODO add all the edges to the edgeList and make sure they have both faces
	face *face1;
	face *face2;
    edge *old_edge;
    bool edge_already_exists;
    vector<edge*> edge_vector;

	// create a loop to go through each face in faceList
	for (int i = 0; i < faceCount; i++) {
		face1 = faceList[i];

		for (int j = i+1; j < faceCount-1; j++) {
			face2 = faceList[j];

			// find -> beginning of array, end of array, int to find, returns idx of int to find if found
			bool share0, share1, share2;
			share0 = find(begin(face1->vertexList), end(face1->vertexList), face2->vertexList[0]) != end(face1->vertexList);
			share1 = find(begin(face1->vertexList), end(face1->vertexList), face2->vertexList[1]) != end(face1->vertexList);
			share2 = find(begin(face1->vertexList), end(face1->vertexList), face2->vertexList[2]) != end(face1->vertexList);

			// faces share edge composed of face2->vertexList[0] and face2->vertexList[1]
			if (share0 && share1) {
				edge *new_edge = new edge();
				new_edge->vertices[0] = face2->vertexList[0];
				new_edge->vertices[1] = face2->vertexList[1];
				new_edge->faces[0] = i; // index of face1
				new_edge->faces[1] = j; // index of face2
				
				// check that edge does not exist in edgeList
                edge_already_exists = false;
				for (int k = 0; k < edgeCount; k++){
                    old_edge = edge_vector[k];

                    // call edge class equality function
                    if (old_edge->equals(*new_edge)) {
                        edge_already_exists = true;
                        break;
                    }
				}

                if (edge_already_exists) {delete new_edge;}
                else {
                    edge_vector.push_back(new_edge);
                    edgeCount++;
                }
			}

            // faces share edge composed of face2->vertexList[0] and face2->vertexList[2]
			if (share0 && share2) {
				edge *new_edge = new edge();
				new_edge->vertices[0] = face2->vertexList[0];
				new_edge->vertices[1] = face2->vertexList[2];
				new_edge->faces[0] = i; // index of face1
				new_edge->faces[1] = j; // index of face2
				
				// check that edge does not exist in edgeList
                edge_already_exists = false;
				for (int k = 0; k < edgeCount; k++){
                    old_edge = edge_vector[k];

                    // call edge class equality function
                    if (old_edge->equals(*new_edge)) {
                        edge_already_exists = true;
                        break;
                    }
				}

                if (edge_already_exists) {delete new_edge;}
                else {
                    edge_vector.push_back(new_edge);
                    edgeCount++;
                }
			}

            // faces share edge composed of face2->vertexList[1] and face2->vertexList[2]
			if (share1 && share2) {
				edge *new_edge = new edge();
				new_edge->vertices[0] = face2->vertexList[1];
				new_edge->vertices[1] = face2->vertexList[2];
				new_edge->faces[0] = i; // index of face1
				new_edge->faces[1] = j; // index of face2
				
				// check that edge does not exist in edgeList
                edge_already_exists = false;
				for (int k = 0; k < edgeCount; k++){
                    old_edge = edge_vector[k];

                    // call edge class equality function
                    if (old_edge->equals(*new_edge)) {
                        edge_already_exists = true;
                        break;
                    }
				}

                if (edge_already_exists) {delete new_edge;}
                else {
                    edge_vector.push_back(new_edge);
                    edgeCount++;
                }
			}
		}
	}
    
    // populate edgeList with all edges
    edgeList = new edge*[edgeCount];
    for(int i = 0; i < edgeCount; i++){
        edgeList[i] = edge_vector[i];
    }
}


/* Desc: Renders the silhouette
 * Precondition: Edges are known
 */
void ply::renderSilhouette(glm::vec3 lookVector) {
	//TODO Iterate through the edgeList, and if you want to draw an edge,
	//call glVertex3f once for each vertex in that edge.  
	glPushMatrix();
	glBegin(GL_LINES);

    for (int i = 0; i < edgeCount; i++) {
        // if frontFace values are not equal, they are either [1,0] or [0,1]
        // in either case, one face is front-facing and the other is back-facing, so we draw
        
        int face1_idx = edgeList[i]->faces[0];
        int face2_idx = edgeList[i]->faces[1];

        int face1_front = faceList[face1_idx]->frontFace;
        int face2_front = faceList[face2_idx]->frontFace;

        if (face1_front != face2_front) {
            vertex *vertex1 = vertexList[edgeList[i]->vertices[0]];
            vertex *vertex2 = vertexList[edgeList[i]->vertices[1]]; 
            glVertex3f(vertex1->position[0], vertex1->position[1], vertex1->position[2]);
            glVertex3f(vertex2->position[0], vertex2->position[1], vertex2->position[2]);
        }
    }

	glEnd();
	glPopMatrix();
}

/*  ===============================================
	  Desc: Prints some statistics about the file you have read in
	  This is useful for debugging information to see if we parse our file correctly.
	=============================================== */
void ply::printAttributes() {
	cout << "==== ply Mesh Attributes=====" << endl;
	cout << "vertex count:" << vertexCount << endl;
	cout << "face count:" << faceCount << endl;
	cout << "properties:" << properties << endl;
}

/*  ===============================================
	  Desc: Iterate through our array and print out each vertex.
	=============================================== */
void ply::printVertexList() {
	if (vertexList == NULL) {
		return;
	}
	else {
		for (int i = 0; i < vertexCount; i++) {
			cout << vertexList[i]->position.x << "," << vertexList[i]->position.y << "," << vertexList[i]->position.z << endl;
		}
	}
}

/*  ===============================================
	  Desc: Iterate through our array and print out each face.
	=============================================== */
void ply::printFaceList() {
	if (faceList == NULL) {
		return;
	}
	else {
		// For each of our faces
		for (int i = 0; i < faceCount; i++) {
			// Get the vertices that make up each face from the face list
			for (int j = 0; j < 3; j++) {
				// Print out the vertex
				int index = faceList[i]->vertexList[j];
				cout << vertexList[index]->position.x << "," << vertexList[index]->position.y << "," << vertexList[index]->position.z << endl;
			}
		}
	}
}
