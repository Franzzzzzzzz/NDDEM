#include <vector>
#include <string>
#include <cmath>
#include "Typedefs.h"

#ifndef READER
#define READER

class Reader {
public:
    virtual std::vector<std::vector<double>> get_bounds() {return {} ; }
    virtual int get_dimension () {return 3 ;}
    virtual int get_numts() {return -1; }
    virtual int get_num_particles () {return -1;}
    virtual int get_num_contacts () {return -1;}
    virtual double * get_data([[maybe_unused]] DataValue datavalue, [[maybe_unused]] int dd) {return nullptr ; } 
    virtual int build_index () {return -1 ;}
    virtual int read_timestep ([[maybe_unused]] int ts) {return -1 ; }
    
    void set_default_radiusdensity (double radius, double density) {Radius=radius ; Density=density ;}
    void set_default_radius (double radius) {Radius=radius ; }
    void set_default_density (double density) {Density=density ; }
    double get_default_radius () 
    {
        static bool info=true ;
        if (Radius==-1)
        {
          if (info)
          {
              printf("ERR: the default radius was requested but not set!!!!! set to 1\n") ; 
              info=false ; 
          }
          return 1. ;
        }
        else 
            return Radius ;
    }
    double get_default_density () 
    {
        static bool info=true ;
        if (Density==-1)
        {
          if (info)
          {
              printf("ERR: the default density was requested but not set!!!!! set to 1\n") ; 
              info=false ; 
          }
          return 1. ;
        }
        else 
            return Density;
    }
    
    //virtual double * get_data(DataValue, int dd) {return nullptr;}
    
    bool is_seekable = false ; 
    bool is_mapped_ts = false ; 
    std::vector <std::streampos> mapped_ts ; 
    
    void build_pospqlpq_from_ids (v2d & contactarray , int idx_id1, int idx_id2, int idx_pospq, int idx_lpq,
                                  v2d & particlearray, int idx_pos, int idx_r=-1)
    {
        //size_t N =  particlearray[idx_pos].size() ; 
        size_t Nc = contactarray[idx_id1].size() ; 
        int d=get_dimension() ; 
        for (int i=0 ; i<d ; i++)
        {
            contactarray[idx_pospq+i].resize(Nc) ; 
            contactarray[idx_lpq+i].resize(Nc) ; 
        }
        
        for (size_t i=0 ; i<Nc ; i++)
        {
            int id1=static_cast<int>(contactarray[idx_id1][i]) ; 
            int id2=static_cast<int>(contactarray[idx_id2][i]) ;
            double fraction = 0.5 ; 
            for (int dd=0 ; dd<d ; dd++)
            {
                contactarray[idx_lpq+dd][i] = particlearray[idx_pos+dd][id1] - particlearray[idx_pos+dd][id2] ; 
                if (idx_r!=-1)
                {
                    fraction = particlearray[idx_r][id2] / (particlearray[idx_r][id1]+particlearray[idx_r][id2]) ; 
                }                    
                contactarray[idx_pospq+dd][i] = particlearray[idx_pos+dd][id2] + fraction * contactarray[idx_lpq+dd][i] ;
            }
        }
    }
    
    int clean_contacts (v2d & contactarray, int id1, int id2, int idx_lpq, v2d & particlearray, int idx_r)
    {
        // WORK IN PROGRESS
        printf("NEED TO WORK ON THAT ...") ; return 0 ;
        /*size_t Nc = contactarray[0].size() ; 
        double dst, rr ; 
        std::vector<bool> rm (Nc,false) ; 
        
        for (size_t i=0 ; i<Nc ; i++)
        {
            dst=0 ;
            for (int dd=0 ; dd<get_dimension() ; dd++)
                dst += (contactarray[i][idx_lpq+dd]*contactarray[i][idx_lpq+dd]) ;
            dst=sqrt(dst) ; 
            rr=particlearray[idx_r][contactarray[i][id1]]+particlearray[idx_r][contactarray[i][id1]] ;
            
        }*/
        
    }
                        
    
    
    
protected:
    std::vector<std::pair<double,streampos>> index ; 

private:
    double Radius=-1, Density=-1 ; 
    const double pbcthreshold = 2. ; 
} ; 

#endif
