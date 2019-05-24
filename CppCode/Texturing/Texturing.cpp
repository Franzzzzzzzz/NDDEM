//#include "../Dem/Tools.h"
#include "Texturing.h"

#include <regex>
#include <limits>



#define DeltaX 0.1
//#define Nlambda 32
//#define Ntheta 32

using namespace std ;

void dispvector (const v1d & a) {for (auto v: a) printf("%g ", v); printf("\n") ; fflush(stdout) ; }
void dispvector (const v1f & a) {for (auto v: a) printf("%g ", v); printf("\n") ; fflush(stdout) ; }

void runthread_timeloop (Texturing * T, v1d View, uint tsint, int nrotate) 
{T->timeloop(View, tsint,nrotate) ; }
void runthread_spaceloop (Texturing * T, v1d View, uint tsint, int nrotate, int dim) 
{T->spaceloop(View, tsint,nrotate, dim) ; }

//==================================================
int Texturing::initialise (map <string,string> & args)
{
 d = atoi(args["ND"].c_str()) ; 
 Tools::initialise(d) ;
 View.resize(d,0) ;
 string Directory=args["path"] ; 
 DirectorySave= args["texturepath"] ; 
 
 set_grid (atoi(args["resolution"].c_str())) ; 

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
 TsName.resize(filelistloc.size(), 0) ; 
 for (uint i=0 ; i<filelistloc.size() ; i++) TsName[i]=filelistloc[i].first ;

 // Let's read everything
 Ts.resize(filelistloc.size()) ;
 for (uint i=0 ; i<filelistloc.size() ; i++)
  {
    R.clear() ;
    csvread_XR (filelistloc[i].second.c_str(), Ts[i].X, R, d) ;
  }
 for (uint i=0 ; i<filelistA.size() ; i++) csvread_A(filelistA[i].second.c_str(), Ts[i].A, d) ;
 N = Ts[0].X.size() ;
 int nrotate=3 ; // Rotate all the coordinates already
 for (auto & v : Ts)
 {
    for (int i=0 ; i<N ; i++)
        rotate(v.X[i].begin(), v.X[i].begin()+nrotate, v.X[i].end()) ;
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
 if (static_cast<uint>(d-1)>colors.size()) printf("ERR: not enough color gradients!!\n") ;
 if (d-3<allcolors.size()) colors=allcolors[d-3] ;

 ViewPoint.resize(d-3+1, INT_MIN) ; 
 Threads.resize(d-3+1) ; 
 justloaded=true ; 
 return 0 ; 
} 


//==================================================
int Texturing::clean()
{
Tools::clear() ;
vector<Timestep>().swap(Ts) ; 
return 0 ;
}
//=================================================
int Texturing::MasterRender(map <string,string> args)
{
 int Time=atoi(args["ts"].c_str()) ;
 char dimstr[10] ; 
 View[0]=View[1]=View[2]=NAN ; 
 for (uint dd=3 ; dd<d ; dd++)
 {
   sprintf(dimstr, "x%d", dd) ; 
   View[dd]=atof(args[dimstr].c_str()) ; 
 }
 vector <int> NewViewPoint(d-3+1,0) ;
    
 int nrotate = viewpermute (View, d) ;
 if (nrotate != 3) printf("WHAT? No the first 3D should be NaNs ...") ; 
 for (uint i=0 ; i<d-3 ; i++) {NewViewPoint[i] = static_cast<int>(round(View[i]/DeltaX));}
    
 auto tmp  = find(TsName.begin(), TsName.end(), Time) ;
 if (tmp == TsName.end()) {printf("WARN: not found timestep\n") ; return -1 ; }
 NewViewPoint[d-3] = tmp-TsName.begin() ; 

 //printf("%d %d | %d %d\n", ViewPoint[0], ViewPoint[1], NewViewPoint[0], NewViewPoint[1]) ;
 // Alright, lets start the threads
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
    Threads[i] = std::thread(runthread_spaceloop, this, View, NewViewPoint[d-3], nrotate, i) ;
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
   Threads[d-3] = std::thread(runthread_timeloop, this, View, NewViewPoint[d-3], nrotate) ;
   }
 }

 if (d==3 && justloaded)
 {
   Threads[d-3] = std::thread(runthread_timeloop, this, View, NewViewPoint[d-3], nrotate) ;
   justloaded=false ;
 }
ViewPoint=NewViewPoint ;
return 0 ; 
}

//===========================================================
void Texturing::spaceloop (v1d View, uint tsint, int nrotate, int dim)
{
for (auto & v : FileList[dim]) experimental::filesystem::remove(v.c_str()) ;

FileList[dim].clear() ;

auto Viewdec = View ;
while (Viewdec[dim]>Boundaries[0][dim] || View[dim]<Boundaries[1][dim])
{
  Viewdec[dim] -= DeltaX ;
  View[dim] += DeltaX ;

  printf("s") ; fflush(stdout) ;

  if (Viewdec[dim]>Boundaries[0][dim]) Render(FileList[dim], Viewdec, nrotate, TsName[tsint], Ts[tsint].X, R, Ts[tsint].A) ;
  if (View[dim]<Boundaries[1][dim])    Render(FileList[dim], View,    nrotate, TsName[tsint], Ts[tsint].X, R, Ts[tsint].A) ;
}
}
//-----------------------------------------------------
void Texturing::timeloop (v1d View, uint tsint, int nrotate)
{
int dim=FileList.size()-1 ; 
for (auto & v : FileList[dim]) experimental::filesystem::remove(v.c_str()) ;

FileList[dim].clear() ;
uint timeidxinit=tsint ;
do {
Render(FileList[dim],View, nrotate, TsName[tsint], Ts[tsint].X, R, Ts[tsint].A) ;
tsint++ ;
if (tsint>=Ts.size()) tsint=0 ;
} while (tsint != timeidxinit) ;
}

//-----------------------------------------------------
void Texturing::Render (vector <string> & filerendered, cv1d & View, int nrotate, int time, cv2d &X, cv1d & R, cv2d &A)
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
             phi2color (img.begin() + n*3, phinew, d, colors) ;
             n++ ;
         }

     char path[5000] ;
     filepathname(path, i, time, View) ;
     write_img(path, Ntheta, Nlambda, img.data(), i) ;
     filerendered.push_back(path);
 }
return ; 
}
//--------------------------------------------------------
int Texturing::set_grid(int nb)
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
int Texturing::viewpermute (v1d & View, int d)
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
//====================================================
void Texturing::filepathname (char * path, int n, int time, cv1d &View)
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
void phi2color (vector<uint8_t>::iterator px, cv1d & phi, int d, vector<vector<float>> & colors)
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

/*int render (int argc, char * argv[])
{
 //FILE * piping ; char line[5000] ;
 //piping=fopen("pipe", "r") ; if (piping==NULL) {printf("ERR: a pipe is expected\n") ; exit(1) ; }

 bool run = true ;
 vector <vector<string>> FileList (d-3+1) ;
 vector <std::thread> Threads (d-3+1) ;
 int TimeCur ;
 bool firstrun = true ;
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

   if (!strcmp(line, "stop"))
   {
     for (auto &w : FileList)
        for (auto & v : w)
          experimental::filesystem::remove(v.c_str()) ;
     for (auto & v : Threads)
     {
       if (v.joinable())
       {
         auto ThreadID = v.native_handle() ;
         pthread_cancel(ThreadID);
         v.join() ;
       }
     }
     run=false ;
   }
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
     for (uint i=0 ; i<d-3 ; i++) {NewViewPoint[i] = static_cast<int>(round(View[i]/DeltaX));}
     NewViewPoint[d-3]= TimeCur ;

     uint TimeCurInt = find_if(filelistloc.begin(), filelistloc.end(), [=](std::pair<int,string>a){return (a.first==TimeCur);})-filelistloc.begin() ;

     printf("%d %d | %d %d\n", ViewPoint[0], ViewPoint[1], NewViewPoint[0], NewViewPoint[1]) ;
     // Alright, lets start the threads
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
            Threads[i] = std::thread(spaceloop, std::ref(FileList[i]), View, nrotate, i, TimeCur, std::ref(X[TimeCurInt]), std::ref(R), std::ref(A[TimeCurInt])) ;
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
         Threads[d-3] = std::thread(timeloop, std::ref(FileList[d-3]), View, nrotate, std::ref(timelst), TimeCurInt, std::ref(X), std::ref(R), std::ref(A)) ;
       }
     }

     if (d==3) // Special case for d=3, run anyway
     {
       if (firstrun)
       {
         Threads[d-3] = std::thread(timeloop, std::ref(FileList[d-3]), View, nrotate, std::ref(timelst), TimeCurInt, std::ref(X), std::ref(R), std::ref(A)) ;
         firstrun=false ;
       }
     }

     ViewPoint=NewViewPoint ;
   }
   else {usleep(10000) ; } // A little sleep :)
 }

 return 0 ;
}*/
