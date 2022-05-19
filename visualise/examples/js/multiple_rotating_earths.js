import * as THREE from "three";
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

var root_dir = window.location.origin + '/';
if ( window.location.hostname.includes('benjymarks') ) { root_dir = 'http://www.benjymarks.com/nddem/' }
else if ( window.location.hostname.includes('github') ) { root_dir = 'https://franzzzzzzzz.github.io/NDDEM/' }

function update_texture(t) {
    if ( sphere1 !== undefined ) {
        sphere1.rotation.z = -t/5.;/// console.log(t);
    }
    if ( sphere2 !== undefined ) {
        sphere2.rotation.y = -t/5.;/// console.log(t);
    }
    if ( sphere3 !== undefined ) {
        sphere3.rotation.x = t/5.;/// console.log(t);
    }
    if ( sphere4 !== undefined ) {
        sphere4.rotation.z = t/5.;/// console.log(t);
    }
    if ( sphere5 !== undefined ) {
        sphere5.rotation.y = t/5.;/// console.log(t);
    }
    if ( sphere6 !== undefined ) {
        sphere6.rotation.x = -t/5.;/// console.log(t);
    }
};

var fname = "visualise/resources/earthmap.jpg";

const urlParams = new URLSearchParams(window.location.search);
if ( urlParams.has('fname') ) { fname = urlParams.get('fname'); };

let clock = new THREE.Clock();
var scene = new THREE.Scene();
scene.background = new THREE.Color( 0x111111 );
var camera = new THREE.PerspectiveCamera( 50, window.innerWidth/window.innerHeight, 0.1, 1000 );
camera.position.z = 10;
// camera.position.x = 1.5;
// camera.position.y = -1.5;

var renderer = new THREE.WebGLRenderer();
// var controls = new THREE.TrackballControls( camera, renderer.domElement );
renderer.setSize( window.innerWidth, window.innerHeight );
document.body.appendChild( renderer.domElement );

var background_light = new THREE.AmbientLight( 0xffffff );
scene.add( background_light );
var light = new THREE.DirectionalLight(0xaaaaaa);
light.position.x = -2
light.position.z = 2
scene.add( light );

var sphere1, sphere2, sphere3,sphere4, sphere5, sphere6;

var loader1 = new THREE.TextureLoader()
    .load( root_dir + fname, function( texture ) {
        texture.wrapS = THREE.RepeatWrapping;
        texture.wrapT = THREE.RepeatWrapping;
        var sphere_geometry = new THREE.SphereGeometry( 1, 32, 32 );

        var material = new THREE.MeshStandardMaterial( { map: texture } );
        sphere1 = new THREE.Mesh( sphere_geometry, material );
        sphere1.position.x = 2.5
        sphere1.position.y = 0
        // sphere1.position.y = 2.5
        //sphere1.rotation.y = Math.PI/2.;

        scene.add( sphere1 );

        sphere2 = new THREE.Mesh( sphere_geometry, material );
        sphere2.position.x = 2.5
        sphere2.position.y = 2.5
        //sphere2.rotation.y = Math.PI/2.;

        scene.add( sphere2 );

        sphere3 = new THREE.Mesh( sphere_geometry, material );
        sphere3.position.x = 0
        sphere3.position.y = 2.5
        // sphere3.position.y = -2.5
        //sphere3.rotation.y = Math.PI/2.;

        scene.add( sphere3 );

        sphere4 = new THREE.Mesh( sphere_geometry, material );
        // sphere4.position.x = 2.5
        sphere4.position.y = -2.5
        // sphere1.position.y = 2.5
        //sphere1.rotation.y = Math.PI/2.;

        scene.add( sphere4 );

        sphere5 = new THREE.Mesh( sphere_geometry, material );
        sphere5.position.x = -2.5
        sphere5.position.y = -2.5
        //sphere2.rotation.y = Math.PI/2.;

        scene.add( sphere5 );

        sphere6 = new THREE.Mesh( sphere_geometry, material );
        sphere6.position.x = -2.5
        // sphere6.position.y = 2.5
        // sphere3.position.y = -2.5
        //sphere3.rotation.y = Math.PI/2.;

        scene.add( sphere6 );
    } );

// var gui = new dat.GUI();
// gui.add( slice, 'loc').min(-1).max(1).step(0.01).listen().name('Slice').onChange( function( val ) { update_spheres(val); }) ;
// gui.open();

var animate = function () {
    // controls.update();
    update_texture( clock.getElapsedTime() );
    requestAnimationFrame( animate );
    renderer.render( scene, camera );

};
window.addEventListener( 'resize', onWindowResize, false );
animate();

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize( window.innerWidth, window.innerHeight );
    controls.handleResize();
};
