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

let camera, scene, renderer, stats, panel, controls;
let gui;
let S;
let show_stats = false;

var params = {
    dimension: 2,
    boundary0: {
        type : 'WALL',
        min : -0.08,
        max : 0.08,
        max1 : 0.08,
        max2 : 200
    },
    boundary1: {
        type : 'WALL',
        min : -0.095,
        max : 0.095,
    },
    boundary2: {
        type : 'None',
        min : 0,
        max : 0,
    },

    // N: 400,
    gravity: true,
    theta : 10,
    paused: false,
    r_min: 0.0025,
    r_max: 0.005,
    omega: 5, // rotation rate
    lut: 'Size',
    quality: 5,
    vmax: 20, // max velocity to colour by
    omegamax: 20, // max rotation rate to colour by
    wallremoved: false,
    particle_density : 2700,
    phi_s : 0.5,
    target_nu : 0.58,
    show_gui : true,
    period : 5000
}


function setup() {
    let V_solid = params.target_nu*(params.boundary0.max - params.boundary0.min)*(params.boundary1.max - params.boundary1.min);
    let V_small = params.phi_s*V_solid;
    let V_large = (1-params.phi_s)*V_solid;
    params.N_small = Math.floor(V_small/(Math.PI*params.r_min*params.r_min));
    params.N_large = Math.floor(V_large/(Math.PI*params.r_max*params.r_max));
    params.N = params.N_small + params.N_large;
    params.number_ratio = params.N_small/params.N;

    params.average_radius = (params.r_min + params.r_max)/2.;

    params.particle_volume = Math.PI*Math.pow(params.average_radius,2);
    params.particle_mass = params.particle_volume*params.particle_density
    console.log(params.particle_mass)   

    params.F_mag_max = 50*params.density_ratio;
}

if ( urlParams.has('no_stats') ) {
    show_stats = false;
}
if ( urlParams.has('quality') ) { params.quality = parseInt(urlParams.get('quality')); }
if ( urlParams.has('theta') ) { params.theta = -parseFloat(urlParams.get('theta')); }

if ( urlParams.has('auto') ) {
    setInterval( () => {
        params.wallremoved = !params.wallremoved;
        update_wall();
    }, params.period);
}
if ( urlParams.has('nogui') ) { params.showgui = false; }
if ( urlParams.has('target_nu') ) { params.target_nu = parseFloat(urlParams.get('target_nu')); }

SPHERES.createNDParticleShader(params).then( init );

async function init() {
    setup();
    // console.log(r)
    await NDDEMPhysics();
    

    // camera = new THREE.PerspectiveCamera( 50, window.innerWidth / window.innerHeight, 0.1, 1000 );
    let aspect = window.innerWidth / window.innerHeight;
    let zoom = 1.5;
    let right = params.boundary0.min + (params.boundary1.max - params.boundary1.min)*aspect;
    camera = new THREE.OrthographicCamera( params.boundary0.min*zoom, zoom*right, zoom*params.boundary1.max, zoom*params.boundary1.min, -100, 1000 );
    camera.up.set(0, 1, 0);

    camera.position.x = (params.boundary0.max + params.boundary0.min)/2.;
    camera.position.y = (params.boundary1.max + params.boundary1.min)/2.;
    camera.position.z = (params.boundary0.max - params.boundary0.min)*2.;

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x000000 );

    const hemiLight = new THREE.AmbientLight();
    // hemiLight.intensity = 0.35;
    scene.add( hemiLight );


    // let axesHelper = new THREE.AxesHelper();
    // scene.add(axesHelper);

    SPHERES.add_spheres(S,params,scene);
    WALLS.createWalls(scene);

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;
    document.body.appendChild( renderer.domElement );
    
    // gui
    if ( params.showgui ) {
        gui = new GUI();

        gui.width = 300;

        gui.add ( params, 'wallremoved').name('Remove the dam wall').listen().onChange(update_wall);
    }


    controls = new OrbitControls( camera, renderer.domElement );
    controls.target.x = params.boundary0.min + (params.boundary0.max - params.boundary0.min)/2;
    controls.target.y = params.boundary1.min + (params.boundary1.max - params.boundary1.min)/2;
    controls.target.z = params.boundary2.min + (params.boundary2.max - params.boundary2.min)/2;

    controls.update();

    window.addEventListener( 'resize', onWindowResize, false );

    if ( show_stats ) { make_graph(); }

    animate();
}

function update_wall() {
    let theta;
    if ( params.wallremoved ) { 
        theta = params.theta*Math.PI/180;
        params.boundary0.max = params.boundary0.max2;
    } else { 
        theta = 0;
        params.boundary0.max = params.boundary0.max1;
    }
    S.simu_interpret_command("boundary 0 " + String(params.boundary0.type) + " " + String(params.boundary0.min) + " "+String(params.boundary0.max));
    S.simu_interpret_command("gravity " + String(-10*Math.sin(theta)) + " " + String(-10*Math.cos(theta)));
    if ( ! params.wallremoved ) {
        S.simu_interpret_command("auto location randomdrop");
        crazy_damp();
    }
}


function onWindowResize(){

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth, window.innerHeight );

}

function animate() {
    requestAnimationFrame( animate );
    SPHERES.move_spheres(S,params);

    S.simu_step_forward(15);
    controls.update();
    WALLS.update(params);

    renderer.render( scene, camera );
}

async function NDDEMPhysics() {

    if ( 'DEMCGND' in window === false ) {

        console.error( 'NDDEMPhysics: Couldn\'t find DEMCGND.js' );
        return;

    }

    await DEMCGND().then( (NDDEMCGLib) => {
        S = eval('new NDDEMCGLib.DEMCG' + String(params.dimension) + 'D (params.N)');
        finish_setup();
    } );

    function finish_setup() {
        S.simu_interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
        let m_small = Math.PI*params.r_min*params.r_min*params.particle_density;
        let m_large = Math.PI*params.r_max*params.r_max*params.particle_density;
        let min_mass = Math.min(m_small,m_large);
        for ( let i=0; i<params.N_small; i++ ) {
            S.simu_setRadius(i,params.r_min);
            S.simu_setMass(i,m_small);
        }
        for ( let i=params.N_small; i<params.N; i++ ) {
            S.simu_setRadius(i,params.r_max);
            S.simu_setMass(i,m_large);
        }
        
        S.simu_interpret_command("auto rho");
        S.simu_interpret_command("auto inertia");

        S.simu_interpret_command("boundary 0 " + String(params.boundary0.type) + " " + String(params.boundary0.min) + " "+String(params.boundary0.max));
        S.simu_interpret_command("boundary 1 " + String(params.boundary1.type) + " " + String(params.boundary1.min) + " "+String(params.boundary1.max));
        
        // S.simu_interpret_command("auto location randomsquare");
        S.simu_interpret_command("auto location randomdrop");
        // S.simu_interpret_command("gravity 0 -10");
        S.simu_interpret_command("gravity " + String(-10*Math.sin(0)) + " " + String(-10*Math.cos(0)));

        let tc = 2e-3;
        let rest = 0.5;
        let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, min_mass)

        S.simu_interpret_command("set Kn " + String(vals.stiffness));
        S.simu_interpret_command("set Kt " + String(0.8*vals.stiffness));
        S.simu_interpret_command("set GammaN " + String(vals.dissipation));
        S.simu_interpret_command("set GammaT " + String(vals.dissipation));
        S.simu_interpret_command("set Mu 1");
        S.simu_interpret_command("set Mu_wall 1");
        // S.simu_interpret_command("set damping 0.0001");
        S.simu_interpret_command("set T 150");
        S.simu_interpret_command("set dt " + String(tc/10));
        S.simu_interpret_command("auto skin");
        S.simu_finalise_init () ;

        crazy_damp();
        
    }
}

function crazy_damp() {
    for ( let t=0; t<10; t++) {
        S.simu_step_forward(5);

        for ( let i=0; i<params.N; i++ ) {
            S.simu_setVelocity(i,[0,0]);
            // S.simu_setOmega(i,[0,0]);
        }
    }
}
