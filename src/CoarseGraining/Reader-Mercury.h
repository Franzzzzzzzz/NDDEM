#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>

#include "Reader.h"
#include "Typedefs.h"

class MercuryReader : public Reader {
public :
    int get_dimension() {return dimension;}
    virtual double * get_data([[maybe_unused]] DataValue datavalue, [[maybe_unused]] int dd, [[maybe_unused]] std::string name="") {return nullptr ; } 
    
    int dimension=3 ; 
    std::ifstream file_in ; 
    
    void reset()
    {
        file_in.clear(); 
        file_in.seekg(0); 
        curts=0 ; 
    }
} ; 

//----------------------------------------------------------
class MercuryReader_vtu_particles: public MercuryReader {
public:
    MercuryReader_vtu_particles(std::string ppath) ; 
    int get_numts() {return numts; }
    int get_num_particles () {return N;}
    
    double * get_data(DataValue datavalue, int dd, std::string name="" ) 
    { 
        switch(datavalue) {
            case DataValue::pos : return &(data[0+dd][0] ) ;
            case DataValue::vel : return &(data[dimension*1+dd][0] ) ;
            case DataValue::radius : return &(data[dimension*2][0]) ;
            case DataValue::mass : return &(data[dimension*2+1][0]) ;
            
            default : return (nullptr) ; 
        }
    } 
    
    int read_timestep (int ts) ;
      
    v2d data ; 
private :
    int numts = -1 ; 
    int N=0 ; 
};


//----------------------------------------------------------
class MercuryReader_data_particles: public MercuryReader {
public:
    MercuryReader_data_particles(std::string ppath) ; 
    int get_numts() 
    { 
        if (!is_fullymapped)
            build_index() ; 
        return mapped_ts.size() ; 
    }    
    
    int build_index () ;
    std::vector<std::vector<double>> get_bounds() ;
    int get_num_particles () {return N;}
    int read_timestep (int ts) ;
    
    double * get_data(DataValue datavalue, int dd, std::string name="") 
    {
        switch(datavalue) {
            case DataValue::pos :    return &(data[0+dd][0] ) ;
            case DataValue::vel :    return &(data[dimension*1+dd][0] ) ;
            case DataValue::radius : return &(data[dimension*2][0]) ;
            case DataValue::omega :  return &(data[dimension*2 + 1 +(dimension==3?3:1)+dd][0] ) ;
            case DataValue::mass :   return &(dataextra[0][0]) ;
            case DataValue::Imom :   return &(dataextra[1][0] ) ;
            
            default : return (nullptr) ; 
        }
    } 
    
    v2d data ; 
    v2d dataextra ; 
    
private:
    bool is_vtu=false ; 
    int N ;
    
} ; 


//-----------------------------
class MercuryReader_data_contacts: public MercuryReader {
public: 
    MercuryReader_data_contacts(std::string ppath, Reader *d);
    int build_index () ;
    int get_num_contacts () {return Nc;}
    int read_timestep(int ts) ; 
    virtual double * get_data (DataValue datavalue, int dd, std::string name="") {
        switch(datavalue) {
            case DataValue::id1 : return &(data[0][0]) ;
            case DataValue::id2 : return &(data[1][0] ) ;
            
            case DataValue::pospq : return &(data[2+dd][0] ) ;
            case DataValue::lpq   : return &(data[5+dd][0] ) ;
            case DataValue::fpq   : return &(data[8+dd][0] ) ;
            case DataValue::mpq   : return nullptr ;
            case DataValue::mqp   : return nullptr ;
            
            default : return (nullptr) ; 
        }
    }
    
    v2d data ; 
private :
    MercuryReader_data_particles * dump ; 
    unsigned int Nc;
    const int growth=100 ; 
    
} ; 
