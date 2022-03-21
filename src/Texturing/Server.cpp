/** \addtogroup Texturing Texturing server
 * This module handles the Texturing of higher dimensional rotating hyperspheres. It is designed to produced 2D textures that can be wrapped around spheres for visualisations.
 *  @{ */

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

map<string,string> parse_url (string & url) ; ///< \deprecated Do not use
template <int d>
void runthread_MasterRender (Texturing<d> * T) {T->MasterRender() ; } ///< Function to call Texturing::MasterRender()
std::thread MasterRenderThread ;
std::mutex LockRender ;
bool blenderrender=false ;

Texturing<3> Texturing3 ;
Texturing<4> Texturing4 ;
Texturing<5> Texturing5 ;
Texturing<6> Texturing6 ;
Texturing<7> Texturing7 ;
Texturing<8> Texturing8 ;
Texturing<9> Texturing9 ;
Texturing<10> Texturing10 ;


int curd = -1 ;

/** \brief Starts the server and handles the dispatching of the various commands. Default port is 54321
 *  \details URL Commands:
 *  \details /load : load the data. Should always be called first.
 *  \details /render : renders the current timestep and location, and may also cache some additional ts and locations
 *  \details /forcerender : DEPRECATED
 *  \details /vtkcolormap : export the colormap in vtk format
 *  \details /nrrdcolormap : export the colormap in nrrd format
 *  \details /vtkmap : export the textures as vtk surfaces through the colormap instead of plane textures.
 */
int main(int argc, char * argv[])
{
    using namespace httplib;

    if (argc>1)
        if (!strcmp(argv[1], "blender"))
            blenderrender = true ;

    if (blenderrender)
        printf("Rendering in blender compatible format\n") ;
    else
        printf("The texturing server now defaults to render textures for Threejs mapping. Use the blender command line option to render in Blender compatible textures.\n") ;

    Server svr;

    #ifdef TEXTURINGPATH
      svr.set_base_dir(TEXTURINGPATH "/..");
    #else
      svr.set_base_dir("..");
    #endif
    printf("Starting\n") ; fflush(stdout) ;
    /*svr.Get("/hi", [](const Request& req, Response& res) {
        res.set_content("Hello World!", "text/plain");
    });*/
    //printf("[%s]", Texturing4.DirectorySave.c_str()) ; fflush(stdout);
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
            case 4: Texturing4.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing4.N, Texturing4.Ts.size(), Texturing4.R.size()) ; fflush(stdout) ;
                    break ;
            case 5: Texturing5.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing5.N, Texturing5.Ts.size(), Texturing5.R.size()) ; fflush(stdout) ;
                    break ;
            case 6: Texturing6.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing6.N, Texturing6.Ts.size(), Texturing6.R.size()) ; fflush(stdout) ;
                    break ;
            /*case 7: Texturing7.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing7.N, Texturing7.Ts.size(), Texturing7.R.size()) ; fflush(stdout) ;
                    break ;
            case 8: Texturing8.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing8.N, Texturing8.Ts.size(), Texturing8.R.size()) ; fflush(stdout) ;
                    break ;
            case 9: Texturing9.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing9.N, Texturing9.Ts.size(), Texturing9.R.size()) ; fflush(stdout) ;
                    break ;
            case 10:Texturing10.initialise(req.params) ;
                    printf("%d %lu %lu Data loaded\n", Texturing10.N, Texturing10.Ts.size(), Texturing10.R.size()) ; fflush(stdout) ;*/

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
            printf("S%d", curd) ;

            switch (curd)
            {
            case 3: Texturing3.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<3>, &Texturing3) ;
                    while (!Texturing3.isrendered()) ;
                    break ;
            case 4: Texturing4.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<4>, &Texturing4) ;
                    while (!Texturing4.isrendered()) ;
                    break ;
            case 5: Texturing5.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<5>, &Texturing5) ;
                    while (!Texturing5.isrendered()) ;
                    break ;
            case 6: Texturing6.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<6>, &Texturing6) ;
                    while (!Texturing6.isrendered()) ;
                    break ;
            case 7: Texturing7.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<7>, &Texturing7) ;
                    while (!Texturing7.isrendered()) ;
                    break ;
            case 8: Texturing8.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<8>, &Texturing8) ;
                    while (!Texturing8.isrendered()) ;
                    break ;
            case 9: Texturing9.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<9>, &Texturing9) ;
                    while (!Texturing9.isrendered()) ;
                    break ;
            case 10:Texturing10.SetNewViewPoint(req.params) ;
                    if (MasterRenderThread.joinable()) MasterRenderThread.join() ;
                    MasterRenderThread = std::thread(runthread_MasterRender<10>, &Texturing10) ;
                    while (!Texturing10.isrendered()) ;
                    break ;
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
            case 3: Texturing3.write_colormap_vtk_base() ; break ;
            case 4: Texturing4.write_colormap_vtk_base() ; break ;
            case 5: Texturing5.write_colormap_vtk_base() ; break ;
            case 6: Texturing6.write_colormap_vtk_base() ; break ;
            case 7: Texturing7.write_colormap_vtk_base() ; break ;
            case 8: Texturing8.write_colormap_vtk_base() ; break ;
            case 9: Texturing9.write_colormap_vtk_base() ; break ;
            case 10: Texturing10.write_colormap_vtk_base() ; break ;
            default : printf("ERR: Not an available texturing dimension (%d)\n", curd) ; break ;
        }
    });

    svr.Get(R"(/nrrdcolormap)", [&](const Request& req, Response& res) {
        switch (curd)
        {
            case 3: Texturing3.write_colormap_nrrd_base (req.params) ; break ;
            case 4: Texturing4.write_colormap_nrrd_base (req.params) ; break ;
            case 5: Texturing5.write_colormap_nrrd_base (req.params) ; break ;
            case 6: Texturing6.write_colormap_nrrd_base (req.params) ; break ;
            case 7: Texturing7.write_colormap_nrrd_base (req.params) ; break ;
            case 8: Texturing8.write_colormap_nrrd_base (req.params) ; break ;
            case 9: Texturing9.write_colormap_nrrd_base (req.params) ; break ;
            case 10: Texturing10.write_colormap_nrrd_base (req.params) ; break ;
            default : printf("ERR: Not an available texturing dimension (%d)\n", curd) ; break ;
        }
        printf("NRRD colormap done\n") ;
    });

    svr.Get(R"(/vtkmap)", [&](const Request& req, Response& res) {
        switch (curd)
        {
            case 3: Texturing3.write_vtkmap(req.params) ; break ;
            case 4: Texturing4.write_vtkmap(req.params) ; break ;
            case 5: Texturing5.write_vtkmap(req.params) ; break ;
            case 6: Texturing6.write_vtkmap(req.params) ; break ;
            case 7: Texturing7.write_vtkmap(req.params) ; break ;
            case 8: Texturing8.write_vtkmap(req.params) ; break ;
            case 9: Texturing9.write_vtkmap(req.params) ; break ;
            case 10: Texturing10.write_vtkmap(req.params) ; break ;
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


/** @} */
