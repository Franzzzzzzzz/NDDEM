#include "Tools.h"
#include "TinyPngOut.hpp"

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
int write_img (int w, int h, uint8_t * px, int idx) ; 
int csvread_A (char path[], v2d &result, int d) ; 
int csvread_XR (char path[], v2d & result, v1d &R, int d) ;

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
 int d ;
 d = atoi(argv[1]) ;  
 Tools::initialise(d) ; 
 v1d phi (d-1,0) ; // Angles of the hyperspherical coordinates. All angles between 0 and pi, except the last one between 0 and 2pi
 
 v1d View(d,0) ; // Use NaN for the 3D coordinates (careful, should always follow each others
 for (int i=0 ; i<d ; i++) View[i]=atof(argv[i+2]) ; 
 int Nlambda=2, Ntheta=2 ; 
 v1d lambdagrid(Nlambda,0), thetagrid(Ntheta,0) ; //lambda:latitude (0:pi), theta: longitude (0:2pi)
 vector<uint8_t> img (Nlambda*Ntheta*3) ; 
 v1d sp (d,0),spturned (d,0) ; // Surface point (point on the syrface of the sphere)
 
 v2d X, A ; v1d R ; 
 csvread_XR (argv[argc-2], X, R, d) ;
 csvread_A  (argv[argc-1], A, d) ;
 int N = X.size() ; 
 
 // Setting up the grid in latitude-longitude
 for (int i=0 ; i<Nlambda ; i++) lambdagrid[i]=  M_PI/(2.*Nlambda)+  M_PI/Nlambda*i ; 
 for (int i=0 ; i<Ntheta ; i++)  thetagrid[i] =2*M_PI/(2.*Ntheta )+2*M_PI/Ntheta *i ; 
 
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
     double rsqr = R[i]*R[i] ; printf("F") ; fflush(stdout) ; 
     rotate(X[i].begin(), X[i].begin()+nrotate, X[i].end()) ; 
     for (int j=0 ; j<d-3 ; j++)
         rsqr -= (View[j]-X[i][j])*(View[j]-X[i][j]) ; 
     if (rsqr<=0) continue ;  
      
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
             phi[d-2]=lambda ; 
             phi[d-1]=theta ;  
             for (int dd=0 ; dd<d ; dd++) {printf("!"); fflush(stdout) ;sp[dd]=View[dd]-X[i][dd] ;} 
             //sp = View-X[i] ; // All the dimensions except the last 3 are now correct
             
             sp[d-3] = R[i] ;
             for (int j=0 ; j<d-3 ; j++) sp[d-3] *= sin(phi[j]) ; 
             sp[d-2]=sp[d-3] ; sp[d-1]=sp[d-3] ; 
             sp[d-3] *= cos(phi[d-3]) ; 
             sp[d-2] *= sin(phi[d-3])*cos(phi[d-2]) ; 
             sp[d-1] *= sin(phi[d-3])*sin(phi[d-2]) ;
             
             // Now sp should be right, let's check
             printf("Checking the point on surface: {%g} {%g} should be equal\n", Tools::norm(sp), R[i] ) ;  
             printf("A") ; fflush(stdout) ; 
             // Rotating the point vector back in dimensions, and then rotating in space according to the basis A
             rotate(sp.begin(), sp.begin()+(7-nrotate), sp.end()) ; 
             printf("C") ; fflush(stdout) ; 
             spturned = Tools::matvecmult(A[i], sp) ;
             printf("D") ; fflush(stdout) ; 
             Tools::hyperspherical_xtophi (sp, phi) ; 
             printf("E %d", sp.size()) ; fflush(stdout) ; 
             //phi2color (img.begin() + n*3, phi, d) ; 
             n++ ;printf("B") ; fflush(stdout) ; 
         }
     //write_img(Ntheta, Nlambda, img.data(), i) ; 
 }
    
 return 0 ;    
}


// =============================
void phi2color (vector<uint8_t>::iterator px, v1d & phi, int d)
{
    float v ; int vbyte ; 

    vector <float> ctmp (3,0) ; 
    vector <float> cfinal(3,0) ; 
    phi[d-2] = phi[d-2]>M_PI?2*M_PI-phi[d-2]:phi[d-2] ;  
    for (int i=0 ; i<d-1 ; i++)
    {
        v = phi[i] / M_PI ;
        if (v<0.5)
          ctmp = colors[i]*(v*2) ; 
        else
          ctmp = (vector<float>(3,1.0) - colors[i]) * ((v-0.5)*2)  ;
        cfinal += ctmp ; 
    }
    cfinal /= (d-1) ; 
    for (int i=0 ; i<3 ; i++) 
    {
        vbyte = cfinal[i]*256 ; 
        vbyte=vbyte>256?256:vbyte ; 
        vbyte=vbyte<0?0:vbyte ; 
        *(px+i) = vbyte ;
    }
    return ;
}
//-------------------------------------------------------
int write_img (int w, int h, uint8_t * px, int idx)
{
	char path[500] ; 
    sprintf(path, "Texture-%d.png", idx) ;
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
int csvread_A (char path[], v2d & result, int d)
{
 FILE *in ; int n=0 ; double tmp ; 
 in=fopen(path, "r") ; if (in==NULL) {printf("Cannot open input file %s\n", path) ; return 1 ; } 
 
 fscanf(in,"%*[^\n]%*c") ; // Skipping header
 
 while (!feof(in))
 {
     fscanf(in, "%lg%*c", &tmp) ; 
     if (feof(in)) break ; 
     
     result.push_back(v1d (d*d,0)) ; 
     result[n][0]=tmp ; 
     for (int i=1 ; i<d*d ; i++)
         fscanf(in, "%lg%*c", &result[n][i]) ; 
         
     fscanf(in, "%*s%*c") ; 
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
 
 fscanf(in,"%*[^\n]%*c") ; // Skipping header
 
 while (!feof(in))
 {
     fscanf(in, "%lg%*c", &tmp) ; 
     if (feof(in)) break ; 
     
     result.push_back(v1d (d,0)) ; 
     R.push_back(0) ;
     result[n][0]=tmp ; 
     for (int i=1 ; i<d ; i++)
         fscanf(in, "%lg%*c", &result[n][i]) ; 
     fscanf(in, "%lg%*c", &R[n]) ; 
         
     fscanf(in, "%*s%*c") ; 
     n++ ; 
 }
 
 fclose(in) ; 
 return 0 ; 
}







