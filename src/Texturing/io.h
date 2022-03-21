/** \addtogroup Texturing Texturing server
 * This module handles the Texturing of higher dimensional rotating hyperspheres. It is designed to produced 2D textures that can be wrapped around spheres for visualisations. 
 *  @{ */

#include "TinyPngOut.hpp"
#ifdef NRRDIO
#ifndef NRRDIOINCLUDE
#define NRRDIOINCLUDE
#include "../NrrdIO-1.11.0-src/NrrdIO.h"
#endif
#endif
#include <string>
#include <vector>

#ifndef TOOLS
#include "Tools.h"
#endif

#include <fstream>

using namespace std ;

int write_colormap_vtk (int d, vector<vector<float>> & colors) ; ///< Writer for VTK colormaps
int write_NrrdIO (string path, int d, vector<vector<float>> & colors) ; ///< Writer for NRRD colormaps
int write_img (char path[], int w, int h, uint8_t * px) ; ///< Write for png textures
int csvread_A (const char path[], v2d &result, int d) ; ///< Reader for CSV orientation informations
int csvread_XR (const char path[], v2d & result, v1d &R, int d) ; ///< Reader for CSV location informations
void filepathname (char * path, int n, int time, cv1d View) ; ///< Filename generator

// External function
extern void phi2color (vector<uint8_t>::iterator px, cv1d & phi, int d, vector<vector<float>> & colors) ;

extern v1d lambdagrid, thetagrid ;
extern string DirectorySave ;
extern uint d ; extern int N ;
extern v2d Boundaries ;


/** @} */
