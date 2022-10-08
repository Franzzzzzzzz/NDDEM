#include "Xml.h"


void XMLWriter::openbranch (string name, vector < pair<string,string>> attributes)
{
  hierarchy.push(name) ;
  for (uint i=0 ; i<hierarchy.size() ; i++) fic << " " ;
  fic << "<" << name ;
  for (auto i:attributes)
  {
    fic << " " <<i.first << '=' << i.second  ;
  }
  fic << ">\n" ;
}
bool XMLWriter::closebranch ()
{
  if (hierarchy.size()==0) return false ;
  auto name=hierarchy.top() ; hierarchy.pop() ;
  for (uint i=0 ; i<hierarchy.size()+1 ; i++) fic << " " ;
  fic << "</" << name << ">\n" ;
  return true ;
}
template <class T> void XMLWriter::smallbranch (string name, T value)
{
  for (uint i=0 ; i<hierarchy.size() ; i++) fic << " " ;
  fic << " <" << name << '>' << value << "</" << name << ">\n" ;
}
template <class T> void XMLWriter::smallbranch (string name, vector < pair<string,string>> attributes, T value)
{
  for (uint i=0 ; i<hierarchy.size() ; i++) fic << " " ;
  fic << " <" << name ;
  for (auto i:attributes) fic << " " <<i.first << '=' << i.second  ;
  fic << '>' << value << "</" << name << ">\n" ;
}


//---------------------------------------------------------------------------
void XMLWriter::header (int dimension, string input)
{
 std::time_t result = std::time(nullptr); string temp ;
 char date[100] ; strcpy(date, std::asctime(std::localtime(&result))) ;
 date[strlen(date)-1]=0 ; temp=date ;
 fic << "<?xml version='1.0' encoding='UTF-8'?>\n" ;
 openbranch("demnd", {make_pair("date", quote(temp)), make_pair("dimensions", to_string(dimension)), make_pair("input", quote(input))}) ;
}
//----------------------------------------------------------------------------
void XMLWriter::writeTs (double ts, tuple<string,vector<vector<double>>*, ArrayType> a)
{
 index.push_back(make_pair(float(ts), fic.tellp())) ;
 openbranch("timestep", {make_pair("ts", to_string(ts))}) ;
 writeArray(get<0>(a), get<1>(a), get<2>(a) ) ;
 closebranch() ;
}
void XMLWriter::startTS (double ts)
{
 index.push_back(make_pair(float(ts), fic.tellp())) ;
 openbranch("timestep", {make_pair("ts", to_string(ts))}) ;
}
void XMLWriter::stopTS ()
{
  closebranch() ;
}
//----------------------------------------------------------------------------
void XMLWriter::writeArray(string name, vector<vector<double>>*x, int beg, int length, ArrayType t, EncodingType te)
{
 if (t==ArrayType::particles) openbranch("particles") ; //fic << "  <particles>\n" ;
 else if (t==ArrayType::contacts) openbranch("contacts") ; //fic << "  <contacts>\n" ;
 else openbranch("unknown") ; //fic << "  <unknown>\n" ;
 smallbranch("name", name) ; //fic << "   <name>" << name << "</name>\n" ;
 if (te==EncodingType::ascii)      {smallbranch("encoding","ascii");}
 else if (te==EncodingType::base64){smallbranch("encoding","base64");}

 smallbranch("nrows", x->size()) ;
 smallbranch("ncols", length) ;
 openbranch("data", {make_pair("length", to_string(x->size()*length))}) ;

 int n=0 ;
 if (te==EncodingType::ascii)
 {
   for (auto v:*x)
     for (int i=beg ; i<beg+length ; i++)
     {
         fic << v[i] << " " ;
         n++ ;
         if (n%25==0) fic << endl << "    " ;
     }
 }
 else if (te==EncodingType::base64)
 {
   printf("WARN: Encode base 64 not available for 2D array with subarray in XML writing ...\n") ; 
 }
 closebranch() ; closebranch() ;
}
//----------------------------------------------------------------------------
void XMLWriter::writeArray(string name, vector<vector<double>>*x, ArrayType t, EncodingType te)
{
 if (t==ArrayType::particles) openbranch("particles") ; //fic << "  <particles>\n" ;
 else if (t==ArrayType::contacts) openbranch("contacts") ; //fic << "  <contacts>\n" ;
 else openbranch("unknown") ; //fic << "  <unknown>\n" ;
 smallbranch("name", name) ; //fic << "   <name>" << name << "</name>\n" ;
 if (te==EncodingType::ascii)      {smallbranch("encoding","ascii");}
 else if (te==EncodingType::base64){smallbranch("encoding","base64");}

 smallbranch("nrows", x->size()) ;
 smallbranch("ncols", (*x)[0].size()) ;
 openbranch("data", {make_pair("length", to_string(x->size()*(*x)[0].size()))}) ;

 int n=0 ;
 if (te==EncodingType::ascii)
 {
   for (auto v:*x)
     for (auto w:v)
     {
         fic << w << " " ;
         n++ ;
         if (n%25==0) fic << endl << "    " ;
     }
 }
 else if (te==EncodingType::base64)
 {
   for (auto v:*x)
     encodebase64f(fic, v) ;
   encodebase64f_end(fic);
 }
 closebranch() ; closebranch() ;
}
//----------------------------------------------------------------------------
void XMLWriter::writeArray(string name, vector<double>*x, ArrayType t, EncodingType te)
{
 if (t==ArrayType::particles) openbranch("particles") ; //fic << "  <particles>\n" ;
 else if (t==ArrayType::contacts) openbranch("contacts") ; //fic << "  <contacts>\n" ;
 else openbranch("unknown") ; //fic << "  <unknown>\n" ;
 smallbranch("name", name) ; //fic << "   <name>" << name << "</name>\n" ;
 if (te==EncodingType::ascii)      {smallbranch("encoding","ascii");}
 else if (te==EncodingType::base64){smallbranch("encoding","base64");}

 smallbranch("nrows", x->size()) ;
 smallbranch("ncols", 1) ;
 openbranch("data", {make_pair("length", to_string(x->size()))}) ;

 int n=0 ;
 if (te==EncodingType::ascii)
 {
   for (auto v:*x)
   {
    fic << v << " " ;
    n++ ;
    if (n%25==0) fic << endl << "    " ;
   }
 }
 else if (te==EncodingType::base64)
 {
    printf("WARN: Encode base 64 not available for 1D array in XML writing ...\n") ; 
   /*for (auto v:*x)
     encodebase64f(fic, v) ;
   encodebase64f_end(fic);*/
 }
 closebranch() ; closebranch() ;
}
//---------------------------------------------------------------
void XMLWriter::close ()
{
 openbranch("index") ;
 for (auto v:index)
 {
   smallbranch("entry",{make_pair("ts", to_string(v.first))}, v.second) ;
 }

 std::time_t result = std::time(nullptr);
 string temp ; char date[100] ;
 strcpy(date, std::asctime(std::localtime(&result))) ;
 date[strlen(date)-1]=0 ; temp=date ;
 while (closebranch()) ;
 smallbranch("note", "Finish at "+temp) ;

 fic.close() ;
}
//---------------------------------------------------------------
void XMLWriter::emergencyclose()
{
std::time_t result = std::time(nullptr);
string temp ; char date[100] ;
strcpy(date, std::asctime(std::localtime(&result))) ;
date[strlen(date)-1]=0 ; temp=date ;
while (closebranch()) ;
smallbranch("note", "Emergency finish at "+temp) ;
fic.close() ;
}


//=========================================================================================
XMLReader_base::XMLReader_base (string path)
{
 string line ;
 fic.open(path) ;
 if (!fic.is_open()) {printf("ERR: the file %s couldn't be opened.\n", path.c_str()) ; return ; }
 getline(fic, line) ;
 if (line != "<?xml version='1.0' encoding='UTF-8'?>") printf("ERR:Unexpected header line\n") ;
}
//----------------------------------------------------------
pair <string,map<string,string>> XMLReader_base::gettag()
{
    char l ;
    string line, word ;
    getline(fic, line, '>') ;
    stringstream ss(line) ;
    //printf("\n[%s]", line.c_str()) ;
    ss >> l ;
    while (isspace(l)) ss>>l ;
    if (l!='<') printf("ERR: gettag should start with a < character %c\n", l) ;
    ss >> l ;
    if (l=='/')
    {
     ss >> word ;
     if (word!=tree.top()) printf("ERR: malformed XML, removing tag is not the last inserted tag\n") ;
     else tree.pop() ;
     word="/"+word ;
     return (make_pair(word, map<string,string>())) ;
    }
    else
    {
        map<string,string> tmp ;
        ss.putback(l) ;
        ss>>word ;
        tree.push(word) ;

        while (!ss.eof())
        {
            getline(ss,word, '=') ;
            word.erase(remove_if(word.begin(), word.end(), [](char v){return (v==' ');}), word.end());
            if (ss.eof()) break ;
            ss>>quoted(line, '\'') ;
            //printf("{%s|%s}", word.c_str(), line.c_str()) ;
            tmp[word]=line ;
        }
        return (make_pair(tree.top(), tmp)) ;
    }
}
//----------------------------------------------------------
tuple <string, map<string,string> , std::vector<double> > XMLReader_base::gettagdata()
{
    auto tag=gettag() ;
    vector <double> values ;
    int length= stoi(tag.second["length"]) ;
    values.resize(length, 0) ;
    for (int i=0 ; i<length ; i++) fic >> values[i] ;
    gettag() ;
    return (make_tuple(tag.first, tag.second, values)) ;
}
//------------------------------------------------
string XMLReader_base::getcontent()
{
    string res ;
    getline(fic, res, '<') ;
    fic.unget() ;
    return res ;
}
//============================================================
int XMLReader::read_boundaries (vector <vector <double>> & boundaries)
{
    auto results = gettagdata() ;
    if (get<0>(results) != "boundaries") {printf("Wrong tag order, returning") ; return 1; }
    boundaries.resize(2) ; 
    boundaries[0].resize(get<2>(results).size()/2) ; 
    boundaries[1].resize(get<2>(results).size()/2) ; 
    for (unsigned int i=0 ; i<get<2>(results).size() ; i+=2)
    {
        boundaries[0][i/2] = get<2>(results)[i] ;
        boundaries[1][i/2] = get<2>(results)[i+1] ; 
    }
    return 0 ; 
}
//-------------------------------------------------------------
int XMLReader::read_radius (vector <double > & radius)
{
    auto results = gettagdata() ;
    if (get<0>(results) != "radius") {printf("Wrong tag order, returning") ; return 1; }
    radius = get<2>(results) ; 
    return 0 ; 
}
//-------------------------------------------------------------
double XMLReader::read_nextts(vector<string> &names, vector<vector<vector<double>>> & data)
{
 ArrayType type [[maybe_unused]] ;
 map <string, string> prop ;
 int n=0 ;
 double time ; 
 auto a=gettag() ;
 if (a.first != "timestep") {printf("ERR: not the right XML element (%s instead of timestep)\n", a.first.c_str()) ; return -1 ; }
 //printf("%s ", a.second["ts"].c_str()) ; 
 time = atof (a.second["ts"].c_str()) ; 
 while (a.first !="/timestep")
 {
    a=gettag() ;
    if (a.first=="/timestep") break ;
    if (a.first=="particles") type=ArrayType::particles ;
    else if (a.first=="contacts") type=ArrayType::contacts ;
    else printf("ERR: unknown array type\n") ;
    while (a.first!= "data")
    {
        a=gettag() ;
        if (a.first=="data") break ;
        auto b=getcontent() ;
        prop[a.first]=b ;
        a=gettag() ; // closing tag hopefully
    }
    int col=stoi(prop["ncols"]), row=stoi(prop["nrows"]) ;
    names.push_back(prop["name"]) ;
    data.push_back(vector<vector<double>>(row, vector<double>(col,0))) ;
    if (prop["encoding"]=="ascii")
    {
        for (int i=0 ; i<row ; i++)
            for (int j=0 ; j<col ; j++)
                fic>>data[n][i][j] ;
    }
    else if (prop["encoding"]=="base64")
    {
        decodebase64f_2dd(fic,data[n]) ;
    }
    else
        printf("ERR: unknown encoding for data") ;
    a=gettag() ; if (a.first != "/data") printf("ERR:unexpected tag %s instead of /data\n",a.first.c_str()) ;
    a=gettag() ; if (a.first != "/particles" && a.first != "/contacts") printf("ERR:unexpected tag %s instead of /particles or /contacts\n",a.first.c_str()) ;
    n++ ;
 }
 return time ;
}
//-------------------------------------------------------------
std::vector<std::pair<double,std::streampos>> XMLReader::read_index ()
{
    std::streampos originalposition = fic.tellg() ; 
    std::vector<std::pair<double,std::streampos>> res ; 
    int value ; 
    char text[500] ; char * textloc ; 
    pair <string,map<string,string>> a ; 
    fic.seekg(-1, ios_base::end) ; 
    do {
        fic.seekg(-1000, ios_base::cur) ; 
        fic.ignore(1000, '<') ; 
        textloc = text ;  do { fic.get(*textloc) ; textloc++ ; } while (*(textloc-1) != ' ' && *(textloc-1) != '>') ; *(textloc-1)=0 ; 
    } while (!strcmp(text,"entry") || !strcmp(text,"/entry")) ;
    
    while (strcmp(text,"index")) 
    {
        fic.ignore(1000, '<') ; 
        textloc = text ;  do { fic.get(*textloc) ; textloc++ ; } while (*(textloc-1) != ' ' && *(textloc-1) != '>') ; *(textloc-1)=0 ;
    }
    fic.seekg(-8, ios_base::cur) ; 
    a=gettag() ;
    while (a.first!="/index") 
    {
        a=gettag() ;
        if (a.first == "/index") break ; 
        if (a.first != "entry") printf("ERR: expecting an 'entry' tag.\n") ; 
        fic >> value ; 
        res.push_back(std::make_pair(atof(a.second["ts"].c_str()), static_cast<streampos>(value))) ;
        a=gettag() ;
        if (a.first != "/entry") printf("ERR: expecting a '/entry' tag.\n") ; 
    }
    fic.seekg(originalposition) ; 
    return res ; 
    
}

//-----------------------------------------------
int XMLWriter::encodebase64f (ostream &out, vector<double>& val)
{
    static char lst[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" ;
    static_assert(sizeof(float)==4, "Bad: float are not 4 bytes") ;
    static_assert(sizeof(unsigned int)==4, "Bad: uint are not 4 bytes") ;
    float vf ; unsigned int v ;
    static int n=0 ;
    for (auto & vv:val)
    {
        vf=float(vv) ;
        memcpy(&v, &vf, 4) ;
        switch(state) {
            case 0: out << lst[v>>(32-6) & 63] ; out << lst[v>>(32-12) & 63] ; out << lst[v>>(32-18) & 63] ;  out << lst[v>>(32-24) & 63] ; out << lst[v>>(32-30) & 63] ;
                    remainer=(v&3)<<4 ; state++ ; n+=5 ;
                    break ;
            case 1: out << lst[(v>>(32-4) & 15)|remainer] ; out << lst[v>>(32-10) & 63] ;  out << lst[v>>(32-16) & 63] ; out << lst[v>>(32-22) & 63] ;  out << lst[v>>(32-28) & 63] ;
                    remainer=(v&15) << 2 ; state++ ; n+=5 ;
                    break ;
            case 2: out << lst[(v>>(32-2) & 3)|remainer] ; out << lst[v>>(32-8) & 63] ; out << lst[v>>(32-14) & 63] ; out << lst[v>>(32-20) & 63] ; out << lst[v>>(32-26) & 63] ;
                    out << lst[v>>(32-32) & 63] ;
                    remainer=0 ; state=0 ; n+=6 ;
                    break ;
            default: printf("ERR: unknown state un encodebase64f\n") ;

        }
        if (n>150) {out<<endl ; n=0;} // To avoid long lines, change the constant to adapt the line length
    }
return 0 ;
}
//------------------------------------------------------------------
int XMLReader::decodebase64f (istream &in, vector<float>& val)
{
    char lstb[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" ;
    vector <unsigned int> lst (255,1234) ;
    for (int i=0 ; i<64 ; i++) lst[lstb[i]]=i ;
    lst[' ']=lst['\n']=3333 ;

    static_assert(sizeof(float)==4, "Bad: float are not 4 bytes") ;
    static_assert(sizeof(unsigned int)==4, "Bad: uint are not 4 bytes") ;
    unsigned int v=0 ;
    char l ;
    char needed=0 ;
    float fl=0 ;

    needed=32 ;
    while (!in.eof())
    {
        in.get(l) ;
        if (lst[l]==1234) break ;
        if (lst[l]==3333) continue ;
        if (needed>6) v |= (lst[l] << (needed-6)) ;
        else v |= (lst[l] >> (6-needed)) ;
        needed-=6 ;
        if (needed<=0)
        {
            memcpy (&fl, &v,4) ;
            val.push_back(fl) ;
            v=0 ;
            if (needed==-4) v |= (lst[l] &15) <<28 ;
            else if (needed==-2) v|= (lst[l] & 3) << 30 ;
            needed=32+needed ;
        }
    }
return 0 ;
}
int XMLReader::decodebase64f_2dd (istream &in, vector<vector<double>>& val)
{
    int col, row ;
    if (val.size() == 0 || val[0].size()==0) {row=col=0 ; printf("ERR: the 2D array of double should be initialised first\n") ;}
    else {row=val.size() ; col=val[0].size(); }
    char lstb[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" ;
    vector <unsigned int> lst (255,1234) ;
    for (int i=0 ; i<64 ; i++) lst[lstb[i]]=i ;
    lst[' ']=lst['\n']=3333 ;

    static_assert(sizeof(float)==4, "Bad: float are not 4 bytes") ;
    static_assert(sizeof(unsigned int)==4, "Bad: uint are not 4 bytes") ;
    unsigned int v=0 ;
    char l ;
    char needed=0 ;
    float fl=0 ;

    bool stop=false ;
    int nrow=0, ncol=0 ;
    needed=32 ;
    while (!in.eof() && stop==false )
    {
        in.get(l) ;
        if (lst[l]==1234) break ;
        if (lst[l]==3333) continue ;
        if (needed>6) v |= (lst[l] << (needed-6)) ;
        else v |= (lst[l] >> (6-needed)) ;
        needed-=6 ;
        if (needed<=0)
        {
            memcpy (&fl, &v,4) ;
            val[nrow][ncol]=fl ;
            ncol++ ;
            if (ncol==col) {nrow++ ; ncol=0 ; }
            if (nrow==row) stop=true ;
            v=0 ;
            if (needed==-4) v |= (lst[l] &15) <<28 ;
            else if (needed==-2) v|= (lst[l] & 3) << 30 ;
            needed=32+needed ;
        }
    }
return 0 ;
}

//=============== TEST ==================
/*int main ()
{
    vector<vector<double>> data = {{1,0.0215,0.0000000000365445548,1.52536902e24},{52.4e12,12.3,25,36.2},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{52.4e12,12.3,25,36.2},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{52.4e12,12.3,25,36.2},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{52.4e12,12.3,25,36.2},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24},{1,0.0215,0.0000000000365445548,-1.52536902e24}} ;

    XMLWriter X("Test.xml") ;
    X.header(4, "in.txt") ;
    X.writeTs(10.2, make_tuple("Position", &data, ArrayType::particles));
    X.startTS(10.3);
    X.writeArray("Velocity", &data, ArrayType::particles, EncodingType::base64);
    X.writeArray("Position", &data, ArrayType::particles, EncodingType::base64);
    X.writeArray("Ctx", &data, ArrayType::contacts, EncodingType::base64);
    X.stopTS();
    X.writeTs(10.40000001, make_tuple("Position", &data, ArrayType::particles));
    X.writeTs(10.6, make_tuple("Position", &data, ArrayType::particles));
    X.writeTs(10.8, make_tuple("Position", &data, ArrayType::particles));
    X.writeTs(11, make_tuple("Position", &data, ArrayType::particles));
    X.writeTs(156, make_tuple("Position", &data, ArrayType::particles));
    X.close() ;

    XMLReader Y("Test.xml") ;
    vector<vector<vector<double>>> v ;
    vector<string> names ;
    Y.read_nextts(names, v) ;
    printf("%ld %ld %ld\n", v.size(), v[0].size(), v[0][0].size() ) ;
    v.clear() ;
    Y.read_nextts(names, v) ;
    printf("%ld %ld %ld\n", v.size(), v[0].size(), v[0][0].size() ) ;
    printf("%g ", v[2][10][3]) ;


}*/
