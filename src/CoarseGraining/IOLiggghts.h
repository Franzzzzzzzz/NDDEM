#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <sstream>

#include "gzip.hpp"
//#include "termcolor.hpp"
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/stream.hpp>

#include "Typedefs.h"

#include "Coarsing.h"

using namespace std;


class Datafile {
public:
    Datafile() : Radius(0.00075), Rho(2500) { }

    istream *in, *incf ; //, *incf ;

    int curts ; int N, Ncf ;
    double Radius, Rho ;

    vector <string> fields, fieldscf ;
    v2d data, tdata, datacf ;

    // Functions
    int open(string path) ;
    int opencf(string path) ;
    int read_full_ts(bool keep) ;
    int set_data(struct Data & D) ;


private:
    int read_next_ts(istream *is, bool iscf, bool keep) ;
    int do_post_atm() ;
    int do_post_cf() ;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> filt_in;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> filt_incf;
    ifstream * file_in, *file_incf ;
} ;
