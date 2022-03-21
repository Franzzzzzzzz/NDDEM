#include "IOLiggghts.h"
#include<unistd.h>
//=======================================================
int main(int argc, char * argv[])
{
  //Datafile D("/Users/FGuillard/Simulations/MD/PostProcessing/dump.test") ;
  Param P ;

  if (argc<2) {printf("Expecting a json file as argument\n") ; std::exit(1) ; }

  std::ifstream i(argv[1]);
  if (i.is_open()==false) {printf("Cannot find the json file provided as argument\n") ; std::exit(1) ; }
  json param;
  try { i >> param; }
  catch(...)
  {
    printf("This is not a legal json file, here is what we already got:\n") ;
    cout << param ;
  }
  P.from_json(param) ;

  printf("Initializing\n") ; fflush(stdout) ;
  Datafile D ;
  D.cfmapping = P.cfmapping ;
  //D.open("/home/franz/Desktop/PostDoc_Local/EnergyBalance/002/dump.test.gz") ;
  //D.opencf("/home/franz/Desktop/PostDoc_Local/EnergyBalance/002/dump.forceEnergy.gz") ;

  D.open(P.atmdump) ;
  if (P.hascf) D.opencf(P.cfdump) ;
  D.periodicity=P.periodicity ;
  D.delta = P.delta ;
  D.boundaries = P.boundaries ;

  int maxT=P.maxT ;

  Coarsing C(P.dim, P.boxes, P.boundaries, maxT) ;
  for (auto i : P.extrafields)
    C.add_extra_field(i.name, i.order, i.type) ;
  if (P.window == Windows::LucyND_Periodic)      
    C.setWindow(P.window, P.windowsize, P.periodicity, P.boxes, P.delta) ; 
  else
    C.setWindow(P.window,P.windowsize) ;
  C.set_flags(P.flags) ;
  auto extrafieldmap = C.grid_setfields() ;

  C.cT=-1 ;

  for (int i=0 ; i<P.skipT; i++)
  {
    printf("\rSkipping timestep: %d | ", i) ; fflush(stdout) ;
    D.read_full_ts(false) ;
  }

  for (int i=0 ; i<maxT ; i++)
  {
    printf("\rTimestep: %d |", i) ; fflush(stdout) ;

    D.read_full_ts(true) ;
    C.cT++ ;
    D.set_data(C.data, extrafieldmap) ;

    if (P.maxlevel>=1)
      C.pass_1() ;

    /*if (P.maxlevel>=2)
    {
      C.compute_fluc_vel() ;
      //C.compute_fluc_rot() ;
      C.pass_2() ;
    }
    if (P.maxlevel>=3)
      C.pass_3() ;*/

  }

  C.mean_time(true) ;
  D.reopen(P.atmdump) ;
  if (P.hascf) D.reopencf(P.cfdump) ;

  C.cT=-1 ;
  for (int i=0 ; i<P.skipT; i++)
  {
    printf("\rSkipping timestep: %d | ", i) ; fflush(stdout) ;
    D.read_full_ts(false) ;
  }

  for (int i=0 ; i<maxT ; i++)
  {
    printf("\rTimestep: %d |", i) ; fflush(stdout) ;

    D.read_full_ts(true) ;
    C.cT++ ;
    D.set_data(C.data, extrafieldmap) ;

    //if (P.maxlevel>=1)
    //  C.pass_1() ;

    if (P.maxlevel>=2)
    {
      C.compute_fluc_vel(true) ;
      //C.compute_fluc_rot() ;
      C.pass_2() ;
    }
    if (P.maxlevel>=3)
      C.pass_3() ;
    

  }

  printf("\n") ;

  if (P.dotimeavg)
  {
    C.mean_time() ;
    //if (P.maxlevel>=4)
    C.cT=0 ; 
    C.pass_4() ;
  }

  if (std::find(P.saveformat.begin(), P.saveformat.end(), "netCDF")!=P.saveformat.end())   C.write_netCDF(P.save) ;
  if (std::find(P.saveformat.begin(), P.saveformat.end(), "vtk")!=P.saveformat.end()) C.write_vtk (P.save) ;
  if (std::find(P.saveformat.begin(), P.saveformat.end(), "mat")!=P.saveformat.end()) C.write_matlab(P.save, true) ;
  if (std::find(P.saveformat.begin(), P.saveformat.end(), "numpy")!=P.saveformat.end()) C.write_numpy(P.save, true) ;
  ///else printf("Unknown writing format, unfortunately.\n") ;

  printf("\n") ;
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
int Datafile::reopen(string path)
{
file_in->close() ; file_in=nullptr;
file_in= new ifstream(path, ios_base::in | ios_base::binary);
if (!file_in->is_open()) {printf("ERR: cannot open file %s\n", path.c_str()) ; return 1;}
if (path.find(".gz")!=string::npos)
  filt_in2.push(boost::iostreams::gzip_decompressor()) ;
filt_in2.push(*file_in);
in=new istream (&(filt_in2)) ;
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
int Datafile::reopencf(string path)
{
file_incf->close() ; file_incf=nullptr ;
file_incf= new ifstream(path, ios_base::in | ios_base::binary);
if (!file_incf->is_open()) {printf("ERR: cannot open file %s\n", path.c_str()) ; return 1;}
if (path.find(".gz")!=string::npos)
  filt_incf2.push(boost::iostreams::gzip_decompressor()) ;
filt_incf2.push(*file_incf);
incf=new istream (&(filt_incf2)) ;
return 0 ;
}
//-----------------------------------------------------------
vector<vector<double>> Datafile::get_bounds()
{
 vector<vector<double>> res(2, vector<double>(3,0)) ;
 string line ;

 getline(*in, line) ; //should be ITEM: TIMESTEP
 getline(*in, line) ;
 getline(*in, line) ; //should be NUMBER OF SOMETHING
 getline(*in, line) ;

 getline(*in, line) ; //should be BOX
 *in >> res[0][0] >> res[1][0] ;
 *in >> res[0][1] >> res[1][1] ;
 *in >> res[0][2] >> res[1][2] ;
 return (res) ;
}
//------------------------------------------------------------
int Datafile::get_numts()
{
 int numts = 0 ;
 string line ;
 do
 {
   getline(*in, line) ;
   if (line == "ITEM: TIMESTEP") numts++ ;
 } while (! in->eof()) ;
 return numts ;
}
//------------------------------------------------------------
int Datafile::read_full_ts(bool keep)
{
for (size_t i=0 ; i<data.size() ; i++) data[i].resize(0) ;
for (size_t i=0 ; i<datacf.size() ; i++) datacf[i].resize(0) ;
for (size_t i=0 ; i<tdata.size() ; i++) tdata[i].resize(0) ;
tdata.resize(0) ;
read_next_ts(in, false, keep) ;
if (incf != nullptr) read_next_ts(incf, true, keep) ;
return 0 ;
}
//--------------------------------------------------------------
int Datafile::read_next_ts(istream *is, bool iscf, bool keep)
{
 string line, word ; char blank ; int NN, nid ;

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

return 0 ;
}
//-------------------------------------------------
int Datafile:: do_post_atm()
{
 int i, k ; v1d nanvec (fields.size(),NAN) ; int nadded=0 ;
 auto j= tdata.begin() ;
 int idloc = std::find(fields.begin(), fields.end(), "id") - fields.begin();
 sort(tdata.begin(), tdata.end(), [=](auto v1, auto v2) {return v1[idloc] < v2[idloc] ; }); //WARNING idx 0 should be the particle ID

 for (i=0, j=tdata.begin() ; i<N ; i++, j++)
 {
  if (i+1<(*j)[idloc])
  {
    j=tdata.insert(j, nanvec) ;
    (*j)[idloc]=i+1 ;
    nadded++ ;
  }
 }

 for (i=0,j=tdata.begin() ; j<tdata.end() ; j++,i++)
 {
   (*j)[idloc]-- ;
   if ( (*j)[idloc] != i)
     printf("ERR shouldn't happen %d %g\n", i, (*j)[idloc]) ;
 }
 printf(" %d null atom / %d | ", nadded, N) ;
 N+=nadded ;

 const int nvalue = 13 ;
 data.resize(nvalue, v1d (0,0)) ; //Order: radius mass Imom posxyz velxyz omegaxyz
 vector<string>::iterator it ;
 vector<int> lst ;
 vector<string> flst = {"radius", "mass", "I","x","y","z","vx","vy","vz","omegax","omegay","omegaz","type"} ;
 for (size_t i=0 ; i<flst.size() ; i++)
 {
    it=std::find(fields.begin(), fields.end(), flst[i] ) ;
    if ( it != fields.end()) lst.push_back(it-fields.begin()) ;
    else lst.push_back(-1) ;
 }


 /*if (info)
 {
  info=false ;
  //cout << termcolor::green << "Found\t" << termcolor::yellow << "Contructed\t" << termcolor::red << "Missing\n" ;
  for (i=0 ; i<nvalue ; i++)
  {
   if (lst[i]>0){}// cout << termcolor::green ;
   else
   {
       //if (i==0 || i==1 || i==2 ) cout << termcolor::yellow ;
       //else cout << termcolor::red ;
   }
  cout << flst[i] << " " << lst[i] << "\n" ;
  }
 //cout << "\n" << termcolor::reset ;
 if (lst[0]<0 || lst[1]<0 || lst[2]<0) cout << "Reconstruction using Radius=" << Radius << " and density=" << Rho <<" if needed...\n";
}*/


 for (i=0 ; i<nvalue ; i++)
 {
     if (lst[i]>=0)
     {
         data[i].resize(N,0) ;
         for (k=0 ; k<N ; k++)
         {
           if (periodicity.size()>0 && (i==3 || i==4 || i==5) && (periodicity[i-3])) // Handle atom outside the simulation due to pbc. Important also for handling contact forces through PBC in the next function ...
           {
             if (tdata[k][lst[i]]<boundaries[0][i-3])
               data[i][k]=tdata[k][lst[i]]+delta[i] ;
             else if (tdata[k][lst[i]]>boundaries[1][i-3])
               data[i][k]=tdata[k][lst[i]]-delta[i] ;
             else
               data[i][k]=tdata[k][lst[i]] ;
           }
           else if (i==12)
           {
             data[i][k]=tdata[k][lst[i]]-1 ;
           }
           else
             data[i][k]=tdata[k][lst[i]] ;
         }
     }
     else
     {
         data[i].resize(N,0) ;
         if (i==0) for (k=0 ; k<N ; k++) data[i][k]=Radius ;
         if (i==1) for (k=0 ; k<N ; k++) data[i][k]=4/3. * M_PI * data[0][k] * data[0][k] * data[0][k] * Rho ;
         if (i==2) for (k=0 ; k<N ; k++) data[i][k]=2/5. * data[1][k] * data[0][k] * data[0][k] ;
     }
 }

 return nadded ;
}
//-------------------------------------------------
int Datafile::do_post_cf()
{
 const int nvalue=17 ; int swap ;

 datacf.resize(nvalue, vector <double> (0)) ;
 for (int i=0 ; i<nvalue ; i++) datacf[i].resize(Ncf,0) ;

 vector<string>::iterator it ;
 vector<int> lst ;
 vector<string> tofind = {"id1", "id2", "per", "fx", "fy", "fz", "mx", "my", "mz"} ;

 for (auto name : tofind)
 {
   auto mapper=cfmapping.find(name) ;
   if (mapper != cfmapping.end())
   {
       it=std::find(fieldscf.begin(), fieldscf.end(), mapper->second) ;
       if ( it != fieldscf.end())
           lst.push_back(it-fieldscf.begin()) ;
       else
           lst.push_back(-1) ;
   }
   else
    lst.push_back(-1) ;
 }

 /*it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[1]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //id1
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[2]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //id2
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[3]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //per
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[4]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //fx
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[5]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //fy
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[6]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //fz
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[7]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //mx
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[8]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //my
 it=std::find(fieldscf.begin(), fieldscf.end(), "c_cout[9]") ; if ( it != fieldscf.end()) lst.push_back(it-fieldscf.begin()) ; else lst.push_back(-1) ; //mz*/

 static bool messagefirst = true ;
 if (messagefirst)
 {
   printf("Assuming all the cf data are here (c_cout[1] to 9)...\n") ; fflush(stdout) ;
   messagefirst=false ;
 }

 int k=0 ;
 for (int j=0 ; j<Ncf ; j++)
 {


     //if (tdata[j][lst[2]]==1) continue ; // Remove any chainforce going through the PBC

     if (tdata[j][lst[0]]>tdata[j][lst[1]]) swap=1 ;
     else swap = 0 ;

     datacf[0][k]=tdata[j][lst[swap]]-1 ;                                       //id1
     datacf[1][k]=tdata[j][lst[1-swap]]-1 ;                                     //id2

     datacf[2][k]=  (data[3][datacf[0][k]] + data[3][datacf[1][k]]) /2.0 ;      //pos[0] //WARNING pb through PBC //WARNING WITH POLYDISPERSITY!!
     datacf[3][k]=  (data[4][datacf[0][k]] + data[4][datacf[1][k]]) /2.0 ;      //pos[1]
     datacf[4][k]=  (data[5][datacf[0][k]] + data[5][datacf[1][k]]) /2.0 ;      //pos[2]

     if (swap==1) swap=-1 ;
     else swap=1 ;
     datacf[5][k]=  (data[3][datacf[0][k]] - data[3][datacf[1][k]]) ;      //lpq[0] //WARNING pb through PBC
     datacf[6][k]=  (data[4][datacf[0][k]] - data[4][datacf[1][k]]) ;      //lpq[1]
     datacf[7][k]=  (data[5][datacf[0][k]] - data[5][datacf[1][k]]) ;      //lpq[2]

     // Let's handle the PBC now ...
     if (tdata[j][lst[2]]==1)
     {
       bool corrected=false ;
       for (size_t i=0 ; i<periodicity.size() ; i++)
       {
         if (periodicity[i])
         {
           if (fabs(fabs(data[3+i][datacf[0][k]] - data[3+i][datacf[1][k]]) - delta[i]) < fabs(datacf[5+i][k])) // Going through this PBC decreased the length of lpq
           {
             if (data[3+i][datacf[0][k]] - data[3+i][datacf[1][k]]<0) // it is either x1->x1+Delta or x2->x2-Delta
             {
               datacf[5+i][k] += delta[i]  ;
               if (datacf[2+i][k]+delta[i]/2. >= boundaries[0][i] && datacf[2+i][k]+delta[i]/2.<= boundaries[1][i])
                  datacf[2+i][k]+=delta[i]/2. ;
               else
                  datacf[2+i][k]-=delta[i]/2. ;
             }
             else // it is either x1->x1-Delta or x2->x2+Delta
             {
               datacf[5+i][k] -= delta[i]  ;
               if (datacf[2+i][k]+delta[i]/2. >= boundaries[0][i] && datacf[2+i][k]+delta[i]/2.<= boundaries[1][i])
                  datacf[2+i][k]+=delta[i]/2. ;
               else
                  datacf[2+i][k]-=delta[i]/2. ;

             }
           corrected=true ;
           }
         }
       }
       if (corrected==false)
            printf("- WARN: a contact force traversing the PBC was not corrected. - ") ;
     }

     if (lst[3]>=-1) datacf[8][k]=  swap*tdata[j][lst[3]] ;                                     //f[0]
     if (lst[4]>=-1) datacf[9][k]=  swap*tdata[j][lst[4]] ;                                     //f[1]
     if (lst[5]>=-1) datacf[10][k]= swap*tdata[j][lst[5]] ;                                     //f[2]

     //printf("%g\n", datacf[5][k]*datacf[8][k]+datacf[6][k]*datacf[9][k]+datacf[7][k]*datacf[10][k]) ;
     //std::exit() ;

     if (swap==-1) swap=1 ;
     else swap=0 ;
     if (lst[6]>-1) datacf[11][k]= tdata[j][lst[6]]*data[0][datacf[swap][k]]  ;           //mpq
     if (lst[7]>-1) datacf[12][k]= tdata[j][lst[7]]*data[0][datacf[swap][k]]  ;
     if (lst[8]>-1) datacf[13][k]= tdata[j][lst[8]]*data[0][datacf[swap][k]]  ;
     if (lst[6]>-1) datacf[14][k]= tdata[j][lst[6]]*data[0][datacf[1-swap][k]] ;          //mqp
     if (lst[7]>-1) datacf[15][k]= tdata[j][lst[7]]*data[0][datacf[1-swap][k]]  ;
     if (lst[8]>-1) datacf[16][k]= tdata[j][lst[8]]*data[0][datacf[1-swap][k]]  ;

     k++ ;
 }
 printf("-%d per contacts /%d | ", Ncf-k, Ncf) ;
 Ncf=k ;
return 0 ;
}


//==================================
int Datafile::set_data(struct Data & D, std::map<string,size_t> extrafieldmap)
{
    D.N=N ; D.Ncf=Ncf ;
    D.radius=&(data[0][0]) ;
    D.mass=&(data[1][0]) ;
    D.Imom=&(data[2][0]) ;

    D.pos.resize(3) ; D.pos={&(data[3][0]), &(data[4][0]), &(data[5][0])} ;
    D.vel.resize(3) ; D.vel={&(data[6][0]), &(data[7][0]), &(data[8][0])} ;
    D.omega.resize(3); D.omega={&(data[9][0]), &(data[10][0]), &(data[11][0])} ;

    if (incf != nullptr)
    {
      D.id1=&(datacf[0][0]) ;
      D.id2=&(datacf[1][0]) ;
      D.pospq.resize(3) ; D.pospq={&(datacf[2][0]),  &(datacf[3][0]),  &(datacf[4][0])} ;
      D.lpq.resize (3); D.lpq={&(datacf[5][0]),  &(datacf[6][0]),  &(datacf[7][0])} ;
      D.fpq.resize (3); D.fpq={&(datacf[8][0]),  &(datacf[9][0]),  &(datacf[10][0])} ;
      D.mpq.resize (3); D.mpq={&(datacf[11][0]), &(datacf[12][0]), &(datacf[13][0])} ;
      D.mqp.resize (3); D.mqp={&(datacf[14][0]), &(datacf[15][0]), &(datacf[16][0])} ;
    }

    for (auto &v: extrafieldmap)
    {
      if (D.extra.size() < v.second+1)
        D.extra.resize(v.second+1) ;
      D.extra[v.second] = &(data[12][0]) ;
    }

return 0 ;
}
