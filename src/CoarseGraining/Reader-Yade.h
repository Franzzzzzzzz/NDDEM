#ifndef YADEREADER
#define YADEREADER
#include "Reader.h"
#include "Typedefs.h"
#include "pugixml.hpp"
#include <algorithm>
#include <vector>
#include <map>
#include "zlib.h"

class YadeReader : public Reader {
public:
    virtual std::vector<std::vector<double>> get_bounds() 
    {
      if (curts==-1) read_timestep(0) ; 
      else read_timestep(curts) ; 
      
      vector<vector<double>> res(2, vector<double>(3,0)) ;
      auto x = std::minmax_element(data[0].begin(), data[0].end()) ; 
      auto y = std::minmax_element(data[1].begin(), data[1].end()) ; 
      auto z = std::minmax_element(data[2].begin(), data[2].end()) ;
      res[0][0]=*(x.first) ; res[1][0]=*(x.second) ; 
      res[0][1]=*(y.first) ; res[1][1]=*(y.second) ; 
      res[0][2]=*(z.first) ; res[1][2]=*(z.second) ; 
      
      return res ; 
    }
    virtual std::vector<double> get_minmaxradius() 
    {
      if (curts==-1) read_timestep(0) ; 
      else read_timestep(curts) ; 
      auto r = std::minmax_element(data[6].begin(), data[6].end()) ; 
      return {*r.first, *r.second} ; 
    }
    
    virtual int get_num_particles () {return N;}
    virtual int get_num_contacts () {return -1;}
    
    std::map <DataValue, size_t> data_mapping = {{DataValue::pos, 0}, {DataValue::vel, 3}, {DataValue::radius, 6}, {DataValue::mass, 7}, {DataValue::omega,8}} ; 
    virtual double * get_data(DataValue datavalue, int dd, std::string name="") 
    { 
      for (const auto& [key, idx] : data_mapping)
        if (key==datavalue) 
          return data[idx].size()==0 ? nullptr : &(data[idx+dd][0]) ;
      return nullptr ; 
      /*try {
       int idx = data_mapping.at(datavalue) ;  
       return data[idx].size()==0 ? nullptr : &(data[idx+dd][0]) ;
      }
      catch (const std::out_of_range &e) { return nullptr ; }*/
    }
   
  
    // Base64 functions 
    int read_timestep(int ts) ; 
    int getfield_from_data (std::string name, std::vector<std::string> &variables, std::vector<size_t> &var_offsets, std::string &raw64) ; 
    std::vector<uint8_t> base64decode (const std::vector<uint8_t> & base64) ; 
    size_t length_int2base64 (uint64_t bytes) 
    { 
      size_t res = ceil(bytes*4*8/6.) ; 
      if (res%4) res = res + (4-(res%4)) ;
      return res ; 
    }
    size_t length_byte2base64 (uint64_t bytes) 
    { 
      size_t res = ceil(bytes*8/6.) ; 
      if (res%4) res = res + (4-(res%4)) ;
      return res ; 
    }
    uint32_t bytes2uint32 (const uint8_t * a)
    {
      return ((*a) | *(a+1)<<8 | *(a+2)<<16 | *(a+3)<<24) ; 
    }
    
    double byte2double (const uint8_t * b, char n)
    {
      static_assert((sizeof(float)==4)) ; 
      static_assert((sizeof(double)==8)) ;
        
      #pragma GCC diagnostic push
      #pragma GCC diagnostic ignored "-Wstrict-aliasing"
      if (n==4)
      {
        uint32_t v = b[0] | b[1]<<8 | b[2]<<16 | b[3]<<24 ;
        return static_cast<double>(*reinterpret_cast<float*>(&v)) ; 
      }
      else if (n==8)
      {
        uint64_t v=b[7] ;
        for (int i=1 ; i<8 ; i++) {v<<=8; v|=b[7-i] ;} 
        //printf("%lX %d %d %d %d %d %d %d %d %g\n", v, b[7], b[6], b[5], b[4], b[3], b[2], b[1], b[0], *reinterpret_cast<double *>(&v)) ; 
        return *reinterpret_cast<double *>(&v) ; 
      }
      else return 0 ; 
      #pragma GCC diagnostic pop
    }
    
    virtual int get_numts ()
    {
      return Reader::get_numts() ; 
    }
        
    std::string path ;
    int dimension=3 ; 
    v2d data ;
    int N ; 
    
private: 
    std::vector<std::string> variables ; 
    std::vector<size_t> var_offsets ; 
    std::string raw64 ; 
} ;     

#endif
