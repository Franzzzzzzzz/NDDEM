#include "Reader-Mercury.h"

MercuryReader_particles::MercuryReader_particles(std::string ppath)
{
  path=ppath ; 
  std::size_t found = path.find_last_of(".");
  if (path.substr(found+1)=="data")
     is_vtu=false ; 
  else if (path.substr(found+1)=="vtu")
     {is_vtu=true ; printf("ERR: MercuryReader_particles VTU format not implemented.\n") ; }
  else
     printf("ERR: MercuryReader_particles file extension unknown\n") ;   

  file_in.open(path, ios_base::in | ios_base::binary); 
  if (!file_in.is_open()) printf("ERR: cannot open file %s\n", path.c_str()) ;

  std::string line ; 
  getline(file_in, line) ;  
  int res=std::count(line.begin(), line.end(), ' ');
  printf("%d ", res) ; fflush(stdout) ; 
  if (res==8) 
  {
    dimension=3 ;
    data.resize(14) ;
    dataextra.resize(2) ; 
  }
  else if (res==6) 
  {
    dimension=2 ; 
    data.resize(8) ; 
    dataextra.resize(2) ; 
  }
  else
    printf("ERR: unknown .data file format (first line wrong)\n") ;

  is_seekable= true ; 
  reset() ; 
}
//------
MercuryReader_contacts::MercuryReader_contacts(std::string ppath, Reader *d): dump(dynamic_cast<MercuryReader_particles*>(d))
{
  path=ppath ; 
  dimension=dump->get_dimension() ; 
  file_in.open(path, ios_base::in | ios_base::binary); 
  if (!file_in.is_open()) printf("ERR: cannot open file %s\n", path.c_str()) ;
  data.resize(11) ; 
  is_seekable=true ; 
}

//--------------------------------------------
int MercuryReader_particles::build_index () 
{
  reset() ;
  std::string line ;
  mapped_ts.clear() ; 
  while (!file_in.eof())
  {
      getline(file_in, line) ;
      int res=std::count(line.begin(), line.end(), ' ');
      if (res==8 || res==6) 
          mapped_ts.push_back(file_in.tellg()-static_cast<std::streamoff>(line.size()+1)) ;  
  }
  is_mapped_ts=true ; 
  reset() ; 
  return mapped_ts.size() ; 
}
//-------
int MercuryReader_contacts::build_index () 
{
  reset() ;
  std::string line ;
  int header=2 ;
  mapped_ts.clear() ; 
  while (!file_in.eof())
  {
      getline(file_in, line) ;
      if (line[0]=='#')
      {
          header++ ; 
          if (header>=3)
          {
            mapped_ts.push_back(file_in.tellg()-static_cast<std::streamoff>(line.size()+1)) ;  
            header=1 ; 
          }
      }
  }
  is_mapped_ts=true ; 
  reset() ; 
  return mapped_ts.size() ; 
}

//--------------------------------------------    
std::vector<std::vector<double>> MercuryReader_particles::get_bounds() 
{
  std::vector<std::vector<double>> res ; 
  res.resize(2, std::vector<double>(dimension, 0)) ; 
  file_in.seekg(0) ;
  double tmp ; 
  file_in >> tmp ; 
  file_in >> tmp ;
  for (int i=0 ; i<2 ; i++)
      for (int j=0 ; j<dimension ; j++)
          file_in >> res[i][j] ; 
  reset() ;  
  return res ; 
}
 
//--------------------------------------------
int MercuryReader_particles::read_timestep (int ts) 
{
  if (ts==curts) return 0;
  
  if (mapped_ts.size()<=static_cast<size_t>(ts))
  {
      std::string line ;
      while (ts>curts+1)
      {
          getline(file_in, line) ;
          int res=std::count(line.begin(), line.end(), ' ');
          if (res==8 || res==6) 
          {
              curts++ ; 
              if (mapped_ts.size()<static_cast<size_t>(curts+2)) mapped_ts.resize(curts+2) ; 
              mapped_ts[curts+1]=file_in.tellg()-static_cast<std::streamoff>(line.size()+1) ; 
          }
      }
  }
  
  file_in.seekg(mapped_ts[ts]) ; 
  
  //---- Should finally be at the right location ...
  double tmp ; 
  file_in >> N ; 
  
  int Nfield = data.size() ; 
  for (auto & v: data) v.resize(N) ; 
  for (auto & v: dataextra) v.resize(N) ; 
  for (int i=0 ; i<1+dimension*2 ; i++) file_in >> tmp ; // don't care
  for (int i=0 ; i<N ; i++)
  {
      for (int j=0 ; j<Nfield ; j++)
          file_in >> data[j][i] ; 
      
      if (dimension==2)
      {
          dataextra[0][i]=get_default_density() * M_PI*data[4][i]*data[4][i] ; 
          dataextra[1][i]=1./2. * dataextra[0][i] * data[4][i] * data[4][i] ;
      }
      else
      {
          dataextra[0][i]=get_default_density() * 4./3.*M_PI*data[6][i]*data[6][i]*data[6][i] ; 
          dataextra[1][i]=2./5. * dataextra[0][i] * data[6][i] * data[6][i] ;
      }
  }
  
  curts=ts ; 
  if (mapped_ts.size()<static_cast<size_t>(curts+2)) mapped_ts.resize(curts+2) ; 
  mapped_ts[curts+1]=file_in.tellg() ;
  
  return 0 ; 
}

//-------
int MercuryReader_contacts::read_timestep (int ts) 
{
  std::string line ;
      
  if (ts==curts) return 0;
  
  if (mapped_ts.size()<=static_cast<size_t>(ts))
  {
      int header=3 ; 
      while (ts>curts+1)
      {
          getline(file_in, line) ;
          if (line[0]=='#')
          {
            header++ ; 
            if (header>=3)
            {
              mapped_ts.push_back(file_in.tellg()-static_cast<std::streamoff>(line.size()+1)) ;  
              header=1 ; 
            }
          }
      }
  }
  
  file_in.seekg(mapped_ts[ts]) ; 
  
  //---- Should finally be at the right location ...
  getline(file_in, line) ; if (line[0]!='#') printf("ERR: expected a hash, got '%s'.\n", line.c_str()) ;  
  getline(file_in, line) ; if (line[0]!='#') printf("ERR: expected a hash, got '%s'.\n", line.c_str()) ; 
  getline(file_in, line) ; if (line[0]!='#') printf("ERR: expected a hash, got '%s'.\n", line.c_str()) ; 
  
  Nc=0 ; 
  double fn, ft, tmp ; 
  assert((dimension==3)) ; 
  while (file_in.good() && file_in.peek()!='#')
  {
      Nc++ ;
      if (data[0].size()<=Nc)
          for (int i=0 ; i<11 ; i++)
              data.resize(Nc+growth) ;
          
      file_in >> tmp ; // Time, discard
      file_in >> data[0][Nc-1] ; // i 
      file_in >> data[1][Nc-1] ; // j
      file_in >> data[2][Nc-1] ; //posx
      file_in >> data[3][Nc-1] ; //posy
      file_in >> data[4][Nc-1] ; //posz
      file_in >> tmp ; // deltan, discard
      file_in >> tmp ; // deltat, discard
      data[5][Nc-1]=data[6][Nc-1]=data[7][Nc-1]=0 ; 
      file_in >> fn ; 
      file_in >> ft ; 
      file_in >> tmp ; data[5][Nc-1]+= fn*tmp ; 
      file_in >> tmp ; data[6][Nc-1]+= fn*tmp ; 
      file_in >> tmp ; data[7][Nc-1]+= fn*tmp ; 
      file_in >> tmp ; data[5][Nc-1]+= ft*tmp ; 
      file_in >> tmp ; data[6][Nc-1]+= ft*tmp ; 
      file_in >> tmp ; data[7][Nc-1]+= ft*tmp ; 
      
      if (data[0][Nc-1]>data[1][Nc-1])
      {
          std::swap(data[0][Nc-1], data[1][Nc-1]) ; 
          data[5][Nc-1] = -data[5][Nc-1] ;  
          data[6][Nc-1] = -data[6][Nc-1] ;  
          data[7][Nc-1] = -data[7][Nc-1] ; 
      }
      getline(file_in, line) ; 
  }
  
  build_pospqlpq_from_ids (data, 0, 1, 2, 8, dump->data, 0, 6) ;
  
  curts=ts ; 
  if (mapped_ts.size()<static_cast<size_t>(curts+2)) mapped_ts.resize(curts+2) ; 
  mapped_ts[curts+1]=file_in.tellg() ;
  
  return 0 ; 
}
















