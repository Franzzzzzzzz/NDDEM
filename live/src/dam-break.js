import css from "../css/main.css";

import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';

var urlParams = new URLSearchParams(window.location.search);

if ( !urlParams.has('lut') ) { urlParams.set('lut','Size') }

var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel, controls;
let physics, position;
let gui;
let boxes, spheres, boundary;
let floor, roof, left, right, front, back;
let S;
let NDDEMLib;
let pointer;
let frameRate = 60;
let v;
let pressure = 0;
let shear = 0;
let density = 0;
let pressure_time = [];
let shear_time = [];
let density_time = [];
// var radius = 0.5;
let radii;
let particle_volume;
let started = false;
let show_stats = false;
// const thickness = radius;
const material_density = 2700;
let old_time = 0;
let new_time = 0;
let counter = 0;
let p_controller, q_controller;
let NDsolids, material, STLFilename;
let meshes = new THREE.Group();

const raycaster = new THREE.Raycaster();
const mouse = new THREE.Vector2();
let intersection_plane = new THREE.Plane();
let camera_direction = new THREE.Vector3();

let INTERSECTED = null;
let last_intersection = null;
let locked_particle = null;
let ref_location;

let loading_method = 'strain_controlled';
if ( urlParams.has('stress_controlled') ) {
    loading_method = 'stress_controlled';
}

var params = {
    dimension: 3,
    H: 20, //system size
    L: 10, //system size
    LL: 100, //system size
    D: 2, //system size
    N: 600,
    // packing_fraction: 0.5,
    axial_strain: 0,
    volumetric_strain: 0,
    gravity: true,
    paused: false,
    H_cur: 0,
    pressure_set_pt: 1e4,
    deviatoric_set_pt: 0,
    d4: { cur:0 },
    d5: { cur:0 },
    r_min: 0.25,
    r_max: 0.5,
    omega: 5, // rotation rate
    lut: 'None',
    quality: 5,
    vmax: 20, // max velocity to colour by
    omegamax: 20, // max rotation rate to colour by
    wallremoved: false,
}

params.average_radius = (params.r_min + params.r_max)/2.;
let thickness = params.average_radius;

particle_volume = 4./3.*Math.PI*Math.pow(params.average_radius,3);
if ( urlParams.has('dimension') ) {
    params.dimension = parseInt(urlParams.get('dimension'));
}
if ( params.dimension === 4) {
    params.L = 3.5;
    params.N = 500
    particle_volume = Math.PI*Math.PI*Math.pow(params.average_radius,4)/2.;
}
else if ( params.dimension === 5) {
    params.L = 2.5;
    params.N = 500
    particle_volume = Math.PI*Math.PI*Math.pow(params.average_radius,4)/2.;
}
if ( urlParams.has('no_stats') ) {
    show_stats = false;
}
if ( urlParams.has('quality') ) { params.quality = parseInt(urlParams.get('quality')); }


params.L_cur = params.L;
params.packing_fraction = params.N*particle_volume/Math.pow(2*params.L,3);
params.back = -params.D;
params.front = params.D;
params.left = 0.;
params.right = params.H;
params.floor = 0.;
params.roof = params.L;

// update_L();
//
// function update_L() {
//     var L = params.N*4./3.*Math.PI*Math.pow(radius,3);
//     L = Math.pow(solid_volume/params.packing_fraction,1./3.)
// }

SPHERES.createNDParticleShader(params).then( init );

async function init() {

    physics = await NDDEMPhysics();
    // physics.main(params.dimensions, params.N, inputfile)
    position = new THREE.Vector3();

    //

    camera = new THREE.PerspectiveCamera( 50, window.innerWidth / window.innerHeight, 0.1, 1000 );
    camera.position.set( 0, 3*params.L, -3*params.L );
    camera.up.set(1, 0, 0);

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x666666 );

    // const axesHelper = new THREE.AxesHelper( 50 );
    // scene.add( axesHelper );

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add( hemiLight );

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set( 5, 5, 5 );
    dirLight.castShadow = true;
    dirLight.shadow.camera.zoom = 2;
    scene.add( dirLight );

    //const wall_geometry = new THREE.BoxGeometry( params.L*2 + thickness*2, thickness, params.L*2 + thickness*2 );
    //const wall_geometry = new THREE.SphereGeometry( params.L, 32, 32 );
    const wall_material = new THREE.MeshLambertMaterial();
    wall_material.wireframe = true;

    const wall_geometry = new THREE.BoxGeometry(1, 1, 1);
    left = new THREE.Mesh(wall_geometry, wall_material);
    left.scale.y = 2*thickness;
    left.scale.z = 2*params.D;
    left.scale.x = params.H;
    left.position.x = params.H / 2.;
    
    //floor.rotation.x = Math.PI / 2.;
    //floor.position.z = - params.L * params.aspect_ratio - params.thickness / 2.;
    scene.add(left);
    
    right = new THREE.Mesh(wall_geometry, wall_material);
    right.scale.y = 2*thickness;
    right.scale.z = 2*params.D;
    right.scale.x = params.H;
    right.position.x = params.H / 2.;
    right.position.y = params.L ;
    scene.add(right);
    
    floor = new THREE.Mesh(wall_geometry, wall_material);
    floor.scale.x = 2*thickness;
    floor.scale.y = params.LL;
    floor.scale.z = 2*params.D;
    floor.position.y = params.LL / 2.;
    // left.receiveShadow = true;
    scene.add(floor);
    
    /*floor = new THREE.Mesh(wall_geometry, wall_material);
    floor.scale.y = params.thickness;
    floor.rotation.x = Math.PI / 2.;
    floor.position.z = - params.L * params.aspect_ratio - params.thickness / 2.;
    // left.receiveShadow = true;
    scene.add(floor);*/


    SPHERES.add_spheres(S,params,scene);
    //

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;
    document.body.appendChild( renderer.domElement );

    // stats = new Stats();
    // panel = stats.addPanel( new Stats.Panel( 'Pressure', 'white', 'black' ) );
    // stats.showPanel( 3 ); // 0: fps, 1: ms, 2: mb, 3+: custom
    // // document.body.appendChild( stats.dom );
    // var thisParent = document.getElementById("stats");
    // thisParent.appendChild( stats.domElement );
    //
    // var statsALL = document.getElementById("stats").querySelectorAll("canvas");
    //
    // for(var i=0; i<statsALL.length; i++){
    //     statsALL[i].style.width = "240px";
    //     statsALL[i].style.height = "160px";
    // }

    // gui
    gui = new GUI();

    gui.width = 300;

    if ( params.dimension > 3 ) {
        gui.add( params.d4, 'cur', -params.L,params.L, 0.001)
            .name( 'D4 location').listen()
            // .onChange( function () { update_walls(); } );
            .onChange( update_boundary );
    }
    if ( params.dimension > 4 ) {
        gui.add( params.d5, 'cur', -params.L,params.L, 0.001)
            .name( 'D5 location').listen()
            // .onChange( function () { update_walls(); } );
            .onChange( update_boundary );
    }
    gui.add ( params, 'wallremoved').name('Remove the dam wall').listen().onChange( () => {
        right.position.y = params.LL ;
        S.simu_interpret_command("boundary 1 WALL 0 "+String(params.LL));
        });
    
    controls = new OrbitControls( camera, renderer.domElement );
    controls.target.y = params.L/2.;
    controls.target.x = params.H/2.;
    controls.update();

    window.addEventListener( 'resize', onWindowResize, false );
    window.addEventListener( 'mousemove', onMouseMove, false );
    window.addEventListener( 'keypress', onSelectParticle, false );

    if ( show_stats ) { make_graph(); }

    // update_walls();
    animate();
}

function onMouseMove( event ) {

    // calculate mouse position in normalized device coordinates
    // (-1 to +1) for both components

    mouse.x = ( event.clientX / window.innerWidth ) * 2 - 1;
    mouse.y = - ( event.clientY / window.innerHeight ) * 2 + 1;

}

function onSelectParticle( event ) {
    // console.log(camera.getWorldDirection() )
    if ( event.code === 'Space' ) {
        if ( locked_particle === null ) {
            locked_particle = INTERSECTED;
            // console.log(locked_particle);
            ref_location = locked_particle.position;

            camera.getWorldDirection( camera_direction ); // update camera direction
            // set the plane for the particle to move along to be orthogonal to the camera
            intersection_plane.setFromNormalAndCoplanarPoint( camera_direction,
                                                              locked_particle.position );
        }
        else {
            locked_particle = null;
        }
    }
}

function update_boundary() {
    if ( params.dimension === 4 ) {
        var s = 2*Math.sqrt(params.L*params.L/4 - params.d4.cur*params.d4.cur/4)/params.L;
    } else if ( params.dimension === 5 ) {
        var s = 2*Math.sqrt(params.L*params.L/4 - params.d4.cur*params.d4.cur/4 - params.d5.cur*params.d5.cur/4)/params.L;
    }

    boundary.scale.setScalar(s);
    if ( urlParams.has('stl') ) {
        meshes = renderSTL( meshes, NDsolids, scene, material, params.d4.cur );
    }

}


function onWindowResize(){

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth, window.innerHeight );

}

function update_walls(dt=0.001) {
    // console.log(dt)
    if ( params.dimension == 3 ) {
        params.packing_fraction = (params.N*particle_volume)/params.L_cur/params.L_cur/(params.L_cur - params.H_cur)/8.;
    }
    else if ( params.dimension == 4) {
        params.packing_fraction = (params.N*particle_volume)/params.L_cur/params.L_cur/(params.L_cur - params.H_cur)/8./params.L_cur;
    }


    if ( loading_method == 'strain_controlled') {

        params.L_cur =  params.L*(1-params.volumetric_strain);
        params.H_cur =  params.L*params.axial_strain; // TODO: THIS FORMULA IS WRONG!!!!!

    }
    else if ( loading_method == 'stress_controlled' ) {
        let delta_p = p_controller.update(params.pressure_set_pt,pressure,dt);
        let delta_q = q_controller.update(params.deviatoric_set_pt,shear,dt)
        // console.log(pressure)
        params.L_cur -= delta_p;
        params.H_cur += delta_q;

    }
    params.front =  params.L_cur;
    params.back  = -params.L_cur;
    params.left  = -params.L_cur;
    params.right =  params.L_cur;
    params.floor = -params.L_cur + params.H_cur;
    params.roof  =  params.L_cur - params.H_cur;

    S.setBoundary(0, [params.back,params.front]) ; // Set location of the walls in x
    S.setBoundary(1, [params.left,params.right]) ; // Set location of the walls in y
    S.setBoundary(2, [params.floor,params.roof]) ; // Set location of the walls in z
    for (var j = 0; j < params.dimension - 3; j++) {
        S.setBoundary(j + 3, [-params.L_cur,params.L_cur]) ; // Set location of the walls in z
    }
    back.position.x = params.back - thickness/2.;
    front.position.x = params.front + thickness/2.;
    left.position.y = params.left - thickness/2.;
    right.position.y = params.right + thickness/2.;
    floor.position.z = params.floor - thickness/2.;
    roof.position.z = params.roof + thickness/2.;

    var horiz_walls = [floor,roof];
    var vert_walls = [left,right,front,back];

    vert_walls.forEach( function(mesh) {
        mesh.scale.x = 2*params.L_cur + 2*thickness;
        mesh.scale.z = 2*(params.L_cur-params.H_cur) + 2*thickness;
    });

    horiz_walls.forEach( function(mesh) {
        mesh.scale.x = 2*params.L_cur + 2*thickness;
        mesh.scale.z = 2*params.L_cur + 2*thickness;
    });

}

function animate() {
    requestAnimationFrame( animate );
    SPHERES.move_spheres(S,params);

    S.simu_step_forward(5);
    //let angle = S.simu_getTime()*params.omega;
    //camera.up.set(-Math.cos(angle), -Math.sin(angle), 0);
    controls.update();

    renderer.render( scene, camera );
}

async function NDDEMPhysics() {

    if ( 'DEMCGND' in window === false ) {

        console.error( 'NDDEMPhysics: Couldn\'t find DEMCGND.js' );
        return;

    }

    await DEMCGND().then( (NDDEMCGLib) => {
        if ( params.dimension == 3 ) {
            S = new NDDEMCGLib.DEMCG3D (params.N);
        }
        else if ( params.dimension == 4 ) {
            S = new NDDEMCGLib.DEMCG4D (params.N);
        }
        else if ( params.dimension == 5 ) {
            S = new NDDEMCGLib.DEMCG5D (params.N);
        }
        finish_setup();
    } );

    function finish_setup() {
        S.simu_interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
        S.simu_interpret_command("radius -1 0.5");
        S.simu_interpret_command("mass -1 1");
        S.simu_interpret_command("auto rho");
        // S.simu_interpret_command("auto radius uniform "+params.r_min+" "+params.r_max);
        S.simu_interpret_command("auto mass");
        S.simu_interpret_command("auto inertia");

        S.simu_interpret_command("boundary 0 WALL 0 "+String(params.H));
        S.simu_interpret_command("boundary 1 WALL 0 "+String(params.L));
        S.simu_interpret_command("boundary 2 WALL -"+String(params.D)+" "+String(params.D));
        
        // S.simu_interpret_command("auto location randomsquare");
        S.simu_interpret_command("auto location randomdrop");
        S.simu_interpret_command("gravity -100 0 0 "); // intensity, omega, rotdim0, rotdim1

        S.simu_interpret_command("set Kn 2e5");
        S.simu_interpret_command("set Kt 8e4");
        S.simu_interpret_command("set GammaN 75");
        S.simu_interpret_command("set GammaT 75");
        S.simu_interpret_command("set Mu 0.25");
        S.simu_interpret_command("set Mu_wall 1");
        S.simu_interpret_command("set T 150");
        S.simu_interpret_command("set dt 0.001");
        S.simu_interpret_command("auto skin");
        S.simu_finalise_init () ;
    }
}
