#include "Tools.h"


void Tools::initialise (int dd) 
{
 d=dd ;
 Tools::MSigns.resize(d, vector <int> (d, 0)) ; 
 for (int i=0 ; i<d ; i++) for (int j=0 ; j<d ; j++) MSigns[i][j]=(i<j)*(1)+(i>j)*(-1) ; 
 
 MIndexAS.resize(d, vector < int > (d,0)) ; 
 int n=0 ; 
 for (int i=0 ; i<d ; i++)
      for (int j=i+1 ; j<d ; j++,n++)
      {
          MIndexAS[i][j]=n ; MIndexAS[j][i]=n ; 
          MASIndex.push_back(make_pair(i, j)) ; 
      }
      
  Eye.clear() ; Eye.resize(d*d,0) ;
  for (int de=0 ; de<d ; de++) Eye[de*d+de]=1 ; //initial orientation matrix    
}
//===================================
int Tools::savetxt(char path[], const v2d & table, char header[])
{
 FILE * out ; 
 out = fopen(path, "w") ; if (out==NULL) {printf("ERR: cannot write in file %s\n", path) ; return 1 ; }
 fprintf(out, "%s\n", header) ; 
 for (int i=0 ; i<table.size() ; i++)
 {
     for (int j=0 ; j<table[i].size() ; j++)
     {
         fprintf(out, "%.6g",table[i][j]) ;
         if (j<table[i].size()-1) fprintf(out, ",") ; 
     }
     if (i<table.size()-1) fprintf(out, "\n") ; 
 }
 fclose(out) ; 
 return 0 ; 
}
//=======================================
int Tools::write1D (char path[], v1d table)
{
 FILE * out ; 
 out = fopen(path, "w") ; if (out==NULL) {printf("ERR: cannot write in file %s\n", path) ; return 1 ; }
 for (int i=0 ; i<table.size() ; i++)
   fprintf(out, "%g\n", table[i]) ; 
 fclose(out) ; 
return 0 ;  
}

//=====================================
void Tools::savecsv (char path[], cv2d & X, cv1d &r)
{
 FILE *out ;
 out=fopen(path, "w") ; if (out==NULL) {printf("Cannot open out file\n") ; return ;}
 
 fprintf(out, "X,Y,Z,W,R\n") ; 
 for (int i=0 ; i<X.size() ; i++)
 {
  fprintf(out, "%.6g,%.6g,%.6g,%.6g,%.6g\n", X[i][0], X[i][1], X[i][2], X[i][3], r[i]) ;   
 }
 fclose(out) ; 
}
//=====================================
void Tools::savevtk (char path[], cv2d & X)
{
 FILE *out ;
 out=fopen(path, "w") ; if (out==NULL) {printf("Cannot open out file\n") ; return ;}
 
 fprintf(out, "# vtk DataFile Version 2.0\nMost Useless DEM (tm) output file\nASCII\nDATASET POLYDATA\n") ;
 
 fprintf(out, "POINTS %ld float\n", X.size()) ;  
 for (int i=0 ; i<X.size() ; i++) fprintf(out, "%g %g %g\n", X[i][0], X[i][1], X[i][2]) ; 
 fprintf(out, "VERTICES %ld %ld\n", X.size(), 2*X.size()) ;  
 for (int i=0 ; i<X.size() ; i++) fprintf(out, "1 %d\n", i) ;

 for (int j=3 ; j<X[0].size() ; j++)
 {
   fprintf(out, "\nSCALARS Dimension%d float 1 \nLOOKUP_TABLE default \n", j) ;
   for (int i=0 ; i<X.size() ; i++)
       fprintf(out, "%g ", X[i][j]) ; 
 }
 fclose(out) ; 
}
//============================================
int Tools::writeinline(initializer_list< v1d > list)
{
  if (outs.size()==0)
  {
    char path[5000] ; 
    outs.resize(list.size(), NULL) ; 
    int i=0 ; 
    for (auto iter=list.begin() ; iter<list.end() ; iter++, i++)
    {
      sprintf(path, "Res-%d.txt", i) ;  
      outs[i]=fopen(path, "w") ; 
      if (outs[i]==NULL) {printf("Error cannot open writeinline file %d", i) ; return (1) ; }
    }
  }
  
  int i=0 ; 
  for (auto iter=list.begin() ; iter<list.end() ; iter++, i++)
  {
    for (int j=0 ; j<iter->size() ; j++ )
      fprintf(outs[i], "%g ", iter->at(j)) ; 
    fprintf(outs[i], "\n") ; 
  }
  
  return 0 ;
}
int Tools::writeinline_close(void) 
{
  for (int i=0 ; i<outs.size() ; i++)
    fclose(outs[i]) ; 
  return 0 ;
}
//=========================================
v1d Tools::randomize_vec (cv1d v) 
{
  v1d res ; res.resize(v.size(),0) ; 
  for (int i=0 ; i<res.size() ; i++) res[i]=rand() ; 
  double n=Tools::norm(v) ; double nr=Tools::norm(res) ; 
  for (int i=0 ; i<res.size() ; i++) res[i]=res[i]/nr*n ;
  return (res) ; 
}

//=========================================
// All vector operators
v1d operator* (v1d a, double b)  {for (int i=0 ; i<a.size() ; i++) a[i]*=b    ; return a ; }
v1d operator* (v1d a, cv1d b)    {for (int i=0 ; i<a.size() ; i++) a[i]*=b[i] ; return a ; }
v1d operator+ (v1d a, double b)  {for (int i=0 ; i<a.size() ; i++) a[i]+=b    ; return a ; }
v1d operator+ (v1d a, cv1d b)    {for (int i=0 ; i<a.size() ; i++) a[i]+=b[i] ; return a ; }
v1d operator- (v1d a, double b)  {for (int i=0 ; i<a.size() ; i++) a[i]-=b    ; return a ; }
v1d operator- (v1d a, cv1d b)    {for (int i=0 ; i<a.size() ; i++) a[i]-=b[i] ; return a ; }
v1d operator- (v1d a)            {for (int i=0 ; i<a.size() ; i++) a[i]=-a[i] ; return a ; }
v1d operator/ (v1d a, double b)  {for (int i=0 ; i<a.size() ; i++) a[i]/=b    ; return a ; }
v1d & operator-= (v1d & a, cv1d b)  {for (int i=0 ; i<a.size() ; i++) a[i]-=b[i] ;}
v1d & operator*= (v1d & a, double b)  {for (int i=0 ; i<a.size() ; i++) a[i]*=b ;}
v1d & operator+= (v1d & a, cv1d b)  {for (int i=0 ; i<a.size() ; i++) a[i]+=b[i] ;}
v1d & operator/= (v1d & a, double b)  {for (int i=0 ; i<a.size() ; i++) a[i]/=b ;}
//-----------------------------------
v1d Tools::skewmatvecmult (cv1d & M, cv1d & v) 
{
 v1d res (d,0) ; 
 for (int i=0 ; i<d ; i++)
 {
     for (int j=0 ; j<d ; j++)
     {
         if (j==i) continue ; 
         res[i]+= MSigns[i][j]*M[MIndexAS[i][j]]*v[j] ; 
     }
 }
 return res ; 
}
//------------------------------------
void Tools::skewmatvecmult (v1d & r, cv1d & M, cv1d & v) 
{
 for (int i=0 ; i<d ; i++)
 {
     for (int j=0 ; j<d ; j++)
     {
         if (j==i) continue ; 
         r[i]+= MSigns[i][j]*M[MIndexAS[i][j]]*v[j] ; 
     }
 }
}
//------------------------------------
v1d Tools::skewmatsquare(cv1d & A) 
{
    v1d res (d*d, 0) ; 
    for (int i=0 ; i<d ; i++)
        for (int j=i ; j<d ; j++)
        {
         for (int k=0 ; k<d ; k++)
             res[i*d+j]+=A[MIndexAS[i][k]]*A[MIndexAS[k][j]]*MSigns[i][k]*MSigns[k][j] ; 
        }
    
    for (int i=1 ; i<d ; i++)
        for (int j=0 ; j<i ; j++)
            res[i*d+j]=res[j*d+i] ; 
        
    return res ; 
}
//-------------------------------------
void Tools::skewmatsquare(v1d & r, cv1d & A) 
{
    for (int i=0 ; i<d ; i++)
        for (int j=i ; j<d ; j++)
        {
         for (int k=0 ; k<d ; k++)
             r[i*d+j]+=A[MIndexAS[i][k]]*A[MIndexAS[k][j]]*MSigns[i][k]*MSigns[k][j] ; 
        }
    
    for (int i=1 ; i<d ; i++)
        for (int j=0 ; j<i ; j++)
            r[i*d+j]=r[j*d+i] ; 
        
}
//-------------------------------
v1d Tools::skewexpand(cv1d & A) 
{
    v1d res (d*d, 0) ; 
    for (int i=0 ; i<d*d; i++)
    {
        res[i]=A[MIndexAS[i/d][i%d]]*MSigns[i/d][i%d] ; 
    }
    return res ; 
}
//-------------------------------
void Tools::skewexpand(v1d & r, cv1d & A) 
{
    for (int i=0 ; i<d*d; i++)
    {
        r[i]=A[MIndexAS[i/d][i%d]]*MSigns[i/d][i%d] ; 
    }
}
//-------------------------------
v1d Tools::matmult (cv1d &A, cv1d &B)
{
    v1d res (d*d, 0) ; 
    for (int i=0 ; i<d; i++)
        for (int j=0 ; j<d ; j++)
            for (int k=0 ; k<d ; k++)
                res[i*d+j]+=A[i*d+k]*B[k*d+j] ; 
    return res ; 
}
//-------------------------------
void Tools::matmult (v1d & r, cv1d &A, cv1d &B)
{
    for (int i=0 ; i<d; i++)
        for (int j=0 ; j<d ; j++)
            for (int k=0 ; k<d ; k++)
                r[i*d+j]+=A[i*d+k]*B[k*d+j] ; 
}
//----------------------------------------
v1d Tools::wedgeproduct (cv1d &a, cv1d &b) 
{
  v1d res (d*(d-1)/2, 0) ; int k ; 
  auto iter = MASIndex.begin() ; 
  for (iter, k=0 ; iter!= MASIndex.end() ; iter++, k++)
      res[k]=a[iter->first]*b[iter->second]-a[iter->second]*b[iter->first] ; 
  return (res) ; 
}
//----------------------------------------
void Tools::wedgeproduct (v1d &res, cv1d &a, cv1d &b) 
{
  int k ; 
  auto iter = MASIndex.begin() ; 
  for (iter, k=0 ; iter!= MASIndex.end() ; iter++, k++)
      res[k]=a[iter->first]*b[iter->second]-a[iter->second]*b[iter->first] ; 
}
//-----------------------------------
void Tools::unitvec (vector <double> & v, int d, int n)
{
  for (int i=0 ; i<d ; i++) v[i]=(i==n?1:0) ; 
}


//==================================
double Tools::Volume (double R)
{
  if (d%2==0)
    return (pow(boost::math::double_constants::pi,d/2)*pow(R,d)/( boost::math::factorial<double>(d/2) )) ;  
  else
  {
   int k=(d-1)/2 ; 
   return(2* boost::math::factorial<double>(k) * pow(4*boost::math::double_constants::pi, k) *pow(R,d) / (boost::math::factorial<double>(d))) ;  
  }
}
//----------------------------------------
double Tools::InertiaMomentum (double R, double rho)
{
 if (d>MAXDEFDIM)
 {
  printf("[WARN] Inertia InertiaMomentum not guaranteed for dimension > %d\n", MAXDEFDIM) ; 
 }

 double res ; 
 if (d%2==0)
 {
   int k=d/2 ; 
   res=pow(boost::math::double_constants::pi,k)*pow(R, d+2) / boost::math::factorial<double>(k+1) ;
   return (res*rho) ;
 }
 else
 {
   int k=(d-1)/2 ; 
   res=pow(2,k+2) * pow(boost::math::double_constants::pi, k) * pow(R, d+2) / boost::math::double_factorial<double> (d+2) ; 
   return (res*rho) ; 
 }
}
  
/*  
Analytical functions for the momentum of inertia (cf mat script) 

(8*pi*R^5*rho)/15 d=3 double factorial of 5
(16*R^7*rho*pi^2)/105 d=5 double factorial of 7
(32*R^9*rho*pi^3)/945 d=7 double factorial of 9 etc
(64*R^11*rho*pi^4)/10395
(128*R^13*rho*pi^5)/135135
(256*R^15*rho*pi^6)/2027025
(512*R^17*rho*pi^7)/34459425
(1024*R^19*rho*pi^8)/654729075
(2048*R^21*rho*pi^9)/13749310575
(4096*R^23*rho*pi^10)/316234143225
(8192*R^25*rho*pi^11)/7905853580625
(16384*R^27*rho*pi^12)/213458046676875
(32768*R^29*rho*pi^13)/6190283353629375
(65536*R^31*rho*pi^14)/191898783962510625  
  
(pi*R^4*rho)/2
(R^6*rho*pi^2)/6
(R^8*rho*pi^3)/24
(R^10*rho*pi^4)/120
(R^12*rho*pi^5)/720
(R^14*rho*pi^6)/5040
(R^16*rho*pi^7)/40320
(R^18*rho*pi^8)/362880
(R^20*rho*pi^9)/3628800
(R^22*rho*pi^10)/39916800
(R^24*rho*pi^11)/479001600
(R^26*rho*pi^12)/6227020800
(R^28*rho*pi^13)/87178291200
(R^30*rho*pi^14)/1307674368000 (factorial 15)
(R^32*rho*pi^15)/20922789888000
  */




//=====================================================================================================NETCDF
#ifdef NETCDF
int NetCDFWriter::initialise (string path, initializer_list< v2d > & list, vector <string> names) 
{
int dimids[3] ; 
int ret = nc_create((path+".nc").c_str(), NC_CLOBBER, &ncid) ; 
if (ret) {printf("An error occured creating the netCDF file\n") ; return 0 ; }
 
nc_def_dim (ncid, "Grains", list.begin()->size(), dimensions) ; 
nc_def_dim (ncid, "Scalar", 1, dimensions+1) ; 
nc_def_dim (ncid, "Vector", Tools::getdim(), dimensions+2) ; 
nc_def_dim (ncid, "Tensor", Tools::getdim()*Tools::getdim(), dimensions+3) ; 
nc_def_dim (ncid, "SkewTensor", Tools::getdim()*(Tools::getdim()-1)/2, dimensions+4) ; 
nc_def_dim (ncid, "Time", NC_UNLIMITED, dimensions+5) ; 

dimids[0]=dimensions[5] ; dimids[2]=dimensions[0] ; // Dimension order: Time, grains, dim 
for (auto i=names.begin() ; i<names.end() ; i++)
{
 if (list.begin()->at(0).size() == 1) dimids[1]=dimensions[1] ; 
 else if (list.begin()->at(0).size() == Tools::getdim()) dimids[1]=dimensions[2] ; 
 else if (list.begin()->at(0).size() == Tools::getdim()*Tools::getdim()) dimids[1]=dimensions[3] ; 
 else if (list.begin()->at(0).size() == Tools::getdim()*(Tools::getdim()-1)/2) dimids[1]=dimensions[4] ; 
 else {printf("ERR: Unknown dimension size to write in netCDF\n") ; return 1 ; }
 varid.push_back(0) ;
 nc_def_var(ncid, (*i).c_str() , NC_DOUBLE, 3, dimids, &(varid[varid.size()-1])) ;
}
nc_enddef(ncid) ; 
first = false ; 
return 0 ; 
}
//--------------------------------------------------------------
int NetCDFWriter::saveNetCDF (initializer_list< v2d > & list) 
{
size_t start[3], count[3] ; 
for (auto i = varid.begin() ; i<varid.end() ; i++)
{
    auto & X=*(list.begin()) ; 
    start[0]=timerecord ; start[1]=X.size() ;  ; start[2]=0 ; 
    count[0]=count[1]=1 ; count[2]=X[0].size() ; 
    for (auto j=X.begin() ; j < X.end() ; j++)
        nc_put_vara_double (ncid, *i , start, count, j->data());
}
timerecord++ ; 
return timerecord ; 
}  
#endif


































