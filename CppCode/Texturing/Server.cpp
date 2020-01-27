#include "Server.h"
#include "io.h"
#include "Texturing.h"

// needed for Tools
/*uint Tools::d=0 ;
vector < vector <int> > Tools::MSigns ;
vector < vector <int> > Tools::MIndexAS ;
vector < double > Tools::Eye ;
vector < pair <int,int> > Tools::MASIndex ;
vector <FILE *> Tools::outs ;
boost::random::mt19937 Tools::rng ;
boost::random::uniform_01<boost::mt19937> Tools::rand(rng) ;*/

map<string,string> parse_url (string & url) ;
template <int d>
void runthread_MasterRender (Texturing<d> * T) {T->MasterRender() ; }
std::thread MasterRenderThread ;
std::mutex LockRender ;

Texturing<3> Texturing3 ; 
Texturing<4> Texturing4 ; 
Texturing<5> Texturing5 ; 
Texturing<6> Texturing6 ; 
Texturing<7> Texturing7 ; 
Texturing<8> Texturing8 ; 
Texturing<9> Texturing9 ; 
Texturing<10> Texturing10 ; 


int curd = -1 ; 

int main(void)
{
    using namespace httplib;
        
    Server svr;
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
        //if (curd!=-1) Texturings[d]->clean() ;
        
        curd = atoi(req.params.at("ND").c_str()) ;
        switch (curd)
        {
            case 3: Texturing3.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing3.N, Texturing3.Ts.size(), Texturing3.R.size()) ; fflush(stdout) ;
                    break ;  
            /*case 4: Texturing4.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing4.N, Texturing4.Ts.size(), Texturing4.R.size()) ; fflush(stdout) ;
                    break ; 
            case 5: Texturing5.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing5.N, Texturing5.Ts.size(), Texturing5.R.size()) ; fflush(stdout) ;
                    break ; 
            case 6: Texturing6.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing6.N, Texturing6.Ts.size(), Texturing6.R.size()) ; fflush(stdout) ;
                    break ; 
            case 7: Texturing7.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing7.N, Texturing7.Ts.size(), Texturing7.R.size()) ; fflush(stdout) ;
                    break ; 
            case 8: Texturing8.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing8.N, Texturing8.Ts.size(), Texturing8.R.size()) ; fflush(stdout) ;
                    break ; 
            case 9: Texturing9.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing9.N, Texturing9.Ts.size(), Texturing9.R.size()) ; fflush(stdout) ;
                    break ; 
            case 10:Texturing10.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing10.N, Texturing10.Ts.size(), Texturing10.R.size()) ; fflush(stdout) ;
                    break ; */
            default : printf("ERR: Not an available texturing dimension (%d)\n", curd) ; break ; 
        }
        
        
        //Texturings[d]->initialise(req.params) ;
        //printf("%d %lu %lu Data loaded\n", Texturings[d]->N, Texturings[d]->Ts.size(), Texturings[d]->R.size()) ; fflush(stdout) ;

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
            
            switch (curd)
            {
            case 3: Texturing3.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<3>, &Texturing3) ;
                    while (!Texturing3.isrendered()) ;
                    break ;  
            /*case 4: Texturing<4>.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread<4>.joinable()) MasterRenderThread<4>.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<4>, &Texturings<4>) ;
                    while (!Texturings<4>.isrendered()) ;
                    break ; 
            case 5: Texturing<5>.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread<5>.joinable()) MasterRenderThread<5>.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<5>, &Texturings<5>) ;
                    while (!Texturings<5>.isrendered()) ;
                    break ; 
            case 6: Texturing<6>.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread<6>.joinable()) MasterRenderThread<6>.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<6>, &Texturings<6>) ;
                    while (!Texturings<6>.isrendered()) ;
                    break ; 
            case 7: Texturing<7>.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread<7>.joinable()) MasterRenderThread<7>.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<7>, &Texturings<7>) ;
                    while (!Texturings<7>.isrendered()) ;
                    break ; 
            case 8: Texturing<8>.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread<8>.joinable()) MasterRenderThread<8>.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<8>, &Texturings<8>) ;
                    while (!Texturings<8>.isrendered()) ;
                    break ; 
            case 9: Texturing<9>.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread<9>.joinable()) MasterRenderThread<9>.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<9>, &Texturings<9>) ;
                    while (!Texturings<9>.isrendered()) ;
                    break ; 
            case 10:Texturing<10>.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread<10>.joinable()) MasterRenderThread<10>.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<10>, &Texturings<10>) ;
                    while (!Texturings<10>.isrendered()) ;
                    break ; */
            default : printf("ERR: Not an available texturing dimension (%d)\n", curd) ; break ; 
            }

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
        switch (curd)
        {
            case 3: Texturing3.write_colormap_vtk_base() ;
                break ; 
            default : printf("ERR: Not an available texturing dimension (%d)\n", curd) ; break ; 
        }
    });

    svr.Get(R"(/nrrdcolormap)", [&](const Request& req, Response& res) {
        switch (curd)
        {
            case 3: Texturing3.write_colormap_nrrd_base (req.params) ;
                break ; 
            default : printf("ERR: Not an available texturing dimension (%d)\n", curd) ; break ; 
        }
        printf("NRRD colormap done\n") ; 
    });

    svr.Get(R"(/vtkmap)", [&](const Request& req, Response& res) {
        switch (curd)
        {
            case 3: Texturing3.write_vtkmap(req.params) ;
                break ; 
            default : printf("ERR: Not an available texturing dimension (%d)\n", curd) ; break ; 
        }
        
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
