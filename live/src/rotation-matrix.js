import css from "../css/main.css";

import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';
import { Lut } from 'three/examples/jsm/math/Lut.js';

import * as SPHERES from "../libs/SphereHandler.js"

var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel, controls;
let gui;
let S;
let cg_mesh, colorbar_mesh;

var params = {
    dimension: 4,
    radius: 0.5,
    L: 2, //system size
    N: 12,
    zoom: 50,
    paused: false,
    d4: {cur:0},
    lut: 'None',
    quality: 7,
}

if ( urlParams.has('dimension') ) {
    params.dimension = parseInt(urlParams.get('dimension'));
}

if ( params.dimension === 3 ) {
    params.N = 6;
}

SPHERES.createNDParticleShader(params).then( init );

async function init() {

    await NDDEMCGPhysics();

    // camera = new THREE.PerspectiveCamera( 50, window.innerWidth / window.innerHeight, 0.1, 1000 );
    var aspect = window.innerWidth / window.innerHeight;
    camera = new THREE.OrthographicCamera(
        (-100 * aspect) / params.zoom,
        (100 * aspect) / params.zoom,
        100 / params.zoom,
        -100 / params.zoom,
        -1000,
        1000
    );
    camera.position.set( 0, 0, -5*params.L );
    camera.up.set(0, 0, 1);
    camera.lookAt(0,0,0);
    // console.log(camera)

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x111 );

    // const hemiLight = new THREE.HemisphereLight();
    // hemiLight.intensity = 0.35;
    // scene.add( hemiLight );
    //
    // const dirLight = new THREE.DirectionalLight();
    // dirLight.position.set( 5, -5, -5 );
    // dirLight.castShadow = true;
    // scene.add( dirLight );

    SPHERES.add_spheres(S,params,scene);

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );

    var container = document.getElementById( 'canvas' );
    container.appendChild( renderer.domElement );

    if ( params.dimension == 4 ) {
        gui = new GUI();
        gui.width = 320;
        gui.add( params.d4, 'cur', -params.radius,params.radius, 0.001).name( 'D4 location').listen()
    }

    window.addEventListener( 'resize', onWindowResize, false );

    animate();
}

function onWindowResize(){

    var aspect = window.innerWidth / window.innerHeight;
    renderer.setSize( window.innerWidth, window.innerHeight );
}

function animate() {
    requestAnimationFrame( animate );
    SPHERES.move_spheres(S,params);
    S.simu_step_forward(5);
    renderer.render( scene, camera );
}


async function NDDEMCGPhysics() {

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
        S.simu_interpret_command("radius -1 0.45");
        S.simu_interpret_command("mass -1 1");
        S.simu_interpret_command("auto rho");
        S.simu_interpret_command("auto inertia");

        S.simu_interpret_command("boundary 0 PBC -"+String(params.L)+" "+String(params.L));
        S.simu_interpret_command("boundary 1 PBC -"+String(params.L)+" "+String(params.L));
        S.simu_interpret_command("boundary 2 PBC -"+String(params.r_max)+" "+String(params.r_max));
        S.simu_interpret_command("gravity 0 0 " + "0 ".repeat(params.dimension - 3))

        if ( params.dimension == 4 ) {
            S.simu_interpret_command("boundary 3 PBC -"+String(params.L)+" "+String(params.L));

            S.simu_interpret_command("location 0 -0.5 1.5 0 0");
            S.simu_interpret_command("location 1 0.5 1.5 0 0");
            S.simu_interpret_command("location 2 1.5 1.5 0 0");
            S.simu_interpret_command("location 3 0.5 0.5 0 0");
            S.simu_interpret_command("location 4 1.5 0.5 0 0");
            S.simu_interpret_command("location 5 1.5 -0.5 0 0");
            S.simu_interpret_command("location 6 -1.5 0.5 0 0");
            S.simu_interpret_command("location 7 -1.5 -0.5 0 0");
            S.simu_interpret_command("location 8 -0.5 -0.5 0 0");
            S.simu_interpret_command("location 9 -1.5 -1.5 0 0");
            S.simu_interpret_command("location 10 -0.5 -1.5 0 0");
            S.simu_interpret_command("location 11 0.5 -1.5 0 0");
            S.simu_interpret_command("omega 0 0 0 0 0.1 0 0");
            S.simu_interpret_command("omega 1 0 0.1 0 0 0 0");
            S.simu_interpret_command("omega 2 0 0 0.1 0 0 0");
            S.simu_interpret_command("omega 3 -0.1 0 0 0 0 0");
            S.simu_interpret_command("omega 4 0 0 0 0 0.1 0");
            S.simu_interpret_command("omega 5 0 0 0 0 0 0.1");
            S.simu_interpret_command("omega 6 0 0 0 -0.1 0 0");
            S.simu_interpret_command("omega 7 0 -0.1 0 0 0 0");
            S.simu_interpret_command("omega 8 0.1 0 0 0 0 0");
            S.simu_interpret_command("omega 9 0 0 -0.1 0 0 0");
            S.simu_interpret_command("omega 10 0 0 0 0 -0.1 0");
            S.simu_interpret_command("omega 11 0 0 0 0 0 -0.1");
        }
        else if ( params.dimension === 3 ) {
            S.simu_interpret_command("location 0 0 1 0");
            S.simu_interpret_command("location 1 1 1 0");
            S.simu_interpret_command("location 2 1 0 0");
            S.simu_interpret_command("location 3 -1 0 0");
            S.simu_interpret_command("location 4 -1 -1 0");
            S.simu_interpret_command("location 5 0 -1 0");
            S.simu_interpret_command("omega 0 0 0 0.1");
            S.simu_interpret_command("omega 1 0 0.1 0");
            S.simu_interpret_command("omega 2 -0.1 0 0");
            S.simu_interpret_command("omega 3 0 0 -0.1");
            S.simu_interpret_command("omega 4 0 -0.1 0");
            S.simu_interpret_command("omega 5 0.1 0 0");
        }

        let tc = 0.5;
        let rest = 0.5; // super low restitution coeff to dampen out quickly
        let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, params.particle_mass)

        S.simu_interpret_command("set Kn " + String(vals.stiffness));
        S.simu_interpret_command("set Kt " + String(0.8*vals.stiffness));
        S.simu_interpret_command("set GammaN " + String(vals.dissipation));
        S.simu_interpret_command("set GammaT " + String(vals.dissipation));
        S.simu_interpret_command("set Mu 0.5");
        // S.simu_interpret_command("set damping 10");
        S.simu_interpret_command("set T 150");
        S.simu_interpret_command("set dt " + String(tc/10));
        S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
        S.simu_interpret_command("auto skin");
        S.simu_finalise_init () ;

    }
}
