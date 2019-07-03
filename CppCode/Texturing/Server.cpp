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
void runthread_MasterRender (Texturing * T) {T->MasterRender() ; }
std::thread MasterRenderThread ;
std::mutex LockRender ;

int main(void)
{
    using namespace httplib;

    Server svr;
    Texturing Texture ;

    Texture.BasePath = "../../Samples/" ;
    svr.set_base_dir("../..");
    printf("Starting\n") ; fflush(stdout) ;
    /*svr.Get("/hi", [](const Request& req, Response& res) {
        res.set_content("Hello World!", "text/plain");
    });*/

    //svr.Get("/something", [](const Request& req, Response& res) {
    //});

    svr.Get(R"(/load)", [&](const Request& req, Response& res) {
        printf("Loading data") ; fflush(stdout) ;
        if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
        Texture.clean() ;
        Texture.initialise(req.params) ;

        printf("%d %lu %lu Data loaded\n", Texture.N, Texture.Ts.size(), Texture.R.size()) ; fflush(stdout) ;

        res.set_content("Done", "text/plain");
    });

    svr.Get(R"(/render)", [&](const Request& req, Response& res) {
        if (!LockRender.try_lock())
        {
            res.status=429 ;
            printf("B") ; fflush(stdout) ;
        }
        else
        {
            printf("S") ;
            Texture.SetNewViewPoint(req.params) ;
            if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
            MasterRenderThread = std::thread(runthread_MasterRender, &Texture) ;

            while (!Texture.isrendered()) ;

            printf("R") ; fflush(stdout) ;
            res.set_content("Done", "text/plain");
            LockRender.unlock() ;
        }
    });

    svr.Get(R"(/forcerender)", [&](const Request& req, Response& res) {
        printf("NOT IMPLEMENTED ANYMORE Force Rendering data") ; fflush(stdout) ;
        //Texture.ViewPoint = vector<int> (Texture.d-3+1, INT_MIN) ;
        //Texture.MasterRender(req.params) ;
        printf("Rendered!\n") ; fflush(stdout) ;
        res.set_content("Done", "text/plain");
    });

    svr.Get(R"(/vtkcolormap)", [&](const Request& req, Response& res) {
        Texture.write_colormap_vtk_base() ;
    });

    svr.Get(R"(/vtkmap)", [&](const Request& req, Response& res) {
        Texture.write_vtkmap(req.params) ;
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
