#include "Xml.h"



//---------------------------------------------------------------------------
void XMLWriter::header (int dimension, string input)
{
 std::time_t result = std::time(nullptr);
 char date[100] ; strcpy(date, std::asctime(std::localtime(&result))) ; 
 date[strlen(date)-1]=0 ;
 fic << "<?xml version='1.0' encoding='UTF-8'?>\n" ;    
 fic << "<demnd date='"<< date << "' dimensions='" << dimension ;
 if (input!="") fic << "' input='"<< input ;
 fic <<  "'>\n" ;
}
//----------------------------------------------------------------------------
void XMLWriter::writeTs (double ts, tuple<string,vector<vector<double>>*, ArrayType> a)
{
 index.push_back(make_pair(float(ts), fic.tellp())) ; 
 fic << " <timestep ts='" << float(ts) <<"'>\n" ; 
 writeArray(get<0>(a), get<1>(a), get<2>(a) ) ; 
 fic << " </timestep>\n" ;   
}
void XMLWriter::startTS (double ts)
{
 index.push_back(make_pair(float(ts), fic.tellp())) ; 
 fic << " <timestep ts='" << float(ts) <<"'>\n" ; 
}
void XMLWriter::stopTS ()
{
 fic << " </timestep>\n" ; 
}
//----------------------------------------------------------------------------
void XMLWriter::writeArray(string name, vector<vector<double>>*x, ArrayType t, EncodingType te)
{
 if (t==ArrayType::particles) fic << "  <particles>\n" ; 
 else if (t==ArrayType::contacts) fic << "  <contacts>\n" ; 
 else fic << "  <unknown>\n" ; 
 fic << "   <name>" << name << "</name>\n" ;
 if (te==EncodingType::ascii) fic << "   <encoding>ascii</encoding>\n" ;
 else if (te==EncodingType::base64) fic << "   <encoding>base64</encoding>\n" ;
 fic << "   <nrows>" << x->size() << "</nrows>\n" ; 
 fic << "   <ncols>" << (*x)[0].size() << "</ncols>\n" ; 
 fic << "   <data length='" << x->size()*(*x)[0].size() << "'>" ;//<< setprecision(6); 
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
 fic << "\n   </data>\n" ;
 if (t==ArrayType::particles) fic << "  </particles>\n" ; 
 else if (t==ArrayType::contacts) fic << "  </contacts>\n" ; 
 else fic << "  </unknown>\n" ; 
}
//---------------------------------------------------------------
void XMLWriter::close () 
{
 fic << " <index>\n" ;
 for (auto v:index) 
     fic << "  <entry ts='" << v.first << "'>" << v.second << "</entry>\n" ; 
 fic << " </index>\n" ; 
 fic << "</demnd>\n" ; 
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
    if (l!='<') printf("ERR: gettag should start with a < character\n") ; 
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
//------------------------------------------------
string XMLReader_base::getcontent()
{
    string res ; 
    getline(fic, res, '<') ; 
    fic.unget() ; 
    return res ; 
}
//============================================================
int XMLReader::read_nextts(vector<string> &names, vector<vector<vector<double>>> & data)
{
 ArrayType type ; 
 map <string, string> prop ; 
 int n=0 ; 
 auto a=gettag() ; 
 if (a.first != "timestep") printf("ERR: not the right XML element (%s instead of timestep)\n", a.first.c_str()) ; 
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
}
int XMLReader::decodebase64f_2dd (istream &in, vector<vector<double>>& val)
{
    int col, row ; 
    if (val.size() == 0 || val[0].size()==0) printf("ERR: the 2D array of double should be initialised first\n") ;
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