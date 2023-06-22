import css from "../css/main.css";

import * as THREE from "three";
// import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
// import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
// import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';
import * as RAYCAST from '../libs/RaycastHandler.js';
// import * as CGHANDLER from '../libs/CGHandler.js';

let info_div = document.createElement("div")
info_div.innerHTML = "Throw the ball"
info_div.style.color = "white";
info_div.style.position = "absolute";
info_div.style.left = "15px";
info_div.style.top = "15px";
info_div.style.padding = "10px";

document.body.appendChild(info_div);

let reset_div = document.createElement("div")
reset_div.innerHTML = "Reset"
reset_div.style.color = "black";
reset_div.style.backgroundColor = "white";
reset_div.style.borderRadius = "5px";
reset_div.style.position = "absolute";
reset_div.style.right = "15px";
reset_div.style.top = "15px";
reset_div.style.padding = "10px";
// document.body.appendChild(reset_div);

// reset_div.addEventListener("click", reset_particle);

let boundary_select = document.createElement("select");
boundary_select.style.position = "absolute";
boundary_select.style.right = "15px";
boundary_select.style.top = "15px";
boundary_select.style.font_size = "large";
boundary_select.style.padding = "5px 10px";
boundary_select.style.border_radius = "11px";
boundary_select.style.text_align = "center";
let options = [
               'Square',
               'Triangle',
               'Circle',
               'Ellipse',
            ];
//Create and append the options
for (var i = 0; i < options.length; i++) {
    var option = document.createElement("option");
    option.value = options[i];
    option.text = options[i];
    boundary_select.appendChild(option);
}

document.body.appendChild(boundary_select);
boundary_select.addEventListener("change", update_boundary);


var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();
let NDDEMCGLib;
let camera, scene, renderer, stats, panel, controls;
let gui;
let S;

var params = {
    dimension: 2,
    L: 0.06, //system size
    N: 1,
    zoom: 1.3,
    g_mag: 1e3,
    theta: 0, // slope angle in DEGREES
    d4: {cur:0},
    r_max: 0.005,
    r_min: 0.005,
    particle_density: 2700,
    freq: 0.05,
    new_line: false,
    shear_rate: 10,
    lut: 'White',
    cg_field: 'Density',
    quality: 5,
    cg_width: 50,
    cg_height: 50,
    cg_opacity: 0.8,
    cg_window_size: 3,
    particle_opacity: 1,
    F_mag_max: 0.005,
    aspect_ratio: undefined,
    ellipse_ratio: 0.75,
    boundary : 'Square',
}

// params.aspect_ratio = window.innerHeight / window.innerWidth;
let window_aspect_ratio = window.innerHeight / window.innerWidth;
params.aspect_ratio = 1;

params.average_radius = (params.r_min + params.r_max)/2.;
params.thickness = params.average_radius;

params.particle_volume = Math.PI*Math.pow(params.average_radius,2);

params.particle_mass = params.particle_volume * params.particle_density;

if ( urlParams.has('cg_width') ) { params.cg_width = parseInt(urlParams.get('cg_width')); }
if ( urlParams.has('cg_height') ) { params.cg_height = parseInt(urlParams.get('cg_height')); }
if ( urlParams.has('cg_opacity') ) { params.cg_opacity = parseFloat(urlParams.get('cg_opacity')); }
if ( urlParams.has('particle_opacity') ) { params.particle_opacity = parseFloat(urlParams.get('particle_opacity')); }

if ( urlParams.has('quality') ) { params.quality = parseInt(urlParams.get('quality')); }

SPHERES.createNDParticleShader(params).then( init );

async function init() {

    await NDDEMCGPhysics();

    // camera = new THREE.PerspectiveCamera( 50, window.innerWidth / window.innerHeight, 0.1, 1000 );
    // var aspect = window.innerWidth / window.innerHeight;
    console.log(window_aspect_ratio);
    let left, top;
    if ( window_aspect_ratio > 1 ) {
        left = params.L*params.zoom;
        top = params.L*params.zoom*window_aspect_ratio;;
    } else {
        left = params.L*params.zoom/window_aspect_ratio;
        top = params.L*params.zoom;
    }
    camera = new THREE.OrthographicCamera(
        -left,left,top,-top,
        -1000,
        1000
    );
    camera.position.set( 0, 0, -5*params.L );
    camera.up.set(0, 1, 0);
    camera.lookAt(0,0,0);
    // console.log(camera)

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x111111 );

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add( hemiLight );

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set( 5, -5, -5 );
    dirLight.castShadow = true;
    scene.add( dirLight );

    update_boundary();
    SPHERES.add_spheres(S,params,scene);

    RAYCAST.update_world(S, camera, params)
    RAYCAST.add_ghosts(scene, 2000, params.average_radius/4., 0xeeeeee);


    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;

    var container = document.getElementById( 'canvas' );
    container.appendChild( renderer.domElement );

    window.addEventListener( 'resize', onWindowResize, false );
    

    animate();
}

function onWindowResize(){
    window_aspect_ratio = window.innerHeight / window.innerWidth;
    let left, top;
    if ( window_aspect_ratio > 1 ) {
        left = params.L*params.zoom;
        top = params.L*params.zoom*window_aspect_ratio;;
    } else {
        left = params.L*params.zoom/window_aspect_ratio;
        top = params.L*params.zoom;
    }
    camera.left = -left;
    camera.right = left;
    camera.bottom = -top;
    camera.top = top;

    camera.updateProjectionMatrix();
    renderer.setSize( window.innerWidth, window.innerHeight );
}


function animate() {
    requestAnimationFrame( animate );
    SPHERES.move_spheres(S,params);
    // RAYCAST.animate_locked_particle(S, camera, SPHERES.spheres, params);

    S.simu_step_forward(50);
    // CGHANDLER.update_2d_cg_field(S,params);
    RAYCAST.update_ghosts();
    // SPHERES.draw_force_network(S, params, scene);
    renderer.render( scene, camera );
    // if ( Math.abs(SPHERES.spheres.children[0].position.x) > params.L*params.aspect_ratio ) {S.simu_fixParticle(0,[0,0]); }
    // if ( Math.abs(SPHERES.spheres.children[0].position.y) > params.L ) {S.simu_fixParticle(0,[0,0]); }
}

function update_boundary() {
    params.boundary = boundary_select.value; // yay duplication!

    if ( S !== undefined ) {
        S.simu_finalise();
        reset_particle();
    }
    
    if ( params.dimension == 2 ) {
        S = new NDDEMCGLib.DEMCG2D (params.N);
    }
    else if ( params.dimension == 3 ) {
        S = new NDDEMCGLib.DEMCG3D (params.N);
    }
    else if ( params.dimension == 4 ) {
        S = new NDDEMCGLib.DEMCG4D (params.N);
    }
    else if ( params.dimension == 5 ) {
        S = new NDDEMCGLib.DEMCG5D (params.N);
    }
    finish_setup();

    WALLS.remove_all_walls(scene);

    if ( boundary_select.value === "Square" ) {
        // console.log(scene);
        WALLS.add_back(params, scene);
        WALLS.add_left(params, scene);
        WALLS.add_right(params, scene);
        WALLS.add_front(params, scene);
        WALLS.back.scale.y = params.thickness;
        
        WALLS.left.scale.x = 2*params.L*params.aspect_ratio + 2*params.thickness;
        WALLS.right.scale.x = 2*params.L*params.aspect_ratio + 2*params.thickness;

        WALLS.back.scale.x = 2*params.L + 2*params.thickness;
        WALLS.front.scale.x = 2*params.L + 2*params.thickness;

        WALLS.back.position.x = -params.L*params.aspect_ratio - params.thickness/2.;
        WALLS.front.position.x = params.L*params.aspect_ratio + params.thickness/2.;

    } else if ( boundary_select.value === 'Circle' ) {
        params.R = params.L;
        WALLS.add_circle_wall(params, scene);
    } else if ( boundary_select.value === 'Triangle' ) {
        WALLS.add_left(params, scene); // this is the bottom wall
        WALLS.left.scale.x = 2*params.L + 0.5*params.thickness;
        WALLS.left.position.y = -params.H/2.;
       
        WALLS.add_back(params, scene); // this is the slanted wall on the right
        WALLS.back.scale.x = 2*params.L + 0.5*params.thickness;
        WALLS.back.rotateZ(Math.PI/6.);
        WALLS.back.position.x = params.L/2.;
        WALLS.back.position.y = 0;

        WALLS.add_front(params, scene); // this is the slanted wall on the right
        WALLS.front.scale.x = 2*params.L + 0.5*params.thickness;
        WALLS.front.rotateZ(-Math.PI/6.);
        WALLS.front.position.x = -params.L/2.;
        WALLS.front.position.y = 0;
        
    } else if ( boundary_select.value === 'Ellipse' ) {
        params.R = params.L;
        WALLS.add_circle_wall(params, scene);
        WALLS.left.scale.y = params.ellipse_ratio;
    }
    WALLS.wall_material.wireframe = false;
    reset_particle();
    RAYCAST.update_world(S, camera, params);
}

function reset_particle(){
    S.simu_fixParticle(0,[0,0]);
    S.simu_setVelocity(0,[0,0]);
    // S.simu_setExternalForce(0,[0,0]);
    RAYCAST.reset_ghosts();
}

async function NDDEMCGPhysics() {

    if ( 'DEMCGND' in window === false ) {

        console.error( 'NDDEMPhysics: Couldn\'t find DEMCGND.js' );
        return;

    }

    await DEMCGND().then( (lib) => {
        NDDEMCGLib = lib;
    } );
}


function finish_setup() {
    S.simu_interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
    S.simu_interpret_command("radius -1 0.5");
    let m;
    if ( params.dimension === 2) {
        m = Math.PI*0.5*0.5*params.particle_density;
    } else {
        m = 4./3.*Math.PI*0.5*0.5*0.5*params.particle_density;
    }

    S.simu_interpret_command("mass -1 " + String(m));
    S.simu_interpret_command("auto rho");
    S.simu_interpret_command("auto radius uniform "+params.r_min+" "+params.r_max);
    S.simu_interpret_command("auto mass");
    S.simu_interpret_command("auto inertia");
    S.simu_interpret_command("auto skin");

    S.simu_interpret_command("gravity 0 " + "0 ".repeat(params.dimension - 2))

    S.simu_interpret_command("location 0 0 0");

    let tc = 1e-3;
    let rest = 1.0 - 1e-10; // super low restitution coeff to dampen out quickly
    let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, params.particle_mass)

    S.simu_interpret_command("set Kn " + String(vals.stiffness));
    S.simu_interpret_command("set Kt " + String(0.8*vals.stiffness));
    S.simu_interpret_command("set GammaN " + String(vals.dissipation));
    S.simu_interpret_command("set GammaT " + String(vals.dissipation));
    S.simu_interpret_command("set Mu 0.0");
    S.simu_interpret_command("set Mu_wall 0.0");
    // S.simu_interpret_command("set damping 0.00001");
    S.simu_interpret_command("set T 150");
    S.simu_interpret_command("set dt " + String(tc/20));
    S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
    S.simu_interpret_command("auto skin");

    if ( boundary_select.value === "Square" ) {
        S.simu_interpret_command("boundary 0 WALL -"+String(params.L*params.aspect_ratio)+" "+String(params.L*params.aspect_ratio));
        S.simu_interpret_command("boundary 1 WALL -"+String(params.L)+" "+String(params.L));

    } else if ( boundary_select.value === "Circle" ) { 
        S.simu_interpret_command("boundary "+String(params.dimension)+" SPHERE "+String(params.L)+" 0 0"); // add a sphere!
        S.simu_interpret_command("boundary 0 WALL -"+String(2*params.L)+" "+String(2*params.L));
        S.simu_interpret_command("boundary 1 WALL -"+String(2*params.L)+" "+String(2*params.L));
    } else if ( boundary_select.value === "Ellipse" ) {
        S.simu_interpret_command("boundary 0 WALL -"+String(2*params.L)+" "+String(2*params.L));
        S.simu_interpret_command("boundary 1 WALL -"+String(2*params.L)+" "+String(2*params.L));
        S.simu_interpret_command("boundary "+String(params.dimension)+" ELLIPSE "+String(params.L)+ " " + String(params.L*params.ellipse_ratio) + " 0 0"); // radius x, radius y, centre x, centre y
        S.simu_interpret_command("set gradientdescent_gamma 100");
        S.simu_interpret_command("set gradientdescent_tol 1e-5");
    } else if ( boundary_select.value === "Triangle" ) {
        params.H = Math.sqrt(3*params.L*params.L);
        S.simu_interpret_command("location 0 0 "+String(-params.H/6.));
        S.simu_interpret_command('mesh string {"dimension":2,"objects":[{"dimensionality":1,"vertices":[['+String(-params.L)+','+String(-params.H/2.)+'],[0,'+String(params.H/2)+']]},{"dimensionality":1,"vertices":[['+String(params.L)+','+String(-params.H/2.)+'],[0,'+String(params.H/2.)+']]}]}');
        S.simu_interpret_command("boundary 0 WALL -"+String(10*params.L)+" "+String(10*params.L));
        S.simu_interpret_command("boundary 1 WALL -"+String(params.H/2.)+" "+String(10*params.L));
    }

    S.simu_finalise_init () ;
}
