#include "Server.h"
#include "io.h"
#include "Texturing.h"

// needed for Tools
uint Tools::d=0 ;
vector < vector <int> > Tools::MSigns ;
vector < vector <int> > Tools::MIndexAS ;
vector < double > Tools::Eye ;
vector < pair <int,int> > Tools::MASIndex ;
vector <FILE *> Tools::outs ;
boost::random::mt19937 Tools::rng ;
boost::random::uniform_01<boost::mt19937> Tools::rand(rng) ;

map<string,string> parse_url (string & url) ;

int main(void)
{
    using namespace httplib;

    Server svr;
    Texturing Texture ; 
    
    svr.set_base_dir("../../");
    printf("Starting\n") ; fflush(stdout) ; 
    /*svr.Get("/hi", [](const Request& req, Response& res) {
        res.set_content("Hello World!", "text/plain");
    });*/

    //svr.Get("/something", [](const Request& req, Response& res) {
    //});

    svr.Get(R"(/load)", [&](const Request& req, Response& res) {
        printf("Loading data") ; fflush(stdout) ; 
        Texture.clean() ; 
        Texture.initialise(req.params) ; 
        
        printf("%d %d %d Data loaded\n", Texture.N, Texture.Ts.size(), Texture.R.size()) ; fflush(stdout) ; 
        
        //res.set_content(numbers, "text/plain");
    });
    
    svr.Get(R"(/render)", [&](const Request& req, Response& res) {
        printf("Rendering data") ; fflush(stdout) ; 
        Texture.MasterRender(req.params) ;
        printf("Rendered!") ; fflush(stdout) ; 
        //res.set_content(numbers, "text/plain");
    });
    
    svr.Get(R"(/forcerender)", [&](const Request& req, Response& res) {
        printf("Force Rendering data") ; fflush(stdout) ; 
        Texture.ViewPoint = vector<int> (Texture.d-3+1, INT_MIN) ; 
        Texture.MasterRender(req.params) ;
        printf("Rendered!\n") ; fflush(stdout) ; 
        //res.set_content(numbers, "text/plain");
    });
    

    svr.listen("localhost", 54321);
}

//=======================================================
map<string,string> parse_url (string & url) // Unused
{
    map<string,string> res ; 
    
    size_t found=0 ; 
    while (found!=std::string::npos)
    {
        found=url.find("&") ; 
        auto token = url.substr(0,found) ; 
        auto f = token.find("=") ;
        res[token.substr(0,f)]=token.substr(f+1,string::npos) ; 
        
        url = url.substr(found+1, string::npos) ;
    }
    return (res) ;
}
