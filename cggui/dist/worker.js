{
let isinitialised = false ; 
let ismounted = false ;
let CGlib ;
let CG ; 

async function initialise() {
        CGlib = await CoarseGraining();
        CG = await new CGlib.CoarseGraining() ;
}

onmessage = async function(e) {
    if (!isinitialised)
    {
        await initialise() ;
        isinitialised=true ; 
    }
    
    try {
    /*if (ismounted && e.data[0]=='initialise')
    { 
        CGlib.FS.unmount('/work') ; 
        ismounted=false ; 
    }    */
    if (!ismounted)
    {
        ismounted=true ; 
    }
    }
    catch (error) {
        console.error(error);
    }
    /*else if (isinitialised && e.data[0]=='initialise')
    { 
        console.log("Bla")
        CGlib.FS.unmount('/work') ; 
        console.log("Bla")
    }*/
    
    try {
    if (e.data[0] == 'initialise')
    {   
        for (var i=0 ; i<2 ; i++)
        {
            if (e.data[2+i].length==0) continue ; 
            if (e.data[1]["file"][i].action=="donothing") continue ; 
            
            if (i==0)
            {
                var name = e.data[1]["file"][i].filename.split(/(\\|\/)/g).pop()
                e.data[1]["file"][i].filename = '/work/'+name ; 
                CGlib.FS.mkdir('/work'); 
                CGlib.FS.mount(CGlib.WORKERFS, { files: e.data[2+i] }, '/work');
            }
            if (i==1)
            {
                var name = e.data[1]["file"][i].filename.split(/(\\|\/)/g).pop()
                e.data[1]["file"][i].filename = '/contact/'+name ; 
                CGlib.FS.mkdir('/contact'); 
                CGlib.FS.mount(CGlib.WORKERFS, { files: e.data[2+i] }, '/contact');
            }
        }
        
        var cstring = JSON.stringify(e.data[1]) ; 
        console.log(cstring) ;
        CG.param_from_json_string (cstring) ;
        CG.param_from_json_string ("{}") ;
        var nts= CG.param_get_numts(0) ;
        console.log(nts) ; 
        var bounds = CG.param_get_bounds(0) ;
        var windowsize = CG.param_get_minmaxradius(0) ;
        console.log(bounds) ; 
        postMessage(['initialised', nts, bounds, windowsize]) ; 
    }
    else if (e.data[0] == 'setparameters')
    {
        var cstring = JSON.stringify(e.data[1]) ; 
        console.log(cstring) ; 
        CG.param_from_json_string(cstring) ; 
        CG.param_post_init() ; 
        CG.setup_CG () ;
        postMessage(['parametrised', e.data[1]['fields']]) ; 
    }
    else if (e.data[0] == 'processts')
    {
        console.log(e.data[1])
        CG.process_timestep(e.data[1], false) ;
        console.log("B")
        postMessage(['tsprocessed', e.data[1]]) ; 
        console.log("C")
        
    }
    else if(e.data[0] == 'getresult')
    {
        var res=CG.get_result(e.data[1], e.data[2], e.data[3]) ;
        var res2=CG.get_gridinfo() ;
        postMessage(["resultobtained", res, res2]) ; 
    }
    else if (e.data[0] == 'getspheres')
    {
        var res = CG.get_spheres(e.data[1]) ;
        postMessage(["sphereinfos", res]) ;
    }
    }
    catch (error) {
  console.error(error);
  // Expected output: ReferenceError: nonExistentFunction is not defined
  // (Note: the exact output may be browser-dependent)
}
    
    /*void param_from_json_string (std::string param)  { json jsonparam =json::parse(param) ; return P.from_json(jsonparam) ; }
    std::vector<std::vector<double>> param_get_bounds
    (int file = 0) {return P.files[file].reader->get_bounds() ; }
    int  param_get_numts(int file = 0) {return P.files[file].reader->get_numts(); }
    int param_read_timestep(int n) {return P.read_timestep(n) ; }
    void param_post_init () {return P.post_init() ; }*/
    
    
    //[fileHandle] = await showOpenFilePicker();
 

    //const f = e.data[0];    
    //console.log(f) ;
    //var cppread = await testread();
    
    //var fr = new FileReader();
    //fr.onload = function () {
    //    var data = new Uint8Array(fr.result);

        //Module['FS_createDataFile']('/', 'filename', data, true, true, true);
        
        
        //Module.ccall('readFile', null, [], null);
    //};
}

self.importScripts('CoarseGraining.js');
}
//self.importScripts('testread.js');
