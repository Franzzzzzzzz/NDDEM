#include "TinyPngOut.hpp"
#ifdef NRRDIO
#ifndef NRRDIOINCLUDE
#define NRRDIOINCLUDE
#include "../CoarseGraining/NrrdIO-1.11.0-src/NrrdIO.h"
#endif
#endif
#include <string>
#include <vector>

#ifndef TOOLS
#include "Tools.h"
#endif

#include <fstream>

using namespace std ;

int write_colormap_vtk (int d, vector<vector<float>> & colors) ;
int write_NrrdIO (string path, int d, vector<vector<float>> & colors) ;
int write_img (char path[], int w, int h, uint8_t * px) ;
int csvread_A (const char path[], v2d &result, int d) ;
int csvread_XR (const char path[], v2d & result, v1d &R, int d) ;
void filepathname (char * path, int n, int time, cv1d View) ;

// External function
extern void phi2color (vector<uint8_t>::iterator px, cv1d & phi, int d, vector<vector<float>> & colors) ;

extern v1d lambdagrid, thetagrid ;
extern string DirectorySave ;
extern uint d ; extern int N ;
extern v2d Boundaries ;
