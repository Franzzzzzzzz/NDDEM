#include "Reader-Liggghts.h"

//-----------------------------------------------------------
vector<vector<double>> LiggghtsReader::get_bounds()
{
 vector<vector<double>> res(2, vector<double>(3,0)) ;
 string line ;

 do 
 {
   getline(*in, line) ;
 }
 while ( in->good() && line.find("BOX BOUNDS")==string::npos) ; 
 
 if (!in->good()) return {} ; 
 
 *in >> res[0][0] >> res[1][0] ;
 *in >> res[0][1] >> res[1][1] ;
 *in >> res[0][2] >> res[1][2] ;
 reset() ; 
 return (res) ;
}
//-----------------------------------------------------------
vector<double> LiggghtsReader::get_minmaxradius()
{
  read_timestep(0) ; 
  double * r = get_data(DataValue::radius, 0) ; 
  int n = get_num_particles() ; 
  double minr=r[0], maxr=r[0] ; 
  for (int i=0 ; i<n ; i++)
  {
    if (r[i]>maxr) maxr=r[i] ; 
    if (r[i]<minr) minr=r[i] ;
  }
  return {minr, maxr} ; 
}
//------------------------------------------------------------
int LiggghtsReader::get_numts()
{
 int numts = 0 ;
 string line ;
 reset() ; 
 do
 {
   getline(*in, line) ;
   if (line[0]=='I' && line == "ITEM: TIMESTEP") 
   { 
       numts++ ;
       if (is_seekable)
       {
           mapped_ts.resize(numts) ; 
           mapped_ts[numts-1] = in->tellg()-static_cast<std::streamoff>(14) ; 
       }
   }
 } while (in->good()) ;
 if (is_seekable) is_fullymapped = true ;
 reset() ; 
 return numts ;
}

//=============================================================
int LiggghtsReader::open(string path)
{
file_in= new ifstream(path, ios_base::in | ios_base::binary);
if (!file_in->is_open()) {printf("ERR: cannot open file %s\n", path.c_str()) ; return 1;}
if (path.find(".gz")!=string::npos)
{
    is_seekable=false ; 
    filt_in = new boost::iostreams::filtering_streambuf<boost::iostreams::input> ;
    filt_in->push(boost::iostreams::gzip_decompressor()) ;
    filt_in->push(*file_in);
    in=new istream (filt_in) ;
}
else
{
    is_seekable = true ; 
    filt_in_seekable = new boost::iostreams::filtering_streambuf<boost::iostreams::input_seekable> ;
    filt_in_seekable->push(*file_in);
    in=new istream (filt_in_seekable) ;
}
return 0 ;
}
//------------------------------------------------------------
int LiggghtsReader::reset()
{
if (! is_seekable)
{
    if (in==nullptr || filt_in == nullptr || file_in == nullptr)
    {
        printf("ERR: reset function is supposed to be called with initialised files.\n") ; 
        return 1 ;
    }
    delete(in) ; 
    delete(filt_in) ; 
    file_in->close() ; 
    delete(file_in) ;
    open(path) ; 
}
else
{
    in->clear() ; 
    in->seekg(0) ;
}
return 0 ;
}
//-------------------------------------------------------------
int LiggghtsReader::read_timestep(int ts)
{
  if (ts<mapped_ts.size())
  {
    if (mapped_ts[ts].has_value())
    {
      in->seekg(mapped_ts[ts].value()) ;
      curts = ts-1 ;
      read_timestep_impl(ts) ;
      return 0 ; // All good, seek the ts and read it
    }
  }

  // any other case, manual searching (
  if (ts<=curts)
  {
      printf("WARN: file is not seekable, resetting it") ;
      reset() ;
  }
  else
  {
      while (ts-1>curts)
          read_timestep_impl(ts, true) ;
      read_timestep_impl(ts,false) ;
  }

//  if (is_seekable)
//  {
//      if (ts > static_cast<signed int>(mapped_ts.size()))
//      {
//          printf("ERR: the requested timestep %d is above max timestep %ld.", ts, mapped_ts.size()) ;
//          return -1 ;
//      }
//      in->seekg(mapped_ts[ts]) ;
//      curts = ts-1 ;
//      read_timestep_impl(ts) ;
//  }
//  else
//  {
//     printf("// %d %d\n", ts, curts) ; fflush(stdout) ;
//      if (ts<=curts)
//      {
//          printf("WARN: file is not seekable, resetting it") ;
//          reset() ;
//      }
//      else
//      {
//          while (ts-1>curts)
//              read_timestep_impl(ts, true) ;
//          read_timestep_impl(ts,false) ;
//      }
//  }
 return 0 ;
}
//---------------------------------------------------------------------------------
int LiggghtsReader::read_timestep_impl(int ts, bool skip)
{
 string line, word ; char blank ; int nid ;

 if (is_seekable)
 {
  if (mapped_ts.size()<=curts+1) mapped_ts.resize(curts+2) ;
  mapped_ts[curts+1]=in->tellg() ;
 }

 getline(*in, line) ; //should be ITEM: TIMESTEP
 *in >> actualts >> blank ; curts++ ; 
 printf("\rReading: %d skipping:%d", actualts, skip) ; fflush(stdout) ;
 getline(*in, line) ; //should be NUMBER OF SOMETHING
 *in >> Nitem >> blank;
 
 getline(*in, line) ; //should be BOX
 {stringstream s (line) ; s>>word ; s>>word ; s>>word ; // ITEM: BOX BOUNDS discarded
 for (int i=0 ; i<3 ; i++)
 {
     s>>word ; 
     if (word == "pp") periodicity[i] = true ; 
     else periodicity[i]=false ; 
     *in >> boundaries[0][i] ; *in >> boundaries[1][i] ;
     boundaries[2][i]=boundaries[1][i]-boundaries[0][i] ; 
 }
 getline(*in, line) ;    
 }

 getline(*in, line) ;
 stringstream s (line) ;
 s>>word ; s>>word ; // Discard these
 if (curts != ts || skip)
   in->ignore(numeric_limits<streamsize>::max(), 'I') ; //If we don't care about the data, read until the next timestep (starts with an 'I'tem timestep)
 else
 {
  fields.resize(0);
  while (!s.eof())
  {
    s>>word ;
    fields.push_back(word) ;
  }
  fields.pop_back() ; nid=fields.size() ; // Last field is wrong, it was the newline ...

  tdata.resize(Nitem, v1d (fields.size(),0)) ;

  for (int i=0 ; i<Nitem; i++)
    for (int j=0 ; j<nid ; j++)
      *in>>tdata[i][j] ;
  *in>>blank ; // Get rid of the last newline

  do_post_read() ; 
  //if (iscf) do_post_cf() ; // TODO
  //else do_post_atm() ;
 }

return 0 ;
}
//======================================================================================
int LiggghtsReader_particles::do_post_read()
{
 int i, k ; v1d nanvec (fields.size(),NAN) ; int nadded=0 ;
 auto j= tdata.begin() ;
 
 // sort by id and fill missing id with nan (needed if contact particles are calculated)
 int idloc = std::find(fields.begin(), fields.end(), "id") - fields.begin();
 if (idloc == static_cast<signed int>(fields.size())) has_id_data=false ; 
 else
 {
    has_id_data=true ; 
    sort(tdata.begin(), tdata.end(), [=](auto v1, auto v2) {return v1[idloc] < v2[idloc] ; }); //WARNING idx 0 should be the particle ID
    
    for (i=0, j=tdata.begin() ; i<Nitem ; i++, j++)
    {
        if (i+1<(*j)[idloc])
        {
            j=tdata.insert(j, nanvec) ;
            (*j)[idloc]=i+1 ;
            nadded++ ;
        }
    }

    for (i=0,j=tdata.begin() ; j<tdata.end() ; j++,i++)
    {
        (*j)[idloc]-- ;
        if ( (*j)[idloc] != i)
            printf("ERR shouldn't happen %d %g\n", i, (*j)[idloc]) ;
    }
    printf(" %d null atom / %d | ", nadded, Nitem) ;
    Nitem+=nadded ;
 }

 const int nvalue = 13 ;
 data.resize(nvalue, v1d (0,0)) ; //Order: radius mass Imom posxyz velxyz omegaxyz
 vector<string>::iterator it ;
 vector<int> lst ;
 vector<string> flst = {"radius", "mass", "I","x","y","z","vx","vy","vz","omegax","omegay","omegaz","type"} ;
 for (size_t i=0 ; i<flst.size() ; i++)
 {
    it=std::find(fields.begin(), fields.end(), flst[i] ) ;
    if ( it != fields.end()) lst.push_back(it-fields.begin()) ;
    else lst.push_back(-1) ;
 }

 for (i=0 ; i<nvalue ; i++)
 {
     if (lst[i]>=0)
     {
         data[i].resize(Nitem,0) ;
         for (k=0 ; k<Nitem ; k++)
         {
           if (periodicity.size()>0 && (i==3 || i==4 || i==5) && (periodicity[i-3])) // Handle atom outside the simulation due to pbc. Important also for handling contact forces through PBC in the next function ...
           {
             if (tdata[k][lst[i]]<boundaries[0][i-3])
               data[i][k]=tdata[k][lst[i]]+boundaries[2][i] ;
             else if (tdata[k][lst[i]]>boundaries[1][i-3])
               data[i][k]=tdata[k][lst[i]]-boundaries[2][i] ;
             else
               data[i][k]=tdata[k][lst[i]] ;
           }
           else if (i==12)
           {
             data[i][k]=tdata[k][lst[i]]-1 ;
           }
           else
             data[i][k]=tdata[k][lst[i]] ;
         }
     }
     else
     {
         data[i].resize(Nitem,0) ;
         if (i==0) for (k=0 ; k<Nitem ; k++) data[i][k]=get_default_radius() ; //TODO
         if (i==1) for (k=0 ; k<Nitem ; k++) data[i][k]=4/3. * M_PI * data[0][k] * data[0][k] * data[0][k] * get_default_density() ;
         if (i==2) for (k=0 ; k<Nitem ; k++) data[i][k]=2/5. * data[1][k] * data[0][k] * data[0][k] ;
     }
 }

 return nadded ;
}

//-----------------------------------------------------
int LiggghtsReader_contacts::do_post_read()
{
 const int nvalue=17 ; int swap ;

 data.resize(nvalue, vector <double> (0)) ;
 for (int i=0 ; i<nvalue ; i++) data[i].resize(Nitem,0) ;

 if (dump->has_id_data == false)
 {
     printf("ERR: the id datas in the particle dump are required to process the contact forces") ; 
     return -1 ; 
 }
 vector<string>::iterator it ;
 vector<int> lst ;
 vector<string> tofind = {"id1", "id2", "per", "fx", "fy", "fz", "mx", "my", "mz"} ;

 for (auto name : tofind)
 {
   auto mapper=cfmapping.find(name) ;
   if (mapper != cfmapping.end())
   {
       it=std::find(fields.begin(), fields.end(), mapper->second) ;
       if ( it != fields.end())
           lst.push_back(it-fields.begin()) ;
       else
           lst.push_back(-1) ;
   }
   else
    lst.push_back(-1) ;
 }


 int k=0 ;
 for (int j=0 ; j<Nitem ; j++)
 {
     if (tdata[j][lst[2]]==1) continue ; // Remove any chainforce going through the PBC

     if (tdata[j][lst[0]]>tdata[j][lst[1]]) swap=1 ;
     else swap = 0 ;

     data[0][k]=tdata[j][lst[swap]]-1 ;                                       //id1
     data[1][k]=tdata[j][lst[1-swap]]-1 ;                                     //id2

     data[2][k]=  (dump->data[3][data[0][k]] + dump->data[3][data[1][k]]) /2.0 ;      //pos[0] //WARNING pb through PBC //WARNING WITH POLYDISPERSITY!!
     data[3][k]=  (dump->data[4][data[0][k]] + dump->data[4][data[1][k]]) /2.0 ;      //pos[1]
     data[4][k]=  (dump->data[5][data[0][k]] + dump->data[5][data[1][k]]) /2.0 ;      //pos[2]

     if (swap==1) swap=-1 ;
     else swap=1 ;
     data[5][k]=  (dump->data[3][data[0][k]] - dump->data[3][data[1][k]]) ;      //lpq[0] //WARNING pb through PBC
     data[6][k]=  (dump->data[4][data[0][k]] - dump->data[4][data[1][k]]) ;      //lpq[1]
     data[7][k]=  (dump->data[5][data[0][k]] - dump->data[5][data[1][k]]) ;      //lpq[2]

     // Let's handle the PBC now ...
     if (tdata[j][lst[2]]==1)
     {
       bool corrected=false ;
       for (size_t i=0 ; i<periodicity.size() ; i++)
       {
         if (periodicity[i])
         {
           if (fabs(fabs(dump->data[3+i][data[0][k]] - dump->data[3+i][data[1][k]]) - boundaries[2][i]) < fabs(data[5+i][k])) // Going through this PBC decreased the length of lpq
           {
             if (dump->data[3+i][data[0][k]] - dump->data[3+i][data[1][k]]<0) // it is either x1->x1+Delta or x2->x2-Delta
             {
               data[5+i][k] += boundaries[2][i]  ;
               if (data[2+i][k]+boundaries[2][i]/2. >= boundaries[0][i] && data[2+i][k]+boundaries[2][i]/2.<= boundaries[1][i])
                  data[2+i][k]+=boundaries[2][i]/2. ;
               else
                  data[2+i][k]-=boundaries[2][i]/2. ;
             }
             else // it is either x1->x1-Delta or x2->x2+Delta
             {
               data[5+i][k] -= boundaries[2][i]  ;
               if (data[2+i][k]+boundaries[2][i]/2. >= boundaries[0][i] && data[2+i][k]+boundaries[2][i]/2.<= boundaries[1][i])
                  data[2+i][k]+=boundaries[2][i]/2. ;
               else
                  data[2+i][k]-=boundaries[2][i]/2. ;

             }
           corrected=true ;
           }
         }
       }
       if (corrected==false)
            printf("- WARN: a contact force traversing the PBC was not corrected. - ") ;
     }

     if (lst[3]>=-1) data[8][k]=  swap*tdata[j][lst[3]] ;                                     //f[0]
     if (lst[4]>=-1) data[9][k]=  swap*tdata[j][lst[4]] ;                                     //f[1]
     if (lst[5]>=-1) data[10][k]= swap*tdata[j][lst[5]] ;                                     //f[2]

     //printf("%g\n", data[5][k]*data[8][k]+data[6][k]*data[9][k]+data[7][k]*data[10][k]) ;
     //std::exit() ;

     if (swap==-1) swap=1 ;
     else swap=0 ;
     if (lst[6]>-1) data[11][k]= tdata[j][lst[6]]*dump->data[0][data[swap][k]]  ;           //mpq
     if (lst[7]>-1) data[12][k]= tdata[j][lst[7]]*dump->data[0][data[swap][k]]  ;
     if (lst[8]>-1) data[13][k]= tdata[j][lst[8]]*dump->data[0][data[swap][k]]  ;
     if (lst[6]>-1) data[14][k]= tdata[j][lst[6]]*dump->data[0][data[1-swap][k]] ;          //mqp
     if (lst[7]>-1) data[15][k]= tdata[j][lst[7]]*dump->data[0][data[1-swap][k]]  ;
     if (lst[8]>-1) data[16][k]= tdata[j][lst[8]]*dump->data[0][data[1-swap][k]]  ;

     k++ ;
 }
 printf("-%d per contacts /%d | ", Nitem-k, Nitem) ;
 Nitem=k ;
return 0 ;
}
