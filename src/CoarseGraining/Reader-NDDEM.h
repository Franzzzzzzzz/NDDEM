#include "../Dem/Xml.h"
#ifndef NDDEMREADER
#define NDDEMREADER

class NDDEMReader : public Reader {
public:
    NDDEMReader(std::string ppath): path(ppath) {
        XML = new XMLReader(path) ;
        dimension=atoi(XML->tags.second["dimensions"].c_str()) ;
        XML->read_boundaries(bounds) ;
        XML->read_radius (radius) ;
        Nparticles = radius.size() ;
        reorganised_pos.resize(dimension, nullptr) ;
        reorganised_vel.resize(dimension, nullptr) ;
        reorganised_omega.resize((dimension*(dimension-1))/2, nullptr) ;
        startingpoint = XML->fic.tellg() ;
    }
    ~NDDEMReader () {XML->close() ; delete(XML) ;}

    void post_init() {
      for (int i=0 ; i<Nparticles ; i++)
      {
          mass.push_back(Volume(dimension,radius[i]) * get_default_density()) ;
          Imom.push_back(InertiaMomentum(dimension,radius[i],get_default_density())) ;
      }
    }

    int get_dimension () {return dimension;}
    std::vector<std::vector<double>> get_bounds() {return bounds ; }
    int get_numts() {return build_index() ; }

    int read_timestep (int ts)  ;
    double * get_data(DataValue datavalue, int dd, std::string name="") ;
    int get_num_particles () {return Nparticles;}
    int get_num_contacts () {return Ncontacts;}

    int build_index() {index = XML->read_index() ; return index.size() ; }
    double actualts=0 ;

private:
    XMLReader * XML ;
    std::string path ;
    std::streampos startingpoint ;
    int dimension ;
    std::vector<std::vector<double>> bounds ;
    std::vector <double> radius ;
    std::vector <double> mass, Imom ;
    int Nparticles=-1, Ncontacts=-1 ;
    std::vector<std::string> names ;
    std::vector<std::vector<std::vector<double>>> data ;
    std::vector<std::pair<double,std::streampos>> index ;

    double InertiaMomentum (int d , double R, double rho) ;
    void clear_tsdata () {names.clear() ; for (auto & v:data) for (auto & w: v) w.clear() ; for (auto & v:data) v.clear() ; data.clear() ; }

    std::vector<double*> reorganised_pos ;
    std::vector<double*> reorganised_vel ;
    std::vector<double*> reorganised_omega ;
} ;

//=================================================================================
int NDDEMReader::read_timestep (int ts)
{
    if (index.size()>0)
    {
        XML->fic.seekg(index[ts].second, ios_base::beg) ;
        clear_tsdata() ;
        actualts = XML->read_nextts(names, data) ;
        curts = ts ;
        if (static_cast<float>(actualts) != static_cast<float>(index[ts].first)) printf("WARN: the timestep read is not the one that was expected %.10g %.10g\n", actualts, index[ts].first) ;
    }
    else
    {
        if (curts>ts)
        {
            printf("WARN: this file can only read forward. Restarting if from scratch. \n") ;
            XML->fic.seekg(startingpoint) ;
            curts=-1 ;
        }
        while (curts<ts)
        {
            clear_tsdata() ;
            actualts = XML->read_nextts(names, data) ;
            curts ++ ;
        }
    }
    return 0 ;
}
//------------------------------------------------------------------------
 double * NDDEMReader::get_data(DataValue datavalue, int dd, std::string name)
{
    int delta ;
    switch (datavalue)
    {
        case DataValue::radius : return (radius.data()) ;
        case DataValue::mass : return (mass.data()) ;
        case DataValue::Imom : return (Imom.data()) ;
        case DataValue::pos :
            delta = find(names.begin(), names.end(), "Position")-names.begin() ;
            if (reorganised_pos[dd]==nullptr) reorganised_pos[dd] = (double *) malloc (sizeof(double)*Nparticles) ;
            for (int j=0 ; j<Nparticles ; j++) reorganised_pos[dd][j] = data[delta][j][dd] ;
            return reorganised_pos[dd] ;
        case DataValue::vel :
            delta=find(names.begin(), names.end(), "Velocity")-names.begin() ;
            if (reorganised_vel[dd]==nullptr) reorganised_vel[dd] = (double *) malloc (sizeof(double)*Nparticles) ;
            for (int j=0 ; j<Nparticles ; j++) reorganised_vel[dd][j] = data[delta][j][dd] ;
            return reorganised_vel[dd] ;
        case DataValue::omega:
            delta=find(names.begin(), names.end(), "Omega")-names.begin() ;
            if (delta==names.size()) return nullptr ; 
            if (reorganised_omega[dd]==nullptr) reorganised_omega[dd] = (double *) malloc (sizeof(double)*Nparticles) ;
            for (int j=0 ; j<Nparticles ; j++) reorganised_omega[dd][j] = data[delta][j][dd] ;
            return reorganised_omega[dd] ;
        default :
            return nullptr ;
    }
}


#define MAXDEFDIM 30
double NDDEMReader::InertiaMomentum (int d , double R, double rho)
{
 if (d>MAXDEFDIM)
 {
  printf("[WARN] Inertia InertiaMomentum not guaranteed for dimension > %d\n", MAXDEFDIM) ;
 }

 double res ;
 if (d%2==0)
 {
   unsigned int k=d/2 ;
   res=pow(boost::math::double_constants::pi,k)*pow(R, d+2) / boost::math::factorial<double>(k+1) ;
   return (res*rho) ;
 }
 else
 {
   unsigned int k=(d-1)/2 ;
   res=pow(2,k+2) * pow(boost::math::double_constants::pi, k) * pow(R, d+2) / boost::math::double_factorial<double> (d+2) ;
   return (res*rho) ;
 }
}
#endif
