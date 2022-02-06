{
let isinitialised = false ; 
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
    
    if (e.data[0] == 'initialise')
    {
        nbfile = e.data[1]["file"].length ; 
        var data = {}
        data["file"] = e.data[1]["file"] ; 
        
        CGlib.FS.mkdir('/work');
        for (var i=0 ; i<nbfile ; i++)
        {
            console.log(e.data[1]["file"][i].filename) ;
            CGlib.FS.mount(CGlib.WORKERFS, { files: e.data[2+i] }, '/work');
            var name = data["file"][i].filename.split(/(\\|\/)/g).pop()
            data["file"][i].filename = '/work/'+name ; 
            if (e.data[2+i].length>1)
                data["file"][i].filename=data["file"][i].filename.replace(/[0-9]+/i,"%d") ; 
        }
        console.log(data) ; 
        var cstring = JSON.stringify(data) ; 
        CG.param_from_json_string (cstring) ;
        CG.param_from_json_string ("{}") ;
        var nts= CG.param_get_numts(0) ;
        console.log(nts) ; 
        var bounds = CG.param_get_bounds(0) ;
        postMessage(['initialised', nts, bounds]) ; 
    }
    else if (e.data[0] == 'setparameters')
    {
        delete e.data[1]['file'] ; 
        delete e.data[1]['saveformat'] ; 
        delete e.data[1]['save'] ; 
        var cstring = JSON.stringify(e.data[1]) ; 
        console.log(cstring) ; 
        CG.param_from_json_string(cstring) ; 
        CG.param_post_init() ; 
        CG.setup_CG () ;
        postMessage(['parametrised', e.data[1]['fields']]) ; 
    }
    else if (e.data[0] == 'processts')
    {
        CG.process_timestep(e.data[1], false) ;
        postMessage(['tsprocessed', e.data[1]]) ; 
        
    }
    else if(e.data[0] == 'getresult')
    {
        var res=CG.get_result(e.data[1], e.data[2], e.data[3]) ;
        var res2=CG.get_gridinfo() ;
        postMessage(["resultobtained", res, res2]) ; 
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
