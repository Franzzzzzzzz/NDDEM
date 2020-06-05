import * as THREE from '../node_modules/three/build/three.module.js';

async function load_world(params,time,world) {

};

function load_initial_spheres(params,time) {
    if ( params.cache ) { var filename = params.data_dir + "Samples/" + params.fname }
    else { var filename = params.data_dir + "Samples/" + params.fname + "?_="+ (new Date).getTime(); }
    Papa.parse(filename, {
        download: true,
        dynamicTyping: true,
        header: true,
        preview: 1, // just load one line!
        complete: function(results) {
            particles = new THREE.Group();
            scene.add( particles );
            first_row = results.data;
            num_particles = first_row[0];
            console.log(first_row)
        }
    });
    console.log(num_particles)
};

function load_current_spheres(params,time,changed_higher_dim_view) {

};

function load_current_orientation(params,time,changed_higher_dim_view) {

};

export { load_world, load_initial_spheres, load_current_spheres, load_current_orientation }
