//#include "../Dem/Tools.h"
#include "Texturing.h"

#include <regex>
#include <limits>

//#define Nlambda 32
//#define Ntheta 32
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
    //dispvector(colors[0]) ;
    for (int i=0 ; i<d-2 ; i++)
    {
        ctmp += (colors[i] * fabs(sin(3*phi[i]))) ;
        sum += colors[i] ;
    }
    ctmp += (colors[d-2] * fabs(sin(4*phi[d-2]/2.))) ;
    sum += colors[d-2] ;
    rescale(ctmp,sum) ; //printf("%g %g %g\n", sum[0], sum[1], sum[2]);
    //for (int i=0 ; i<d-2 ; i++) ctmp *= sin(phi[i]) ;
    for (int i=0 ; i<d-2 ; i++)
      ctmp *= fabs(sin(phi[i])) ;
    //printf("%g %g %g\n", ctmp[0], ctmp[1], ctmp[2]) ;
    //ctmp = (colors[0]*sin(phi[0]) + colors[1]*sin(phi[1]/2)) * sin(phi[0]) ;
    cfinal = ctmp ;
    for (int i=2 ; i<3 ; i++)
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
