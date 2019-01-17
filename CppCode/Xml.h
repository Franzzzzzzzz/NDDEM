#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <iostream>
#include <ctime>
#include <vector>
#include <tuple>
#include <stack>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <algorithm>
#include <stack>

#include "Typedefs.h"

using namespace std ;

enum class ArrayType {particles, contacts} ;
enum class EncodingType {ascii, base64} ;
class XMLWriter
{
public:
    XMLWriter (string path) {fic.open(path.c_str()) ; if (!fic) printf("ERR: cannot open %s in writing \n", path.c_str()) ; }
    void header (int dimension, string input) ;
    void writeTs (double ts, tuple<string,vector<vector<double>>*, ArrayType> a);
    void startTS (double ts) ;
    void writeArray(string name, std::vector< std::vector< double > >* x, ArrayType t, EncodingType te=EncodingType::ascii) ;
    void stopTS () ;
    void close() ;
    void emergencyclose() ;

private:
    ofstream fic ;
    vector <pair<double,streampos>> index ;
    int encodebase64f (ostream &out, vector<double>& val) ;
    void encodebase64f_end (ostream &out) {static char lst[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" ; if (state!=0) out <<lst[remainer] ; remainer=state=0 ; }
    unsigned char state=0, remainer=0 ;

    stack<string> hierarchy;

    void openbranch (string name, vector < pair<string,string>> attributes) ;
    void openbranch (string name) {return openbranch(name, {}) ; }
    template <class T> void smallbranch (string name, T value) ;
    template <class T> void smallbranch (string name, vector < pair<string,string>> attributes, T value) ;
    bool closebranch() ;
    string quote (string a) {return "\""+a+"\"" ; }
} ;
//===============================================================
class XMLReader_base
{
public:
    XMLReader_base (string path) ;
    stack<string> tree ;
    ifstream fic ;
    std::pair<string, map<string,string>> gettag() ;
    string getcontent() ;
} ;

class XMLReader : public XMLReader_base
{
public:
    XMLReader(string path): XMLReader_base(path) {auto u=gettag() ; if (u.first!="demnd") printf("ERR:unexpected first entry (should be demnd)\n") ;
        cout << "File date: "<< u.second["date"]<< "\nInput file: " << u.second["input"] <<"\nDimensions: "<< u.second["dimensions"] <<"\n" ;
    };
    int read_nextts(vector<string> &names, vector<vector<vector<double>>> & data) ;
    int decodebase64f (istream &in, vector<float>& val) ;
    int decodebase64f_2dd (istream &in, vector<vector<double>>& val) ;
} ;
