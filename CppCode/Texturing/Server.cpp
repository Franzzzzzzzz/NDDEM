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
    svr.set_base_dir("../../");

    /*svr.Get("/hi", [](const Request& req, Response& res) {
        res.set_content("Hello World!", "text/plain");
    });*/

    //svr.Get("/something", [](const Request& req, Response& res) {
    //});

    svr.Get(R"(/load&(.+))", [&](const Request& req, Response& res) {
        string arguments = req.matches[1];
        parse_url(arguments) ; 
        
        
        
        
        //res.set_content(numbers, "text/plain");
    });
    
    svr.Get(R"(/render&(.+))", [&](const Request& req, Response& res) {
        string arguments = req.matches[1];
        parse_url(arguments) ; 
        //res.set_content(numbers, "text/plain");
    });

    svr.listen("localhost", 66666);
}

//=======================================================
map<string,string> parse_url (string & url)
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
