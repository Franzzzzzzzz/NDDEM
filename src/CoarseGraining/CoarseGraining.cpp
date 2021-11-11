#include "CoarseGraining.h"


using namespace std;
using json = nlohmann::json;

// TESTING FOR EMSCRIPTEN to activate FS
/*extern "C" {
void readFile(std::string filename){
    printf("%s\n", filename.c_str()) ;  
    FILE * in = fopen (filename.c_str(), "r") ; 
    printf("File open %X\n", in) ; 
    usleep(10000000);
    if (in != nullptr)
    {
        printf("Starting reading\n") ; 
        while (!feof(in))
            fscanf(in,"%*c") ; 
        fclose(in) ;
        printf("Finished reading\n") ; 
    }
}
}*/

//===================================================

