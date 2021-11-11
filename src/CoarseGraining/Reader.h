#include <vector>
#include <string>
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
    
    //virtual double * get_data(DataValue, int dd) {return nullptr;}
    
    bool is_seekable = false ; 
    bool is_mapped_ts = false ; 
    std::vector <std::streampos> mapped_ts ; 
    
protected:
    double Radius=0.00075, Density=2500 ; 
    std::vector<std::pair<double,streampos>> index ; 
} ; 

#endif
