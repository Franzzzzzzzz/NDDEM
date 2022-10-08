#ifndef XML_H
#define XML_H

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
    void writeArray(string name, vector<vector<double>>*x, int beg, int length, ArrayType t, EncodingType te=EncodingType::ascii) ; 
    void writeArray(string name, std::vector< std::vector< double > >* x, ArrayType t, EncodingType te=EncodingType::ascii) ;
    void writeArray(string name, std::vector<double>* x, ArrayType t, EncodingType te=EncodingType::ascii) ;
    void stopTS () ;
    void close() ;
    void emergencyclose() ;

    void openbranch (string name, vector < pair<string,string>> attributes) ;
    void openbranch (string name) {return openbranch(name, {}) ; }
    template <class T> void smallbranch (string name, T value) ;
    template <class T> void smallbranch (string name, vector < pair<string,string>> attributes, T value) ;
    bool closebranch() ;

    ofstream fic ;


private:
    vector <pair<double,streampos>> index ;
    int encodebase64f (ostream &out, vector<double>& val) ;
    void encodebase64f_end (ostream &out) {static char lst[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" ; if (state!=0) out <<lst[remainer] ; remainer=state=0 ; }
    unsigned char state=0, remainer=0 ;

    stack<string> hierarchy;

    string quote (string a) {return "'"+a+"'" ; }
} ;
//===============================================================
class XMLReader_base
{
public:
    XMLReader_base (string path) ;
    stack<string> tree ;
    ifstream fic ;
    std::pair<string, map<string,string>> gettag() ;
    tuple <string, map<string,string> , std::vector<double> > gettagdata() ;
    string getcontent() ;
    void close () {fic.close() ; }
} ;

class XMLReader : public XMLReader_base
{
public:
    XMLReader(string path): XMLReader_base(path) {tags=gettag() ; if (tags.first!="demnd") printf("ERR:unexpected first entry (should be demnd)\n") ;
        cout << "File date: "<< tags.second["date"]<< "\nInput file: " << tags.second["input"] <<"\nDimensions: "<< tags.second["dimensions"] <<"\n" ;
    };
    int read_boundaries (vector <vector <double>> &boundaries) ;
    int read_radius (vector <double> &radius) ;
    double read_nextts(vector<string> &names, vector<vector<vector<double>>> & data) ;
    std::vector<std::pair<double,std::streampos>> read_index () ;
    int decodebase64f (istream &in, vector<float>& val) ;
    int decodebase64f_2dd (istream &in, vector<vector<double>>& val) ;

    std::pair<string, map<string,string>> tags ;
} ;

#endif
