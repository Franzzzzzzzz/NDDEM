/** \addtogroup Texturing Texturing server
 *
 * It acts as an http server receiving calls from the visualation side, and producing textures as individual png files per particle/location/timestep. Unknown commands are trying to provide the requested file. <br>
 * Default port is 54321 <br>
 * Example: http://localhost:54321/load?path=XXXX&resolution=5<br>
 * Example: http://localhost:54321/render?ts=XXX&x4=2.5&5=3.2 <br>
 *
 * More commands are available, cf. the main() function.
 *
 *
 *  @{ */


#include "Tools.h"
#include "io.h"

#include <thread>
#include <regex>
#include <limits>
#include <filesystem>

extern bool blenderrender ;

#define DeltaX 0.1 ///< Step in location for the visualisation \warning ad-hoc \todo should be runtime value.
#define FilePerLine 100 ///< Line size for single file tiled output \deprecated not really used at the moment.

void phi2color (vector<uint8_t>::iterator px, cv1d & phi, int d, vector<vector<float>> & colors) ; ///< Convert from hyperspherical coordinates to actual pixel color, using the provided vector of colors.
using namespace std ;

/** \brief Individual timestep data */
class Timestep {
public:
    v2d X ; ///< Particle locations
    v2d A ; ///< Particle attached frame
} ;

/** \brief Handles all the computation for the textures */
template <int d>
class Texturing {
public :
  int Nlambda=32 ; ///< Resolution in latitude
  int Ntheta=32 ; ///< Resolution in longitude
  vector<vector<float>> colors ; ///< Colors vector for the different dimensions
  const vector<vector<float>> allcolorslist = {
      {1,0,0},
      {0,1,0},
      {0,0,1},
      {1,1,0},
      {0,1,1},
      {1,0,1}} ; ///< Default colors

  const vector<vector<vector<float>>> allcolors = {
      {{231./256., 37./256., 100./256.}}, // official NDDEM pink
      {{1,1,0},{0,1,1}}}; ///< Default colors

  v1d lambdagrid ; ///< Pixel grid in latitude
  v1d thetagrid ; ///< Pixel grid in longitude
  #ifdef TEXTURINGPATH
  string BasePath = TEXTURINGPATH "/../Samples/" ; ///< Default directory for simulation results
  string DirectorySave = TEXTURINGPATH "/../Textures/" ; ///< Default directory for saving the resulting textures
  #else
  string BasePath = "../Samples/" ; ///< Default directory for simulation results
  string DirectorySave = "../Textures/" ; ///< Default directory for saving the resulting textures
  #endif
  int N ; ///< Number of grains
  v2d Boundaries ; ///< List of boundaries

  vector <Timestep> Ts ; ///< List of Timestep
  vector <int> TsName ; ///< Actual integer timestep
  v1d R ; ///< Particle radii
  v1d View ; ///< Location of the rendering in the ND space
  vector <int> ViewPoint ; ///< Current location&timestep for rendering
  vector <int> NewViewPoint ; ///< Used if the location&timestep require a modification. Eventually transfered to ViewPoint when the rendering has been processed.
  vector <int> RenderedAlready ; ///< Keeps track of what location/timesteps have already been rendered and do not need to be recomputed.
  bool runfast ; ///< Render only the current location&timestep, not doing any additional rendering for caching purposes.
  bool singlerendered ; ///< Render a single location&timestep

  bool justloaded ; ///< Keep track if no render happened yet (used for the 3D simulation)
  int nrotate ; ///< \deprecated used to keep track if the rendered view is not using the first 3 dimensions. Effectively not used currently as this feature is not implemented in the visualisation side. \todo Implement in the visualisation the possible of rendering other dimensions than 1-2-3
  vector<vector<string>> FileList ; ///< Data file list
  vector <std::thread> Threads; ///< Rendering thread list
  const bool singlefiles = true ; ///< \deprecated Render the textures as a tilemap instead of individual files. \todo implement tile rendering in the visualisation.

  // function
  int initialise (map <string,string> args) ; ///< Loads all the data for the requested simulation in memory \warning No check is made that they fit in memory, be careful for large datasets
  int clean() ; ///< Clean the data to get ready for another simulation rendering
  int set_grid (int nb) ; ///< Set the grid in latitude-longitude
  void spaceloop (v1d View, uint tsint, int nrotate, int dim) ; ///< Run all the rendering in the current location, at all timesteps
  void timeloop  (v1d View, uint tsint, int nrotate) ; ///< Run all the rendering for the curent timestep, for all other location in dimensions higher than 3D view (ie. 4th dim, 5dim etc), varying only 1 dimension at a time.
  void hereandnow(v1d View, uint tsint, int nrotate) ; ///< Render the current location at the current timestep
  int MasterRender() ; ///< Handle the rendering threads
  int SetNewViewPoint (map <string,string>  args) ; ///< Modify the current location and/or timestep en render.
  bool isrendered () ; ///< Verify if the current location/timestep is already rendered
  void Render (vector <string> & filerendered, cv1d & View, int nrotate, int time, cv2d &X, cv1d & R, cv2d &A) ; ///< Do the actual rendering of the textures
  int viewpermute (v1d & View) ; ///< Rotate the viewpoint so that the rendered view is using the first 3 dimensions. Effectively unused since the visualisation only handles the rendring of the first 3 dimensions currently.
  void rescale (v1f & c, cv1f sum) ; ///< Coloring rescaling
  void filepathname (char * path, int n, int time, cv1d & View); ///< Generates the proper texture name
  void filepathname (char * path, int time, cv1d & View); ///< Generates the proper texture name

  int write_vtkmap (map <string,string> args) ; ///< Outputs the texture as a vtk surface
  int write_colormap_vtk_base () ; ///< Output the colormap as a vtk volume \warning (only working for 3D)
  int write_colormap_nrrd_base (map <string,string> args) ; ///< Output the texture as an NRRD file

} ;


/*****************************************************************************************************
 *                                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 * IMPLEMENTATIONS                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 * ***************************************************************************************************/
inline void dispvector (const v1d & a) {for (auto v: a) printf("%g ", v); printf("\n") ; fflush(stdout) ; } ///< Convenient function to print a vector on screen
inline void dispvector (const v1f & a) {for (auto v: a) printf("%g ", v); printf("\n") ; fflush(stdout) ; } ///< Convenient function to print a vector on screen
inline void dispvector (const vector<int> & a) {for (auto v: a) printf("%d ", v); printf("\n") ; fflush(stdout) ; } ///< Convenient function to print a vector on screen

template <int d>
void runthread_timeloop (Texturing<d> * T, v1d View, uint tsint, int nrotate)
{T->timeloop(View, tsint,nrotate) ; } ///< Function to call Texturing::timeloop()

template <int d>
void runthread_spaceloop (Texturing<d> * T, v1d View, uint tsint, int nrotate, int dim)
{T->spaceloop(View, tsint,nrotate, dim) ; } ///< Function to call Texturing::spaceloop()
//=========================================================================

template <int d>
int Texturing<d>::initialise (map <string,string> args)
{
 Tools<d>::initialise() ;
 printf("{%s}", DirectorySave.c_str()) ;
 string Directory=args["path"] ;
 //DirectorySave= args["texturepath"] ;
 set_grid (atoi(args["resolution"].c_str())) ;

 //Get all the relevent files in the Directory, sort them and identify the timesteps
 std::filesystem::directory_iterator D(BasePath + Directory) ;
 vector <string> tmpfilelst ;
 vector <std::pair<int,string>> filelistloc, filelistA ;
 for (auto& p : std::filesystem::directory_iterator(BasePath+Directory)) tmpfilelst.push_back(p.path().string()) ;
 regex exprloc{".*dump-([0-9]+).csv"};
 regex exprA{".*dumpA-([0-9]+).csv"};
 smatch what;
 for (auto &a : tmpfilelst)
 {
     if (regex_match(a, what, exprloc)) filelistloc.push_back(make_pair(stoi(what[1].str()), a)) ;
     if (regex_match(a, what, exprA)) filelistA.push_back(make_pair(stoi(what[1].str()), a)) ;
 }
 std::sort(filelistloc.begin(), filelistloc.end()) ;
 std::sort(filelistA.begin()  , filelistA.end()  ) ;
 TsName.resize(filelistloc.size(), 0) ;
 for (uint i=0 ; i<filelistloc.size() ; i++) TsName[i]=filelistloc[i].first ;

 // Let's read everything
 Ts.resize(filelistloc.size()) ;
 for (uint i=0 ; i<filelistloc.size() ; i++)
  {
    R.clear() ;
    csvread_XR (filelistloc[i].second.c_str(), Ts[i].X, R, d) ;
    printf("%d ", i) ; fflush(stdout) ;
  }
 for (uint i=0 ; i<filelistA.size() ; i++) csvread_A(filelistA[i].second.c_str(), Ts[i].A, d) ;
 N = Ts[0].X.size() ;
 int nrotate=3 ; // Rotate all the coordinates already
 for (auto & v : Ts)
 {
    for (int i=0 ; i<N ; i++)
    {
        rotate(v.X[i].begin(), v.X[i].begin()+nrotate, v.X[i].end()) ;
        Tools<d>::transpose_inplace(v.A[i]) ;
    }
 }
 Boundaries.resize(2) ;
 Boundaries[0].resize(d-3, INT_MAX) ;
 Boundaries[1].resize(d-3, INT_MIN) ;
 for (auto &v : Ts)
  for (auto &w : v.X)
  {
    for (uint i=0 ; i<d-3 ; i++)
    {
      if (w[i]<Boundaries[0][i]) Boundaries[0][i] = w[i] ;
      if (w[i]>Boundaries[1][i]) Boundaries[1][i] = w[i] ;
    }
  }
 auto tmpbound = max_element(R.begin(), R.end()) ;
 for (uint i=0 ; i<d-3 ; i++) {Boundaries[0][i] -= (*tmpbound) ;  Boundaries[1][i] += (*tmpbound) ; }

 // Color gradient initialisation
 if (static_cast<uint>(d-1)>allcolorslist.size()) printf("ERR: not enough color gradients!!\n") ;
 //if (d-3<allcolors.size()) colors=allcolors[d-3] ; //TODO
 //else
     colors=allcolorslist ;

 View.resize(d, 0) ;
 printf("[[[%ld]]]", View.size()) ; fflush(stdout) ; 
 RenderedAlready.resize(2*(d-3+1), 0) ;
 ViewPoint.resize(d-3+1, INT_MIN) ;
 NewViewPoint.resize(d-3+1, 0) ;
 FileList.resize(d-3+1) ;
 Threads.resize(d-3+1) ;
 justloaded=true ;
 singlerendered=false ;
 return 0 ;
}

//==================================================
template <int d>
int Texturing<d>::clean()
{
colors.clear() ;
lambdagrid.clear() ;
thetagrid.clear() ;

Boundaries.clear() ;
TsName.clear() ;
R.clear() ;
View.clear() ;
ViewPoint.clear() ;
NewViewPoint.clear() ;

for (auto & Thr : Threads)
    if (Thr.joinable())
    {
        auto ThrID = Thr.native_handle() ;
        pthread_cancel(ThrID);
        Thr.join() ;
    }

Threads.clear();
Tools<d>::clear() ;
vector<Timestep>().swap(Ts) ;
Boundaries.clear() ;
ViewPoint.clear() ;
NewViewPoint.clear() ;
View.clear() ;
RenderedAlready.clear() ;
for (auto & u : FileList)
    for (auto & v: u)
        std::filesystem::remove(v.c_str()) ;

return 0 ;
}
//=================================================
template <int d>
int Texturing<d>::SetNewViewPoint (map <string,string>  args)
{
  printf("|%s|", args["ts"].c_str()) ; fflush(stdout) ;
  int Time=atoi(args["ts"].c_str()) ;
  printf("H") ; fflush(stdout) ;
  char dimstr[10] ;
  printf("[]%ld[]", View.size() ) ; fflush(stdout) ;
  View[0]=View[1]=View[2]=NAN ;
  printf("E") ; fflush(stdout) ;
  for (uint dd=3 ; dd<d ; dd++)
  {
    sprintf(dimstr, "x%d", dd+1) ;
    View[dd]=atof(args[dimstr].c_str()) ;
  }
  printf("F") ; fflush(stdout) ;
  nrotate = viewpermute (View) ;
  if (nrotate != 3 && d>3) printf("WHAT? No the first 3D should be NaNs ...") ;
  for (uint i=0 ; i<d-3 ; i++) {NewViewPoint[i] = static_cast<int>(round(View[i]/DeltaX));}
printf("B") ; fflush(stdout) ;
  auto tmp  = find(TsName.begin(), TsName.end(), Time) ;
  if (tmp == TsName.end()) {printf("WARN: not found timestep\n") ; return -1 ; }
  NewViewPoint[d-3] = tmp-TsName.begin() ;
printf("C") ; fflush(stdout) ;
  if (atoi(args["running"].c_str())==1) runfast=true ;
  else runfast=false ;
  printf("D") ; fflush(stdout) ;
  return 0 ;
}
//=====================================================
template <int d>
bool Texturing<d>::isrendered ()
{
  if (singlerendered) {singlerendered=false ; return true ; }

  for (uint i=0 ; i<d-3 ; i++)
    if (NewViewPoint[i]<RenderedAlready[i*2] || NewViewPoint[i]>RenderedAlready[i*2+1])
      return false ;

  int t ;
  if (NewViewPoint[d-3]<RenderedAlready[(d-3)*2]) t = NewViewPoint[d-3] +Ts.size() ;
  else t= NewViewPoint[d-3] ;

  if (t>=RenderedAlready[(d-3)*2]+RenderedAlready[(d-3)*2+1]) return false ;

  return true ;
}


//=====================================================
template <int d>
int Texturing<d>::MasterRender()
{
 //printf("%d %d | %d %d\n", ViewPoint[0], ViewPoint[1], NewViewPoint[0], NewViewPoint[1]) ;
 // Alright, lets start the threads

 if (! isrendered() && runfast) {dispvector(RenderedAlready) ; hereandnow(View, NewViewPoint[d-3], nrotate) ; singlerendered=true ; }
 if (! runfast)
 {
    for (uint i=0 ; i<d-3 ; i++)
    {
        for (uint j=0 ; j<d-3+1 ; j++)
        {
            if (j==i) continue ;
            if (NewViewPoint[j] != ViewPoint[j])
            {
            if (Threads[i].joinable())
            {
                auto ThreadID = Threads[i].native_handle() ;
                pthread_cancel(ThreadID);
                Threads[i].join() ;
            }
            dispvector(NewViewPoint) ;
            Threads[i] = std::thread(runthread_spaceloop<d>, this, View, NewViewPoint[d-3], nrotate, i) ;
            break ;
            }
        }
    }
    for (uint j=0 ; j<d-3 ; j++)
    {
    if (NewViewPoint[j] != ViewPoint[j])
        {
            if (Threads[d-3].joinable())
            {
            auto ThreadID = Threads[d-3].native_handle() ;
            pthread_cancel(ThreadID) ;
            Threads[d-3].join() ;
            }
        dispvector(NewViewPoint) ;
        Threads[d-3] = std::thread(runthread_timeloop<d>, this, View, NewViewPoint[d-3], nrotate) ;
        }
    }

    if (d==3 && justloaded)
    {
        Threads[d-3] = std::thread(runthread_timeloop<d>, this, View, NewViewPoint[d-3], nrotate) ;
        justloaded=false ;
    }
 }
ViewPoint=NewViewPoint ;
return 0 ;
}

//===========================================================
template <int d>
void Texturing<d>::spaceloop (v1d View, uint tsint, int nrotate, int dim)
{
//printf("S") ; fflush(stdout) ;
for (auto & v : FileList[dim]) std::filesystem::remove(v.c_str()) ;
FileList[dim].clear() ;

auto Viewdec = View ;
while (Viewdec[dim]>Boundaries[0][dim] || View[dim]<Boundaries[1][dim])
{
  Viewdec[dim] -= DeltaX ;
  View[dim] += DeltaX ;

  //printf("s") ; fflush(stdout) ;

  if (Viewdec[dim]>Boundaries[0][dim]) Render(FileList[dim], Viewdec, nrotate, TsName[tsint], Ts[tsint].X, R, Ts[tsint].A) ;
  RenderedAlready[2*dim] = Viewdec[dim] / DeltaX ;
  if (View[dim]<Boundaries[1][dim])    Render(FileList[dim], View,    nrotate, TsName[tsint], Ts[tsint].X, R, Ts[tsint].A) ;
  RenderedAlready[2*dim+1] = View[dim] / DeltaX ;
}
RenderedAlready[2*dim] = INT_MIN ;
RenderedAlready[2*dim+1] = INT_MAX ;
printf("Spaceloop is done") ; fflush(stdout) ;
}
//-----------------------------------------------------
template <int d>
void Texturing<d>::timeloop (v1d View, uint tsint, int nrotate)
{
int dim=FileList.size()-1 ;
for (auto & v : FileList[dim]) std::filesystem::remove(v.c_str()) ;

FileList[dim].clear() ;
uint timeidxinit=tsint ;
//if (tsint>=Ts.size()) tsint=0 ;
RenderedAlready[2*(d-3)] = tsint ;
RenderedAlready[2*(d-3)+1] = 0;
do {
Render(FileList[dim],View, nrotate, TsName[tsint], Ts[tsint].X, R, Ts[tsint].A) ;
printf("%d ", tsint) ; fflush(stdout) ;
tsint++ ;
printf("%d ", tsint) ; fflush(stdout) ;
RenderedAlready[2*(d-3)+1] ++ ;
if (tsint>=Ts.size()) tsint=0 ;
} while (tsint != timeidxinit) ;
printf("Timeloop is done") ; fflush(stdout) ;
}
//-----------------------------------------------------
template <int d>
void Texturing<d>::hereandnow (v1d View, uint tsint, int nrotate)
{
int dim=FileList.size()-1 ;
for (auto & v : FileList[dim]) std::filesystem::remove(v.c_str()) ;

FileList[dim].clear() ;
//uint timeidxinit=tsint ;
Render(FileList[dim],View, nrotate, TsName[tsint], Ts[tsint].X, R, Ts[tsint].A) ;
}
//-----------------------------------------------------
template <int d>
void Texturing<d>::Render (vector <string> & filerendered, cv1d & View, int nrotate, int time, cv2d &X, cv1d & R, cv2d &A)
{
v1d sp (d,0) ; v1d spturned (d,0) ; // Surface point (point on the surface of the sphere)
v1d phi (d-1,0), phinew(d-1,0) ; // Angles of the hyperspherical coordinates. All angles between 0 and pi, except the last one between 0 and 2pi
vector<uint8_t> img ; int stride ;
int imgx0=0 ;
if (singlefiles)
{
    img.resize(Nlambda*Ntheta*3,0) ;
    stride=Ntheta ;
}
else
{
    img.resize( Ntheta * FilePerLine * (N/FilePerLine+1)* Nlambda*3,0) ;
    stride = Nlambda * FilePerLine ;
}

for (int i=0 ; i<N ; i++)
{
     // Check if we are in view
     double rsqr = R[i]*R[i] ;
     //rotate(X[i].begin(), X[i].begin()+nrotate, X[i].end()) ;
     for (uint j=0 ; j<d-3 ; j++)
         rsqr -= (View[j]-X[i][j])*(View[j]-X[i][j]) ;
     if (rsqr<=0)
     {
       //char path[5000] ;
       //sprintf(path, "%s/Texture-%d-%d.png", Directory.c_str(), TimeCur->first, i) ;
       //experimental::filesystem::remove(path) ;
       continue ;
     }

     // We are in view, let's get to it let's get the first phi's (constants)
     for (uint j=0 ; j<d-3 ; j++)
     {
       double cosine = (View[j]-X[i][j])/R[i] ;
       for (uint k=0 ; k<j ; k++)
         cosine /= sin(phi[k]) ;
       phi[j] = acos(cosine) ;
     }


     for (int j = 0 ; j<Nlambda ; j++)
         for (int k=0 ; k<Ntheta ; k++)
         {
             // Finalising the phi array (useless, but just because
             phi[d-3]=lambdagrid[j] ;
             phi[d-2]=thetagrid[k] ;

             for (uint dd=0 ; dd<d-3 ; dd++) {sp[dd]=View[dd]-X[i][dd] ;}
             //sp = View-X[i] ; // All the dimensions except the last 3 are now correct
             sp[d-3] = R[i] ;
             for (uint j=0 ; j<d-3 ; j++) sp[d-3] *= sin(phi[j]) ;
             sp[d-2]=sp[d-3] ; sp[d-1]=sp[d-3] ;

             //********** The mathematical version (logical):
             //sp[d-3] *= cos(phi[d-3]) ;
             //sp[d-2] *= sin(phi[d-3])*cos(phi[d-2]) ;
             //sp[d-1] *= sin(phi[d-3])*sin(phi[d-2]) ;

             if (blenderrender)
             {
                 //********** Version agreeing with default uv sphere mapping in Blender 2.79
                 sp[d-3] *= sin(phi[d-3])*cos(phi[d-2]-M_PI/2.) ;
                 sp[d-2] *= sin(phi[d-3])*sin(phi[d-2]-M_PI/2.) ;
                 sp[d-1] *= cos(phi[d-3]) ;
             }
             else
             {
                 //********** Version agreeing with the Threejs visualisation
                sp[d-3] *= cos(phi[d-3]) ;
                sp[d-2] *= sin(phi[d-3])*cos(-phi[d-2]) ;
                sp[d-1] *= sin(phi[d-3])*sin(-phi[d-2]) ;
             }

             //dispvector(sp) ;
             // Now sp should be right, let's check
             //printf("Checking the point on surface: {%g} {%g} should be equal\n", Tools::norm(sp), R[i] ) ;
             // Rotating the point vector back in dimensions, and then rotating in space according to the basis A
             rotate(sp.begin(), sp.begin()+((d-nrotate)%d), sp.end()) ;
             Tools<d>::matvecmult(spturned, A[i], sp) ;
             // and ... rotating back :)
             rotate(spturned.begin(), spturned.begin()+nrotate, spturned.end()) ;
             Tools<d>::hyperspherical_xtophi (spturned, phinew) ;
             /*if (i==9)
             {
                 printf("=======================\n") ;
                 dispvector(sp) ;
                 dispvector(A[i]) ;
                 dispvector(spturned) ;
                 //printf("%g %g %g %g| %g %g %g %g\n", phi[0], phi[1], phi[2], phi[3], phinew[0], phinew[1], phinew[2], phinew[3]) ;
             }*/
             //if (phi[1]==0) phi[1]=M_PI ;
             phi2color (img.begin() + imgx0 * 3 + k*3 + j*stride*3, phinew, d, colors) ;
         }

     if (singlefiles)
     {
        char path[5000] ;
        filepathname(path, i, time, View) ;
        write_img(path, Ntheta, Nlambda, img.data()) ;
        filerendered.push_back(path);
     }
     else
     {
         imgx0 += Ntheta ;
         if (imgx0 >= Ntheta * FilePerLine) imgx0 = 0 + i/FilePerLine * (Ntheta*Nlambda*FilePerLine) ;
     }
}

if (!singlefiles)
{
 char path[5000] ;
 filepathname(path, time, View) ;
 write_img(path, Ntheta*FilePerLine, Nlambda * (N/FilePerLine +1), img.data()) ;
 filerendered.push_back(path);
}
return ;
}
//--------------------------------------------------------
template <int d>
int Texturing<d>::set_grid(int nb)
{
  // Set Lambda and Theta grids
  if (nb>0 && nb<16)
      Nlambda=Ntheta=(1<<nb);
  else if (nb>=16)
      Nlambda=Ntheta=nb ;
  else
  {
      colors=allcolors[1] ;
      write_colormap_vtk(4, colors) ;
      exit(0) ;
  }
  lambdagrid.resize(Nlambda,0) ;
  thetagrid.resize(Ntheta,0) ; //lambda:latitude (0:pi), theta: longitude (0:2pi)

  // Setting up the grid in latitude-longitude
  for (int i=0 ; i<Nlambda ; i++) lambdagrid[i]=  M_PI/(2.*Nlambda)+  M_PI/Nlambda*i ;
  for (int i=0 ; i<Ntheta-1 ; i++)  thetagrid[i] =2*M_PI/(2.*(Ntheta-1) )+2*M_PI/(Ntheta-1) *i ;
  thetagrid[Ntheta-1]=thetagrid[0];
  return 0 ;
}
//=================================================
template <int d>
int Texturing<d>::viewpermute (v1d & View)
{
if (d>3) //All view dimensions are NaN if d==3
{
   int nrotate=0 ;
   while (isnan(View[0]))
   {
       rotate(View.begin(), View.begin()+1 , View.end()) ;
       nrotate++ ;
   }
   if (nrotate==0)
   {
       auto b = find_if(View.begin(), View.end(), [](double dd) { return std::isnan(dd); } ) ;
       rotate(View.begin(), b+3, View.end()) ;
       nrotate = b-View.begin()+3 ;
   }
   nrotate %= View.size() ;
return nrotate ;
}
else return 0 ;
}
//====================================================
template <int d>
void Texturing<d>::filepathname (char * path, int n, int time, cv1d &View)
{
    char tmp[1000] ; 
    sprintf (path, "%s/Texture-%d-%05d", DirectorySave.c_str(), n, time) ;
    for (uint i=0 ; i<d-3 ; i++)
    {
        strcpy(tmp, path) ; 
        sprintf(path, "%s-%.1f", tmp, View[i]) ;
    }
    strcat(path, ".png") ;
}
template <int d>
void Texturing<d>::filepathname (char * path, int time, cv1d &View)
{
    char tmp[1000] ; 
    sprintf (path, "%s/TextureTile-%05d", DirectorySave.c_str(), time) ;
    for (uint i=0 ; i<d-3 ; i++)
    {
        strcpy(tmp, path) ; 
        sprintf(path, "%s-%.1f", tmp, View[i]) ;
    }
    strcat(path, ".png") ;
}
//================================================
template <int d>
int Texturing<d>::write_colormap_vtk_base (){
  write_colormap_vtk(d, colors) ;
  return 0 ;
}
template <int d>
int Texturing<d>::write_colormap_nrrd_base (map <string,string> args){
  write_NrrdIO(args["path"], d, colors) ;
  return 0 ;
}
///----------------------------------------------------------
template <int d>
int Texturing<d>::write_vtkmap (map <string,string> args)
{
  v1d View ; int nrotate=3 ;
  View.push_back(atof(args["x4"].c_str())) ;
  int time = atoi(args["ts"].c_str()) ;
  int idx = atoi(args["N"].c_str()) ;

	v1d sp (d,0) ; v1d spturned (d,0) ; // Surface point (point on the surface of the sphere)
	v1d phi (d-1,0), phinew(d-1,0) ; // Angles of the hyperspherical coordinates. All angles between 0 and pi, except the last one between 0 and 2pi
	vector<uint8_t> img ;

	img.resize(3*Nlambda*(Ntheta-1),0) ;

	FILE *vtkout ;
  char path[5000] ;
  sprintf(path,"Colormap-Surface-%d-%d.vtk", idx, time) ;
	vtkout=fopen(path, "w") ;
	fprintf(vtkout,"# vtk DataFile Version 2.0\nTexture surface for ND DEM\nASCII\nDATASET UNSTRUCTURED_GRID\nPOINTS %d float\n", Nlambda*(Ntheta-1)) ;

   // Check if we are in view
   double rsqr = R[idx]*R[idx] ;
   for (uint j=0 ; j<d-3 ; j++)
       rsqr -= (View[j]-Ts[time].X[idx][j])*(View[j]-Ts[time].X[idx][j]) ;
   if (rsqr<=0)
       return -1 ;

   // We are in view, let's get to it let's get the first phi's (constants)
   for (uint j=0 ; j<d-3 ; j++)
   {
     double cosine = (View[j]-Ts[time].X[idx][j])/R[idx] ;
     for (uint k=0 ; k<j ; k++)
       cosine /= sin(phi[k]) ;
     phi[j] = acos(cosine) ;
   }

   for (int j = 0 ; j<Nlambda ; j++)
       for (int k=0 ; k<Ntheta-1 ; k++)
	         {

	             // Finalising the phi array (useless, but just because
	             phi[d-3]=lambdagrid[j] ;
	             phi[d-2]=thetagrid[k] ;

	             for (uint dd=0 ; dd<d-3 ; dd++) {sp[dd]=View[dd]-Ts[time].X[idx][dd] ;}
	             //sp = View-X[i] ; // All the dimensions except the last 3 are now correct
	             sp[d-3] = R[idx] ;
	             for (uint j=0 ; j<d-3 ; j++) sp[d-3] *= sin(phi[j]) ;
	             sp[d-2]=sp[d-3] ; sp[d-1]=sp[d-3] ;
	             sp[d-3] *= cos(phi[d-3]) ;
	             sp[d-2] *= sin(phi[d-3])*cos(phi[d-2]) ;
	             sp[d-1] *= sin(phi[d-3])*sin(phi[d-2]) ;
	             //dispvector(sp) ;
	             // Now sp should be right, let's check
	             //printf("Checking the point on surface: {%g} {%g} should be equal\n", Tools::norm(sp), R[i] ) ;
	             // Rotating the point vector back in dimensions, and then rotating in space according to the basis A
	             rotate(sp.begin(), sp.begin()+((d-nrotate)%d), sp.end()) ;
	             Tools<d>::matvecmult(spturned, Ts[time].A[idx], sp) ;
	             // and ... rotating back :)
	             rotate(spturned.begin(), spturned.begin()+nrotate, spturned.end()) ;
	             Tools<d>::hyperspherical_xtophi (spturned, phinew) ;

							 fprintf(vtkout, "%g %g %g\n", phinew[0], phinew[1], phinew[2]) ;

	             phi2color (img.begin()+j*(Ntheta-1)*3+k*3, phinew, d, colors) ;
	         }

  fprintf(vtkout, "\nCELLS %d %d\n", (Nlambda-1)*(Ntheta-2), (Nlambda-1)*(Ntheta-2)*5 ) ;
  for (int i=0 ; i<Nlambda-1 ; i++)
    for (int j=0 ; j<Ntheta-2 ; j++)
      fprintf(vtkout, "4 %d %d %d %d\n", i*(Ntheta-1)+j, i*(Ntheta-1)+j+1, (i+1)*(Ntheta-1)+j+1, (i+1)*(Ntheta-1)+j) ;

   fprintf(vtkout, "\nCELL_TYPES %d\n", (Nlambda-1)*(Ntheta-2)) ;
   for (int i=0 ; i<(Nlambda-1)*(Ntheta-2) ; i++) fprintf(vtkout, "9 ") ;
   fprintf(vtkout, "\n") ;

  fprintf(vtkout, "\nPOINT_DATA %d\nCOLOR_SCALARS Color 3\n", Nlambda*(Ntheta-1)) ;
  for (int j = 0 ; j<Nlambda ; j++)
      for (int k=0 ; k<Ntheta-1 ; k++)
		 	    fprintf(vtkout, "%g %g %g\n", img[j*(Ntheta-1)*3+k*3]/256., img[j*(Ntheta-1)*3+k*3+1]/256., img[j*(Ntheta-1)*3+k*3+2]/256.) ;
  fclose(vtkout) ;

  return 0 ;
}


/** @} */
