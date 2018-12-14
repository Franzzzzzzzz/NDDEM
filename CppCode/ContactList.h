#include <vector>
#include <list>
#include "Typedefs.h"
#include "Parameters.h"
#include "Tools.h"


#ifndef CONTACTLIST
#define CONTACTLIST

class Action {
public :
    vector <double> Fn, Ft, Torquei, Torquej ;
    void set (v1d a, v1d b, v1d c, v1d d) {Fn=a ; Ft=b ; Torquei=c ; Torquej=d ; }
    void setzero (int d) {Fn=(v1d(d,0)) ; Ft=(v1d(d,0)) ; Torquei=(v1d(d*(d-1)/2,0)) ; Torquej=(v1d(d*(d-1)/2)) ; }
} ;


class cp // Contact properties class
{
public:
 cp (int ii, int jj, int d, double ctlength, Action * default_action) : i(ii), j(jj), contactlength(ctlength), tspr (vector <double> (d, 0)), infos(default_action), owninfos(false){} //creator
 ~cp () { if (owninfos) delete(infos) ; }
 cp & operator= (const cp & c)
 {
     i=c.i ;
     if (c.i<0) return *this ; //is a deleted element, just keep moving
     j=c.j ; // copy everything else if it is not a deleted element
     isghost=c.isghost ;
     tspr=c.tspr ;
     contactlength=c.contactlength ;
     infos=c.infos ;
 }
 Action & getinfo () {return *infos ; }
 //void setinfo (Action & a) {if (!infos) infos = new Action ; *infos=a ; }
 void setinfo (Action * a) {infos=a ; }

 int i, j ;
 double contactlength ;
 int8_t isghost ;        // LIMIT d<128
 vector <double> tspr;
 Action * infos ;
 bool owninfos ;
} ;

//--------------------------
class ContactList
{
public:
 void reset() {it = v.begin() ;}
 int insert(const cp& a) ;
 void finalise () { while (it!=v.end()) it=v.erase(it) ; }
 list <cp> v ;
 Action * default_action () {return (&def) ; }
 int cid=0 ;

private:
 list<cp>::iterator it ;
 Action def ;
};


inline bool operator< (const cp &a, const cp &b) {if (a.i==b.i) return (a.j<b.j) ; return a.i<b.i ; }
inline bool operator== (const cp &a, const cp &b) {return (a.i==b.i && a.j==b.j) ; }

//============================================================================================================
//------------------ Quite slow unfortunately. Probably better to use the list implementation ----------------
/*class sparsevector
{
public:
    sparsevector (int m) : max_size(m) {}
    void insert_in_place (vector <cp> & v1) ;
    int find_next_insertion (vector <cp> &v, cp &a) ;
    void mark_to_delete (vector <cp> & v) ;
    void finalise (vector<cp> &v) ;
    void reset(void) {idx=0 ;}
private:
    vector <cp> vin ;
    vector <int> location ;
    void postpone_insertion(cp & a, int idx) ;
    void compact (vector <cp> & v) ;
    int max_size ;
    int idx ;
    bool isdeleted (const cp &a) {return (a.i<0) ;}
} ;

*/
#endif
