#include <vector>
#include <string>
#include <cmath>
#include <optional>
#include "Typedefs.h"

#ifndef READER
#define READER

class Reader {
public:
    virtual std::vector<std::vector<double>> get_bounds() {return {} ; }
    virtual std::vector<double> get_minmaxradius() {return {} ; }
    virtual int get_dimension () {return 3 ;}
    virtual int get_numts() 
    {
        if (filenumbering.numts != -1) return filenumbering.numts ; 
        if (filenumbering.ismultifile)
        {             
            filenumbering.numts=0 ; 
            FILE * in ; 
            in=fopen(getpath(filenumbering.numts).c_str(), "r") ;  
            while ( in != nullptr )
            {
                printf(".") ; 
                filenumbering.numts++ ; 
                fclose(in) ; 
                //printf("%s\n", getpath(filenumbering.numts).c_str()) ; 
                in=fopen(getpath(filenumbering.numts).c_str(), "r") ;  
            }
            printf("Found %d files\n", filenumbering.numts) ; 
            return filenumbering.numts ; 
        }
        else
            return -1; 
    }
    virtual int get_num_particles () {return -1;}
    virtual int get_num_contacts () {return -1;}
    virtual double * get_data([[maybe_unused]] DataValue datavalue, [[maybe_unused]] int dd, [[maybe_unused]] std::string name="") {return nullptr ; }
    virtual int build_index () {return -1 ;}
    virtual int read_timestep ([[maybe_unused]] int ts) {return -1 ; }
    virtual void post_init() {}

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
    bool is_fullymapped = false ;
    std::vector <std::optional<std::streampos>> mapped_ts ;

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

    // Filebuilder
    std::string path ; 
    int curts=-1 ; 
    struct {
        double initial = 0 ; 
        double delta = 1 ;
        int numts = -1 ;
        bool ismultifile = false ;  
    } filenumbering ; 
    std::string getpath (int ts) 
    {
        char * tmp = nullptr ; 
        size_t pos = path.find('%') ;
        for ( ; !isalpha(path[pos]) ; pos++) ;
        if (path[pos]=='e' || path[pos]=='f' || path[pos]=='g' || path[pos]=='E' || path[pos]=='F' || path[pos]=='G') 
        {
            int len = snprintf(NULL, 0, path.c_str(), filenumbering.initial+filenumbering.delta*ts) ; 
            tmp = (char*) malloc(len) ;
            sprintf(tmp, path.c_str(), filenumbering.initial+filenumbering.delta*ts) ; 
        }
        else if (path[pos]=='d'|| path[pos]=='i' || path[pos]=='u' || path[pos]=='h' || path[pos]=='l' || path[pos]=='j' || path[pos]=='z')
        {
            int len = snprintf(NULL, 0, path.c_str(), static_cast<long long int>(filenumbering.initial+filenumbering.delta*ts)) ; 
            tmp = (char*) malloc(len+1) ;
            sprintf(tmp, path.c_str(), static_cast<long long int>(filenumbering.initial+filenumbering.delta*ts)) ;             
        }
        else printf("ERR: unknown format specifier in file path: %c.\n", path[pos]) ; 
        std::string res = tmp ; 
        free(tmp) ; 
        return res ;
    }



protected:
    std::vector<std::pair<double,streampos>> index ;

private:
    double Radius=-1, Density=-1 ;
} ;

#endif
