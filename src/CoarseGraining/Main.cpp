#include "CoarseGraining.h"
#include<regex>

int process_envvar(json &j)
{
    if (j.is_array() || j.is_object())
    {
        for (auto & jj: j)
            process_envvar(jj) ;
    }
    else if (j.is_string() && !j.is_number())
    {
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
    if (i.is_open()==false) {printf("Cannot find the json file provided as argument [%s]\n", argv[0]) ; std::exit(1) ; }
    json param;
    try { i >> param; }
    catch(...)
    {
        printf("This is not a legal json file, here is what we already got:\n") ;
        cout << param ;
    }
    
    process_envvar(param) ;

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
