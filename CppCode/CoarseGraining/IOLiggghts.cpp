#include "IOLiggghts.h"

// Parameters
#define TORUN 2

#if TORUN==1
struct Param {
  string atmdump="/Users/FGuillard/Simulations/MD/EnergyBalance/005/dump.test.gz" ;
  string cfdump="/Users/FGuillard/Simulations/MD/EnergyBalance/005/dump.forceEnergy.gz" ; 
  int skipT=0 ; 
  int maxT = 3 ; 
  vector <string> flags = {"RHO", "I", "VAVG", "TC", "TK", "ROT", "MC", "MK", "mC", "EKT", "eKT", "EKR", "eKR", "qTC", "qTK", "qRC", "qRK", "zT", "zR"} ; 
  int dim=3 ; 
  vector <int> boxes= {10,10,14} ;
  vector <vector <double> > boundaries={{-0.0075,-0.0075,0},{0.0075,0.0075,0.021}} ; 
  string save="/Users/FGuillard/Simulations/MD/EnergyBalance/005/CG" ; 
} P; 

#elif TORUN==2
struct Param {
  string atmdump="/Users/FGuillard/Simulations/MD/Ellipsoids/Proper/Stresses/dump.testEll006.gz" ;
  string cfdump="/Users/FGuillard/Simulations/MD/Ellipsoids/Proper/Stresses/dump.forceEll006.gz" ; 
  int skipT=500 ; 
  int maxT = 450 ; 
  vector <string> flags = {"RHO", "VAVG", "TC", "TK",} ; 
  int dim=3 ; 
  vector <int> boxes= {10,10,10} ;
  vector <vector <double> > boundaries={{-0.015,-0.015,0},{0.015,0.015,0.03}} ; 
  string save="/Users/FGuillard/Simulations/MD/Ellipsoids/Proper/Stresses/CG" ; 
} P; 

#endif

//=======================================================
int main(int argc, char * argv[])
{
  //Datafile D("/Users/FGuillard/Simulations/MD/PostProcessing/dump.test") ; 

  if (argc==4)
  {
   P.atmdump=argv[1] ; 
   P.cfdump=argv[2] ; 
   P.save=argv[3] ;
  }
  else if (argc>1)
    {printf("ERROR: must give either 0 or 3 arguments\n") ; std::exit(1) ; }
  
  printf("Initializing\n") ; fflush(stdout) ;  
  Datafile D ; 
  //D.open("/home/franz/Desktop/PostDoc_Local/EnergyBalance/002/dump.test.gz") ; 
  //D.opencf("/home/franz/Desktop/PostDoc_Local/EnergyBalance/002/dump.forceEnergy.gz") ; 
  D.open(P.atmdump) ; 
  D.opencf(P.cfdump) ; 
  
  int maxT=P.maxT ; 
  Coarsing C(P.dim, P.boxes, P.boundaries, maxT) ; 
  C.set_flags(P.flags) ; 
  C.grid_setfields() ; 

  C.cT=-1 ; 
  
  for (int i=0 ; i<P.skipT; i++) 
  {
    printf("Timestep: %d\n", i) ; fflush(stdout) ;
    D.read_full_ts(false) ; 
    printf("\e[1A") ; 
  }
  
  for (int i=0 ; i<maxT ; i++)
  {
    printf("Timestep: %d\n", i) ; fflush(stdout) ; 
    D.read_full_ts(true) ; 
    C.cT++ ; 
    D.set_data(C.data) ;
    C.pass_1() ; 
    C.compute_fluc_vel() ; 
    C.compute_fluc_rot() ;
    C.pass_2() ;
    C.pass_3() ;
    if (i==0) printf("\e[9A\e[0J") ; 
    else printf("\e[13A\e[0J") ; 
  }
  
  C.mean_time() ; 
  C.write_netCDF(P.save) ; 
  //C.write_vtk (P.save) ; 
  //C.write_vtk("/home/franz/Desktop/PostDoc_Local/EnergyBalance/002/CG") ; 

}


//=============================================================
int Datafile::open(string path)
{
file_in= new ifstream(path, ios_base::in | ios_base::binary);
if (!file_in->is_open()) {printf("ERR: cannot open file %s\n", path.c_str()) ; return 1;}
if (path.find(".gz")!=string::npos) 
  filt_in.push(boost::iostreams::gzip_decompressor()) ; 
filt_in.push(*file_in);
in=new istream (&(filt_in)) ;   
return 0 ; 
}
int Datafile::opencf(string path)
{
file_incf= new ifstream(path, ios_base::in | ios_base::binary);
if (!file_incf->is_open()) {printf("ERR: cannot open file %s\n", path.c_str()) ; return 1;}
if (path.find(".gz")!=string::npos) 
  filt_incf.push(boost::iostreams::gzip_decompressor()) ; 
filt_incf.push(*file_incf);
incf=new istream (&(filt_incf)) ;   
return 0 ; 
}
//------------------------------------------------------------
int Datafile::read_full_ts(bool keep) 
{ 
for (int i=0 ; i<data.size() ; i++) data[i].resize(0) ; 
for (int i=0 ; i<datacf.size() ; i++) datacf[i].resize(0) ; 
for (int i=0 ; i<tdata.size() ; i++) tdata[i].resize(0) ; 
tdata.resize(0) ; 
read_next_ts(in, false, keep) ; 
read_next_ts(incf, true, keep) ; 
return 0 ; 
}
//--------------------------------------------------------------
int Datafile::read_next_ts(istream *is, bool iscf, bool keep)
{
 string line, word ; char blank ; int NN, nid ; 
 double val ; 
 getline(*is, line) ; //should be ITEM: TIMESTEP
 *is >> curts >> blank ; 
 getline(*is, line) ; //should be NUMBER OF SOMETHING
 *is >> NN >> blank;
 if (iscf) Ncf=NN ; else N=NN ; 

 getline(*is, line) ; //should be BOX
 getline(*is, line) ; //should be xx
 getline(*is, line) ; //should be yy
 getline(*is, line) ; //should be zz

 getline(*is, line) ;
 stringstream s (line) ; 
 s>>word ; s>>word ; // Discard these
 
 if (!keep) 
   is->ignore(numeric_limits<streamsize>::max(), 'I') ; //If we don't care about the data, read until the next timestep (starts with an 'I'tem timestep)
 else
 {
  if (iscf) fieldscf.resize(0) ;
  else fields.resize(0);
  while (!s.eof())
  {
    s>>word ; 
    if (iscf) fieldscf.push_back(word) ; 
    else fields.push_back(word) ;  
  }
  if (iscf) {fieldscf.pop_back() ; nid=fieldscf.size() ; }
  else {fields.pop_back() ; nid=fields.size() ; } // Last field is wrong, it was the newline ...
  
  tdata.resize(NN, v1d (fields.size(),0)) ; 
  
  for (int i=0 ; i<NN; i++) 
    for (int j=0 ; j<nid ; j++)
      *is>>tdata[i][j] ;  
  *is>>blank ; // Get rid of the last newline
  
  if (iscf) do_post_cf() ; 
  else do_post_atm() ;
 }
}
//-------------------------------------------------
int Datafile:: do_post_atm()
{
 int i, k ; v1d nanvec (fields.size(),NAN) ; int nadded=0 ; static bool info=true ; 
 auto j= tdata.begin() ;
 sort(tdata.begin(), tdata.end(), [](auto v1, auto v2) {return v1[0] < v2[0] ; }); //WARNING idx 0 should be the particle ID
 for (i=0, j=tdata.begin() ; i<N ; i++, j++)
 {
  if (i+1<(*j)[0])
  {
    j=tdata.insert(j, nanvec) ;  
    (*j)[0]=i+1 ; 
    nadded++ ; 
  }
 }

 for (i=0,j=tdata.begin() ; j<tdata.end() ; j++,i++)
 {
   (*j)[0]-- ; 
   if ( (*j)[0] != i) 
     printf("ERR shouldn't happen %d %g\n", i, (*j)[0]) ;
 }
 printf("%d atom null added\n", nadded) ; 
 N+=nadded ; 
 
 const int nvalue = 12 ;
 data.resize(nvalue, v1d (0,0)) ; //Order: mass Imom posxyz velxyz omegaxyz
 vector<string>::iterator it ;
 vector<int> lst ; 
 vector<string> flst = {"radius", "mass", "I","x","y","z","vx","vy","vz","omegax","omegay","omegaz"} ;
 for (i=0 ; i<flst.size() ; i++)
 {
    it=std::find(fields.begin(), fields.end(), flst[i] ) ; 
    if ( it != fields.end()) lst.push_back(it-fields.begin()) ; 
    else lst.push_back(-1) ; 
 }
 
 if (info)
 {
  info=false ; 
  cout << termcolor::green << "Found\t" << termcolor::yellow << "Contructed\t" << termcolor::red << "Missing\n" ; 
  for (i=0 ; i<nvalue ; i++) 
  {
   if (lst[i]>0) cout << termcolor::green ; 
   else
   {
       if (i==0 || i==1 || i==2 ) cout << termcolor::yellow ; 
       else cout << termcolor::red ; 
   }
  cout << flst[i] << " " ; 
  }
 cout << "\n" << termcolor::reset ;
 if (lst[0]<0 || lst[1]<0 || lst[2]<0) cout << "Reconstruction using Radius=" << Radius << " and density=" << Rho <<".\n"; 
 }
 
 for (i=0 ; i<nvalue ; i++)
 {
     if (lst[i]>=0)
     {
         data[i].resize(N,0) ; 
         for (k=0 ; k<N ; k++)
             data[i][k]=tdata[k][lst[i]] ; 
     }
     else
     {
         if (i==0 || i==1 || i==2) data[i].resize(N,0) ; 
         if (i==0) for (k=0 ; k<N ; k++) data[i][k]=Radius ; 
         if (i==1) for (k=0 ; k<N ; k++) data[i][k]=4/3. * M_PI * Radius * Radius * Radius * Rho ; 
         if (i==2) for (k=0 ; k<N ; k++) data[i][k]=2/5. * data[1][k] * Radius * Radius ; 
     }
 }

 return nadded ;
}
//-------------------------------------------------
int Datafile::do_post_cf()
{
 const int nvalue=17 ; static bool info=true ; int swap ;
 int i, j ; 
 
 datacf.resize(nvalue, vector <double> (0)) ; 
 for (i=0 ; i<nvalue ; i++) datacf[i].resize(Ncf,0) ; 
 
 vector<string>::iterator it ;
 vector<int> lst ; 
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[1]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //id1
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[2]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //id2
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[3]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //per
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[4]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //fx
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[5]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //fy
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[6]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //fz
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[7]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //mx
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[8]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //my
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[9]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //mz
 
 cout << "Assuming all the cf data are here (c_cout[1] to 9)...\n" ; fflush(stdout) ; 
 
 int k ; 
 for (j=0, k=0 ; j<Ncf ; j++)
 {
   
   
     if (tdata[j][lst[2]]==1) continue ; // Remove any chainforce going through the PBC 
   
     if (tdata[j][lst[0]]>tdata[j][lst[1]]) swap=1 ; 
     else swap = 0 ; 
     
     datacf[0][k]=tdata[j][lst[swap]]-1 ;                                       //id1
     datacf[1][k]=tdata[j][lst[1-swap]]-1 ;                                     //id2

     datacf[2][k]=  (data[3][datacf[0][k]] + data[3][datacf[1][k]]) /2.0 ;      //pos[0] //WARNING pb through PBC
     datacf[3][k]=  (data[4][datacf[0][k]] + data[4][datacf[1][k]]) /2.0 ;      //pos[1]
     datacf[4][k]=  (data[5][datacf[0][k]] + data[5][datacf[1][k]]) /2.0 ;      //pos[2]
     
     if (swap==1) swap=-1 ; 
     else swap=1 ; 
     datacf[5][k]=  (data[3][datacf[1][k]] - data[3][datacf[0][k]]) ;      //lpq[0] //WARNING pb through PBC
     datacf[6][k]=  (data[4][datacf[1][k]] - data[4][datacf[0][k]]) ;      //lpq[1]
     datacf[7][k]=  (data[5][datacf[1][k]] - data[5][datacf[0][k]]) ;      //lpq[2]
     
     datacf[8][k]=  swap*tdata[j][lst[3]] ;                                     //f[0]
     datacf[9][k]=  swap*tdata[j][lst[4]] ;                                     //f[1]
     datacf[10][k]= swap*tdata[j][lst[5]] ;                                     //f[2]
     
     //printf("%g\n", datacf[5][k]*datacf[8][k]+datacf[6][k]*datacf[9][k]+datacf[7][k]*datacf[10][k]) ; 
     //std::exit() ; 
     
     if (swap==-1) swap=1 ;
     else swap=0 ; 
     datacf[11][k]= tdata[j][lst[6]]*data[0][datacf[swap][k]]  ;           //mpq 
     datacf[12][k]= tdata[j][lst[7]]*data[0][datacf[swap][k]]  ;
     datacf[13][k]= tdata[j][lst[8]]*data[0][datacf[swap][k]]  ;
     datacf[14][k]= tdata[j][lst[6]]*data[0][datacf[1-swap][k]] ;          //mqp
     datacf[15][k]= tdata[j][lst[7]]*data[0][datacf[1-swap][k]]  ;
     datacf[16][k]= tdata[j][lst[8]]*data[0][datacf[1-swap][k]]  ;
     
     k++ ; 
 }
 printf("Removed %d periodic contacts out of %d\n", Ncf-k, Ncf) ; 
 Ncf=k ; 
return 0 ; 
}


//==================================
int Datafile::set_data(struct Data & D)
{
    D.N=N ; D.Ncf=Ncf ; 
    D.mass=&(data[1][0]) ; 
    D.Imom=&(data[2][0]) ; 
    
    D.pos.resize(3) ; D.pos={&(data[3][0]), &(data[4][0]), &(data[5][0])} ; 
    D.vel.resize(3) ; D.vel={&(data[6][0]), &(data[7][0]), &(data[8][0])} ; 
    D.omega.resize(3); D.omega={&(data[9][0]), &(data[10][0]), &(data[11][0])} ; 

    D.id1=&(datacf[0][0]) ; 
    D.id2=&(datacf[1][0]) ; 
    D.pospq.resize(3) ; D.pospq={&(datacf[2][0]),  &(datacf[3][0]),  &(datacf[4][0])} ; 
    D.lpq.resize (3); D.lpq={&(datacf[5][0]),  &(datacf[6][0]),  &(datacf[7][0])} ; 
    D.fpq.resize (3); D.fpq={&(datacf[8][0]),  &(datacf[9][0]),  &(datacf[10][0])} ; 
    D.mpq.resize (3); D.mpq={&(datacf[11][0]), &(datacf[12][0]), &(datacf[13][0])} ; 
    D.mqp.resize (3); D.mqp={&(datacf[14][0]), &(datacf[15][0]), &(datacf[16][0])} ; 
}






















