#include "../Dem/Tools.h"
#include "TinyPngOut.hpp"

#define Nlambda 8
#define Ntheta 8

using namespace std ;
// needed for Tools
uint Tools::d=0 ;
vector < vector <int> > Tools::MSigns ;
vector < vector <int> > Tools::MIndexAS ;
vector < double > Tools::Eye ;
vector < pair <int,int> > Tools::MASIndex ;
vector <FILE *> Tools::outs ;
boost::random::mt19937 Tools::rng ;
boost::random::uniform_01<boost::mt19937> Tools::rand(rng) ;

void phi2color (vector<uint8_t>::iterator px, v1d & phi, int d) ;
int write_colormap_vtk (int d) ;
int write_img (char path[], int w, int h, uint8_t * px, int idx) ;
int csvread_A (char path[], v2d &result, int d) ;
int csvread_XR (char path[], v2d & result, v1d &R, int d) ;

void dispvector (v1d & a) {for (auto v: a) printf("%g ", v); printf("\n") ; fflush(stdout) ; }
void dispvector (v1f & a) {for (auto v: a) printf("%g ", v); printf("\n") ; fflush(stdout) ; }

vector<vector<float>> colors = {
    {1,0,0},
    {0,1,0},
    {0,0,1},
    {1,1,0},
    {0,1,1},
    {1,0,1}} ;
//==================================================
//Args: d V1 V2 ... VN locationfile Afile
int main (int argc, char * argv[])
{
 /* TEST
 write_colormap_vtk(4) ;
 exit(0) ; */
 int d ;
 d = atoi(argv[1]) ;
 Tools::initialise(d) ;
 if (static_cast<uint>(d-1)>colors.size()) printf("ERR: not enough color gradients!!\n") ;
 v1d phi (d-1,0) ; // Angles of the hyperspherical coordinates. All angles between 0 and pi, except the last one between 0 and 2pi

 v1d View(d,0) ; // Use NaN for the 3D coordinates (careful, should always follow each others
 for (int i=0 ; i<d ; i++) View[i]=atof(argv[i+2]) ;
 v1d lambdagrid(Nlambda,0), thetagrid(Ntheta,0) ; //lambda:latitude (0:pi), theta: longitude (0:2pi)
 vector<uint8_t> img (Nlambda*Ntheta*3,0) ;
 v1d sp (d,0) ; v1d spturned (d,0) ; // Surface point (point on the syrface of the sphere)

 v2d X, A ; v1d R ;
 char path[5000] ;
 sprintf(path, "%s/dump-%s.csv", argv[argc-1], argv[argc-2]) ;
 csvread_XR (path, X, R, d) ;
 sprintf(path, "%s/dumpA-%s.csv", argv[argc-1], argv[argc-2]) ;
 csvread_A  (path, A, d) ;
 int N = X.size() ;

 // Setting up the grid in latitude-longitude
 for (int i=0 ; i<Nlambda ; i++) lambdagrid[i]=  M_PI/(2.*Nlambda)+  M_PI/Nlambda*i ;
 for (int i=0 ; i<Ntheta-1 ; i++)  thetagrid[i] =2*M_PI/(2.*(Ntheta-1) )+2*M_PI/(Ntheta-1) *i ;
 thetagrid[Ntheta-1]=thetagrid[0];

 // Let's simplify our life and rotate the view so that the 3 last coordinates are the NaN's
 int nrotate = 0 ;
 if (d>3) //All view dimensions are NaN if d==3
 {
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
 }

 for (int i=0 ; i<N ; i++)
 {
     // Check if we are in view
     double rsqr = R[i]*R[i] ;
     rotate(X[i].begin(), X[i].begin()+nrotate, X[i].end()) ;
     for (int j=0 ; j<d-3 ; j++)
         rsqr -= (View[j]-X[i][j])*(View[j]-X[i][j]) ;
     if (rsqr<=0)
     {
       char path[500] ;
       sprintf(path, "%s/Texture-%d.png", argv[argc-1], i) ;
       //experimental::filesystem::remove(path) ;
       continue ;
     }

     // We are in view, let's get to it let's get the first phi's (constants)
     for (int j=0 ; j<d-3 ; j++)
     {
       double cosine = (View[j]-X[i][j])/R[i] ;
       for (int k=0 ; k<j ; k++)
         cosine= View[j] / sin(phi[k]) ;
       phi[j] = acos(cosine) ;
     }

     int n=0 ;
     for (auto lambda : lambdagrid)
         for (auto theta : thetagrid)
         {
             // Finalising the phi array (useless, but just because
             phi[d-3]=lambda ;
             phi[d-2]=theta ;

             for (int dd=0 ; dd<d ; dd++) {sp[dd]=View[dd]-X[i][dd] ;}
             //sp = View-X[i] ; // All the dimensions except the last 3 are now correct

             sp[d-3] = R[i] ;
             for (int j=0 ; j<d-3 ; j++) sp[d-3] *= sin(phi[j]) ;
             sp[d-2]=sp[d-3] ; sp[d-1]=sp[d-3] ;
             sp[d-3] *= cos(phi[d-3]) ;
             sp[d-2] *= sin(phi[d-3])*cos(phi[d-2]) ;
             sp[d-1] *= sin(phi[d-3])*sin(phi[d-2]) ;

             // Now sp should be right, let's check
             //printf("Checking the point on surface: {%g} {%g} should be equal\n", Tools::norm(sp), R[i] ) ;
             // Rotating the point vector back in dimensions, and then rotating in space according to the basis A
             rotate(sp.begin(), sp.begin()+((d-nrotate)%d), sp.end()) ;

             Tools::matvecmult(spturned, A[i], sp) ;
             Tools::hyperspherical_xtophi (spturned, phi) ;
             printf("%g %g %g | %g %g %g | %g %g %g\n", sp[0], sp[1], sp[2], spturned[0], spturned[1],spturned[2], phi[0], phi[1], phi[2]) ;
             //if (phi[1]==0) phi[1]=M_PI ;
             phi2color (img.begin() + n*3, phi, d) ;
             n++ ;
         }
     write_img(argv[argc-1], Ntheta, Nlambda, img.data(), i) ;
 }

 return 0 ;
}


// =============================
void rescale (v1f & c, cv1f sum)
{
  for (int i=0 ; i<c.size() ; i++)
    if (sum[i]!=0)
      c[i]/=sum[i] ;
}
//--------------------------------------------------------
void phi2color (vector<uint8_t>::iterator px, v1d & phi, int d)
{
    int vbyte ;
    vector <float> ctmp (3,0), sum(3,0) ;
    vector <float> cfinal(3,0) ;
    //if (isnan(phi[0])||isnan(phi[1]) || isnan(phi[2])) dispvector(phi) ;
    //phi[d-2] = phi[d-2]>M_PI?2*M_PI-phi[d-2]:phi[d-2] ;
    //phi[d-2] /= 2 ;
    for (int i=0 ; i<d-2 ; i++)
    {
        ctmp += (colors[i] * sin(phi[i])) ;
        sum += colors[i] ;
    }
    rescale(ctmp,sum) ; //printf("%g %g %g\n", sum[0], sum[1], sum[2]);
    //for (int i=0 ; i<d-2 ; i++) ctmp *= sin(phi[i]) ;
    ctmp *= sin(phi[d-2]/2.) ;
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
	char ppath[500] ;
  sprintf(ppath, "%s/Texture-%d.png", path, idx) ;
	try {

		std::ofstream out(ppath, std::ios::binary);
		TinyPngOut pngout(static_cast<uint32_t>(w), static_cast<uint32_t>(h), out);
		pngout.write(px, static_cast<size_t>(w * h));
		return EXIT_SUCCESS;

	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
}
//---------
int csvread_A (char path[], v2d & result, int d)
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
int csvread_XR (char path[], v2d & result, v1d &R, int d)
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
int vvalues=5 ;
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
