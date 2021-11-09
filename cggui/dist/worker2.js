onmessage = function(e) {
    if (e.data[0] == 'initialise')
    {
        nbfile = e.data[1]["file"].length ; 
        var data = {}
        data["file"] = e.data[1]["file"] ; 
        
        FS.mkdir('/work');
        for (var i=0 ; i<nbfile ; i++)
        {
            console.log(e.data[1]["file"][i].filename) ;
            FS.mount(WORKERFS, { files: [e.data[2+i]] }, '/work');
            var name = data["file"][i].filename.split(/(\\|\/)/g).pop()
            data["file"][i].filename = '/work/'+name ; 
        }
        
        console.log(data["file"][0].filename) ;
        Module.ccall('readFile', null, ['string'], data["file"][0].filename);
        
        
    }
    
    
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
