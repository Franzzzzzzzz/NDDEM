#include "CoarseGraining.h"
#include<regex>

int process_envvar(json &j)
{
    std::cout << j << "\n" ; fflush(stdout) ;
    std::cout << j.is_string() << " " ; fflush(stdout) ; 
    std::cout << j.is_number() << " \n" ; fflush(stdout) ; 
    if (j.is_array() || j.is_object())
    {
        for (auto & jj: j)
            process_envvar(jj) ;
    }
    else if (j.is_string())
    {
        try
        {
            std::cout <<"|"<< j <<"|"<<j.is_string() << "|"  << j.is_number()<< "<===\n"; fflush(stdout) ;
            std::string s = j.get<std::string>() ;
            std::regex reg("[$][a-zA-Z0-9]+");
            auto envvar = std::sregex_iterator(s.begin(), s.end(), reg);
            auto endenvvar = std::sregex_iterator();

            for (std::sregex_iterator i = envvar; i != endenvvar; ++i)
            {
                std::smatch match = *i ;
                std::string val =std::string(std::getenv(match.str().substr(1).c_str())) ;
                j=j.get<std::string>().replace(match.position(), match.length(), val) ;
            }
        }
        catch (...) {}
    }
    std::cout << j ;
printf("HHHH") ; fflush(stdout) ; 
return 0 ; 
}
//=======================================================
int main(int argc, char * argv[])
{
    /* TEST */
    /*struct Data D ; 
    double r = 0.5 ; double pos[3] = {1,0.5,0.5} ; 
    D.pos.resize(3) ; D.pos[0] = &pos[0] ; D.pos[1] = &pos[1] ; D.pos[2] = &pos[2] ; 
    D.radius = &r ;     
    LibSphere3DIntersect_MonteCarlo W1 (&D, 1, 3) ; 
    LibSphere3DIntersect W2 (&D, 1, 3) ;
    
    std::mt19937 gen(1234);
    
    // Uniform distribution between -1.0 and 1.0
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    
    for (int i=0 ; i<100 ; i++)
    {
        r= abs(dist(gen))/3. ;
        pos[0] = dist(gen) ; 
        pos[1] = dist(gen) ; 
        pos[2] = dist(gen) ; 
        printf("%g %g\n", W1.distance(0,{0.1,0.2,0.3}), W2.distance(0,{0.1,0.2,0.3})) ; 
    }      
    std::exit(0) ; */
    
    if (argc<2) 
    {
        printf("Coarse graining was run without commandline argument. If you are running in webassembly, this is expected. If you are running standalone, you need to provide a json file as argument.\n") ; 
        std::exit(1) ; 
    }
    std::ifstream i(argv[1]);
    if (i.is_open()==false) {printf("Cannot find the json file provided as argument [%s]\n", argv[0]) ; std::exit(1) ; }
    json param;
    try { i >> param; }
    catch(...)
    {
        printf("This is not a legal json file, here is what we already got:\n") ;
        cout << param ;
    }
    //cout << param ;

    /*std::cout << ">>>" << std::getenv("AR") << "\n" ;
    process_envvar(param) ;
    std::cout << param ; fflush(stdout) ;*/

    CoarseGraining Global ; 
    Global.P.from_json(param) ;
    Global.P.post_init() ; 
    auto a = Global.P.files[0].reader->get_bounds() ; 
    //printf("{%d} ", Global.P.files[0].reader->get_numts()) ; 
    /*for (auto v: Global.P.files[0].reader->mapped_ts)
        printf("%d ", static_cast<int>(v)) ; 
    
    printf("{%d} ", Global.P.files[0].reader->get_numts()) ; 
    for (auto v: Global.P.files[0].reader->mapped_ts)
        printf("%d ", static_cast<int>(v)) ; */

    //auto a = Global.P.files[0].reader->get_bounds() ; 
    for (auto v: a)
        for (auto w: v)
            cout << w << " " ; 
    //Global.P.files[0].reader->build_index() ; 
    Global.setup_CG () ;
    Global.process_all() ; 
    Global.write() ; 
    //int b = Global.P.files[0].reader->get_numts() ; 
    //cout << "\n" << b << "\n" ; 
    //cout << Global.P.files[0].reader->is_seekable ; 
}
