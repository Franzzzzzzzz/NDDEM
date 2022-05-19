import * as THREE from "three";
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

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
if ( urlParams.has('record') ) { addRecordOnKeypress() };
function addRecordOnKeypress() {
    document.addEventListener("keydown", function(event) {
        if (event.code == 'KeyR') {
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
function do_preprogrammed_motion() {
    if (segment == 0) {
        slice.D4 += 0.05;
        if (slice.D4 >= 1) { segment += 1; }
    }
    else if (segment == 1) {
        slice.D4 -= 0.05;
        if (slice.D4 <= -1) { segment += 1; }
    }
    else if (segment == 2) {
        slice.D4 += 0.05;
        if (slice.D4 >= 0) { segment += 1; }
    }
    else if (segment == 3) {
        slice.D4 = 0;
        slice.D5 += 0.05;
        if (slice.D5 >= 1) { segment += 1; }
    }
    else if (segment == 4) {
        slice.D5 -= 0.05;
        if (slice.D5 <= -1) { segment += 1; }
    }
    else if (segment == 5) {
        parametric += 0.05;
        spiral -= 0.0025;
        slice.D4 = spiral*Math.sin(parametric);
        slice.D5 = -spiral*Math.cos(parametric);
        if (spiral <= 0) { segment = 0; recorder.stop();
        recorder.save(); }
    }
    update_spheres();

}
let container = document.getElementById( 'container' );
let N_tag = document.createElement( 'div' );
N_tag.setAttribute("id", "N_tag");
N_tag.innerHTML = "5D";
container.appendChild(N_tag);
function update_spheres() {
    var R_draw = Math.sqrt( 1 -
                            Math.pow( slice.D4, 2) -
                            Math.pow( slice.D5, 2));
    if ( R_draw == 0 ) { sphere.visible = false; }
    else {
        sphere.visible = true;
        sphere.scale.set(R_draw,R_draw,R_draw);
    };

    let phi   = 2.*Math.PI*(slice.D4)/(6) + Math.PI/2.;
    let theta = 2.*Math.PI*(slice.D5)/(6) ;
    let x = (R + r*Math.cos(theta))*Math.cos(phi);
    let y = (R + r*Math.cos(theta))*Math.sin(phi);
    let z = r*Math.sin(theta);
    point.position.set(x,y,z);
    // wall.position.x = x;
};
var record = false; var segment = 0; var parametric = 0; var spiral = 1;
var quality = 7;
var controls;
var slice = {'D4':0, 'D5':0};
var scene = new THREE.Scene();
scene.background = new THREE.Color( 0x111111 );
var camera = new THREE.PerspectiveCamera( 75, window.innerWidth/window.innerHeight, 0.1, 1000 );
camera.position.z = 4;
camera.position.x = 1.5;


var renderer = new THREE.WebGLRenderer();
// var controls = new THREE.TrackballControls( camera, renderer.domElement );
renderer.setPixelRatio( window.devicePixelRatio );
renderer.setSize( window.innerWidth, window.innerHeight );
// document.body.appendChild( renderer.domElement );
container.appendChild( renderer.domElement );

var background_light = new THREE.AmbientLight( 0x555555 );
scene.add( background_light );
var light = new THREE.PointLight(0xa0a0a0);
light.position.z = 8
light.position.x = 5
scene.add( light );

var sphere_geometry = new THREE.SphereGeometry( 1, Math.pow(2,quality), Math.pow(2,quality) );
var sphere_material = new THREE.MeshPhongMaterial( { color: 0xe72564 } );
var sphere  = new THREE.Mesh( sphere_geometry, sphere_material );;
sphere.position.x = -1;
scene.add( sphere );


let R = 1;
let r = R/2.;
var torus_parent = new THREE.Group();
var torus_geometry = new THREE.TorusBufferGeometry( R, r, 2*Math.pow(2,quality), Math.pow(2,quality) );
var torus_material = new THREE.MeshPhongMaterial( { color: 0xffffff } );

let torus = new THREE.Mesh( torus_geometry, torus_material );
torus.castShadow = true;
torus.receiveShadow = true;

var phi_geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., 2*Math.pow(2,quality), Math.pow(2,quality) );
var phi_material = new THREE.MeshPhongMaterial( {
    color: 0x000000,
    // roughness: 0.7,
} );
let torus_phi = new THREE.Mesh( phi_geometry, phi_material );

var theta_geometry = new THREE.TorusBufferGeometry( r, r/10., 2*Math.pow(2,quality), Math.pow(2,quality) );
let torus_theta = new THREE.Mesh( theta_geometry, phi_material );
torus_theta.rotation.y = Math.PI/2;

torus.position.set(3.,0.,0.);
torus_phi.position.set(0.,0.,0.);
torus_theta.position.set(0.,R,0.);
scene.add( torus_parent );
torus_parent.add( torus );
torus.add( torus_phi );
torus.add( torus_theta );
torus.rotation.set(Math.PI/2.,Math.PI/4.,0.);

var pointGeometry = new THREE.SphereGeometry( 1, Math.pow(2,quality), Math.pow(2,quality) );
var pointMaterial = new THREE.MeshPhongMaterial( { color: 0xe72564 } );
var point = new THREE.Mesh( pointGeometry, pointMaterial );
point.scale.set(1./10.,1./10.,1./10.);
torus.add( point );
update_spheres();

var gui = new GUI();
gui.add( slice, 'D4').min(-3).max(3).step(0.01).listen().name('x<sub>4</sub>').onChange( function( val ) { update_spheres(); }) ;
gui.add( slice, 'D5').min(-3).max(3).step(0.01).listen().name('x<sub>5</sub>').onChange( function( val ) { update_spheres(); }) ;
gui.open();

var animate = function () {
    if ( controls !== undefined) { controls.update(); }
    requestAnimationFrame( animate );
    renderer.render( scene, camera );

    if ( record ) {
        recorder.capture(renderer.domElement);
        do_preprogrammed_motion();
        // torus_parent.rotation.y += 0.01;
    }

};
window.addEventListener( 'resize', onWindowResize, false );
animate();

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize( window.innerWidth, window.innerHeight );
    if ( controls !== undefined) { controls.handleResize(); }
};
