#include "Tools.h"

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

int csvread_A (char path[], v2d &result, int d) ; 
int csvread_XR (char path[], v2d & result, v1d &R, int d)

//==================================================
int main (int argc, char * argv[])
{
 
 int d = 3 ; 
 Tools::initialise(d) ; 
 v1d phi (d-1,0) ; // Angles of the hyperspherical coordinates. All angles between 0 and pi, except the last one between 0 and 2pi
 
 v1d View ; // Use NaN for the 3D coordinates (careful, should always follow each others
 int Nlambda=8, Ntheta=8 ; 
 v1d lambdagrid(Nlambda,0), thetagrid(Ntheta,0) ; //lambda:latitude (0:pi), theta: longitude (0:2pi)
 v2d X, A ; v1d R ; 
 int N = X.size() ; 
 
 // Setting up the grid in latitude-longitude
 for (int i=0 ; i<Nlambda ; i++) lambdagrid[i]=M_PI/(2.*Nlambda)+M_PI/Nlambda*i ; 
 for (int i=0 ; i<Ntheta ; i++) thetagrid[i]=2*M_PI/(2.*Ntheta)+2*M_PI/Ntheta*i ; 
 
 // Let's simplify our life and rotate the view so that the 3 last coordinates are the NaN's
 int nrotate = 0 ; 
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
 
 for (int i=0 ; i<N ; i++)
 {
     // Check if we are in view
     double rsqr = R[i]*R[i] ;
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
             
             v1d sp (0,d) ; // Surface point (point on the syrface of the sphere)
             sp = View-X[i] ; // All the dimensions except the last 3 are now correct
             
             sp[d-3] = R[i] ;
             for (int j=0 ; j<d-3 ; j++) sp[d-3] *= sin(phi[j]) ; 
             sp[d-2]=sp[d-3] ; sp[d-1]=sp[d-3] ; 
             sp[d-3] *= cos(phi[d-3]) ; 
             sp[d-2] *= sin(phi[d-3])*cos(phi[d-2]) ; 
             sp[d-1] *= sin(phi[d-3])*sin(phi[d-2]) ;
             
             // Now sp should be right, let's check
             printf("Checking the point on surface: {%g} {%g} should be equal\n", Tools::norm(sp), R[i] ) ;  
             
             // Rotating the point vector back in dimensions, and then rotating in space according to the basis A
             rotate(sp.begin(), sp.begin()+(7-nrotate), sp.end()) ; 
             sp = Tools::matvecmult(A[i], sp) ;
             Tools::hyperspherical_xtophi (sp, phi) ; 
             
             
             
             n++ ;
         }
     
 }
 
    
    
    
 return 0 ;    
}


// =============================
int csvread_A (char path[], v2d & result, int d)
{
 FILE *in ; int n=0 ; double tmp ; 
 in=fopen(path, "r") ; if (in==NULL) {printf("Cannot open input file %s\n", path) ; return 1 ; } 
 
 fscanf(in,"%*[^\n]%*c") ; // Skipping header
 
 while (!feof(in))
 {
     fscanf(in, "%lg%*c", &tmp) ; 
     if (feof(in)) break ; 
     
     result.push_back(v1d (d,0)) ; 
     result[n][0]=tmp ; 
     for (int i=1 ; i<d ; i++)
         fscanf(in, "%lg%*c", &result[n][i]) ; 
         
     fscanf(in, "%*s%*c") ; 
     n++ ; 
 }
 
 fclose(in) ; 
 return 0 ; 
}
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
     r.push_back(0) ;
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







