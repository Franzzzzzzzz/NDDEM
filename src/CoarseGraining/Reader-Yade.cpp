#include "Reader-Yade.h"

//========================================
int YadeReader::read_timestep(int ts)
{
  printf("%d %d\n",ts, curts) ;  
  if (ts==curts) return -1 ;
  
  auto filepath = getpath(ts) ;
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(filepath.c_str());
  if (!result) { printf("ERR: error reading Yade XML %s. File may be not found.\n", filepath.c_str()) ; N=0 ; data.clear() ; return 0; }
  
  N = atoi(doc.child("VTKFile").child("UnstructuredGrid").child("Piece").attribute("NumberOfPoints").value()) ; 
    
  std::vector<std::string> variables ; 
  std::vector<size_t> var_offsets ; 
  std::string raw64 ; 
  
  for (auto v : doc.child("VTKFile").child("UnstructuredGrid").child("Piece"))
    for (auto w: v)
    {
      variables.push_back(w.attribute("Name").value()) ; 
      var_offsets.push_back(atoi(w.attribute("offset").value())) ; 
    }
  
  raw64 = doc.child("VTKFile").child("AppendedData").child_value() ; 
  raw64=raw64.erase(0, raw64.find('_')+1) ;; 
  
  data.resize(11) ; 
  int res = getfield_from_data("Points", variables, var_offsets, raw64) ; if (res==-1) printf("ERR: field Points is required\n") ;
  res = getfield_from_data("radii", variables, var_offsets, raw64) ; 
  if (data[data_mapping[DataValue::radius]].size()==0)
    data[data_mapping[DataValue::radius]].resize(N, get_default_radius()) ; 
  res = getfield_from_data("mass", variables, var_offsets, raw64) ;
  if (data[data_mapping[DataValue::mass]].size()==0)
  {
    data[data_mapping[DataValue::mass]].resize(N, 0) ; 
    for (int i=0 ; i<N ; i++)
      data[data_mapping[DataValue::mass]][i] = 4/3. * M_PI * data[data_mapping[DataValue::radius]][i] * data[data_mapping[DataValue::radius]][i] * data[data_mapping[DataValue::radius]][i] * get_default_density() ; 
  }
  res = getfield_from_data("linVelVec", variables, var_offsets, raw64) ;
  res = getfield_from_data("angVelLen", variables, var_offsets, raw64) ;
  //printf("%g %g %g | %g %g %g | %g %g // %g %g D", data[0][N-1], data[1][N-1],data[2][N-1],data[3][N-1], data[4][N-1], data[5][N-1], data[6][N-1],data[7][N-1], data[5][0], data[6][0]) ; fflush(stdout) ; 
  
  //for (auto v: data[3]) printf("%g ", v) ;  
  
  curts=ts ; 
  return 0 ;
}
//=======================================
int YadeReader::getfield_from_data (std::string name, std::vector<std::string> &variables, std::vector<size_t> &var_offsets, std::string &raw64)
{
  size_t index = std::find(variables.begin(), variables.end(), name) - variables.begin() ;
  if (index==variables.size()) { return -1 ; }
  
  int bytepervalue, idx, ncomponents ; 
  if (name=="Points")           { bytepervalue = 4 ; ncomponents = 3 ; idx = 0 ; data[0].resize(N,0) ; data[1].resize(N,0) ; data[2].resize(N,0) ;} //Expect Float32
  else if (name=="linVelVec")   { bytepervalue = 8 ; ncomponents = 3 ; idx = 3 ; data[3].resize(N,0) ; data[4].resize(N,0) ; data[5].resize(N,0) ;} //Expect Float64
  else if (name=="radii")       { bytepervalue = 8 ; ncomponents = 1 ; idx = 6 ; data[6].resize(N,0) ;}
  else if (name=="mass")        { bytepervalue = 8 ; ncomponents = 1 ; idx = 7 ; data[7].resize(N,0) ;}
  else if (name=="angVelLen")   { bytepervalue = 8 ; ncomponents = 3 ; idx = 8 ; data[8].resize(N,0) ; data[9].resize(N,0) ; data[10].resize(N,0) ;}
  else { printf("ERR: unknown field %s\n", name.c_str()) ; return -2 ; } 
  
  // 1. Need to get the header. 
  int offset = var_offsets[index] ; 
  std::vector<uint8_t> head64 ; 
  std::vector<uint8_t> head ;
  std::vector<uint32_t> header ; 
  
  //1a: get the minimum header to extract the first header byte, which is the number of blocks. (byte2: block size, 3: last block size, 4:4+nblock-1 block compressed size). 
  for (int i=0 ; i<12 ; i++) head64.push_back(raw64[i+offset]) ;
  head = base64decode(head64) ; 
  header.push_back(bytes2uint32(head.data())) ;
  auto headerlength = length_int2base64(header[0]+3) ; 
  head64.clear() ;  
  
  //1b: redoing the header reading now that we know it's length
  for (size_t i=0 ; i<headerlength ; i++) head64.push_back(raw64[i+offset]) ; 
  head=base64decode(head64) ; 
  for (size_t i=4 ; i<head.size() ; i+=4) header.push_back(bytes2uint32(head.data()+i));
      
  // 2. Header found, let's grab the datas ...
  std::vector<uint8_t> data64 ; 
  int totalcompressed = 0 ; 
  for (uint32_t i=0 ; i<header[0] ; totalcompressed+=header[3+i],i++) ;  
  
  //printf(" %d %dX\n", header[0], headerlength) ; fflush(stdout) ; 
  int datalength = length_byte2base64(totalcompressed); 
  for (int i=0 ; i<datalength ; i++) data64.push_back(raw64[i+offset+headerlength]) ;  
  
  std::vector<uint8_t> datacompress = base64decode(data64) ; 
  uint8_t * buffer ; size_t data_offset=0 ; 
  unsigned long int us=header[1]; 
  buffer=(uint8_t*) malloc(header[1]) ;
  int jlast = 0 ;
  for (uint32_t i=0; i<header[0] ; data_offset+=header[3+i], i++, us=header[1])
  { 
    int error = uncompress(buffer, &us, datacompress.data()+data_offset, header[3+i]) ; 
    //printf("%d %ld\n", error, us) ; 
    if (error!=0 || us != ((i+1==header[0])?header[2]:header[1])) printf("Decompression error\n") ; 
                                       
    for (size_t j=jlast ; j<jlast+us/bytepervalue ; j++)
    {
      data[idx+j%ncomponents][j/ncomponents] = byte2double(buffer+(j-jlast)*bytepervalue, bytepervalue) ;
    }
    jlast += us/bytepervalue ; 
  }  
  free (buffer) ; 
  return 0;
}
//==============================================================================
std::vector<uint8_t> YadeReader::base64decode (const std::vector<uint8_t> & base64)
{
  static const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  std::vector<uint8_t> result;

  size_t padding = std::count(base64.begin(), base64.end(), '=');
  size_t length = ((base64.size()- padding) * 6) / 8  ; 

  result.reserve(length);
  for (size_t i = 0; i < base64.size(); i += 4) {
        uint32_t sextet1 = base64Chars.find(base64[i]);
        uint32_t sextet2 = base64Chars.find(base64[i + 1]);
        uint32_t sextet3 = base64Chars.find(base64[i + 2]=='='?'A':base64[i + 2]);
        uint32_t sextet4 = base64Chars.find(base64[i + 3]=='='?'A':base64[i + 3]);

        uint32_t triplet = (sextet1 << 18) | (sextet2 << 12) | (sextet3 << 6) | sextet4;
        
        result.push_back((triplet >> 16) & 0xFF);
        result.push_back((triplet >> 8) & 0xFF);
        result.push_back(triplet & 0xFF);
    }

    result.resize(length);  // Remove any extra padding bytes

    return result;
}
