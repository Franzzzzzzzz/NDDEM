import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls.js";
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

var root_dir = window.location.origin + '/';
if ( window.location.hostname.includes('benjymarks') ) { root_dir = 'http://www.benjymarks.com/nddem/'}
else if ( window.location.hostname.includes('github') ) { root_dir = 'https://franzzzzzzzz.github.io/NDDEM/'; cache=true; }

function update_texture( rot ) {
    if ( sphere1 !== undefined ) {
        sphere1.rotation.x = rot.x;
        sphere1.rotation.y = rot.y;
        sphere1.rotation.z = rot.z;/// console.log(t);
    }
};

var fname = "./visualise/resources/earthmap.jpg";
var rot = {'x':0,'y':0,'z':0};

const urlParams = new URLSearchParams(window.location.search);
if ( urlParams.has('fname') ) { fname = urlParams.get('fname'); };

let clock = new THREE.Clock();
var scene = new THREE.Scene();
scene.background = new THREE.Color( 0x111111 );
var camera = new THREE.PerspectiveCamera( 50, window.innerWidth/window.innerHeight, 0.1, 1000 );
camera.position.z = 5;
// camera.position.x = 1.5;
// camera.position.y = -1.5;

var renderer = new THREE.WebGLRenderer();
var controls = new OrbitControls( camera, renderer.domElement );
renderer.setSize( window.innerWidth, window.innerHeight );
document.body.appendChild( renderer.domElement );

var background_light = new THREE.AmbientLight( 0xffffff );
scene.add( background_light );
var light = new THREE.DirectionalLight(0xaaaaaa);
light.position.x = -2
light.position.z = 2
scene.add( light );

var sphere1, sphere2, sphere3,sphere4, sphere5, sphere6;
var axesHelper = new THREE.AxesHelper(2);
scene.add( axesHelper );

var loader1 = new THREE.TextureLoader()
    .load( root_dir + fname, function( texture ) {
        texture.wrapS = THREE.RepeatWrapping;
        texture.wrapT = THREE.RepeatWrapping;
        var sphere_geometry = new THREE.SphereGeometry( 1, 32, 32 );

        var material = new THREE.MeshStandardMaterial( { map: texture } );
        sphere1 = new THREE.Mesh( sphere_geometry, material );
        // sphere1.position.x = 2.5
        // sphere1.position.y = 0
        // sphere1.position.y = 2.5
        //sphere1.rotation.y = Math.PI/2.;

        scene.add( sphere1 );
    } );

var gui = new GUI();
gui.add( rot, 'x').min(-Math.PI).max(Math.PI).step(0.01).listen().name('x Rotation').onChange( function( val ) { update_texture(rot); }) ;
gui.add( rot, 'y').min(-Math.PI).max(Math.PI).step(0.01).listen().name('y Rotation').onChange( function( val ) { update_texture(rot); }) ;
gui.add( rot, 'z').min(-Math.PI).max(Math.PI).step(0.01).listen().name('z Rotation').onChange( function( val ) { update_texture(rot); }) ;
gui.open();

var animate = function () {
    controls.update();
    // update_texture( clock.getElapsedTime() );
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
