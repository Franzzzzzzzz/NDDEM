import * as THREE from "three";
// import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

var root_dir = window.location.origin + '/';
if ( window.location.hostname.includes('benjymarks') ) { root_dir = 'http://www.benjymarks.com/nddem/'}
else if ( window.location.hostname.includes('github') ) { root_dir = 'https://franzzzzzzzz.github.io/NDDEM/'; var cache=true; }

function update_texture(t) {
    if ( sphere !== undefined ) {
        sphere.material.map.offset.x = t/10.;/// console.log(t);
    }
};

var fname = "visualise/resources/earthmap.jpg";

const urlParams = new URLSearchParams(window.location.search);
const recorder = new CCapture({
    verbose: true,
    display: true,
    framerate: 30,
    quality: 100,
    format: 'png',
    timeLimit: 100,
    frameLimit: 0,
    autoSaveTime: 0
});
var record = false;

if ( urlParams.has('fname') ) { fname = urlParams.get('fname'); };

let clock = new THREE.Clock();
var scene = new THREE.Scene();
scene.background = new THREE.Color( 0x111111 );
var camera = new THREE.PerspectiveCamera( 50, window.innerWidth/window.innerHeight, 0.1, 1000 );
camera.position.z = 6;
camera.position.x = 1.5;

var renderer = new THREE.WebGLRenderer();
// var controls = new THREE.TrackballControls( camera, renderer.domElement );
renderer.setSize( window.innerWidth, window.innerHeight );
document.body.appendChild( renderer.domElement );

var background_light = new THREE.AmbientLight( 0xffffff );
scene.add( background_light );
var light = new THREE.DirectionalLight(0x505050);
light.position.x = -2
light.position.z = 2
scene.add( light );

var sphere;
var rect;

var loader = new THREE.TextureLoader()
    .load( root_dir + fname, function( texture ) {
        texture.wrapS = THREE.RepeatWrapping;
        texture.wrapT = THREE.RepeatWrapping;
        var sphere_geometry = new THREE.SphereGeometry( 1, 32, 32 );
        var rect_geometry = new THREE.PlaneBufferGeometry( 1, 1 );

        var material = new THREE.MeshStandardMaterial( { map: texture } );
        sphere = new THREE.Mesh( sphere_geometry, material );
        rect = new THREE.Mesh( rect_geometry, material );
        sphere.position.x = 4
        rect.position.x = 0
        rect.scale.set(4,2,1);
        sphere.rotation.y = Math.PI/2.;

        scene.add( sphere );
        scene.add( rect );
    } );

// var gui = new GUI();
// gui.add( slice, 'loc').min(-1).max(1).step(0.01).listen().name('Slice').onChange( function( val ) { update_spheres(val); }) ;
// gui.open();

if ( urlParams.has('record') ) { addRecordOnKeypress() };

function addRecordOnKeypress() {
    document.addEventListener("keydown", function(event) {
        if (event.code == 'Space') {
            if (record) {
                recorder.stop();
                recorder.save();
            }
            else {
                recorder.start();
            }
            record = !record;
        }
    }, false);
}

var animate = function () {
    // controls.update();
    update_texture( clock.getElapsedTime() );
    requestAnimationFrame( animate );
    renderer.render( scene, camera );
    if ( record ) {
        recorder.capture(renderer.domElement);
    }

};
window.addEventListener( 'resize', onWindowResize, false );
animate();

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize( window.innerWidth, window.innerHeight );
    if ( controls !== undefined ) { controls.handleResize(); }
};
