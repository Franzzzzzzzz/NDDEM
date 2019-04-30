#include "Coarsing.h"


// int main ()
// {
//  v2d b ; b.resize(2,std::vector <double> (3,0)) ;
//  b[1][0]=b[1][1]=b[1][2]=1 ;
//  v1i n ; n.push_back(3) ; n.push_back(4) ; n.push_back(5) ;
//  Coarsing C(3, n, b, 0.05, 100) ;
//
//  C.data.random_test(20,30,3,C.box) ;
//
//  C.flags=0 ;
//  for (int i=0 ; i<C.FIELDS.size() ; i++)
//      C.flags = (C.flags | C.FIELDS[i].flag)  ;
//
//  printf("%X\n", C.flags) ; fflush(stdout) ;
//
//  C.grid_setfields() ;
//  C.pass_1() ;
//  C.compute_fluc_vel() ;
//  C.compute_fluc_rot() ;
//  C.pass_2() ;
//  C.pass_3() ;
//  C.write_vtk("Test") ;
//
//  //std::exit(0) ;
//  /*for (int i=0 ; i<C.Npt ; i++)
//  {
//      printf("%d %g %g %g %g\n", i, C.CGP[i].location[0], C.CGP[i].location[1], C.CGP[i].location[2], C.CGP[i].location[3]) ;
//      for (int j=0 ; j<C.CGP[i].neighbors.size() ; j++)
//          printf("%d ", C.CGP[i].neighbors[j]) ;
//      printf("\n") ;
//  }*/
//
//
// }



//========================================================
int Coarsing::set_field_struct()
{
// Set the FIELD structure
FIELDS.push_back({0x000001, "RHO"  , "SCALAR"});    //Eq 36 Density
FIELDS.push_back({0x000002, "I"    , "SCALAR"});    //Eq 65 Moment of Inertia
FIELDS.push_back({0x000004, "VAVG" , "VECTOR"});    //Eq 38 Average Velocity
FIELDS.push_back({0x000008, "TC"   , "TENSOR"});    //Eq 63 Contact stress
FIELDS.push_back({0x000010, "TK"   , "TENSOR"});    //Eq 62 Kinetic stress
FIELDS.push_back({0x000020, "ROT"  , "VECTOR"});    //Eq 64 Internal spin density
FIELDS.push_back({0x000040, "MC"   , "TENSOR"});    //Eq 67 Contact couple stress tensor
FIELDS.push_back({0x000080, "MK"   , "TENSOR"});    //Eq 66 Kinetic couple stress tensor
FIELDS.push_back({0x000100, "mC"   , "VECTOR"});    //Eq 68 spin supply from contacts
FIELDS.push_back({0x000200, "EKT"  , "SCALAR"});    // VAVG^2/2 Kinetic energy density of average velocity
FIELDS.push_back({0x000400, "eKT"  , "SCALAR"});    //Eq 69 Kinetic energy density of fluctuating motion
FIELDS.push_back({0x000800, "EKR"  , "SCALAR"});    // (W.K.W)/2, kin energy of avg rotational motion, W angular velocity vector, K moment of inertia tensor
FIELDS.push_back({0x001000, "eKR"  , "SCALAR"});    //Eq 70 Kin energy of fluctuating rotational motion
FIELDS.push_back({0x002000, "qTC"  , "VECTOR"});    //Eq 72
FIELDS.push_back({0x004000, "qTK"  , "VECTOR"});    //Eq 71
FIELDS.push_back({0x008000, "qRC"  , "VECTOR"});    //Eq 74
FIELDS.push_back({0x010000, "qRK"  , "VECTOR"});    //Eq 73
FIELDS.push_back({0x020000, "zT"   , "SCALAR"});    //Eq 75
FIELDS.push_back({0x040000, "zR"   , "SCALAR"});    //Eq 76
}

//=========================================================
int Coarsing::grid_generate()
{
    int i, j ; int check ;
    v1d location ;
    location.resize(d,0) ;
    nptcum.resize(d,1) ;
    for (i=0,Npt=1 ; i<d ; i++)
    {
        for (int k=0 ; k<i ; k++) nptcum[k]*=npt[i] ;
        Npt*=npt[i] ;
        location[i]=box[0][i]+dx[i]/2. ;
        //printf("%g %g %g|", box[0][i], box[1][i], dx[i]) ;
    }
    for (i=0,check=0 ; i<Npt ; i++)
    {
        CGP.push_back(CGPoint(d, location)) ; check++ ;
        location[d-1]+=dx[d-1] ;
        for (int j=d-1 ; j>=1 ; j--)
        {
            if (location[j]>box[1][j])
            {
                location[j]=box[0][j]+dx[j]/2. ;
                location[j-1]+=dx[j-1] ;
            }
            else
                break ;
        }
    }
    return check ;
}
//--------------------------------
int Coarsing::grid_neighbour ()
{
    double dst ;
    for (int i=0 ; i<Npt ; i++)
    {
        CGP[i].neighbors.push_back(i) ;
        for (int j=0 ; j<Npt ; j++)
        {
            if (j==i) continue ;
            dst = normdiff(CGP[i].location, CGP[j].location) ;
            if (dst < cutoff) CGP[i].neighbors.push_back(j) ;
        }
    }
    double nsz=0 ;
    for (int i=0 ; i<Npt ; i++)
        nsz+=CGP[i].neighbors.size() ;
    printf("Typical neighbor number: %g \n",nsz/Npt) ;
}
//---------------------------------
int Coarsing::set_flags (vector <string> s)
{
 flags=0 ; Field * a;
 for (auto i = s.begin() ; i<s.end() ; i++)
 {
     a=get_field(*i) ;
     if (a!=NULL)
         flags |= a->flag ;
 }
 printf("Flags: %X\n", flags) ;
}

//--------------------------------
int  Coarsing::grid_setfields()
{
    v1d tmp=grid_getfields() ;
    for (int i=0 ; i<Npt ; i++)
        CGP[i].fields.resize(Time, tmp) ;
    return 0 ;
}
//--------------------------------
v1d Coarsing::grid_getfields()
{
v1d res ;

auto fl=flags ;
int n=0 ;
for (int i=0 ; i<FIELDS.size() ; i++)
{
    if (fl&1)
    {
        if (FIELDS[i].type.compare("SCALAR")==0)
        {
            Fields.push_back(FIELDS[i].name) ;
            res.push_back(0) ;
            Fidx.push_back(n) ;
            Ftype.push_back(1) ;
            Fname.push_back(FIELDS[i].name) ;
            n++ ;
        }
        else if (FIELDS[i].type.compare("VECTOR")==0)
        {
            for (int dd=0 ; dd<d ; dd++)
            {
             Fields.push_back(FIELDS[i].name+std::to_string(dd)) ;
             res.push_back(0) ;
            }
            Fidx.push_back(n) ;
            Ftype.push_back(2) ;
            Fname.push_back(FIELDS[i].name) ;
            n+=d ;
        }
        else if (FIELDS[i].type.compare("TENSOR")==0)
        {
            for (int dd=0 ; dd<d*d ; dd++)
            {
             Fields.push_back(FIELDS[i].name+std::to_string(dd/d)+"x"+std::to_string(dd%d)) ;
             res.push_back(0) ;
            }
            Fidx.push_back(n) ;
            Ftype.push_back(3) ;
            Fname.push_back(FIELDS[i].name) ;
            n+= d*d ;
        }
    }
    else
    {
        Fidx.push_back(-1) ;
        Ftype.push_back(-1) ;
        Fname.push_back("") ;
    }

    fl = fl >> 1 ;
}
return res ;
}
//----------------------------------------
int Coarsing::find_closest(int id)
{
 int res=0,idtmp=0 ;
 for (int dd=0 ; dd<d ; dd++)
 {
   idtmp=round((data.pos[dd][id]-box[0][dd])/dx[dd]) ;
   if (idtmp<0) idtmp=0 ;
   if (idtmp>=npt[dd]) idtmp=npt[dd]-1 ;
   res += idtmp*nptcum[dd] ;
 }
 return res ;
}
int Coarsing::find_closest_pq(int id)
{
 int res=0,idtmp=0 ;
 for (int dd=0 ; dd<d ; dd++)
 {
   idtmp=round((data.pospq[dd][id]-box[0][dd])/dx[dd]) ;
   if (idtmp<0) idtmp=0 ;
   if (idtmp>=npt[dd]) idtmp=npt[dd]-1 ;
   res += idtmp*nptcum[dd] ;
 }
 return res ;
}
//----------------------------------------
int Coarsing::get_id(string nm)
{
  int j=0 ;
  for (auto i=FIELDS.begin(); i<FIELDS.end() ; i++,j++)
  {
    if ( i->name.compare(nm) == 0) return Fidx[j] ;
  }
  printf("WARN: unexpected field searched %s\n", nm.c_str()) ;
  return -2 ;
}
//-----------------------------------------
struct Field * Coarsing::get_field(string nm)
{
    for (auto i=FIELDS.begin() ; i<FIELDS.end() ; i++)
        if (i->name==nm)
            return &(*i) ;
    return NULL ;
}

//===================================================
int Coarsing::compute_fluc_vel ()
{
  printf("Starting vel fluctuation computation [d=%d]\n", d) ; fflush(stdout) ;
  v1d vavg (d,0) ;
  data.vel_fluc.resize(d, std::vector <double> (data.N, 0.0)) ;

  int idvel=get_id("VAVG") ;
  if (idvel<0) {printf("WARN: cannot perform fluctuation velocity without VAVG set\n") ; }

  for (int i=0 ; i<data.N ; i++)
  {
    if (isnan(data.pos[0][i])) continue ;
    vavg=interpolate_vel (i) ;
    for (int dd=0 ; dd<d ; dd++)
      data.vel_fluc[dd][i]=data.vel[dd][i]-vavg[dd] ;
  }
}
int Coarsing::compute_fluc_rot ()
{
  printf("Starting rot fluctuation computation\n") ; fflush(stdout) ;
  v1d omegaavg (d,0) ;
  data.rot_fluc.resize(d, v1d (data.N, 0)) ;
  int idrot=get_id("ROT") ;
  if (idrot<0) {printf("WARN: cannot perform fluctuation rotation without ROT set\n") ; }
  for (int i=0 ; i<data.N ; i++)
  {
    if (isnan(data.pos[0][i])) continue ;
    omegaavg=interpolate_rot (i) ;
    for (int dd=0 ; dd<d ; dd++)
      data.rot_fluc[dd][i]=data.omega[dd][i]-omegaavg[dd] ;
  }
}

v1d Coarsing::interpolate_vel_nearest (int id)
{
  const static int idvel=get_id("VAVG") ; int idcg=find_closest(id) ; v1d res ;
  for (int i=0 ; i<d ; i++) res.push_back(CGP[idcg].fields[cT][idvel+i]) ;
  return res ;
}
v1d Coarsing::interpolate_rot_nearest (int id)
{
  const static int idrot=get_id("ROT") ; int idcg=find_closest(id) ; v1d res ;
  for (int i=0 ; i<d ; i++) res.push_back(CGP[idcg].fields[cT][idrot+i]) ;
  return res ;
}


//================================= BEGINNING OF THE INTERESTING PART ================================
//================ PASS 1 : "RHO", "VAVG", "ROT", "EKT", "EKR"==============================
int Coarsing::pass_1 ()
{
int i, dd, id ; double wp ;
bool dorho=true, dovel=true, doomega=true, doI=true ;
int rhoid=get_id("RHO") ; if (rhoid<0) {dorho=false ; return 0;}
int Iid = get_id("I") ; if (Iid<0) doI=false ;
int velid=get_id("VAVG"); if (velid<0)  dovel=false ;
int omegaid=get_id("ROT");if (omegaid<0) doomega=false ;
printf("Starting pass 1...\n") ; fflush(stdout) ;

double dm, dI ; v1d dv (d,0), dom(d,0) ; double * CGf ; // Speed things up a bit ...

for (i=0 ; i<data.N ; i++)
{
 if (isnan(data.pos[0][i])) continue ;
 id=find_closest(i) ;
 dm=data.mass[i] ; dI=data.Imom[i] ;
 for (dd=0 ; dd<d ; dd++) {if (dovel) dv[dd]=data.vel[dd][i] ; if (doomega) dom[dd]=data.omega[dd][i] ; }

 for (auto j=CGP[id].neighbors.begin() ; j<CGP[id].neighbors.end() ; j++)
 {
     wp=window(distance(i,CGP[*j].location)) ;
     CGf = &(CGP[*j].fields[cT][0]) ;
     //if (*j>100) printf("%g %g %g | %g %g %g\n", CGP[*j].location[0], CGP[*j].location[1], CGP[*j].location[2], data.pos[0][i],  data.pos[1][i], data.pos[2][i]) ;
     if (wp>0) CGP[*j].natom ++ ;
     if (dorho)
       CGP[*j].fields[cT][rhoid] += wp * data.mass[i] ;
     if (doI)
       *(CGf+Iid) += wp * dm * dI ;
     if (dovel)
       for (dd=0 ; dd<d ; dd++)
         *(CGf+velid+dd) += wp * dm * dv[dd] ;
     if (doomega)
       for (dd=0 ; dd<d ; dd++)
         *(CGf+omegaid+dd) += wp * dm * dom[dd] * dI ;
 }
}

// Intermediate pass (cg points)
bool doEKT=true, doEKR=true ;
int EKTid=get_id("EKT") ; if (EKTid<0) doEKT=false ;
int EKRid=get_id("EKR") ; if (EKRid<0) doEKR=false ;
printf("Starting intermediate pass 1...\n") ; fflush(stdout) ;
for (i=0 ; i<Npt ; i++)
{
    double rho, Imom ;
    rho = CGP[i].fields[cT][rhoid] ;
    if (doI && rho!=0) {CGP[i].fields[cT][Iid] /= rho ; Imom=CGP[i].fields[cT][Iid] ; }
    if (dovel && rho!=0)   for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][velid+dd] /= rho ;
    if (doomega && rho!=0) for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][omegaid+dd] /= (rho*Imom) ;
    if (doEKT)
    {
      for (dd=0 ; dd<d ; dd++)
        CGP[i].fields[cT][EKTid] += CGP[i].fields[cT][velid+dd]*CGP[i].fields[cT][velid+dd] ;
      CGP[i].fields[cT][EKTid] /= 2.0 ;
    }
    if (doEKR)
    {
      for (dd=0 ; dd<d ; dd++)
        CGP[i].fields[cT][EKRid] +=  CGP[i].fields[cT][omegaid+dd]*CGP[i].fields[cT][omegaid+dd] * Imom ;
      CGP[i].fields[cT][EKRid] /= 2.0 ;
    }
}
}

//================ PASS 2 : "TK", "MK", "eKT", "eKR", "qTK", "qRK"==============================
int Coarsing::pass_2()
{
int i, dd ; double velfluc, rotfluc ; int id ; double wp ;
bool doeKT=true , doeKR=true, doqTK=true, doqRK=true, doTK=true, doMK=true ;
int rhoid=get_id("RHO") ; if (rhoid<0) {return 0;}
int eKTid=get_id("eKT") ; if (eKTid<0) doeKT=false ;
int eKRid=get_id("eKR") ; if (eKRid<0) doeKR=false ;
int qTKid=get_id("qTK") ; if (qTKid<0) doqTK=false ;
int qRKid=get_id("qRK") ; if (qRKid<0) doqRK=false ;
int TKid =get_id("TK")  ; if (TKid<0) doTK=false ;
int MKid =get_id("MK")  ; if (MKid<0) doMK=false ;
printf("Starting pass 2...\n") ; fflush(stdout) ;
for (i=0 ; i<data.N ; i++)
{
 if (isnan(data.pos[0][i])) continue ;

 id=find_closest(i) ;
 for (dd=0, velfluc=0 ; dd<d ; dd++) velfluc+=data.vel_fluc[dd][i]*data.vel_fluc[dd][i] ;
 for (dd=0, rotfluc=0 ; dd<d ; dd++) rotfluc+=data.rot_fluc[dd][i]*data.rot_fluc[dd][i] ;

 for (auto j=CGP[id].neighbors.begin() ; j<CGP[id].neighbors.end() ; j++)
 {
     wp=window(distance(i,CGP[*j].location)) ;

     if (doeKT)
         CGP[*j].fields[cT][eKTid] += wp * data.mass[i] * velfluc ;
     if (doeKR)
         CGP[*j].fields[cT][eKRid] += wp * data.mass[i] * rotfluc * data.Imom[i] ;
     if (doTK)
       for (dd=0 ; dd<d*d ; dd++)
         CGP[*j].fields[cT][TKid+dd] += wp * data.mass[i] * data.vel_fluc[dd/d][i] * data.vel_fluc[dd%d][i] ;
     if (doMK)
       for (dd=0 ; dd<d*d ; dd++)
         CGP[*j].fields[cT][MKid+dd] += wp * data.mass[i] * data.vel_fluc[dd/d][i] * data.rot_fluc[dd%d][i] * data.Imom[i] ;
     if (doqTK)
       for (dd=0 ; dd<d ; dd++)
         CGP[*j].fields[cT][qTKid+dd] += wp * data.mass[i] * velfluc * data.vel_fluc[dd][i] ;
     if (doqRK)
       for (dd=0 ; dd<d ; dd++)
         CGP[*j].fields[cT][qRKid+dd] += wp * data.mass[i] * rotfluc * data.Imom[i] * data.vel_fluc[dd][i]  ;
 }
}
// Intermediate pass (cg points): devide by rho when needed
printf("Starting intermediate 2...\n") ; fflush(stdout) ;
for (i=0 ; i<Npt ; i++)
{
    double tworho ;
    tworho = 2*CGP[i].fields[cT][rhoid] ;

    if (doeKT && tworho!=0) CGP[i].fields[cT][eKTid] /= tworho ;
    if (doeKR && tworho!=0) CGP[i].fields[cT][eKRid] /= tworho ;
    if (doqTK) for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][qTKid+dd] *= (-0.5) ;
    if (doqRK) for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][qRKid+dd] *= (-0.5) ;
}
}

//================ PASS 3 : "TC", "MC", "mC", "qTC", "qRC", "zt", "zr"==============================
int Coarsing::pass_3()
{
int i, dd ;
bool dozT=true , dozR=true, doTC=true, doMC=true, domC=true, doqTC=true, doqRC=true ;
int zTid=get_id("zT") ; if (zTid<0) dozT=false ;
int zRid=get_id("zR") ; if (zRid<0) dozR=false ;
int TCid=get_id("TC") ; if (TCid<0) doTC=false ;
int MCid=get_id("MC") ; if (MCid<0) doMC=false ;
int mCid=get_id("mC") ; if (mCid<0) domC=false ;
int qTCid=get_id("qTC") ; if (qTCid<0) doqTC=false ;
int qRCid=get_id("qRC") ; if (qRCid<0) doqRC=false ;
double sum=0 ; int p, q, id ; double rp, rq ; double wpqs, wpqf ;
printf("Starting pass 3...\n") ; fflush(stdout) ;
for (i=0 ; i<data.Ncf ; i++)
{
 id=find_closest_pq(i) ;
 p=data.id1[i] ; q=data.id2[i] ; if (q<=p) printf("ERR: contacts should be ordered so that p<q\n") ;
 //printf("%d ", i) ;
 for (auto j=CGP[id].neighbors.begin() ; j<CGP[id].neighbors.end() ; j++)
 {
  rp=distance(p, CGP[*j].location) ;
  rq=distance(q, CGP[*j].location) ;
  wpqs = window_avg (rp, rq) ;
  wpqf = window_int (rp, rq) ;
  //printf("%g %g %g", rp, rq, wpqf) ;
  if (dozT)
  {
    for (dd=0, sum=0 ; dd<d ; dd++)
      sum+= data.fpq[dd][i] * (data.vel[dd][p]-data.vel[dd][q]) ;
    CGP[*j].fields[cT][zTid] -= wpqs * sum ;
  }
  if (dozR)
  {
    for (dd=0, sum=0 ; dd<d ; dd++)
      sum += (data.mpq[dd][i]*data.omega[dd][p] + data.mqp[dd][i]*data.omega[dd][q]) ;
    CGP[*j].fields[cT][zRid] -= wpqs * sum ;
  }
  if (doqTC)
  {
    for (dd=0,sum=0 ; dd<d ; dd++)
     sum += data.fpq[dd][i] * (data.vel_fluc[dd][p]+data.vel_fluc[dd][q]) ;
    for (dd=0 ; dd<d ; dd++)
      CGP[*j].fields[cT][qTCid+dd] += wpqf * data.lpq[dd][i] * sum ;
  }
  if (doqRC)
  {
    for (dd=0,sum=0 ; dd<d ; dd++)
     sum += data.mpq[dd][i] * data.rot_fluc[dd][p] - data.mqp[dd][i] * data.rot_fluc[dd][q] ;
    for (dd=0 ; dd<d ; dd++)
      CGP[*j].fields[cT][qRCid+dd] += wpqf * data.lpq[dd][i] * sum ;
  }

  if (doTC)
  {
   for (dd=0 ; dd<d*d ; dd++)
       CGP[*j].fields[cT][TCid+dd] += wpqf * data.lpq[dd/d][i] * data.fpq[dd%d][i] ;
  }

  if (doMC)
    for (dd=0 ; dd<d*d ; dd++)
      CGP[*j].fields[cT][MCid+dd] += wpqf * data.lpq[dd/d][i] * (data.mpq[dd%d][i] - data.mqp[dd%d][i]) ;

  if (domC)
  {
    if (d!=3) printf("ERR: mC not properly implemented for d != 3\n") ;
    CGP[*j].fields[cT][mCid+0] += wpqs * (data.lpq[1][i] * data.fpq[2][i] - data.lpq[2][i] * data.fpq[1][i]) ;
    CGP[*j].fields[cT][mCid+1] += wpqs * (data.lpq[2][i] * data.fpq[0][i] - data.lpq[0][i] * data.fpq[2][i]) ;
    CGP[*j].fields[cT][mCid+2] += wpqs * (data.lpq[0][i] * data.fpq[1][i] - data.lpq[1][i] * data.fpq[0][i]) ;
  }

 }
}

//Last intermediate pass
printf("Starting intermediate pass 3...\n") ; fflush(stdout) ;
for (i=0 ; i<Npt ; i++)
{
  //printf("%g %g %g %g %g %g %g %g\n", CGP[i].fields[cT][TCid+0], CGP[i].fields[cT][TCid+1],CGP[i].fields[cT][TCid+2],CGP[i].fields[cT][TCid+3]
  //  , CGP[i].fields[cT][TCid+4], CGP[i].fields[cT][TCid+5], CGP[i].fields[cT][TCid+6], CGP[i].fields[cT][TCid+7], CGP[i].fields[cT][TCid+8]);
    //printf("%g ", CGP[i].natom) ;
    if (doqTC) for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][qTCid] *= 0.5 ;
    if (doqRC) for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][qRCid] *= 0.5 ;
    if (doMC) for (dd=0 ; dd<d*d ; dd++) CGP[i].fields[cT][MCid] *= 0.5 ;
}
}



//=================================================================
// Data conversion tools
int Data::random_test (int NN, int NNcf, int d, v2d box )
{
const double meanrad=0.00075, rho=2500 ;
boost::mt19937 rng ;
boost::uniform_01<boost::mt19937> rand(rng) ;
double rad, tmp1, tmp2, tmp3 ; int tmp ;

N=NN ;
mass=(double*)malloc(N*sizeof(double)) ;
Imom=(double*)malloc(N*sizeof(double)) ;
pos.resize(d,NULL) ; for (int dd=0 ; dd<d ; dd++) pos[dd]=(double*) malloc(N*sizeof(double)) ;
vel.resize(d,NULL) ; for (int dd=0 ; dd<d ; dd++) vel[dd]=(double*) malloc(N*sizeof(double)) ;
omega.resize(d,NULL) ; for (int dd=0 ; dd<d ; dd++) omega[dd]=(double*) malloc(N*sizeof(double)) ;

for (int i=0 ; i<N ; i++)
{
  rad=meanrad+rand()*0.2*meanrad ;
  mass[i]=4/3.*M_PI*rad*rad*rad*rho ;
  Imom[i]=2/5.*mass[i]*rad*rad ;

  for (int dd=0 ; dd<d ; dd++) pos[dd][i]=box[0][dd]+rand()*(box[1][dd]-box[0][dd]) ;
  for (int dd=0 ; dd<d ; dd++) vel[dd][i]=rand() ;
  for (int dd=0 ; dd<d ; dd++) omega[dd][i]=rand() ;
}

Ncf=NNcf ;
id1=(double*)malloc(Ncf*sizeof(double)) ;
id2=(double*)malloc(Ncf*sizeof(double)) ;

pospq.resize(d,NULL) ; for (int dd=0 ; dd<d ; dd++) pospq[dd]=(double*) malloc(Ncf*sizeof(double)) ;
lpq.resize(d,NULL) ; for (int dd=0 ; dd<d ; dd++) lpq[dd]=(double*) malloc(Ncf*sizeof(double)) ;
fpq.resize(d,NULL) ; for (int dd=0 ; dd<d ; dd++) fpq[dd]=(double*) malloc(Ncf*sizeof(double)) ;
mpq.resize(d,NULL) ; for (int dd=0 ; dd<d ; dd++) mpq[dd]=(double*) malloc(Ncf*sizeof(double)) ;
mqp .resize(d,NULL) ; for (int dd=0 ; dd<d ; dd++) mqp[dd]=(double*) malloc(Ncf*sizeof(double)) ;

for (int i=0 ; i<Ncf ; i++)
{
  id1[i]=floor(rand()*N) ;
  do {id2[i]=floor(rand()*N) ; } while (id2[i]==id1[i]) ;
  if (id2[i]<id1[i]) {tmp=id2[i] ; id2[i]=id1[i] ; id1[i]=tmp ; }
  for (int dd=0 ; dd<d ; dd++) pospq[dd][i]=box[0][dd]+rand()*(box[1][dd]-box[0][dd]) ;

  for (int dd=0 ; dd<d ; dd++) fpq[dd][i]=rand() ;
  for (int dd=0 ; dd<d ; dd++) mpq[dd][i]=rand() ;
  for (int dd=0 ; dd<d ; dd++) mqp[dd][i]=rand() ;
}

compute_lpq(d) ;
return 0 ;
}
//---------------------------------------------------
int Data::compute_lpq (int d)
{
 for (int i=0 ; i<Ncf ; i++)
  for (int dd=0 ; dd<d ; dd++)
    lpq[dd][i]=pos[dd][int(id1[i])]-pos[dd][int(id2[i])] ; //TODO CHECK THE ORDER

 return 0 ;
}


//====================================================
int Coarsing::mean_time()
{
  int tf=CGP[0].fields[0].size() ;
  for (int i=0 ; i<Npt ; i++)
    for (int t=1 ; t<Time ; t++)
    {
      for (int f=0 ; f<tf ; f++)
        CGP[i].fields[0][f]+=CGP[i].fields[t][f] ;
    }
  for (int i=0 ; i<Npt ; i++)
    for (int f=0 ; f<tf ; f++)
    {
        CGP[i].fields[0][f] /= Time;
        //CGP[i].fields.resize(1) ;
    }
  Time=1 ;
  return 0 ;
}
//--------------------------------------------
int Coarsing::write_vtk(string sout)
{
  FILE *out ;
  if (d!=3) printf("WARN: the write_vtk function hasn't been implement for dimension different from 3\n") ;
  for (int t=0 ; t<Time ; t++)
  {
    out=fopen((sout+"-"+std::to_string(t)+".vtk").c_str(), "w") ;
    if (out==NULL) {printf("ERR: Cannot open file to write to.") ; return 1 ; }
    fprintf(out, "# vtk DataFile Version 2.0\nSome data\nASCII\nDATASET STRUCTURED_POINTS\nDIMENSIONS %d %d %d\nORIGIN %g %g %g\nSPACING %g %g %g\n", npt[0], npt[1], npt[2], CGP[0].location[0], CGP[0].location[1], CGP[0].location[2], dx[0], dx[1], dx[2]) ;
    fprintf(out, "POINT_DATA %d\n", Npt) ;
    for (int f=0 ; f<Fidx.size() ; f++)
    {
      if (Fidx[f]<0) continue ;
      switch (Ftype[f])
      {
        case 1: fprintf(out, "SCALARS %s float \nLOOKUP_TABLE default \n", Fname[f].c_str()) ;
            for (int k=0 ; k<npt[2] ; k++)
             for (int j=0 ; j<npt[1] ; j++)
              for (int i=0 ; i<npt[0] ; i++)
                fprintf(out, "%g%c", CGP[i*npt[1]*npt[2]+j*npt[2]+k].fields[t][Fidx[f]], i%90==89?'\n':' ') ;
              break ;
        case 2: fprintf(out, "VECTORS %s float \n", Fname[f].c_str()) ;
             for (int k=0 ; k<npt[2] ; k++)
              for (int j=0 ; j<npt[1] ; j++)
               for (int i=0 ; i<npt[0] ; i++)
                for (int dd=0 ; dd<d ; dd++)
                  fprintf(out, "%g%c", CGP[i*npt[1]*npt[2]+j*npt[2]+k].fields[t][Fidx[f]+dd], (i%30==29&&dd==d-1)?'\n':' ') ;
              break ;
        case 3: fprintf(out, "TENSORS %s float \n", Fname[f].c_str()) ;
            for (int k=0 ; k<npt[2] ; k++)
             for (int j=0 ; j<npt[1] ; j++)
              for (int i=0 ; i<npt[0] ; i++)
                for (int dd=0 ; dd<d*d ; dd++)
                  fprintf(out, "%g%c", CGP[i*npt[1]*npt[2]+j*npt[2]+k].fields[t][Fidx[f]+dd], (i%10==9&&dd==d*d-1)?'\n':' ') ;
              break ;
        default: printf("ERR: this should never happen. \n") ;
      }
      fprintf(out, "\n\n") ;
    }
  fclose(out) ;
  }
}
//-------------------------------------------------------
int Coarsing ::write_NrrdIO (string path)
{
#ifdef NRRDIO
    double * outdata ;
    int dimtime=d+2 ;

    Nrrd *nval;
    auto nio = nrrdIoStateNew();
    nrrdIoStateEncodingSet(nio, nrrdEncodingAscii) ; //Change to nrrdEncodingRaw for binary encoding
    nval = nrrdNew();

    // Header infos
    vector <size_t> dimensions (3+d, 0) ;
    for (int dd=0 ; dd<d ; dd++) dimensions[dd+2] = npt[dd] ;
    dimensions[dimtime] = Time ;

    vector <int> nrrdkind (3+d, nrrdKindSpace) ;
    nrrdkind[0]=nrrdkind[1]= nrrdKindVector ;
    nrrdkind[dimtime]=nrrdKindTime ;
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoKind, nrrdkind.data() );

    vector <double> nrrdmin(3+d,0), nrrdmax(3+d,0), nrrdspacing(3+d,0) ;
    for (int dd=0 ; dd<d ; dd++)
    {
        nrrdmin[dd+2]=box[0][dd] ; nrrdmax[dd+2]=box[1][dd] ;
        nrrdspacing[dd+2]=dx[dd] ;
    }
    nrrdmin[0]=nrrdmin[1]=nrrdmax[0]=nrrdmax[1]=nrrdspacing[0]=nrrdspacing[1]=AIR_NAN ;
    nrrdmin[dimtime]=nrrdmax[dimtime]=nrrdspacing[dimtime] ;

    char ** labels;
    labels=(char **) malloc(sizeof(char *) * (d+3)) ;
    for (int dd=0 ; dd<d ; dd++)
    {
        labels[dd+2]=(char *) malloc(sizeof(char) * (1+d/10+1+1)) ;
        sprintf(labels[dd+2], "x%d", dd) ;
    }
    labels[0]=(char *) malloc(sizeof(char) * (15)) ; sprintf(labels[0], "TensorialDim1") ;
    labels[1]=(char *) malloc(sizeof(char) * (15)) ; sprintf(labels[1], "TensorialDim2") ;
    labels[dimtime]=(char *) malloc(sizeof(char) * (5 )) ; sprintf(labels[dimtime], "Time") ;

    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoLabel, labels);
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoMin, nrrdmin.data());
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoMax, nrrdmax.data());
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoSpacing, nrrdspacing.data());

    for (int f=0 ; f<Fidx.size() ; f++)
    {
      if (Fidx[f]<0) continue ;


    // Data are goind fast to slow in NrrdIO ... so probably need some rewrite ...
      switch (Ftype[f])
      {
        case 1: dimensions[0]=dimensions[1]=1 ;  //Scalar
                outdata=(double *) malloc(sizeof(double) * 1 * Npt * Time) ;
                for (int t=0 ; t<Time ; t++)
                    for (int i=0 ; i<Npt ; i++)
                        outdata[t*Npt+i]=CGP[i].fields[t][Fidx[f]] ;
              break ;
        case 2: dimensions[0]=d ; dimensions[1]=1 ; // Vector
                outdata=(double *) malloc(sizeof(double) * d * Npt * Time) ;
                for (int t=0 ; t<Time ; t++)
                    for (int i=0 ; i<Npt ; i++)
                        for (int j=0 ; j<d ; j++)
                            outdata[t*Npt*d + i*d +j]=CGP[i].fields[t][Fidx[f+j]] ;
              break ;
        case 3: dimensions[0]=dimensions[1]=d ; //Tensor
                for (int t=0 ; t<Time ; t++)
                    for (int i=0 ; i<Npt ; i++)
                        for (int j=0 ; j<d*d ; j++)
                            outdata[t*Npt*d*d + i*d*d +j/d*d + j%d]=CGP[i].fields[t][Fidx[f+j]] ; // j/d*d!=j because of integer division
                outdata=(double *) malloc(sizeof(double) * d*d * Npt * Time) ;
              break ;
        default: printf("ERR: this should never happen. \n") ;
      }

      nrrdWrap_nva(nval, outdata, nrrdTypeDouble, d+3, dimensions.data());
      string fullpath ;
      fullpath = path + Fname[f] + ".nrrd" ;
      nrrdSave(fullpath.c_str(), nval, nio);
      free(outdata) ;
    }
#endif
}


//------------------------------------------
int Coarsing::write_netCDF (string sout)
{
#ifdef NETCDF
 float *res ; int ret, ncid ; int dim1, dim2s, dim2v, dim2t, dim3 ; int n ; int err,v ;
 int dimids[3] ; long unsigned int sizes[3] ; long unsigned int zeros[3]={0,0,0} ;
 vector <int> varid ; char dimname[60] ; CGPoint * pt ;
 sout+=".nc" ;
 ret = nc_create(sout.c_str(), NC_CLOBBER, &ncid) ;
 if (ret) {printf("An error occured creating the netCDF file\n") ; return 0 ; }
 res=(float*) malloc(sizeof(float)*1) ;

 // Dimension definitions
 nc_def_dim (ncid, "Points", Npt, &dim1) ;
 nc_def_dim (ncid, "Scalar", 1, &dim2s) ;
 nc_def_dim (ncid, "Vector", d, &dim2v) ;
 nc_def_dim (ncid, "Tensor", d*d, &dim2t) ;
 nc_def_dim (ncid, "Time", Time, &dim3) ;

 // Variable definitions
 varid.resize(2) ;
 dimids[0]=dim2v ; dimids[1]=dim1 ;
 nc_def_var(ncid, "CGPointsLocation", NC_FLOAT, 2, dimids, &(varid[0])) ;
 nc_def_var(ncid, "CGGrid", NC_INT, 1, &dim2v, &(varid[1])) ;

 for (int f=0 ; f<Fidx.size() ; f++)
 {
   if (Fidx[f]<0) continue ;
   switch (Ftype[f])
   {
    case 1:  dimids[0]=dim3 ; dimids[1]=dim2s ; dimids[2]=dim1 ;
             varid.push_back(0) ;
             nc_def_var(ncid, Fname[f].c_str(), NC_FLOAT, 3, dimids, &(varid[varid.size()-1])) ;
             break ;
    case 2:  dimids[0]=dim3 ; dimids[1]=dim2v ; dimids[2]=dim1 ;
             varid.push_back(0) ;
             err = nc_def_var(ncid, Fname[f].c_str(), NC_FLOAT, 3, dimids, &(varid[varid.size()-1])) ;
             break ;
    case 3 : dimids[0]=dim3 ; dimids[1]=dim2t ; dimids[2]=dim1 ;
             varid.push_back(0) ;
             err = nc_def_var(ncid, Fname[f].c_str(), NC_FLOAT, 3, dimids, &(varid[varid.size()-1])) ;
             break ;
    default : printf("ERR: this should never happen. (netCDF write)\n") ;
   }
 }
 nc_enddef(ncid) ;

 // Variable writing
 res=(float*)realloc(res, sizeof(float)*Npt*d) ;
 for (int j=0, n=0 ; j<d ; j++)
  for (int i=0; i<Npt ; i++, n++)
     res[n]=CGP[i].location[j] ;
 sizes[0]=d ; sizes[1]=Npt ;
 nc_put_vara_float(ncid, varid[0], zeros, sizes, res) ;
 unsigned long int uld=d ;
 nc_put_vara_int(ncid, varid[1], zeros, &uld, &(npt[0])) ;

 sizes[0]=Time ; sizes[2]=Npt ;
 for (int f=0, v=2 ; f<Fidx.size() ; f++)
 {
   if (Fidx[f]<0) continue ;
   switch (Ftype[f])
   {
     case 1: res=(float*)realloc(res, sizeof(float)*Time*Npt*1) ;
             for (int t=0, n=0 ; t<Time ; t++)
               for (int i=0 ; i<Npt ; i++, n++)
                  res[n]=CGP[i].fields[t][Fidx[f]] ;
             sizes[1]=1 ;
             err=nc_put_vara_float(ncid, varid[v], zeros, sizes, res) ;
             v++ ;
       break ;
     case 2: res=(float*)realloc(res, sizeof(float)*Time*Npt*d) ;
             for (int t=0,n=0 ; t<Time ; t++)
               for (int dd=0 ; dd<d ; dd++)
                 for (int i=0 ; i<Npt ; i++, n++)
                   res[n]=CGP[i].fields[t][Fidx[f]+dd] ;
             sizes[1]=d ;
             nc_put_vara_float(ncid, varid[v], zeros, sizes, res)  ;
             v++ ;
       break ;
     case 3: res=(float*)realloc(res, sizeof(float)*Time*Npt*d*d) ;
             for (int t=0,n=0 ; t<Time ; t++)
               for (int dd=0 ; dd<d*d ; dd++)
                 for (int i=0 ; i<Npt ; i++, n++)
                   res[n]=CGP[i].fields[t][Fidx[f]+dd] ;
             sizes[1]=d*d ;
             nc_put_vara_float(ncid, varid[v], zeros, sizes, res) ;
             v++ ;
       break ;
     default: printf("ERR: this should never happen. (MAT write)\n") ;
   }
 }

 nc_close(ncid) ;

 printf("WARNING: when reading and reshaping with matlab, the dimension order is reversed (ie: reshape (V, [d3,d2,d1,d0, 4, T]) for a vector V for example") ;
 return 0 ;
#else
printf("Coarsing not compile with netCDF support. Use the macro -DNETCDF at the compiling stage to enable such suppport. Writing aborted.\n") ;
return 1 ;
#endif
}




//-----------------------------------------------
CGPoint * Coarsing::reverseloop (string type)
{
  static vector <int> index ; int idx=0 ;

  if (type == "init") // Init
  {
   index.resize(d+1,0) ;
   for (int i=0 ; i<d ; i++) index[i]=0 ;
   return &(CGP[0]) ;
  }

  else if (type == "next") // next
  {
   int j=0 ; index[j]++ ;
   while (j<d && index[j]>=npt[j])
   {
     index[j]=0 ;
     index[j+1]++ ;
     j++ ;
   }
   if (j==d) return NULL ; // No more CGPoints

   for (int i=0 ; i<d ; i++)
     idx+=index[i]*nptcum[i] ;

   return &(CGP[idx]) ;
  }

return NULL ;
}
