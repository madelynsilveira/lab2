/*  =================== File Information =================
        File Name: ply.h
        Description:
        Author: Michael Shah

        Purpose:        Specification for using
        Examples:       See example below for using PLY class
        ===================================================== */
#ifndef PLY_H
#define PLY_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "geometry.h"

using namespace std;

/*  ============== ply ==============
        Purpose: Load a PLY File

        Note that the ply file inherits from a base class called 'entity'
        Some important data structures are described in geometry.h
        This class stores common transformations that can be applied to 3D entities(such as mesh files, lights, or cameras)

        Example usage: 
        1.) ply* myPLY = new ply (filenamePath);
        2.) myPLY->render();
        3.) delete myPLY;
        ==================================== */ 
class ply {

        public:
                /*      ===============================================
                        Desc: Default constructor for a ply object
                        =============================================== */ 
                ply();

                /*      ===============================================
                        Desc: Destructor for a ply object
                        =============================================== */ 
                void deconstruct();
                ~ply();
                /*      ===============================================
                        Desc: reloads the geometry for a 3D object
                        (usually to see a new .ply file)
                =============================================== */ 
                void reload(string _filePath);
                /*      ===============================================
                        Desc: Draws a filled 3D object
                =============================================== */  
				void render(int frontvBackFace=0);
				void renderNormal();
				//iterates through the geometry to fill in the edgeList
                //draws the silhouette around the ply object
                void renderSilhouette(glm::vec3 lookVector);

				void computeFrontFace(glm::vec3 lookVector);

                /*      ===============================================
                        Desc: Prints some statistics about the file you have read in
                =============================================== */  
                void printAttributes();
                /*  ===============================================
                        Desc: Helper function for you to debug if 
                        you are reading in the correct data.
                        (Generally these would not be public functions,
                        they are here to help you understand the interface)
                        =============================================== */
                void printVertexList();
                void printFaceList();
                
        private:
                /*      ===============================================
                        Desc: Helper function used in the constructor
                        =============================================== */ 
			void findEdges();
			void loadGeometry();
			void computeFaceNormals();
            //makes the points fit in the window
            void scaleAndCenter();

                /*      ===============================================
                        Data
                        These variables are useful to store information
                        about the mesh we are loading.  Often these values
                        are stored in the header, or can be useful for
                        debugging.
                        =============================================== */
                // Store the path to our file
                string filePath;
                // Stores the number of vertics loaded
                int vertexCount;
                // Stores the number of faces loaded
                int faceCount;
				// Stores the number of edges loaded
				int edgeCount;
				// Tells us how many properites exist in the file
                int properties;
                // A dynamically allocated array that stores
                // vertices
                vertex** vertexList;

				// A dynamically allocated array that stores
                // a list of faces                 
				face** faceList;
                //an array of linked lists representing edges
                edge** edgeList;
};

#endif