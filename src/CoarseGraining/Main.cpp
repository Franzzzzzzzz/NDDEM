#include "Main.h"


using namespace std;
using json = nlohmann::json;

// TESTING FOR EMSCRIPTEN to activate FS
extern "C" {
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
}

//=======================================================
int main(int argc, char * argv[])
{
    if (argc<2) 
    {
        printf("Coarse graining was run without commandline argument. If you are running in webassembly, this is expected. If you are running standalone, you need to provide a json file as argument.\n") ; 
        std::exit(1) ; 
    }
    std::ifstream i(argv[1]);
    if (i.is_open()==false) {printf("Cannot find the json file provided as argument\n") ; std::exit(1) ; }
    json param;
    try { i >> param; }
    catch(...)
    {
        printf("This is not a legal json file, here is what we already got:\n") ;
        cout << param ;
    }
    
    CoarseGraining Global ; 
    Global.P.from_json(param) ;
    auto a = Global.P.files[0].reader->get_bounds() ; 
    //Global.P.files[1].reader->get_numts() ; 
    Global.P.read_timestep(5) ; 
    //auto a = Global.P.files[0].reader->get_bounds() ; 
    for (auto v: a)
        for (auto w: v)
            cout << w << " " ; 
    Global.P.files[0].reader->build_index() ; 
    Global.P.post_init() ; 
    Global.setup_CG () ;
    Global.process_all() ; 
    Global.write() ; 
    //int b = Global.P.files[0].reader->get_numts() ; 
    //cout << "\n" << b << "\n" ; 
    //cout << Global.P.files[0].reader->is_seekable ; 
}
//===================================================
void CoarseGraining::setup_CG () 
{
    //P.verify() ; 
    if (C != nullptr) 
        delete C ; 
    
    C = new Coarsing (P.dim, P.boxes, P.boundaries, P.maxT) ;
    printf("%d\n", P.maxT) ;
    for (auto i : P.extrafields)
        C->add_extra_field(i.name, i.order, i.type) ;
    printf("%d \n", static_cast<int>(P.window)) ;
    if (P.window == Windows::LucyND_Periodic)      
        C->setWindow(P.window, P.windowsize, P.periodicity, P.boxes, P.delta) ; 
    else
        C->setWindow(P.window,P.windowsize) ;
    pipeline = C->set_flags(P.flags) ;
    printf("###%X###", static_cast<int>(pipeline)) ; 
    auto extrafieldmap = C->grid_setfields() ;

    C->cT=-1 ;
}
//-----------------------------------------
int CoarseGraining::process_timestep (int ts, bool allow_avg_fluct) 
{
    if ((P.timeaverage == AverageType::Intermediate   || P.timeaverage == AverageType::Both) && 
        (((pipeline & Pass::VelFluct) && !C->hasvelfluct) || 
            ((pipeline & Pass::RotFluct) && !C->hasrotfluct)))
        {
            
            if (!allow_avg_fluct)
            {
                printf("ERR: single timestep cannot run when intermediary average is required") ; 
                return -1 ; 
            }
            else
                process_fluct_from_avg() ; 
        }
    
    bool avg=false ; 
    if (P.timeaverage == AverageType::Intermediate || P.timeaverage == AverageType::Both) avg=true ; 
        
    P.read_timestep(ts+P.skipT) ; 
    C->cT = ts ;
    P.set_data (C->data) ; 
    
    if (pipeline & Pass::Pass1) C->pass_1() ; 
    if (pipeline & Pass::VelFluct) C->compute_fluc_vel (avg) ; 
    if (pipeline & Pass::RotFluct) C->compute_fluc_rot (avg) ;
    if (pipeline & Pass::Pass2) C->pass_2() ; 
    if (pipeline & Pass::Pass3) C->pass_3() ; 
    if (pipeline & Pass::Pass4) C->pass_4() ; 
    if (pipeline & Pass::Pass5) C->pass_5() ; 
    return 0 ; 
}
//------------------------------------------------------------
int CoarseGraining::process_fluct_from_avg() 
{
    for (int ts=0 ; ts<P.maxT ; ts++)
    {
        P.read_timestep(ts+P.skipT) ; 
        C->cT = ts ; 
        P.set_data (C->data) ; 
        if (pipeline & Pass::Pass1) C->pass_1() ;
        printf("\r") ; 
    }
    if (P.timeaverage == AverageType::Intermediate   || P.timeaverage == AverageType::Both)
        C->mean_time(true) ; 
    return 0 ; 
}
//----------------------------------------------------------
void CoarseGraining::process_all ()
{
    printf("%d////\n", P.maxT) ;
    for (int ts=0 ; ts<P.maxT ; ts++)
    {
        printf("\r%d ", ts) ; 
        process_timestep(ts, true) ;
    }
    if (P.timeaverage == AverageType::Final || P.timeaverage == AverageType::Both)
        C->mean_time(false) ; 
}
//------------------------------------------------------
void CoarseGraining::write ()
{
    if (std::find(P.saveformat.begin(), P.saveformat.end(), "netCDF")!=P.saveformat.end())   C->write_netCDF(P.save) ;
    if (std::find(P.saveformat.begin(), P.saveformat.end(), "vtk")!=P.saveformat.end()) C->write_vtk (P.save) ;
    if (std::find(P.saveformat.begin(), P.saveformat.end(), "mat")!=P.saveformat.end()) C->write_matlab(P.save, true) ;
    if (std::find(P.saveformat.begin(), P.saveformat.end(), "numpy")!=P.saveformat.end()) C->write_numpy(P.save, true) ;
}
//------------------------------------------------------
std::vector<double> CoarseGraining::get_result(int ts, std::string field, int component)
{
 if (ts != C->cT) {printf("The requested timestep has not been processed.\n"); return {} ; }
 int idx = C->get_id(field) ; 
 idx += component ; 
 std::vector <double> res ;
 res.resize(C->CGP.size()) ; 
 for (int i = 0 ; i<res.size() ; i++)
     res[i]=C->CGP[C->idx_FastFirst2SlowFirst(i)].fields[C->cT][idx] ; 
 return res ; 
}
//-----------------------------------------------------
std::vector<double> CoarseGraining::get_gridinfo()
{
    std::vector<double> res ;
    //origin
    res.push_back(C->CGP[0].location[0]) ; 
    res.push_back(C->CGP[0].location[1]) ; 
    res.push_back(C->CGP[0].location[2]) ; 
    //spacing
    res.push_back(C->dx[0]) ; 
    res.push_back(C->dx[1]) ; 
    res.push_back(C->dx[2]) ; 
    //dimensions
    res.push_back(C->npt[0]) ; 
    res.push_back(C->npt[1]) ; 
    res.push_back(C->npt[2]) ; 
    return res ; 
}
//-----------------------------------------------------

#ifdef EMSCRIPTEN
    #include "em_bindings.h"
#endif
