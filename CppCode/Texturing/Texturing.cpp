#include "../Dem/Tools.h"
#include "TinyPngOut.hpp"
#include <regex>
#include <limits>
#ifdef NRRDIO
#include "../CoarseGraining/NrrdIO-1.11.0-src/NrrdIO.h"
#endif

#define DeltaX 0.1
//#define Nlambda 32
//#define Ntheta 32

using namespace std ;
// needed for Tools
uint Tools::d=0 ;
int Nlambda=32, Ntheta=32 ;
vector < vector <int> > Tools::MSigns ;
vector < vector <int> > Tools::MIndexAS ;
vector < double > Tools::Eye ;
vector < pair <int,int> > Tools::MASIndex ;
vector <FILE *> Tools::outs ;
boost::random::mt19937 Tools::rng ;
boost::random::uniform_01<boost::mt19937> Tools::rand(rng) ;

void phi2color (vector<uint8_t>::iterator px, cv1d & phi, int d) ;
int write_colormap_vtk (int d) ;
int write_NrrdIO (string path, int d) ;
int write_img (char path[], int w, int h, uint8_t * px, int idx) ;
int csvread_A (const char path[], v2d &result, int d) ;
int csvread_XR (const char path[], v2d & result, v1d &R, int d) ;
int viewpermute (v1d & View, int d) ;
void spaceloop (vector<string> & FileList, v1d View, int nrotate, int dim, int time, cv2d & X, cv1d & R, cv2d & A) ;
void filepathname (char * path, int n, int time, cv1d View) ;
void Render (vector <string> & filerendered, cv1d & View, int nrotate, int time, cv2d &X, cv1d & R, cv2d &A) ;

void dispvector (const v1d & a) {for (auto v: a) printf("%g ", v); printf("\n") ; fflush(stdout) ; }
void dispvector (const v1f & a) {for (auto v: a) printf("%g ", v); printf("\n") ; fflush(stdout) ; }

// Some helpful globals (sorry ...)
vector<vector<float>> colors = {
    {1,1,0},
    {0,1,1},
    {1,0,1},
    {1,1,0},
    {0,1,1},
    {1,0,1}} ;

vector<vector<vector<float>>> allcolors = {
    {{231./256., 37./256., 100./256.}}, // official NDDEM pink
    {{1,1,0},{0,1,1}}};

v1d lambdagrid, thetagrid ;
string DirectorySave ;
uint d ; int N ;

//==================================================
//Args: d V1 V2 ... VN locationfile Afile
int main (int argc, char * argv[])
{
  //TEST
 //write_colormap_vtk(4) ;
 //write_NrrdIO("Colormap.nrrd", 4) ;
 //exit(0) ;

 // Get Number of dimensions and extract viewpoint
 d = atoi(argv[2]) ;
 Tools::initialise(d) ;
 v1d View(d,0) ; // Use NaN for the 3D coordinates (careful, should always follow each others
 //for (uint i=0 ; i<d ; i++) View[i]=atof(argv[i+3]) ;

 string Directory = argv[argc-2] ;
 DirectorySave = argv[argc-1] ;

 //Get all the relevent files in the Directory, sort them and identify the timesteps
 experimental::filesystem::directory_iterator D(Directory) ;
 vector <string> tmpfilelst ;
 vector <std::pair<int,string>> filelistloc, filelistA ;
 for (auto& p : experimental::filesystem::directory_iterator(Directory)) tmpfilelst.push_back(p.path().string()) ;
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

 // Let's read everything
 vector <v2d> X, A ; v1d R ;
 X.resize(filelistloc.size()) ;
 A.resize(filelistloc.size()) ;
 for (uint i=0 ; i<filelistloc.size() ; i++)
  {
    R.clear() ;
    csvread_XR (filelistloc[i].second.c_str(), X[i], R, d) ;
  }
 for (uint i=0 ; i<filelistA.size() ; i++)   csvread_A  (filelistA[i].second.c_str(), A[i], d) ;
 N = X[0].size() ;
 int nrotate=3 ; // Rotate all the coordinates already
 for (auto & v : X)
 {
    for (int i=0 ; i<N ; i++)
        rotate(v[i].begin(), v[i].begin()+nrotate, v[i].end()) ;
 }

 // Set Lambda and Theta grids
 int nb=atoi(argv[1]) ;
 if (nb>0 && nb<16)
     Nlambda=Ntheta=(1<<nb);
 else if (nb>=16)
     Nlambda=Ntheta=nb ;
 else
 {
     colors=allcolors[1] ;
     write_colormap_vtk(4) ;
     exit(0) ;
 }
 lambdagrid.resize(Nlambda,0) ;
 thetagrid.resize(Ntheta,0) ; //lambda:latitude (0:pi), theta: longitude (0:2pi)

 // Color gradient initialisation
 if (static_cast<uint>(d-1)>colors.size()) printf("ERR: not enough color gradients!!\n") ;
 //if (d-3<allcolors.size()) colors=allcolors[d-3] ;


 //auto TimeFirst = find_if(filelistloc.begin(), filelistloc.end(), [=](std::pair<int,string>a){return (a.first==atoi(argv[argc-2])) ; }) ;
 //auto TimeCur = TimeFirst ;

 // Setting up the grid in latitude-longitude
 for (int i=0 ; i<Nlambda ; i++) lambdagrid[i]=  M_PI/(2.*Nlambda)+  M_PI/Nlambda*i ;
 for (int i=0 ; i<Ntheta-1 ; i++)  thetagrid[i] =2*M_PI/(2.*(Ntheta-1) )+2*M_PI/(Ntheta-1) *i ;
 thetagrid[Ntheta-1]=thetagrid[0];

 FILE * piping ; char line[5000] ;
 piping=fopen("pipe", "r") ; if (piping==NULL) {printf("ERR: a pipe is expected\n") ; exit(1) ; }

 bool run = true ;
 vector<int> ViewPoint(d-3+1, INT_MIN), NewViewPoint(d-3+1,0) ;
 vector <vector<string>> FileList (d-3+1) ;
 int TimeCur ;
 printf("Texturing Waiting") ; fflush(stdout) ;
 while (run)
 {
   int l=-1,n ;
   do {
     l++ ;
     n=fscanf(piping, "%c", line+l) ;
   } while (n>0) ;
   if (l>0) line[l-1] = 0 ;
   else line[0] = 0 ;
   clearerr(piping) ;

   if (line[0]!=0) {printf("Text received: %s\n", line) ; fflush(stdout) ;}

   if (!strcmp(line, "stop")) run=false ;
   else if (!strcmp(line, "pass")) {} // Just pass
   else if (line[0]!=0) // Assume we got a new viewpoint
   {
     char * pch;
     pch = strtok (line," ");
     for (uint i=0 ; i<d ; i++)
     {
       View[i] = atof(pch) ;
       pch = strtok (NULL, " ");
     }
     TimeCur=atoi(pch) ;

     int nrotate = viewpermute (View, d) ;
     printf("[%d]", nrotate) ; fflush(stdout) ;
     for (uint i=0 ; i<d-3 ; i++) {NewViewPoint[i] = isnan(View[i+3])?0:static_cast<int>(round(View[i+3]/DeltaX));}
     NewViewPoint[d-3]= TimeCur ;

     int TimeCurInt = find_if(filelistloc.begin(), filelistloc.end(), [=](std::pair<int,string>a){return (a.first==TimeCur);})-filelistloc.begin() ;

     for (auto v:View) printf("%f ", v) ; printf("|") ;
     for (auto v: NewViewPoint) printf("%d ", v) ;
     printf("|%d\n", TimeCur) ; fflush(stdout) ;

     // Alright, lets start the threads
     for (uint i=0 ; i<d-3 ; i++)
     {
         if (NewViewPoint[i]!=ViewPoint[i])
         {
          // Kill previous thread if exist
          // Restart the thread

             spaceloop(FileList[i], View, nrotate, i, TimeCur, X[TimeCurInt], R, A[TimeCurInt]) ;
         }
     }
     if (NewViewPoint[d-3]!=ViewPoint[d-3])
     {
          // Kill previous thread if exist
          // Restart the thread
     }
   }
   else {usleep(10000) ; } // A little sleep :)
 }

 return 0 ;
}
//===========================================================
void spaceloop (vector<string> & FileList, v1d View, int nrotate, int dim, int time, cv2d & X, cv1d & R, cv2d & A)
{
//auto ViewO=View ;
// Let's not put the loop just yet ...
FileList.clear() ;
Render(FileList,View, nrotate, time, X, R, A) ;


}


//-----------------------------------------------------
void Render (vector <string> & filerendered, cv1d & View, int nrotate, int time, cv2d &X, cv1d & R, cv2d &A)
{
v1d sp (d,0) ; v1d spturned (d,0) ; // Surface point (point on the surface of the sphere)
v1d phi (d-1,0), phinew(d-1,0) ; // Angles of the hyperspherical coordinates. All angles between 0 and pi, except the last one between 0 and 2pi
vector<uint8_t> img (Nlambda*Ntheta*3,0) ;
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

     int n=0 ;
     for (auto lambda : lambdagrid)
        for (auto theta : thetagrid)
         {
             // Finalising the phi array (useless, but just because
             phi[d-3]=lambda ;
             phi[d-2]=theta ;

             for (uint dd=0 ; dd<d-3 ; dd++) {sp[dd]=View[dd]-X[i][dd] ;}
             //sp = View-X[i] ; // All the dimensions except the last 3 are now correct

             sp[d-3] = R[i] ;
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
             Tools::matvecmult(spturned, A[i], sp) ;
             // and ... rotating back :)
             rotate(spturned.begin(), spturned.begin()+nrotate, spturned.end()) ;
             Tools::hyperspherical_xtophi (spturned, phinew) ;
             //printf("%g %g %g | %g %g %g \n", phi[0], phi[1], phi[2], phinew[0], phinew[1], phinew[2]) ;
             //if (phi[1]==0) phi[1]=M_PI ;
             phi2color (img.begin() + n*3, phinew, d) ;
             n++ ;
         }

     char path[5000] ;
     filepathname(path, i, time, View) ;
     write_img(path, Ntheta, Nlambda, img.data(), i) ;
     filerendered.push_back(path);
 }
}

//------------------------
void filepathname (char * path, int n, int time, cv1d View)
{
    sprintf (path, "%s/Texture-%d-%05d", DirectorySave.c_str(), n, time) ;
    for (uint i=0 ; i<d-3 ; i++)
        sprintf(path, "%s-%.1f", path, View[i]) ;
    strcat(path, ".png") ;
}

// =============================
void rescale (v1f & c, cv1f sum)
{
  for (uint i=0 ; i<c.size() ; i++)
    if (sum[i]>=1)
      c[i]/=sum[i] ;
}
//--------------------------------------------------------
void phi2color (vector<uint8_t>::iterator px, cv1d & phi, int d)
{
    int vbyte ;
    vector <float> ctmp (3,0), sum(3,0) ;
    vector <float> cfinal(3,0) ;
    //if (isnan(phi[0])||isnan(phi[1]) || isnan(phi[2])) dispvector(phi) ;
    //phi[d-2] = phi[d-2]>M_PI?2*M_PI-phi[d-2]:phi[d-2] ;
    //phi[d-2] /= 2 ;
    for (int i=0 ; i<d-2 ; i++)
    {
        ctmp += (colors[i] * fabs(sin(phi[i]))) ;
        sum += colors[i] ;
    }
    ctmp += (colors[d-2] * fabs(sin(phi[d-2]/2.))) ;
    sum += colors[d-2] ;
    rescale(ctmp,sum) ; //printf("%g %g %g\n", sum[0], sum[1], sum[2]);
    //for (int i=0 ; i<d-2 ; i++) ctmp *= sin(phi[i]) ;
    for (int i=0 ; i<d-2 ; i++)
      ctmp *= fabs(sin(phi[i])) ;
    //printf("%g %g %g\n", ctmp[0], ctmp[1], ctmp[2]) ;
    //ctmp = (colors[0]*sin(phi[0]) + colors[1]*sin(phi[1]/2)) * sin(phi[0]) ;
    cfinal = ctmp ;
    for (int i=0 ; i<3 ; i++)
    {
        cfinal[i] = round(cfinal[i]*256) ;
        cfinal[i]=cfinal[i]>255?255:cfinal[i] ;
        cfinal[i]=cfinal[i]<0?0:cfinal[i] ;
        vbyte=cfinal[i] ;
        *(px+i) = vbyte ;
    }
    return ;
}
//-------------------------------------------------------
int write_img (char path[], int w, int h, uint8_t * px, int idx)
{
	try {

		std::ofstream out(path, std::ios::binary);
		TinyPngOut pngout(static_cast<uint32_t>(w), static_cast<uint32_t>(h), out);
		pngout.write(px, static_cast<size_t>(w * h));
		return EXIT_SUCCESS;

	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
}
//---------
int csvread_A (const char path[], v2d & result, int d)
{
 FILE *in ; int n=0 ; double tmp ;
 in=fopen(path, "r") ; if (in==NULL) {printf("Cannot open input file %s\n", path) ; return 1 ; }
 int r ;

 r=fscanf(in,"%*[^\n]%*c") ; // Skipping header

 while (!feof(in))
 {
     r=fscanf(in, "%lg%*c", &tmp) ;
     if (feof(in)) break ;

     result.push_back(v1d (d*d,0)) ;
     result[n][0]=tmp ;
     for (int i=1 ; i<d*d ; i++)
         r=fscanf(in, "%lg%*c", &result[n][i]) ;

     //dispvector(result[n]) ;
     r=r ;
     n++ ;
 }

 fclose(in) ;
 return 0 ;
}
//-------
int csvread_XR (const char path[], v2d & result, v1d &R, int d)
{
 FILE *in ; int n=0 ; double tmp ;
 in=fopen(path, "r") ; if (in==NULL) {printf("Cannot open input file %s\n", path) ; return 1 ; }
 int r ;

 r=fscanf(in,"%*[^\n]%*c") ; // Skipping header

 while (!feof(in))
 {
     r=fscanf(in, "%lg%*c", &tmp) ;
     if (feof(in)) break ;

     result.push_back(v1d (d,0)) ;
     R.push_back(0) ;
     result[n][0]=tmp ;
     for (int i=1 ; i<d ; i++)
         r=fscanf(in, "%lg%*c", &result[n][i]) ;
     r=fscanf(in, "%lg%*c", &R[n]) ;
     //printf("%g %g %g %g\n", result[n][0], result[n][1], result[n][2], R[n]) ;
     r=fscanf(in, "%*s%*c") ;
     r=r ;
     n++ ;
 }

 fclose(in) ;
 return 0 ;
}
//--------------
int write_colormap_vtk(int d)
{
int nvalues=10 ;
vector<double> p(3), ptmp ;
vector <uint8_t> a ;
a.resize(3,0) ;

if (d!=4) printf("ERROR: cannot export the colormap for d!=4 currently\n") ;

FILE *vtkout ;
vtkout=fopen("Colormap.vtk", "w") ;
fprintf(vtkout,"# vtk DataFile Version 2.0\nTexture for ND DEM\nASCII\nDATASET STRUCTURED_POINTS\nDIMENSIONS %d %d %d\nORIGIN %g %g %g\nSPACING %g %g %g\n\nPOINT_DATA %d\nCOLOR_SCALARS Color 3\n", nvalues, nvalues, nvalues, 0 +M_PI/2/nvalues, 0 + M_PI/2/nvalues, 0+M_PI/nvalues, M_PI/nvalues, M_PI/nvalues, 2*M_PI/nvalues, nvalues*nvalues*nvalues) ;

p[2] = 0 + M_PI/nvalues ;
printf("%g ", p[2]) ;
for (int i=0 ; i<nvalues ; i++)
{
  p[1] = 0 +M_PI/2/nvalues ;
  for (int j=0 ; j<nvalues ; j++)
  {
    p[0] = 0+M_PI/2/nvalues ;
    for (int k=0 ; k<nvalues ; k++)
    {
      ptmp=p ;
      phi2color(a.begin(), ptmp, d) ;
      //printf("%g %g %g\n", p[0], p[1], p[2]) ;
      fprintf(vtkout, "%g %g %g\n", a[0]/256., a[1]/256., a[2]/256.) ;
      p[0] += M_PI/nvalues ;
    }
    p[1] += M_PI/nvalues ;
  }
  p[2] += 2*M_PI/nvalues ;
}
return 0 ;
}
//-------------------------------------------
int write_NrrdIO (string path, int d)
{
#ifdef NRRDIO
    int npoints=16 ;
    vector<double> p(d,0) ;

    Nrrd *nval;
    auto nio = nrrdIoStateNew();
    nrrdIoStateEncodingSet(nio, nrrdEncodingAscii) ; //Change to nrrdEncodingRaw for binary encoding
    nval = nrrdNew();

    // Header infos
    vector <size_t> dimensions (d, npoints) ;
    dimensions[0] = 3 ;
    dimensions[1]= npoints*2 ;

    vector <int> nrrdkind (d, nrrdKindSpace) ;
    nrrdkind[0]=nrrdKindVector ;
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoKind, nrrdkind.data() );

    vector <double> nrrdmin(d,0+M_PI/npoints/2), nrrdmax(d,M_PI-M_PI/npoints/2), nrrdspacing(d,M_PI/npoints) ;
    nrrdmin[0]=nrrdmax[0]=nrrdspacing[0]=AIR_NAN ;
    //nrrdmin[d-1]=0+M_PI/npoints ; nrrdmax[d-1]=2*M_PI+M_PI/npoints ;

    char ** labels;
    labels=(char **) malloc(sizeof(char *) * (d+3)) ;
    labels[0]=(char *) malloc(sizeof(char) * (4)) ;
    sprintf(labels[0], "rgb") ;
    for (int dd=1 ; dd<d ; dd++)
    {
        labels[dd]=(char *) malloc(sizeof(char) * (3+d/10+1+1)) ;
        sprintf(labels[dd], "phi%d", dd) ;
    }

    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoLabel, labels);
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoMin, nrrdmin.data());
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoMax, nrrdmax.data());
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoSpacing, nrrdspacing.data());

    int allpoint=pow(npoints,d-1)*2 ;
    uint8_t * outdata, * pout ;
    outdata=(uint8_t*) malloc(sizeof(uint8_t)*allpoint*3) ;

  //---------------------------------------------------------------------
    std::function <void(int,vector<double>)> lbdrecurse ;
    pout=outdata ;
    lbdrecurse = [&,d](int lvl, vector<double> p)
    {
      if (lvl<d-1)
      {
        p[lvl]=0+M_PI/npoints/2 ;
        for (int i=0 ; i<npoints ; i++)
        {
          lbdrecurse(lvl+1, p) ;
          p[lvl]+=M_PI/npoints ;
        }
      }
      else
      {
        p[lvl]=0+M_PI/npoints/2 ;
        for (int i=0 ; i<2*npoints ; i++)
        {
          auto ptmp=p ; vector <uint8_t> a(3,0) ;
          phi2color(a.begin(), ptmp, d) ;
          *pout=a[0] ; pout++ ;
          *pout=a[1] ; pout++ ;
          *pout=a[2] ; pout++ ;
          p[lvl]+=M_PI/npoints ;
        }
      }
    } ;
    lbdrecurse(0,p) ;
    //---------------------------------------------------------------------

    nrrdWrap_nva(nval, outdata, nrrdTypeUChar, d, dimensions.data());
    string fullpath ;
    fullpath = path ;
    nrrdSave(fullpath.c_str(), nval, nio);
    free(outdata) ;
    printf("%s ", fullpath.c_str()) ;
#endif
return 0 ;
}

//=================================================
int viewpermute (v1d & View, int d)
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
       auto b = find_if(View.begin(), View.end(), [](double d) { return std::isnan(d); } ) ;
       rotate(View.begin(), b+3, View.end()) ;
       nrotate = b-View.begin()+3 ;
   }
   nrotate %= View.size() ;
return nrotate ;
}
else return 0 ;
}
