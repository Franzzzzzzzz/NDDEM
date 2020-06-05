// import * as THREE from '../../node_modules/three/build/three.module.js';

var spheres;
var orientations;
/**
* Make the initial particles
*/
async function load_initial_spheres(params,time) {
    if ( params.cache ) { var filename = params.data_dir + "Samples/" + params.fname + "dump-"+String(time.cur*time.save_rate).padStart(5,'0') +".csv" }
    else {         var filename = params.data_dir + "Samples/" + params.fname + "dump-"+String(time.cur*time.save_rate).padStart(5,'0') +".csv" + "?_="+ (new Date).getTime(); }
    console.log(filename)
    let promise = new Promise( function(resolve, reject) {
        Papa.parse(filename, {
            download: true,
            dynamicTyping: true,
            header: false,
            complete: function(results) {
                spheres = results.data.slice(1); // skip header
                resolve(spheres);
            }
        });
    });
    spheres = await promise.then( function(result) { return result; });
    return spheres;
};

/**
* Update sphere locations
* @param {number} t timestep
* @param {number} changed_higher_dim_view flag to determine if we have changed which dimensions we are representing --- NOTE: CURRENTLY NOT DOING ANYTHING
*/
async function load_current_spheres(params,time,changed_higher_dim_view) {
    if ( params.cache ) { var filename = params.data_dir + "Samples/" + params.fname + "dump-"+String(time.frame*time.save_rate).padStart(5,'0') +".csv" }
    else { var filename = params.data_dir + "Samples/" + params.fname + "dump-"+String(time.frame*time.save_rate).padStart(5,'0') +".csv"+"?_="+ (new Date).getTime() }
    let promise = new Promise( function(resolve, reject) {
        Papa.parse(filename, {
            download: true,
            dynamicTyping: true,
            header: false,
            cache: params.cache,
            complete: function(results) {
                resolve( results.data.slice(1) ); // skip header
            }
        });
    });
    spheres = await promise.then( function(result) { return result; });
    return spheres;
};

async function load_current_orientation(params,time,changed_higher_dim_view) {
    if ( params.cache ) { var filename = params.data_dir + "Samples/" + params.fname + "dumpA-"+String(time.frame*time.save_rate).padStart(5,'0') +".csv" }
    else { var filename = params.data_dir + "Samples/" + params.fname + "dumpA-"+String(time.frame*time.save_rate).padStart(5,'0') +".csv"+"?_="+ (new Date).getTime() }
    let promise = new Promise( function(resolve, reject) {
        Papa.parse(filename, {
            download: true,
            dynamicTyping: true,
            header: false,
            cache: params.cache,
            complete: function(results) {
                resolve( results.data.slice(1) );
            }
        });
    });
    orientations = await promise.then( function(result) { return result; });
    return orientations;
}

export { load_initial_spheres, load_current_spheres, load_current_orientation }
