import * as THREE from '../../node_modules/three/build/three.module.js';
import { VTKLoader } from '../../node_modules/three/examples/jsm/loaders/VTKLoader.js'

var all_locs;

async function load_world(params,time,world) {
    console.warn('Nothing implemented for LIGGGHTS loader!')
    var loader = new VTKLoader();
	loader.load( "models/vtk/bunny.vtk", function ( geometry ) {

		geometry.center();
		geometry.computeVertexNormals();

		var material = new THREE.MeshLambertMaterial( { color: 0xffffff } );
		var mesh = new THREE.Mesh( geometry, material );
		mesh.position.set( - 0.075, 0.005, 0 );
		mesh.scale.multiplyScalar( 0.2 );
		scene.add( mesh );

	} );

};

async function load_initial_spheres(params,time) {
    console.warn('Nothing implemented for LIGGGHTS loader!')
};

async function load_current_spheres(params,time,changed_higher_dim_view) {
    console.warn('Nothing implemented for LIGGGHTS loader!')
};

async function load_current_orientation(params,time,changed_higher_dim_view) {
    console.warn('Rotations not implemented for LIGGGHTS loader!')
};

export { load_world, load_initial_spheres, load_current_spheres, load_current_orientation }
