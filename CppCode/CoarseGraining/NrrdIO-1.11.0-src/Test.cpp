#include <cstdio>
#include <cstdlib>
#include <vector>
#include "NrrdIO.h"

int main (int argc, char * argv [])
{
    Nrrd *nval;
    auto nio = nrrdIoStateNew();
    float tt[24] = {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2.0,2.1,2.2,2.3,2.4} ; 

    nrrdIoStateEncodingSet(nio, nrrdEncodingAscii) ; 
    
    nval = nrrdNew();
    nrrdWrap_va(nval, tt, nrrdTypeFloat, 4, 2, 3, 4, 1);
    
    std::vector <int> v ={nrrdKindSpace, nrrdKindSpace, nrrdKindSpace, nrrdKindVector} ;
    
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoKind, v.data() );
    nrrdAxisInfoSet_va(nval, nrrdAxisInfoLabel, "x", "y", "z", "nnn");
    nrrdAxisInfoSet_va(nval, nrrdAxisInfoMin, 2.25, 1., 1.5, AIR_NAN);
    nrrdAxisInfoSet_va(nval, nrrdAxisInfoMax, 1., 2., 3., AIR_NAN);
    nrrdAxisInfoSet_va(nval, nrrdAxisInfoSpacing, 0.1, 0.5, 1., AIR_NAN);
    nrrdSave("test.nrrd", nval, nio);
    nrrdNix(nval);
    
    
}
