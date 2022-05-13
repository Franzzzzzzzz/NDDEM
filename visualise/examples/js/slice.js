import * as THREE from "three";

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
let container = document.createElement("div");
document.body.appendChild(container);
let N_tag = document.createElement( 'div' );
N_tag.setAttribute("id", "N_tag");
N_tag.innerHTML = "3D";
container.appendChild(N_tag);

function update_spheres(x) {
    var R_draw = Math.sqrt(1 - Math.abs(x));
    if ( R_draw == 0 ) { circle.visible = false; }
    else {
        circle.visible = true;
        circle.scale.set(R_draw,R_draw,R_draw);
    };

    wall.position.x = x;
};
var record = false; var sign = 1;
var controls;
var slice = {'loc':-1};
var scene = new THREE.Scene();
scene.background = new THREE.Color( 0x111111 );
var camera = new THREE.PerspectiveCamera( 75, window.innerWidth/window.innerHeight, 0.1, 1000 );
camera.position.z = 4;
camera.position.x = 1.5;

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

var renderer = new THREE.WebGLRenderer();
// var controls = new THREE.TrackballControls( camera, renderer.domElement );
renderer.setPixelRatio( window.devicePixelRatio );
renderer.setSize( window.innerWidth, window.innerHeight );
renderer.shadowMap.enabled = true;
container.appendChild( renderer.domElement );

var background_light = new THREE.AmbientLight( 0x777777 );
scene.add( background_light );
var light = new THREE.PointLight(0x999999);
light.position.z = 8
light.position.x = 5
scene.add( light );


var sphere_geometry = new THREE.SphereGeometry( 1, 256, 256 );
var circle_geometry = new THREE.CircleGeometry( 1, 256 );
var wall_geometry = new THREE.PlaneBufferGeometry( 1, 1 );
var material = new THREE.MeshStandardMaterial( { color: 0xeeeeee } );
var wall_material = new THREE.MeshStandardMaterial( { color: 0xe72564 } );

var sphere  = new THREE.Mesh( sphere_geometry, material );
var circle  = new THREE.Mesh( circle_geometry, material );
var wall  = new THREE.Mesh( wall_geometry, wall_material );
sphere.position.x = 0
circle.position.x = 3
circle.visible = false;
wall.rotation.y = Math.PI/2.;
wall.position.x = slice.loc;
wall.scale.set(4,4,4);

scene.add( sphere );
scene.add( circle );
scene.add( wall );

var gui = new dat.GUI();
gui.add( slice, 'loc').min(-1).max(1).step(0.01).listen().name('Slice').onChange( function( val ) { update_spheres(val); }) ;
gui.open();

var animate = function () {
    if ( controls !== undefined) { controls.update(); }
    requestAnimationFrame( animate );
    renderer.render( scene, camera );
    if ( record ) {
        recorder.capture(renderer.domElement);
        slice.loc += sign*0.05; update_spheres(slice.loc);
        if ( slice.loc > 1) {sign = -1;}
        else if( slice.loc < -1) { sign = 1;}
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
