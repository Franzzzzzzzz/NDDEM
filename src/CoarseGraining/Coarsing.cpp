#include "Coarsing.h"

//========================================================
int Coarsing::set_field_struct()
{
/* Set the FIELD structure.
 * ids 0x00hhhhhhhh are code defined
 * idx 0x01hhhhhhhh are particle base (pass 1) user defined field
 * idx 0x02hhhhhhhh are fluctuation base (pass 2) user defined fields
 * idx 0x04hhhhhhhh are contact base (pass 3) user defined fields
 */
FIELDS.push_back({                    1, "RHO"  , TensorOrder::SCALAR, FieldType::Defined, Pass::Pass1});                        //Eq 36 Density
FIELDS.push_back({FIELDS.back().flag<<1, "I"    , TensorOrder::SCALAR, FieldType::Defined, Pass::Pass1});    //Eq 65 Moment of Inertia
FIELDS.push_back({FIELDS.back().flag<<1, "VAVG" , TensorOrder::VECTOR, FieldType::Defined, Pass::Pass1});    //Eq 38 Average Velocity
FIELDS.push_back({FIELDS.back().flag<<1, "TC"   , TensorOrder::TENSOR, FieldType::Defined, Pass::Pass3});    //Eq 63 Contact stress
FIELDS.push_back({FIELDS.back().flag<<1, "TK"   , TensorOrder::TENSOR, FieldType::Defined, Pass::VelFluct|Pass::Pass2|Pass::Pass1});    //Eq 62 Kinetic stress
FIELDS.push_back({FIELDS.back().flag<<1, "ROT"  , TensorOrder::VECTOR, FieldType::Defined, Pass::Pass1});    //Eq 64 Internal spin density
FIELDS.push_back({FIELDS.back().flag<<1, "MC"   , TensorOrder::TENSOR, FieldType::Defined, Pass::Pass3});    //Eq 67 Contact couple stress tensor
FIELDS.push_back({FIELDS.back().flag<<1, "MK"   , TensorOrder::TENSOR, FieldType::Defined, Pass::VelFluct|Pass::RotFluct|Pass::Pass2|Pass::Pass1});    //Eq 66 Kinetic couple stress tensor
FIELDS.push_back({FIELDS.back().flag<<1, "mC"   , TensorOrder::VECTOR, FieldType::Defined, Pass::Pass3});    //Eq 68 spin supply from contacts
FIELDS.push_back({FIELDS.back().flag<<1, "EKT"  , TensorOrder::SCALAR, FieldType::Defined, Pass::Pass1});    // VAVG^2/2 Kinetic energy density of average velocity
FIELDS.push_back({FIELDS.back().flag<<1, "eKT"  , TensorOrder::SCALAR, FieldType::Defined, Pass::VelFluct|Pass::Pass2|Pass::Pass1});    //Eq 69 Kinetic energy density of fluctuating motion
FIELDS.push_back({FIELDS.back().flag<<1, "EKR"  , TensorOrder::SCALAR, FieldType::Defined, Pass::Pass1});    // (W.K.W)/2, kin energy of avg rotational motion, W angular velocity vector, K moment of inertia tensor
FIELDS.push_back({FIELDS.back().flag<<1, "eKR"  , TensorOrder::SCALAR, FieldType::Defined, Pass::RotFluct|Pass::Pass2|Pass::Pass1});    //Eq 70 Kin energy of fluctuating rotational motion
FIELDS.push_back({FIELDS.back().flag<<1, "qTC"  , TensorOrder::VECTOR, FieldType::Defined, Pass::VelFluct|Pass::Pass4});    //Eq 72
FIELDS.push_back({FIELDS.back().flag<<1, "qTK"  , TensorOrder::VECTOR, FieldType::Defined, Pass::VelFluct|Pass::Pass2|Pass::Pass1});    //Eq 71
FIELDS.push_back({FIELDS.back().flag<<1, "qRC"  , TensorOrder::VECTOR, FieldType::Defined, Pass::RotFluct|Pass::Pass4});    //Eq 74
FIELDS.push_back({FIELDS.back().flag<<1, "qRK"  , TensorOrder::VECTOR, FieldType::Defined, Pass::VelFluct|Pass::RotFluct|Pass::Pass2|Pass::Pass1});    //Eq 73
FIELDS.push_back({FIELDS.back().flag<<1, "zT"   , TensorOrder::SCALAR, FieldType::Defined, Pass::Pass3});    //Eq 75
FIELDS.push_back({FIELDS.back().flag<<1, "zR"   , TensorOrder::SCALAR, FieldType::Defined, Pass::Pass3});    //Eq 76

FIELDS.push_back({FIELDS.back().flag<<1, "RADIUS"   , TensorOrder::SCALAR, FieldType::Defined, Pass::Pass1});    //Eq 76

// Post processing fields
FIELDS.push_back({FIELDS.back().flag<<1, "TotalStress" ,     TensorOrder::TENSOR, FieldType::Defined, Pass::Pass5});
FIELDS.push_back({FIELDS.back().flag<<1, "Pressure" ,        TensorOrder::SCALAR, FieldType::Defined, Pass::Pass5});
FIELDS.push_back({FIELDS.back().flag<<1, "KineticPressure" , TensorOrder::SCALAR, FieldType::Defined, Pass::Pass5});
FIELDS.push_back({FIELDS.back().flag<<1, "ShearStress" ,     TensorOrder::SCALAR, FieldType::Defined, Pass::Pass5});

FIELDS.push_back({FIELDS.back().flag<<1, "StrainRate",           TensorOrder::TENSOR, FieldType::Defined, Pass::Pass5});
FIELDS.push_back({FIELDS.back().flag<<1, "VolumetricStrainRate", TensorOrder::SCALAR, FieldType::Defined, Pass::Pass5});
FIELDS.push_back({FIELDS.back().flag<<1, "ShearStrainRate",      TensorOrder::SCALAR, FieldType::Defined, Pass::Pass5});
FIELDS.push_back({FIELDS.back().flag<<1, "RotationalVelocity",   TensorOrder::VECTOR, FieldType::Defined, Pass::Pass5});
FIELDS.push_back({FIELDS.back().flag<<1, "RotationalVelocityMag",   TensorOrder::SCALAR, FieldType::Defined, Pass::Pass5});

return 0 ;
} //, "Pressure", "KineticPressure", "ShearStress", "VolumetricStrainRate", "ShearStrainRate"
//----------------------------------------------------------
int Coarsing::add_extra_field(string name, TensorOrder order, FieldType type)
{
  uint64_t tmp = FIELDS.back().flag<<1 ;
  int datalocation ;

  switch(type) {
    case FieldType::Particle :
      datalocation = data.add_extra_field(1, name) ;
      FIELDS.push_back({tmp, name, order, type, Pass::Pass1, datalocation});
      break ;
    case FieldType::Fluctuation :
      datalocation = data.add_extra_field(d, name) ;
      FIELDS.push_back({tmp, name, order, type, Pass::Pass2, datalocation});
      break ;
    case FieldType::Contact :
      datalocation = data.add_extra_field(d*d, name) ;
      FIELDS.push_back({tmp, name, order, type, Pass::Pass3, datalocation});
      break ;
    default: printf("Unknown extra field type, skipping\n") ;
  }
  return tmp;
}

//-------------------------------------------------------
int Coarsing::setWindow (Windows win, double w, vector <bool> per, vector<int> boxes, vector<double> deltas)
{
 switch (win) {
  case Windows::Rect3D :
    setWindow<Windows::Rect3D> (w) ;
    break ;
  case Windows::Sphere3DIntersect :
    setWindow<Windows::Sphere3DIntersect> (w) ;
    break ;
  case Windows::SphereNDIntersect :
    setWindow<Windows::SphereNDIntersect> (w) ;
    break ;
  case Windows::Lucy3D :
    setWindow<Windows::Lucy3D> (w) ;
    break ;
  case Windows::Lucy3DFancyInt:
    setWindow<Windows::Lucy3DFancyInt> (w) ;
    break ;
  case Windows::Hann3D :
    setWindow<Windows::Hann3D> (w) ;
    break ;
  case Windows::RectND :
    setWindow<Windows::RectND> (w) ;
    break ;
  case Windows::LucyND :
    setWindow<Windows::LucyND> (w) ;
    break ;
  case Windows::LucyND_Periodic:
    setWindow<Windows::LucyND_Periodic> (w, per, boxes, deltas) ;
    break ;
  default:
    printf("Unknown window, check Coarsing::setWindow\n") ;
 }
return 0 ;
}

//=========================================================
int Coarsing::grid_generate()
{
    int i; int check ;
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

    /*for (auto v : nptcum) printf("-%d-", v) ;
    printf("\n") ; fflush(stdout) ; */
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
        CGP[i].neighbors.clear() ;
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
return 0 ;
}
//---------------------------------
Pass Coarsing::set_flags (vector <string> s)
{
 Pass Res = static_cast<Pass>(0) ;
 flags=0 ; Field * a;
 for (auto i = s.begin() ; i<s.end() ; i++)
 {
     a=get_field(*i) ;
     if (a!=NULL)
     {
         flags |= a->flag ;
         Res = Res | a->passlevel ;
     }
 }
 printf("Flags: %X | Pipelines %X \n", flags, static_cast<int>(Res) ) ;
return Res ;
}

//--------------------------------
std::map<std::string, size_t> Coarsing::grid_setfields()
{
    vector<FieldType> tmp=grid_getfields() ;
    std::map<std::string, size_t> extrafields ;

    printf("Approximate memory required: %ld MB\n", Npt*tmp.size()*Time*sizeof(double)/1024/1024) ; fflush(stdout) ;

    for (size_t i=0 ; i<tmp.size() ; i++)
      if (tmp[i]!=FieldType::Defined)
        extrafields[Fields[i]] = i ;

    for (int i=0 ; i<Npt ; i++)
        CGP[i].fields.resize(Time, vector<double>(tmp.size(),0)) ;

    return extrafields ;
}
//--------------------------------
vector<FieldType> Coarsing::grid_getfields()
{
vector<FieldType> res ;

auto fl=flags ;
int n=0 ;
for (size_t i=0 ; i<FIELDS.size() ; i++)
{
    if (fl&1)
    {
        if (FIELDS[i].type== TensorOrder::SCALAR)
        {
            Fields.push_back(FIELDS[i].name) ;
            res.push_back(FIELDS[i].ftype) ;
            Fidx.push_back(n) ;
            Ftype.push_back(TensorOrder::SCALAR) ;
            Fname.push_back(FIELDS[i].name) ;
            n++ ;
        }
        else if (FIELDS[i].type==TensorOrder::VECTOR)
        {
            for (int dd=0 ; dd<d ; dd++)
            {
             Fields.push_back(FIELDS[i].name+std::to_string(dd)) ;
             res.push_back(FIELDS[i].ftype) ;
            }
            Fidx.push_back(n) ;
            Ftype.push_back(TensorOrder::VECTOR) ;
            Fname.push_back(FIELDS[i].name) ;
            n+=d ;
        }
        else if (FIELDS[i].type==TensorOrder::TENSOR)
        {
            for (int dd=0 ; dd<d*d ; dd++)
            {
             Fields.push_back(FIELDS[i].name+std::to_string(dd/d)+"x"+std::to_string(dd%d)) ;
             res.push_back(FIELDS[i].ftype) ;
            }
            Fidx.push_back(n) ;
            Ftype.push_back(TensorOrder::TENSOR) ;
            Fname.push_back(FIELDS[i].name) ;
            n+= d*d ;
        }
    }
    else
    {
        Fidx.push_back(-1) ;
        Ftype.push_back(TensorOrder::NONE) ;
        Fname.push_back("") ;
    }

    fl = fl >> 1 ;
}
return res ;
}
//----------------------------------------
v2d Coarsing::get_bounds ()
{
 v2d res (2, v1d (d)) ; // vector 2xd, min & max reach of the CG points

 for (auto & v : res[0]) v=+std::numeric_limits<double>::infinity() ;
 for (auto & v : res[1]) v=-std::numeric_limits<double>::infinity() ;

 for (auto & v : CGP)
     for (int i=0 ; i<d ; i++)
     {
         if (v.location[i]<res[0][i]) res[0][i]= v.location[i] ;
         if (v.location[i]>res[1][i]) res[1][i]= v.location[i] ;
     }
 for (auto & v : res[0]) v -= cutoff ;
 for (auto & v : res[1]) v += cutoff ;
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


//*******************************************
/*int Coarsing::find_closest(int id)
{
 int res=0,idtmp=0 ;
 vector<int> loc (d,0) ;
 for (int dd=0 ; dd<d ; dd++)
 {
   idtmp=floor((data.pos[dd][id]-box[0][dd])/dx[dd]) ;
   if (idtmp<0) idtmp=0 ;
   if (idtmp>=npt[dd]) idtmp=npt[dd]-1 ;
   res += idtmp*nptcum[dd] ;
 }
 return res ;
}*/
//********************************************


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
int Coarsing::compute_fluc_vel (bool usetimeavg)
{
  // printf(" -> VelFluct") ; fflush(stdout) ;
  v1d vavg (d,0) ;
  data.vel_fluc.resize(d, std::vector <double> (data.N, 0.0)) ;

  int idvel=get_id("VAVG") ;
  if (idvel<0) {printf("WARN: cannot perform fluctuation velocity without VAVG set\n") ; }
  for (int i=0 ; i<data.N ; i++)
  {
    if (isnan(data.pos[0][i])) continue ;
    //auto vavg = interpolate_vel (i, usetimeavg) ;
    vavg = interpolate_vel_trilinear(i,usetimeavg) ;
    //vavg = interpolate_vel_nearest(i,usetimeavg) ;
    //printf("%g %g | %g %g | %g %g\n", vavg[0], vavg2[0], vavg[1], vavg2[1], vavg[2], vavg2[2]) ;
    for (int dd=0 ; dd<d ; dd++)
      data.vel_fluc[dd][i]=data.vel[dd][i]-vavg[dd] ;
  }

  hasvelfluct = true ;
  return 0 ;
}
//-----
int Coarsing::compute_fluc_rot (bool usetimeavg)
{
  static bool messagefirst=true ;
  // printf(" -> RotVelFluct") ; fflush(stdout) ;
  v1d omegaavg (d,0) ;
  data.rot_fluc.resize(d, v1d (data.N, 0)) ;
  int idrot=get_id("ROT") ;
  if (idrot<0 && messagefirst) {messagefirst=false ; printf("WARN: cannot perform fluctuation rotation without ROT set\n") ; }
  for (int i=0 ; i<data.N ; i++)
  {
    if (isnan(data.pos[0][i])) continue ;
    omegaavg=interpolate_rot (i, usetimeavg) ;
    for (int dd=0 ; dd<d ; dd++)
      data.rot_fluc[dd][i]=data.omega[dd][i]-omegaavg[dd] ;
  }
  hasrotfluct = true ;
  return 0 ;
}
//-----
v1d Coarsing::interpolate_vel_nearest (int id, bool usetimeavg)
{
  const static int idvel=get_id("VAVG") ; int idcg=find_closest(id) ; v1d res ;

  for (int i=0 ; i<d ; i++)
  {
    if (usetimeavg)
    {
      res.push_back((*CGPtemp)[idcg].fields[0][idvel+i]) ;
    }
    else
      res.push_back(CGP[idcg].fields[cT][idvel+i]) ;
  }
  return res ;
}
//-----
v1d Coarsing::interpolate_rot_nearest (int id, bool usetimeavg)
{
  const static int idrot=get_id("ROT") ; int idcg=find_closest(id) ; v1d res ;
  for (int i=0 ; i<d ; i++)
  {
    if (usetimeavg)
      res.push_back((*CGPtemp)[idcg].fields[0][idrot+i]) ;
    else
      res.push_back(CGP[idcg].fields[cT][idrot+i]) ;
  }
  return res ;
}
//--------
v1d Coarsing::interpolate_vel_trilinear (int id, bool usetimeavg)
{
assert(d==3) ;
int pts[8][3] ;
double Qs[4*3]={0,0,0,0,0,0,0,0,0,0,0,0} ;
double x;

const static int idvel=get_id("VAVG") ;

auto clip = [&](int a, int maxd){if (a<0) return(0) ; else if (a>=maxd) return (maxd-1) ; else return (a) ; } ;

x=clip(floor((data.pos[0][id]-CGP[0].location[0])/dx[0]), npt[0]) ;
for (int i=0 ; i<4 ; i++) pts[i][0]=x;
x=clip(ceil((data.pos[0][id]-CGP[0].location[0])/dx[0]), npt[0]) ;
for (int i=4 ; i<8 ; i++) pts[i][0]=x;
x=clip(floor((data.pos[1][id]-CGP[0].location[1])/dx[1]), npt[1]) ;
pts[0][1]=pts[1][1]=pts[4][1]=pts[5][1]=x;
x=clip(ceil((data.pos[1][id]-CGP[0].location[1])/dx[1]), npt[1]) ;
pts[2][1]=pts[3][1]=pts[6][1]=pts[7][1]=x;
x=clip(floor((data.pos[2][id]-CGP[0].location[2])/dx[2]), npt[2]) ;
for (int i=0 ; i<8 ; i+=2) pts[i][2]=x;
x=clip(ceil((data.pos[2][id]-CGP[0].location[2])/dx[2]), npt[2]) ;
for (int i=1 ; i<8 ; i+=2) pts[i][2]=x;

for (int i=0; i<8 ; i++) pts[i][0] = (nptcum[0]*pts[i][0]+nptcum[1]*pts[i][1]+nptcum[2]*pts[i][2]) ; // Variable reuse ... ...
//printf("| %d %d %d %d %d {%d} %d %d %g |", pts[0][0], pts[1][0], pts[2][0], pts[3][0], pts[4][0], pts[5][0], pts[6][0], pts[7][0], (*CGPtemp)[pts[5][0]].fields[0][idvel]) ; fflush(stdout) ;

/*vector <CGPoint> * CGbase = nullptr ;
if (usetimeavg) CGbase = CGPtemp ;
else CGbase = &CGP ; */

for (int i=0; i<8 ; i++)
    for (int j=0 ; j<3 ; j++)
        if (usetimeavg)
            Qs[j*4] += 1/8. * ((*CGPtemp)[pts[i][0]].fields[0][idvel+j]) ;
        else
            Qs[j*4] += 1/8. * (CGP[pts[i][0]].fields[0][idvel+j]) ;

//printf("%g %g %g|", Qs[0], Qs[4], Qs[8]) ;
for (int i=0; i<4 ; i++)
  if (pts[0+i][0]!=pts[4+i][0])
      for (int j=0; j<3 ; j++)
      {
       if (usetimeavg)
       {
        Qs[i+4*j] = ( data.pos[0][id] - CGP[pts[0+i][0]].location[0])/(CGP[pts[4+i][0]].location[0]-CGP[pts[0+i][0]].location[0]) * (*CGPtemp)[pts[0+i][0]].fields[0][idvel+j]
                   +(-data.pos[0][id] + CGP[pts[4+i][0]].location[0])/(CGP[pts[4+i][0]].location[0]-CGP[pts[0+i][0]].location[0]) * (*CGPtemp)[pts[4+i][0]].fields[0][idvel+j] ;

        if ((data.pos[0][id] - CGP[pts[0+i][0]].location[0])/(CGP[pts[4+i][0]].location[0]-CGP[pts[0+i][0]].location[0]) <0 || (-data.pos[0][id] + CGP[pts[4+i][0]].location[0])/(CGP[pts[4+i][0]].location[0]-CGP[pts[0+i][0]].location[0])<0)
        {
         printf("XXX %g %g %g %d %d\n", data.pos[0][id], CGP[pts[0+i][0]].location[0], CGP[pts[4+i][0]].location[0], pts[0+i][0], pts[4+i][0]) ;
        }

       }
       else
        Qs[i+4*j] = ( data.pos[0][id] - CGP[pts[0+i][0]].location[0])/(CGP[pts[4+i][0]].location[0]-CGP[pts[0+i][0]].location[0]) * CGP[pts[0+i][0]].fields[cT][idvel+j]
                   +(-data.pos[0][id] + CGP[pts[4+i][0]].location[0])/(CGP[pts[4+i][0]].location[0]-CGP[pts[0+i][0]].location[0]) * CGP[pts[4+i][0]].fields[cT][idvel+j] ;
      }
  else
      for (int j=0; j<3 ; j++)
        Qs[i+4*j] = CGP[pts[0+i][0]].fields[cT][idvel+j] ;

for (int i=0; i<4 ; i+=2)
  if (pts[0+i][0]!=pts[1+i][0])
      for (int j=0; j<3 ; j++)
      {
        Qs[i/2+4*j] = ( data.pos[2][id] - CGP[pts[0+i][0]].location[2])/(CGP[pts[1+i][0]].location[2]-CGP[pts[0+i][0]].location[2]) * Qs[0+i+4*j]
                     +(-data.pos[2][id] + CGP[pts[1+i][0]].location[2])/(CGP[pts[1+i][0]].location[2]-CGP[pts[0+i][0]].location[2]) * Qs[1+i+4*j];

        if (( data.pos[2][id] - CGP[pts[0+i][0]].location[2])/(CGP[pts[1+i][0]].location[2]-CGP[pts[0+i][0]].location[2]) <0 ||
            (-data.pos[2][id] + CGP[pts[1+i][0]].location[2])/(CGP[pts[1+i][0]].location[2]-CGP[pts[0+i][0]].location[2])<0)
        {
         printf("ZZZ %g %g %g %d %d\n", data.pos[2][id], CGP[pts[0+i][0]].location[2], CGP[pts[1+i][0]].location[2], pts[1+i][0], pts[0+i][0]) ;
        }
      }
  else
      for (int j=0; j<3 ; j++)
        Qs[i/2+4*j] = Qs[0+i+4*j] ;

if (pts[0][0]!=pts[2][0])
    for (int j=0; j<3 ; j++)
    {
        Qs[4*j] = ( data.pos[1][id] - CGP[pts[0][0]].location[1])/(CGP[pts[2][0]].location[1]-CGP[pts[0][0]].location[1]) * Qs[0+4*j]
                 +(-data.pos[1][id] + CGP[pts[2][0]].location[1])/(CGP[pts[2][0]].location[1]-CGP[pts[0][0]].location[1]) * Qs[1+4*j];

        if (( data.pos[1][id] - CGP[pts[0][0]].location[1])/(CGP[pts[2][0]].location[1]-CGP[pts[0][0]].location[1])<0 || (-data.pos[1][id] + CGP[pts[2][0]].location[1])/(CGP[pts[2][0]].location[1]-CGP[pts[0][0]].location[1])<0)
         printf("YY %g %g %g %d %d\n", data.pos[1][id], CGP[pts[0][0]].location[1], CGP[pts[2][0]].location[1], pts[2][0], pts[0][0]) ;
    }
//else nothing to do, the value are already at the right location ...
vector<double> res = {Qs[0], Qs[4], Qs[8]} ;
return (res);
}

//================================= BEGINNING OF THE INTERESTING PART ================================
//================ PASS 1 : "RHO", "VAVG", "ROT", "EKT", "EKR"==============================
int Coarsing::pass_1 ()
{
int i, dd, id ; double wp ;
bool dorho=true, dovel=true, doomega=true, doI=true, doradius=true ;
int rhoid=get_id("RHO") ; if (rhoid<0) {dorho=false ; return 0;} if (dorho && !data.check_field_availability("RHO")) {printf("Data missing for RHO\n") ; dorho=false ; }
int Iid = get_id("I") ; if (Iid<0) doI=false ;           if (doI     && !data.check_field_availability("I")) {printf("Data missing for I\n") ; doI=false ; }
int velid=get_id("VAVG"); if (velid<0)  dovel=false ;    if (dovel   && !data.check_field_availability("VAVG")) {printf("Data missing for VAVG\n") ; dovel=false ; }
int omegaid=get_id("ROT");if (omegaid<0) doomega=false ; if (doomega && !data.check_field_availability("ROT")) {printf("Data missing for ROT\n") ; doomega=false ; }
int radiusid = get_id("RADIUS") ; if (radiusid<0) doradius=false ; if (doradius && !data.check_field_availability("RADIUS")) {printf("Data missing for RADIUS\n") ; doradius=false ; }

vector<int> extraid, extrancomp, extralocation ;
for (auto &v: FIELDS)
{
  if (v.ftype == FieldType::Defined) continue ; 
  extraid.push_back(get_id(v.name)) ; 
  if (extraid.back()<0) {extraid.pop_back() ; continue ; } // Not found
  switch(v.type) {
    case TensorOrder::SCALAR: extrancomp.push_back(1) ; break ;
    case TensorOrder::VECTOR: extrancomp.push_back(d) ; break ;
    case TensorOrder::TENSOR: extrancomp.push_back(d*d) ; break ;
    default : extraid.pop_back() ; continue ; 
  }
  extralocation.push_back(v.datalocation) ; 
}
bool doextra = (extraid.size()>0)?true:false ;

// Clearing stuff
if (dorho) for (int i=0 ; i<Npt ; CGP[i].fields[cT][rhoid]=0, i++) ;
if (doI)   for (int i=0 ; i<Npt ; CGP[i].fields[cT][Iid]=0, i++) ;
if (dovel) for (int dd=0 ; dd<d ; dd++) for (int i=0 ; i<Npt ; CGP[i].fields[cT][velid+dd]=0, i++) ;
if (doomega) for (int dd=0 ; dd<d*(d-1)/2 ; dd++) for (int i=0 ; i<Npt ; CGP[i].fields[cT][omegaid+dd]=0, i++) ;
if (doradius) for (int i=0 ; i<Npt ; CGP[i].fields[cT][radiusid]=0, i++) ;
if (doextra)
    for (size_t v = 0 ; v<extraid.size() ; v++)
      for (size_t w = 0 ; w<extrancomp[v] ; w++)
        for (int i=0 ; i<Npt ; CGP[i].fields[cT][extraid[v]+w]=0, i++) ;

//printf("Starting pass 1...\r") ; fflush(stdout) ;

double dm, dI ; v1d dv (d,0), dom(d,0) ; double * CGf ; // Speed things up a bit ...
vector<double> totweight(data.N,0) ;

// printf(" -> Pass 1") ; fflush(stdout) ;

for (i=0 ; i<data.N ; i++)
{
 if (isnan(data.pos[0][i])) continue ;
 id=find_closest(i) ;
 dm=data.mass[i] ;
 if (doI || doomega) dI=data.Imom[i] ;
 for (dd=0 ; dd<d ; dd++) {if (dovel) dv[dd]=data.vel[dd][i] ; if (doomega) dom[dd]=data.omega[dd][i] ; }

 for (auto j=CGP[id].neighbors.begin() ; j<CGP[id].neighbors.end() ; j++)
 {
     wp=Window->window(Window->distance(i,CGP[*j].location)) ;
     totweight[i]+=wp ;
     CGf = &(CGP[*j].fields[cT][0]) ;
     //if (*j>100) printf("%g %g %g | %g %g %g\n", CGP[*j].location[0], CGP[*j].location[1], CGP[*j].location[2], data.pos[0][i],  data.pos[1][i], data.pos[2][i]) ;
     //CGP[*j].phi += wp ;
     //if (wp>0) CGP[*j].natom ++ ;
     if (dorho)
       CGP[*j].fields[cT][rhoid] += wp * data.mass[i] ;
     if (doI)
       *(CGf+Iid) += wp * dm * dI ;
     if (dovel)
       for (dd=0 ; dd<d ; dd++)
         *(CGf+velid+dd) += wp * dm * dv[dd] ;
     if (doomega)
       for (dd=0 ; dd<d*(d-1)/2 ; dd++)
         *(CGf+omegaid+dd) += wp * dm * dom[dd] * dI ;
     if (doradius)
       CGP[*j].fields[cT][radiusid] += wp * dm * data.radius[i] ;
     if (doextra)
       for (size_t v = 0 ; v<extraid.size() ; v++)
         for (size_t w = 0 ; w<extrancomp[v] ; w++)
           *(CGf+extraid[v]+w) += wp * dm * data.extra[extralocation[v]+w][i] ;
 }

}

/*int nbzero = 0 ;
for (auto v : totweight)
  if (v==0) nbzero++ ;
printf("%d \n", nbzero) ;fflush(stdout) ; */

// Intermediate pass (cg points)
bool doEKT=true, doEKR=true ;
int EKTid=get_id("EKT") ; if (EKTid<0) doEKT=false ;
int EKRid=get_id("EKR") ; if (EKRid<0) doEKR=false ;
//printf("Starting intermediate pass 1...\r") ; fflush(stdout) ;
for (i=0 ; i<Npt ; i++)
{
    double rho, Imom=0 ;
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
    if (doradius && rho!=0) {CGP[i].fields[cT][radiusid] /= rho ; }
    if (doextra && rho!=0)
       for (size_t v = 0 ; v<extraid.size() ; v++)
         for (size_t w = 0 ; w<extrancomp[v] ; w++)
            CGP[i].fields[cT][extraid[v]+w] /= rho ;
}
return 0 ;
}

//================ PASS 2 : "TK", "MK", "eKT", "eKR", "qTK", "qRK"==============================
int Coarsing::pass_2(bool usetimeavg)
{
int i, dd ; double velfluc, rotfluc ; int id ; double wp ;
bool doeKT=true , doeKR=true, doqTK=true, doqRK=true, doTK=true, doMK=true ;
int rhoid=get_id("RHO") ; if (rhoid<0) {return 0;}
int eKTid=get_id("eKT") ; if (eKTid<0) doeKT=false ; if (doeKT && !data.check_field_availability("eKT")) {printf("Data missing for eKT\n") ; doeKT=false ; }
int eKRid=get_id("eKR") ; if (eKRid<0) doeKR=false ; if (doeKR && !data.check_field_availability("eKR")) {printf("Data missing for eKR\n") ; doeKR=false ; }
int qTKid=get_id("qTK") ; if (qTKid<0) doqTK=false ; if (doqTK && !data.check_field_availability("qTK")) {printf("Data missing for qTK\n") ; doqTK=false ; }
int qRKid=get_id("qRK") ; if (qRKid<0) doqRK=false ; if (doqRK && !data.check_field_availability("qRK")) {printf("Data missing for qRK\n") ; doqRK=false ; }
int TKid =get_id("TK")  ; if (TKid<0) doTK=false ;   if (doTK  && !data.check_field_availability("TK")) {printf("Data missing for TK\n") ; doTK=false ; }
int MKid =get_id("MK")  ; if (MKid<0) doMK=false ;   if (doMK  && !data.check_field_availability("MK")) {printf("Data missing for MK\n") ; doMK=false ; }

if (!hasvelfluct && (doeKT || doTK || doMK || doqTK || doqRK))
  {
    printf("Velocity fluctuations are needed for some fields but not computed\n") ; fflush(stdout) ;
    doeKT = doTK = doMK = doqTK = doqRK = false ;
  }
if (!hasrotfluct && (doeKR||doMK||doqRK))
  {
    printf("Rotational velocity fluctuations are needed for some fields but not computed\n") ; fflush(stdout) ;
    doeKR = doMK = doqRK = false ;
  }

// printf(" -> Pass 2") ; fflush(stdout) ;
for (i=0 ; i<data.N ; i++)
{
 if (isnan(data.pos[0][i])) continue ;

 id=find_closest(i) ;
 velfluc = rotfluc = 0 ;
 if (hasvelfluct) for (dd=0, velfluc=0 ; dd<d ; dd++) velfluc += data.vel_fluc[dd][i]*data.vel_fluc[dd][i] ;
 if (hasrotfluct) for (dd=0, rotfluc=0 ; dd<d ; dd++) rotfluc += data.rot_fluc[dd][i]*data.rot_fluc[dd][i] ;

 for (auto j=CGP[id].neighbors.begin() ; j<CGP[id].neighbors.end() ; j++)
 {
     wp=Window->window(Window->distance(i,CGP[*j].location)) ;

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
// printf(" -> subpass 2") ; fflush(stdout) ;
for (i=0 ; i<Npt ; i++)
{
    double tworho ;
    if (usetimeavg)
      tworho = 2*(*CGPtemp)[i].fields[0][rhoid] ;
    else
      tworho = 2*CGP[i].fields[cT][rhoid] ;
    if (tworho==0) continue ;


    //if (doeKT && tworho!=0) CGP[i].fields[cT][eKTid] /= tworho ;
    if (doeKR && tworho!=0) CGP[i].fields[cT][eKRid] /= tworho ;
    if (doqTK) for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][qTKid+dd] *= (-0.5) ;
    if (doqRK) for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][qRKid+dd] *= (-0.5) ;
}
return 0 ;
}

//================ PASS 3 : "TC", "MC", "mC", "qTC", "qRC", "zt", "zr"==============================
int Coarsing::pass_3()
{
int i, dd ;
bool dozT=true , dozR=true, doTC=true, doMC=true, domC=true;
int zTid=get_id("zT") ; if (zTid<0) dozT=false ; if (dozT && !data.check_field_availability("zT")) {printf("Data missing for zT\n") ; dozT=false ; }
int zRid=get_id("zR") ; if (zRid<0) dozR=false ; if (dozR && !data.check_field_availability("zR")) {printf("Data missing for zR\n") ; dozR=false ; }
int TCid=get_id("TC") ; if (TCid<0) doTC=false ; if (doTC && !data.check_field_availability("TC")) {printf("Data missing for TC\n") ; doTC=false ; }
int MCid=get_id("MC") ; if (MCid<0) doMC=false ; if (doMC && !data.check_field_availability("MC")) {printf("Data missing for MC\n") ; doMC=false ; }
int mCid=get_id("mC") ; if (mCid<0) domC=false ; if (domC && !data.check_field_availability("mC")) {printf("Data missing for mC\n") ; domC=false ; }
double sum=0 ; int p, q, id ; //double rp, rq ; double wpqs, wpqf ;
// printf(" -> Pass 3 ") ; fflush(stdout) ;
for (i=0 ; i<data.Ncf ; i++)
{
 id=find_closest_pq(i) ;
 p=data.id1[i] ; q=data.id2[i] ; if (q<=p) {printf("ERR: contacts should be ordered so that p<q %d %d %d %d\n", p, q, i, data.Ncf) ;  fflush(stdout) ; std::exit(0) ;}

 for (auto j=CGP[id].neighbors.begin() ; j<CGP[id].neighbors.end() ; j++)
 {
  /*rp=Window->distance(p, CGP[*j].location) ;
  rq=Window->distance(q, CGP[*j].location) ;
  wpqs = Window->window_avg (rp, rq) ;
  wpqf = Window->window_int (rp, rq) ;*/
  auto [wpqs, wpqf] = Window->window_contact_weight(p, q, CGP[*j].location) ;
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
// printf(" -> subpass 3") ; fflush(stdout) ;
for (i=0 ; i<Npt ; i++)
{
  //printf("%g %g %g %g %g %g %g %g\n", CGP[i].fields[cT][TCid+0], CGP[i].fields[cT][TCid+1],CGP[i].fields[cT][TCid+2],CGP[i].fields[cT][TCid+3]
  //  , CGP[i].fields[cT][TCid+4], CGP[i].fields[cT][TCid+5], CGP[i].fields[cT][TCid+6], CGP[i].fields[cT][TCid+7], CGP[i].fields[cT][TCid+8]);
    //printf("%g ", CGP[i].natom) ;
    if (doMC) for (dd=0 ; dd<d*d ; dd++) CGP[i].fields[cT][MCid] *= 0.5 ;
}
return 0 ;
}
//================ PASS 4 : "qTC", "qRC"==============================
int Coarsing::pass_4()
{
int i, dd ;
bool doqTC=true, doqRC=true ;
int qTCid=get_id("qTC") ; if (qTCid<0) doqTC=false ; if (qTCid && !data.check_field_availability("qTC")) {printf("Data missing for qTC\n") ; doqTC=false ; }
int qRCid=get_id("qRC") ; if (qRCid<0) doqRC=false ; if (qRCid && !data.check_field_availability("qRC")) {printf("Data missing for qRC\n") ; doqRC=false ; }
double sum=0 ; int p, q, id ; //double rp, rq ; double wpqs, wpqf ;
// printf(" -> Pass 4 ") ; fflush(stdout) ;
for (i=0 ; i<data.Ncf ; i++)
{
 id=find_closest_pq(i) ;
 p=data.id1[i] ; q=data.id2[i] ; if (q<=p) printf("ERR: contacts should be ordered so that p<q\n") ;
 for (auto j=CGP[id].neighbors.begin() ; j<CGP[id].neighbors.end() ; j++)
 {
  /*rp=Window->distance(p, CGP[*j].location) ;
  rq=Window->distance(q, CGP[*j].location) ;
  wpqs = Window->window_avg (rp, rq) ;
  wpqf = Window->window_int (rp, rq) ;*/
  auto [wpqs, wpqf] = Window->window_contact_weight(p, q, CGP[*j].location) ;
  //printf("%g %g %g", rp, rq, wpqf) ;
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
 }
}

//Last intermediate pass
// printf(" -> subpass 4") ; fflush(stdout) ;
for (i=0 ; i<Npt ; i++)
{
    if (doqTC) for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][qTCid] *= 0.5 ;
    if (doqRC) for (dd=0 ; dd<d ; dd++) CGP[i].fields[cT][qRCid] *= 0.5 ;
}
return 0 ;
}



//======================== PASS 5: post-processing of some fields ==================
int Coarsing::pass_5()
{
bool doSig=true, doP=true, doPk=true, doTau=true, doGamdot=true, doGamvdot=true, doGamtau=true, doOmega=true, doOmegaMag=true;

int Sigid=get_id("TotalStress") ;    if (Sigid<0) doSig=false ;
int Pid=get_id("Pressure") ;         if (Pid<0) doP=false ;
int Pkid=get_id("KineticPressure") ; if (Pkid<0) doPk=false ;
int Tauid=get_id("ShearStress") ;    if (Tauid<0) doTau=false ;
int TCid=get_id("TC") ; int TKid=get_id("TK") ;

int Gamdotid=get_id("StrainRate") ;            if (Gamdotid<0) doGamdot=false ;
int Gamvdotid=get_id("VolumetricStrainRate") ; if (Gamvdotid<0) doGamvdot=false ;
int Gamtauid=get_id("ShearStrainRate") ;       if (Gamtauid<0) doGamtau=false ;
int Omegaid=get_id("RotationalVelocity") ;     if (Omegaid<0) doOmega=false ;
int OmegaMagid=get_id("RotationalVelocityMag");if (OmegaMagid<0) doOmegaMag=false ;
int VAVGid=get_id("VAVG") ;
vector <double> gradient ;
if (doGamdot || doGamvdot || doGamtau || doOmega || doOmegaMag) gradient.resize(d*d,0) ;
if ((doGamdot || doGamvdot || doGamtau || doOmega || doOmegaMag) && VAVGid == -1) { printf("Error: missing VAVG to compute velocity gradients ...\n") ; fflush(stdout) ; }

// printf(" -> Pass 5 ") ; fflush(stdout) ;

for (int i=0 ; i< Npt ; i++)
{
    if (doSig)
        for (int dd=0 ; dd<d*d ; dd++)
            CGP[i].fields[cT][Sigid+dd] = CGP[i].fields[cT][TCid+dd] + CGP[i].fields[cT][TKid+dd] ;
    if (doP)
    {
        CGP[i].fields[cT][Pid] = 0 ;
        for (int dd=0 ; dd<d ; dd++)
            CGP[i].fields[cT][Pid] += (CGP[i].fields[cT][TCid+dd*d+dd] + CGP[i].fields[cT][TKid+dd*d+dd]) ;
        CGP[i].fields[cT][Pid] *= (1./d) ;
    }
    if (doPk)
    {
        CGP[i].fields[cT][Pkid] = 0 ;
        for (int dd=0 ; dd<d ; dd++)
            CGP[i].fields[cT][Pkid] += CGP[i].fields[cT][TKid+dd*d+dd] ;
        CGP[i].fields[cT][Pkid] *= (1./d) ;

    }
    if (doTau)
    {
        double pressure = 0 ;
        for (int dd=0 ; dd<d ; dd++)
            pressure += (CGP[i].fields[cT][TCid+dd*d+dd] + CGP[i].fields[cT][TKid+dd*d+dd]) ;
        pressure *= (1./d) ;

        CGP[i].fields[cT][Tauid] = 0 ;
        for (int j=0 ; j<d*d ; j++)
            CGP[i].fields[cT][Tauid] = (CGP[i].fields[cT][TCid+j] + CGP[i].fields[cT][TKid+j] - ((j/d==j%d)?1:0)*pressure)*(CGP[i].fields[cT][TCid+j] + CGP[i].fields[cT][TKid+j] - ((j/d==j%d)?1:0)*pressure) ;
        CGP[i].fields[cT][Tauid]=sqrt(CGP[i].fields[cT][Tauid]) ;
    }

    if (doGamdot || doGamvdot || doGamtau || doOmega || doOmegaMag)
    {
     vector <double> gradient (d*d,0) ;
     // Computing the velocity gradient
     for (int k=0 ; k<d ; k++)
         for (int l=0 ; l<d ; l++)
         {
          if (npt[l]==1) // Cannot compute gradient with a single cell
              gradient[k*d+l] = 0 ;
          else if ((i/nptcum[l]) % npt[l] == 0 ) //forward difference
              gradient[k*d+l] = (CGP[i+nptcum[l]].fields[cT][VAVGid+k]-CGP[i].fields[cT][VAVGid+k])/dx[l] ;
          else if ((i/nptcum[l]) % npt[l] == (npt[l]-1)) //backward difference
              gradient[k*d+l] = (CGP[i-nptcum[l]].fields[cT][VAVGid+k]-CGP[i].fields[cT][VAVGid+k])/dx[l] ;
          else // central difference
              gradient[k*d+l] = (CGP[i-nptcum[l]].fields[cT][VAVGid+k]-CGP[i+nptcum[l]].fields[cT][VAVGid+k])/(2*dx[l]) ;
         }

     // Antisymetric part of the velocity gradient
     if (doOmega)
     {
         assert(d==3) ; //For other dimensions, need to do the full rotation matrix etc...
         CGP[i].fields[cT][Omegaid+0] = 0.5*(gradient[2*d+1] - gradient[1*d+2]) ;
         CGP[i].fields[cT][Omegaid+1] = 0.5*(gradient[0*d+2] - gradient[2*d+0]) ;
         CGP[i].fields[cT][Omegaid+2] = 0.5*(gradient[1*d+0] - gradient[0*d+1]) ;
     }

     if (doOmegaMag)
     {
         assert(d==3) ; //For other dimensions, need to do the full rotation matrix etc...
         CGP[i].fields[cT][OmegaMagid]  = (0.5*(gradient[2*d+1] - gradient[1*d+2])*0.5*(gradient[2*d+1] - gradient[1*d+2])) ;
         CGP[i].fields[cT][OmegaMagid] += (0.5*(gradient[0*d+2] - gradient[2*d+0])*0.5*(gradient[0*d+2] - gradient[2*d+0])) ;
         CGP[i].fields[cT][OmegaMagid] += (0.5*(gradient[1*d+0] - gradient[0*d+1])*0.5*(gradient[1*d+0] - gradient[0*d+1])) ;
         CGP[i].fields[cT][OmegaMagid]=sqrt(CGP[i].fields[cT][OmegaMagid]) ;
     }

     // Symetric part of the velocity gradient
     for (int k=0 ; k<d ; k++)
         for (int l=0 ; l<d ; l++)
             if (l!=k)
             {
                gradient[k*d+l] = 0.5*(gradient[k*d+l] + gradient[l*d+k]);
                gradient[l*d+k] = gradient[k*d+l] ;
             }

     if (doGamdot)
        for (int dd=0 ; dd<d*d ; dd++)
             CGP[i].fields[cT][Gamdotid+dd] = gradient[dd] ;

     if (doGamvdot)
     {
        CGP[i].fields[cT][Gamvdotid] = 0;
        for (int dd=0 ; dd<d ; dd++)
            CGP[i].fields[cT][Gamvdotid] += gradient[dd*d+dd] ;
     }

     if (doGamtau)
     {
        double volumetric = 0;
        for (int dd=0 ; dd<d ; dd++)
            volumetric += gradient[dd*d+dd] ;
        volumetric /= d ;

        CGP[i].fields[cT][Gamtauid] = 0 ;
        for (int dd=0 ; dd<d*d ; dd++)
            CGP[i].fields[cT][Gamtauid] += (gradient[dd] - ((dd/d==dd%d)?1:0)*volumetric)*(gradient[dd] - ((dd/d==dd%d)?1:0)*volumetric) ;
        CGP[i].fields[cT][Gamtauid] = sqrt(CGP[i].fields[cT][Gamtauid]) ;
     }

    }

}


return 0 ;
}

//=================================================================
// Data conversion tools
int Data::random_test (int NN, int NNcf, int d, v2d box )
{
const double meanrad=0.00075, rho=2500 ;
boost::mt19937 rng ;
boost::uniform_01<boost::mt19937> rand(rng) ;
double rad; int tmp ;

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
    lpq[dd][i]=pos[dd][int(id1[i])]-pos[dd][int(id2[i])] ; //ORDER OK

 return 0 ;
}
//---------------------------------------------------
int Data::periodic_atoms (int d, v2d bounds, int pbc, v1d Delta, bool omegainclude)
{
    v2d newpos, newvel, newomega ;
    v1d newmass, newImom ;
    int newN=0 ;

    auto isinside = [&bounds,d](v1d loc){for(int j=0 ; j<d ; j++) if (loc[j]<bounds[0][j] || loc[j]>bounds[1][j]) return false ; return true ; } ;
    //auto push_back = [=](v2d v, double * a) {v.push_back(v1d(d,0)) ; for (int i=0 ; i<d ; i++) (*(v.end()-1))[i]=*(a+i) ; } ;

    std::function <void(int,int,v1d,int,int)> lbd ;
    lbd = [&](int idx, int dim, v1d location, int pbc, int nmodif)
    {
     //printf("%d %d-> ", dim, pbc) ; for (auto v:location) printf("%g ", v) ; printf("\n"); fflush(stdout) ;
     if (pbc==0) // No more pbc, needs to test that final location, and eventually add it
     {
         if (isinside(location) && nmodif>0)
         {
            newN++ ;
            newpos.push_back(location) ;
            newvel.resize(newN) ; newvel[newN-1].resize(d,0) ;
            for (int i=0 ; i<d ; i++) newvel[newN-1][i] = vel[i][idx] ;
            if (omegainclude)
            {
                newomega.resize(newN) ; newomega[newN-1].resize((d*(d-1))/2, 0) ;
                for (int i=0 ; i<(d*(d-1))/2 ; i++) newomega[newN-1][i] = omega[i][idx] ;
            }
            newmass.push_back(mass[idx]) ;
            newImom.push_back(Imom[idx]) ;
            //for (auto v:location) printf("%g ", v) ; printf("\n"); fflush(stdout) ;
         }
    }
    else
    {
        if (pbc&1)
        {
            bool keepgoing=true ;
            v1d locorig=location ;
            lbd(idx, dim+1, location, pbc>>1, nmodif) ; // No modif on that dimension, falling down to lower dimensions
            while (keepgoing) // Images upwards thought the pbc
            {
                location[dim] += Delta[dim] ;
                if (isinside(location))
                    lbd(idx, dim+1, location, pbc>>1, nmodif+1) ;
                else
                    keepgoing=false ;
            }
            keepgoing=true ;
            location=locorig ;
            while (keepgoing) // Images downwards through the pbc
            {
                location[dim] -= Delta[dim] ;
                if (isinside(location))
                    lbd(idx, dim+1, location, pbc>>1, nmodif+1) ;
                else
                    keepgoing=false ;
            }

        }
        else // Not a pbc, falling down through dims
            lbd(idx, dim+1, location, pbc>>1, nmodif) ;

    }
   } ; // end of lambda function

    v1d locbase(d,0) ;
    for (int i=0 ; i<N ; i++)
    {
       for (int j=0 ; j<d ; j++) locbase[j]=pos[j][i] ;
       lbd(i, 0, locbase, pbc, 0) ;
    }

    mass=(double*)realloc(mass, (N+newN)*sizeof(double)) ;
    Imom=(double*)realloc(Imom, (N+newN)*sizeof(double)) ;
    for (auto & v: pos) v = (double *) realloc(v, (N+newN)*sizeof(double)) ;
    for (auto & v: vel) v = (double *) realloc(v, (N+newN)*sizeof(double)) ;
    if (omegainclude) for (auto & v: omega) v = (double *) realloc(v, (N+newN)*sizeof(double)) ;

    for (int i=0 ; i<newN ; i++)
    {
     mass[N+i]=newmass[i] ;
     Imom[N+i]=newmass[i] ;
     for (int j=0 ; j<d ; j++) pos[j][N+i]=newpos[i][j] ;
     for (int j=0 ; j<d ; j++) vel[j][N+i]=newvel[i][j] ;
     if (omegainclude)
         for (int j=0 ; j<(d*(d-1))/2 ; j++)
             omega[j][N+1] = newomega[j][N+1] ;
    }

    printf("Periodic effect: old N = %d, added = %d\n", N, newN) ;
    Nnonper=N ;
    N += newN ;
return 0 ;
}

//----------------------------------------------------------
bool Data::check_field_availability(string name)
{
 if      (name == "RHO" ) return (mass) ;
 else if (name == "I"   ) return (mass && Imom) ;
 else if (name == "VAVG") return (mass && vel[0]) ;
 else if (name == "TC"  ) return (lpq[0] && fpq[0]) ;
 else if (name == "TK"  ) return (mass) ;
 else if (name == "ROT" ) return (mass && Imom && omega[0]) ;
 else if (name == "MC"  ) return (lpq[0] && mqp[0] && mpq[0]) ;
 else if (name == "MK"  ) return (mass && Imom) ;
 else if (name == "mC"  ) return (lpq[0] && fpq[0]) ;
 else if (name == "EKT" ) return (mass && vel[0]) ;
 else if (name == "eKT" ) return (mass) ;
 else if (name == "EKR" ) return (mass && Imom && omega[0]) ;
 else if (name == "eKR" ) return (mass && Imom) ;
 else if (name == "qTC" ) return (lpq[0] && fpq[0]) ;
 else if (name == "qTK" ) return (mass) ;
 else if (name == "qRC" ) return (lpq[0] && mqp[0] && mpq[0]) ;
 else if (name == "qRK" ) return (mass && Imom) ;
 else if (name == "zT"  ) return (fpq[0] && vel[0]) ;
 else if (name == "zR"  ) return (mpq[0] && mqp[0] && omega[0]) ;
 else if (name == "RADIUS"  ) return (radius) ;
 else
     return (true) ;
}

//====================================================
int Coarsing::mean_time(bool temporary)
{
  if (temporary)
  {
    //if (CGPtemp != nullptr) delete(CGPtemp) ; actually no can't do that ....
    CGPtemp = new vector<CGPoint> ;
    for (size_t i=0 ; i<CGP.size() ; i++)
    {
      CGPtemp->push_back (CGPoint(CGP[i].d, CGP[i].location)) ;
      CGPtemp->back().fields.push_back(CGP[i].fields[0]) ;
      //CGPtemp->back().natom = CGP[i].natom ;
      //CGPtemp->back().phi = CGP[i].phi ;
      CGPtemp->back().neighbors = CGP[i].neighbors ;
    }
  }
  else
    CGPtemp = &CGP ;

  int tf=(*CGPtemp)[0].fields[0].size() ;
  for (int i=0 ; i<Npt ; i++)
    for (int t=1 ; t<Time ; t++)
    {
      printf("\rAveraging ... %d", t) ; fflush(stdout) ;
      for (int f=0 ; f<tf ; f++)
        (*CGPtemp)[i].fields[0][f]+=CGP[i].fields[t][f] ;
    }
  for (int i=0 ; i<Npt ; i++)
    for (int f=0 ; f<tf ; f++)
    {
        (*CGPtemp)[i].fields[0][f] /= Time;
        //CGP[i].fields.resize(1) ;
    }

  if (!temporary) Time=1 ;
  return 0 ;
}
//=================================== Writing functions ========================
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

    /*fprintf(out,"SCALARS count double \nLOOKUP_TABLE default \n") ;
    for (int k=0 ; k<npt[2] ; k++)
     for (int j=0 ; j<npt[1] ; j++)
      for (int i=0 ; i<npt[0] ; i++)
       fprintf(out, "%g ", CGP[i*npt[1]*npt[2]+j*npt[2]+k].phi) ;
    fprintf(out,"\n\n") ; */

    for (size_t f=0 ; f<Fidx.size() ; f++)
    {
      if (Fidx[f]<0) continue ;
      switch (Ftype[f])
      {
        case TensorOrder::SCALAR : fprintf(out, "SCALARS %s double \nLOOKUP_TABLE default \n", Fname[f].c_str()) ;
            for (int k=0 ; k<npt[2] ; k++)
             for (int j=0 ; j<npt[1] ; j++)
              for (int i=0 ; i<npt[0] ; i++)
               fprintf(out, "%g%c", CGP[i*npt[1]*npt[2]+j*npt[2]+k].fields[t][Fidx[f]], i%90==89?'\n':' ') ;
            break ;
        case TensorOrder::VECTOR : fprintf(out, "VECTORS %s double \n", Fname[f].c_str()) ;
             for (int k=0 ; k<npt[2] ; k++)
              for (int j=0 ; j<npt[1] ; j++)
               for (int i=0 ; i<npt[0] ; i++)
                for (int dd=0 ; dd<d ; dd++)
                  fprintf(out, "%g%c", CGP[i*npt[1]*npt[2]+j*npt[2]+k].fields[t][Fidx[f]+dd], (i%30==29&&dd==d-1)?'\n':' ') ;
             break ;
        case TensorOrder::TENSOR : fprintf(out, "TENSORS %s double \n", Fname[f].c_str()) ;
            for (int k=0 ; k<npt[2] ; k++)
             for (int j=0 ; j<npt[1] ; j++)
              for (int i=0 ; i<npt[0] ; i++)
               for (int dd=0 ; dd<d*d ; dd++)
                fprintf(out, "%g%c", CGP[i*npt[1]*npt[2]+j*npt[2]+k].fields[t][Fidx[f]+dd], (dd==d*d-1)?'\n':' ') ;
            break ;
        default: printf("ERR: this should never happen. \n") ;
      }
      fprintf(out, "\n\n") ;
    }
  fclose(out) ;
  }
return 0 ;
}
//-------------------------------------------------------
int Coarsing::write_matlab ([[maybe_unused]] string path, [[maybe_unused]] bool squeeze)
{
#ifdef MATLAB
  double * outdata = nullptr ;
  MATFile *pmat;
  pmat = matOpen((path+".mat").c_str(), "w");

  int dimtime=d+2 ;
  vector <long unsigned int> dimensions (3+d, 0) ; // This type to please matlab
  for (int dd=0 ; dd<d ; dd++) dimensions[dd+2] = npt[dd] ;
  dimensions[dimtime] = Time ;

  for (size_t f=0 ; f<Fidx.size() ; f++)
  {
    if (Fidx[f]<0) continue ;

    // Data are goind fast to slow in MATLAB ... so probably need some rewrite ...
    switch (Ftype[f])
    {
      case TensorOrder::SCALAR : dimensions[0]=dimensions[1]=1 ;  //Scalar
              outdata=(double *) mxMalloc(sizeof(double) * 1 * Npt * Time) ;
              for (int t=0 ; t<Time ; t++)
                  for (int i=0 ; i<Npt ; i++)
                      outdata[t*Npt+i]=CGP[idx_FastFirst2SlowFirst(i)].fields[t][Fidx[f]] ;
            break ;
      case TensorOrder::VECTOR : dimensions[0]=d ; dimensions[1]=1 ; // Vector
              outdata=(double *) mxMalloc(sizeof(double) * d * Npt * Time) ;
              for (int t=0 ; t<Time ; t++)
                  for (int i=0 ; i<Npt ; i++)
                      for (int j=0 ; j<d ; j++)
                          outdata[t*Npt*d + i*d +j]=CGP[idx_FastFirst2SlowFirst(i)].fields[t][Fidx[f]+j] ;
            break ;
      case TensorOrder::TENSOR : dimensions[0]=dimensions[1]=d ; //Tensor
              outdata=(double *) mxMalloc(sizeof(double) * d*d * Npt * Time) ;
              for (int t=0 ; t<Time ; t++)
                  for (int i=0 ; i<Npt ; i++)
                      for (int j=0 ; j<d*d ; j++)
                          outdata[t*Npt*d*d + i*d*d +j/d*d + j%d]=CGP[idx_FastFirst2SlowFirst(i)].fields[t][Fidx[f]+j] ; // j/d*d!=j because of integer division
            break ;
      default: printf("ERR: this should never happen. \n") ;
    }

    auto tmpdim = dimensions ;
    if (squeeze)
      tmpdim.erase(std::remove(tmpdim.begin(), tmpdim.end(), 1), tmpdim.end()) ;

    mxArray * pm = mxCreateNumericArray(tmpdim.size(), tmpdim.data(), mxDOUBLE_CLASS, mxREAL);
    mxSetData (pm, outdata) ;
    matPutVariable(pmat, Fname[f].c_str(), pm);
    mxFree(outdata) ;
  }

  // Let's put the location array there as well
  dimensions[0]=d ; dimensions[1] = 1 ; dimensions.pop_back() ;
  auto tmpdim = dimensions ;
  if (squeeze) tmpdim.erase(std::remove(tmpdim.begin(), tmpdim.end(), 1), tmpdim.end()) ;
  mxArray * pm = mxCreateNumericArray(tmpdim.size(), tmpdim.data(), mxDOUBLE_CLASS, mxREAL);
  outdata=(double *) mxMalloc(sizeof(double) * d * Npt) ;
  for (int i=0 ; i<Npt ; i++)
    for (int j=0 ; j<d ; j++)
      outdata[i*d +j]=CGP[idx_FastFirst2SlowFirst(i)].location[j];
  mxSetData (pm, outdata) ;
  matPutVariable(pmat, "GridLocation", pm);
  mxFree(outdata) ;

  matClose(pmat) ;
#else
  printf("Matlab writing not implemented") ;
#endif
return 0 ;
}
//----------------------------------------------------
int Coarsing::write_numpy (string path, bool squeeze)
{
  auto outpath = path + ".npz" ;
  FILE * out = fopen(outpath.c_str(), "wb") ;
  if (out ==nullptr)
  {
    printf("ERR: cannot create file %s.\n", outpath.c_str()) ;
    std::exit(1) ;
  }
  auto write_short = [&](uint16_t a) { fwrite (&a, 2, 1, out) ; } ;
  auto write_int = [&](uint32_t a) { fwrite (&a, 4, 1, out) ; } ;
  auto write_long = [&](uint64_t a) { fwrite (&a, 8, 1, out) ; } ;
  auto getcrc = [&] (uint8_t * buf, int len) { boost::crc_32_type result; result.process_bytes(buf, len); return result.checksum();} ;
  uint8_t *centraldir=nullptr, *centraldirbeg=nullptr ;

  uint16_t nfiles = 0 ;
  for (int f=-1 ; f<static_cast<int>(Fidx.size()) ; f++) //f==-1 is for the location array
  {
    if (f>-1 && Fidx[f]<0) continue ;
    nfiles++ ;

    uint32_t offset = ftell(out) ;
    fwrite ("\x50\x4b\x03\x04", 1, 4, out) ;
    write_short(20) ;
    write_short(0) ; // Flags
    write_short(0) ; // No compression
    write_short(0) ;
    write_short(0) ;

    size_t numbytes ;
    uint8_t* outarray ;
    if (f==-1)
      std::tie(numbytes, outarray) = write_numpy_locbuffer(squeeze) ;
    else
      std::tie(numbytes, outarray) = write_numpy_buffer(f, squeeze) ;
    auto crc = getcrc(outarray, numbytes) ;
    write_int(crc) ; //CRC
    write_int(numbytes) ;
    write_int(numbytes) ;
    if (f==-1)
      write_short(3 +4) ; //"LOC".length()=3 but this is not correct cpp.
    else
      write_short(Fname[f].length() + 4) ;
    write_short(20) ;
    if (f==-1)
      fwrite ("LOC", 1, 3, out) ;
    else
      fwrite (Fname[f].data(), 1, Fname[f].length(), out) ;
    fwrite (".npy", 1, 4, out);

    // Not too sure why we need the extended field but ok ...
    write_short(1) ;
    write_short(16) ;
    write_long(numbytes) ;
    write_long(numbytes) ;

    fwrite (outarray, 1, numbytes, out) ;
    free(outarray) ;

    // Preparing the central directory
    if (centraldir == nullptr)
    {
      if (f==-1)
        centraldir = static_cast<uint8_t*> (malloc(46+3+4)) ;
      else
        centraldir = static_cast<uint8_t*> (malloc(46+Fname[f].length()+4)) ;
      centraldirbeg = centraldir ;
    }
    else
    {
        int off = centraldir-centraldirbeg ;
        if (f==-1)
          centraldirbeg=static_cast<uint8_t*>(realloc(centraldirbeg, off+46+3+4));
        else
          centraldirbeg=static_cast<uint8_t*>(realloc(centraldirbeg, off+46+Fname[f].length()+4));
        centraldir = centraldirbeg + off ;
    }
    memcpy(centraldir, "\x50\x4b\x01\x02", 4) ; centraldir+=4 ;
    *centraldir = 0x14 ; centraldir++ ;
    *centraldir = 0x03 ; centraldir++ ;
    *centraldir = 0x14 ; centraldir++ ;
    *centraldir = 0x00 ; centraldir++ ;
    uint64_t tmpl = 0 ;
    memcpy(centraldir, &tmpl, 8) ; centraldir +=8 ;
    memcpy(centraldir, &crc, 4) ; centraldir +=4 ; //CRC TODO
    memcpy(centraldir, &numbytes, 4) ; centraldir +=4 ;
    memcpy(centraldir, &numbytes, 4) ; centraldir +=4 ;
    uint16_t tmps = ((f==-1)?3:Fname[f].length()) + 4 ;
    memcpy(centraldir, &tmps, 2) ; centraldir +=2 ;
    tmps=0 ;
    memcpy(centraldir, &tmps, 2) ; centraldir +=2 ;
    memcpy(centraldir, &tmps, 2) ; centraldir +=2 ;
    memcpy(centraldir, &tmps, 2) ; centraldir +=2 ;
    memcpy(centraldir, &tmps, 2) ; centraldir +=2 ;

    memcpy(centraldir, &tmps, 2) ; centraldir +=2 ;
    *centraldir = 0x80 ; centraldir ++ ;
    *centraldir = 0x01 ; centraldir ++ ;

    memcpy(centraldir, &offset, 4) ; centraldir +=4 ; // OFFSET
    if (f==-1)
      {memcpy(centraldir, "LOC", 3) ; centraldir += 3 ;}
    else
      {memcpy(centraldir, Fname[f].data(), Fname[f].length()) ; centraldir += Fname[f].length() ;}
    memcpy(centraldir, ".npy", 4) ; centraldir += 4 ;
  }

  int off = centraldir-centraldirbeg ;
  centraldirbeg=static_cast<uint8_t*>(realloc(centraldirbeg, off+22));
  centraldir = centraldirbeg + off ;

  uint32_t centraldirsize = centraldir-centraldirbeg ;
  uint32_t offset = ftell(out) ;
  memcpy(centraldir, "\x50\x4b\x05\x06", 4) ; centraldir+=4 ;
  uint16_t tmps=0 ;
  memcpy(centraldir, &tmps, 2) ; centraldir +=2 ;
  memcpy(centraldir, &tmps, 2) ; centraldir +=2 ;
  memcpy(centraldir, &nfiles, 2) ; centraldir +=2 ;
  memcpy(centraldir, &nfiles, 2) ; centraldir +=2 ;
  memcpy(centraldir, &centraldirsize, 4) ; centraldir+=4 ;
  memcpy(centraldir, &offset, 4) ; centraldir+=4 ;
  memcpy(centraldir, &tmps, 2) ; centraldir +=2 ;

  fwrite(centraldirbeg, 1, centraldir-centraldirbeg, out) ;

  fclose(out);

  return 0 ;
}
//-------
int Coarsing::write_numpy_npy (string path, bool squeeze)
{
 for (size_t f=0 ; f<Fidx.size() ; f++)
 {
   if (Fidx[f]<0) continue ;

   auto outpath = path + "-" + Fname[f] + ".npy" ;
   FILE * out = fopen(outpath.c_str(), "wb") ;
   if (out ==nullptr)
   {
    printf("ERR: cannot create file %s.\n", outpath.c_str()) ;
    std::exit(1) ;
   }

   auto [numbytes, outarray] = write_numpy_buffer(f, squeeze) ;
   fwrite (outarray, 1, numbytes, out) ;
   free(outarray) ;
   fclose(out) ;
 }
return 0 ;
}
//---------
std::pair<size_t, uint8_t*> Coarsing::write_numpy_buffer (int id, bool squeeze)
{
uint8_t *outarray;
size_t numbytes = 0 ;
int dimtime=d+2 ;
vector <long unsigned int> dimensions (3+d, 0) ; // This type to please matlab
for (int dd=0 ; dd<d ; dd++) dimensions[dd+2] = npt[dd] ;
dimensions[dimtime] = Time ;

if (id == -2)
{
  dimensions[0]=d ;
  dimensions[1]=1 ;
  dimensions[dimtime]=1 ;
}
else
{
  switch (Ftype[id])
  {
      case TensorOrder::SCALAR : dimensions[0]=dimensions[1]=1     ; break ;
      case TensorOrder::VECTOR : dimensions[0]=d ; dimensions[1]=1 ; break ;
      case TensorOrder::TENSOR : dimensions[0]=d ; dimensions[1]=d ; break ;
      default: printf("ERR: this should never happen. (wrong TensorOrder...)\n") ;
  }
}

auto tmpdim = dimensions ;
if (squeeze) tmpdim.erase(std::remove(tmpdim.begin(), tmpdim.end(), 1), tmpdim.end()) ;

std::string header = "{'descr': '<f8', 'fortran_order': False, 'shape': (" ;
for (size_t i=0 ; i<tmpdim.size() ; i++)
{
    header += std::to_string(tmpdim[i]) ;
    header += ",";
}
header += "), }" ;
int padding = ceil((header.length() + 6 + 4)/64.)*64 ;
padding = padding - (header.length() + 6 + 4)-1 ;
header += std::string (padding, ' ');
header += "\n" ;
if (id==-2)
  outarray = static_cast<uint8_t*> (malloc (6+2+2+header.length() + dimensions[0]*Npt*8)) ;
else
  outarray = static_cast<uint8_t*> (malloc (6+2+2+header.length() + dimensions[0]*dimensions[1]*Npt*Time*8)) ;
memcpy(outarray, "\x93NUMPY", 6) ;
outarray[6] = 1 ; outarray[7]=0 ;
outarray[8] = header.length()&0xff ; outarray[9]=(header.length()>>8)&0xff ;
memcpy (outarray+10, header.data(), header.length()) ;
numbytes = 10 + header.length() ;
if (id==-2)
  for (long unsigned int k=0 ; k<dimensions[0] ; k++)
      for (int i=0 ; i<Npt ; i++, numbytes+=8)
          memcpy(outarray+numbytes, &(CGP[i].location[k]), 8) ;
else
  for (long unsigned int k=0 ; k<dimensions[0] ; k++)
      for (long unsigned int j=0 ; j<dimensions[1] ; j++)
          for (int i=0 ; i<Npt ; i++)
              for (int t=0 ; t<Time ; t++, numbytes += 8)
                  memcpy(outarray+numbytes, &(CGP[i].fields[t][Fidx[id]+j*d+k]), 8) ;

return (std::make_pair(numbytes, outarray)) ;
}



//--------------------------------------------------------
int Coarsing ::write_NrrdIO ([[maybe_unused]] string path)
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

    for (size_t f=0 ; f<Fidx.size() ; f++)
    {
      if (Fidx[f]<0) continue ;


    // Data are goind fast to slow in NrrdIO ... so probably need some rewrite ...
      switch (Ftype[f])
      {
        case TensorOrder::SCALAR : dimensions[0]=dimensions[1]=1 ;  //Scalar
                outdata=(double *) malloc(sizeof(double) * 1 * Npt * Time) ;
                for (int t=0 ; t<Time ; t++)
                    for (int i=0 ; i<Npt ; i++)
                        outdata[t*Npt+i]=CGP[idx_FastFirst2SlowFirst(i)].fields[t][Fidx[f]] ;
              break ;
        case TensorOrder::VECTOR : dimensions[0]=d ; dimensions[1]=1 ; // Vector
                outdata=(double *) malloc(sizeof(double) * d * Npt * Time) ;
                for (int t=0 ; t<Time ; t++)
                    for (int i=0 ; i<Npt ; i++)
                        for (int j=0 ; j<d ; j++)
                            outdata[t*Npt*d + i*d +j]=CGP[idx_FastFirst2SlowFirst(i)].fields[t][Fidx[f]+j] ;
              break ;
        case TensorOrder::TENSOR : dimensions[0]=dimensions[1]=d ; //Tensor
                outdata=(double *) malloc(sizeof(double) * d*d * Npt * Time) ;
                for (int t=0 ; t<Time ; t++)
                    for (int i=0 ; i<Npt ; i++)
                        for (int j=0 ; j<d*d ; j++)
                            outdata[t*Npt*d*d + i*d*d +j/d*d + j%d]=CGP[idx_FastFirst2SlowFirst(i)].fields[t][Fidx[f]+j] ; // j/d*d!=j because of integer division
              break ;
        default:
            outdata=nullptr ;
            printf("ERR: this should never happen. \n") ;
      }

      nrrdWrap_nva(nval, outdata, nrrdTypeDouble, d+3, dimensions.data());
      string fullpath ;
      fullpath = path + Fname[f] + ".nrrd" ;
      nrrdSave(fullpath.c_str(), nval, nio);
      free(outdata) ;

      printf("%s ", fullpath.c_str()) ;
    }
#else
  printf("ERR: Not compiled with NRRD support.\n") ;
#endif
return 0 ;
}
//------------------------------------------
int Coarsing::write_netCDF ([[maybe_unused]] string sout)
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
//==================================


int Coarsing::idx_FastFirst2SlowFirst (int n)
{
  // Get an Integer in [0, Ncpt] going 1st dimension fast, convert in idx list, then back to an integer between 0 & Ncpt, but with first dimension slow
  vector <int> idx (d,0) ;
  //printf("[%d] ", n) ;
  for (int i=0 ; i<d ; n/=npt[i],i++)
    idx[i]=n%npt[i] ;
  int res=0 ;
  for (int i=0; i<d ; i++)
    res += idx[i]*nptcum[i] ;
  return res ;
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

//==============================================================================
double Volume (int d, double R)
{
  if (d%2==0)
    return (pow(boost::math::double_constants::pi,d/2)*pow(R,d)/( boost::math::factorial<double>(d/2) )) ;
  else
  {
   int k=(d-1)/2 ;
   return(2* boost::math::factorial<double>(k) * pow(4*boost::math::double_constants::pi, k) *pow(R,d) / (boost::math::factorial<double>(d))) ;
  }
}
