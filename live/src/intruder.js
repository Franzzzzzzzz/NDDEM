import css from "../css/main.css";
import Plotly from "plotly.js-dist";

import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';
import { Lut } from 'three/examples/jsm/math/Lut.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';
import * as RAYCAST from '../libs/RaycastHandler.js';
import * as AUDIO from '../libs/audio.js';
import * as CGHANDLER from '../libs/CGHandler.js';
import { update } from "@tweenjs/tween.js";


// let info_div = document.createElement("div")
// info_div.innerHTML = "Click on a particle to grab it"
// info_div.style.color = "white";
// info_div.style.position = "absolute";
// info_div.style.left = "20px";
// info_div.style.top = "20px";
// document.body.appendChild(info_div);

let iter = 0;

var urlParams = new URLSearchParams(window.location.search);

let camera, scene, renderer, stats, panel, controls;
let gui;
let S;
let n_wall;

let graph_fraction = 0.5;
document.getElementById("stats").style.width = String(100*graph_fraction) + '%';
document.getElementById("canvas").style.width = String(100*(1-graph_fraction)) + '%';


var params = {
    dimension: 2,
    L: 16*0.004, //system size
    initial_density: 0.85,
    zoom: 700,
    aspect_ratio: 2,
    // paused: false,
    // g_mag: 1e3,
    // theta: 0, // slope angle in DEGREES
    d4: {cur:0},
    r_max: 0.005,
    r_min: 0.003,
    particle_density: 2700,
    // freq: 0.05,
    // new_line: false,
    shear_rate: 1e-1,
    lut: 'None',
    // lut: 'White',
    cg_field: 'Density',
    quality: 5,
    cg_width: 50,
    cg_height: 25,
    cg_opacity: 0.8,
    cg_window_size: 3,
    particle_opacity: 0.5,
    F_mag_max: 1e4,
    audio: false,
    audio_sensitivity : 1,
    intruder : {
        size_ratio : 4,
        velocity : 1e-1,
        vibration : {
            amplitude : 1e-3,
            frequency : 1e2,
            mode : 'Orthogonal'
        },
    },
    target_pressure : 1e4,
}

let rainbow    = new Lut("rainbow", 512); // options are rainbow, cooltowarm and blackbody
let cooltowarm = new Lut("cooltowarm", 512); // options are rainbow, cooltowarm and blackbody
let blackbody  = new Lut("blackbody", 512); // options are rainbow, cooltowarm and blackbody

params.average_radius = (params.r_min + params.r_max)/2.;
params.thickness = params.average_radius;
params.H = params.L;

if ( urlParams.has('dimension') ) {
    params.dimension = parseInt(urlParams.get('dimension'));
}
if ( params.dimension === 2) {
    params.particle_volume = Math.PI*Math.pow(params.average_radius,2);
}
else if ( params.dimension === 3 ) {
    params.particle_volume = 4./3.*Math.PI*Math.pow(params.average_radius,3);
}
else if ( params.dimension === 4) {

    params.L = 2.5;
    params.N = 300
    params.particle_volume = Math.PI*Math.PI*Math.pow(params.average_radius,4)/2.;
}

params.N = Math.ceil( params.initial_density*4*params.L*params.L*params.aspect_ratio / params.particle_volume );

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
    var aspect = window.innerWidth / window.innerHeight * graph_fraction;
    camera = new THREE.OrthographicCamera(
        (-100 * aspect) / params.zoom,
        (100 * aspect) / params.zoom,
        100 / params.zoom,
        -100 / params.zoom,
        -1000,
        1000
    );
    camera.position.set( 0, 0, -5*params.L );
    camera.up.set(0, 1, 0);
    camera.lookAt(0,0,0);
    // console.log(camera)

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x111 );

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add( hemiLight );

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set( 5, -5, -5 );
    dirLight.castShadow = true;
    scene.add( dirLight );

    SPHERES.add_spheres(S,params,scene);

    CGHANDLER.add_cg_mesh(2*params.L*params.aspect_ratio, 2*params.L, scene);

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth*(1-graph_fraction), window.innerHeight );
    renderer.shadowMap.enabled = true;

    var container = document.getElementById( 'canvas' );
    container.appendChild( renderer.domElement );

    gui = new GUI();
    gui.width = 320;

    if ( params.dimension == 4 ) {
        gui.add( params.d4, 'cur', -params.L,params.L, 0.001)
            .name( 'D4 location').listen()
            .onChange( function () {
                if ( urlParams.has('stl') ) {
                    meshes = renderSTL( meshes, NDsolids, scene, material, params.d4.cur );
                }
            });
    }
    // gui.add ( params, 'particle_opacity', 0, 1).name('Particle opacity').listen().onChange( () => SPHERES.update_particle_material(params,
        // lut_folder
    // ));
    gui.add ( params, 'cg_opacity', 0, 1).name('Coarse grain opacity').listen();
    gui.add ( params, 'cg_field', ['Density', 'Velocity', 'Pressure', 'Shear stress']).name('Field').listen();
    gui.add ( params, 'cg_window_size', 0.5, 6).name('Window size (radii)').listen().onChange( () => {
        update_cg_params(S, params);
    });
    // gui.add ( params, 'shear_rate', 0, 2, 0.001).name('Shear rate').listen().onChange( update_wall_particle_velocities );
    gui.add ( params.intruder, 'size_ratio', 0.1,10).name('Intruder size ratio').listen().onChange( update_intruder );
    gui.add ( params.intruder, 'velocity', 0,1).name('Intruder velocity').listen().onChange( update_intruder );
    gui.add ( params.intruder.vibration, 'amplitude', 0,1e-2).name('Intruder vib amp').listen();
    gui.add ( params.intruder.vibration, 'frequency', 0,1e3).name('Intruder vib freq').listen();
    gui.add ( params.intruder.vibration, 'mode', ['Orthogonal', 'Parallel', 'Rotation', 'Radial']).name('Intruder vib mode').listen();
    gui.add ( params, 'target_pressure', 1e2,1e4).name('Target pressure').listen();
    
    
    gui.add ( params, 'audio_sensitivity', 1, 1e3, 1).name('Audio sensitivity');
    gui.add ( params, 'audio').name('Audio').listen().onChange(() => {
        if ( AUDIO.listener === undefined ) {
            AUDIO.make_listener( camera );
            AUDIO.add_fixed_sound_source ( [0,0,0] );
            // SPHERES.add_normal_sound_to_all_spheres();
        } else {
            // AUDIO.remove_listener( camera ); // doesn't do anything at the moment...
            // SPHERES.mute_sounds();
        }
    });

    window.addEventListener( 'resize', onWindowResize, false );


    controls = new OrbitControls( camera, container );
    controls.update();

    make_graph();

    animate();
}

function make_graph() {
    let { data, layout } = LAYOUT.plotly_2d_graph('Time', 'Force', ['Horizontal Force', 'Vertical Force'])
    // layout.yaxis.range = [0,1e5];
    // layout.yaxis.autorange = false;
    Plotly.newPlot('stats', data, layout);
}

function update_graph(x1,x2,y1,y2) {
    Plotly.extendTraces('stats', {
        'x': [[x1],[x2]],
        'y': [[y1],[y2]],
    }, [0,1])
}

function onWindowResize(){

    // camera.aspect = window.innerWidth / window.innerHeight;
    // camera.updateProjectionMatrix();
    var aspect = window.innerWidth / window.innerHeight;
    camera.left = (-100 * aspect) / params.zoom;
    camera.right = (100 * aspect) / params.zoom;
    camera.bottom = 100 / params.zoom;
    camera.top = -100 / params.zoom;
    camera.updateProjectionMatrix();
    renderer.setSize( window.innerWidth, window.innerHeight );
}

function animate() {
    
    requestAnimationFrame( animate );
    SPHERES.move_spheres(S,params);
    if ( AUDIO.listener !== undefined ){
        SPHERES.update_fixed_sounds(S, params);
    }
    RAYCAST.animate_locked_particle(S, camera, SPHERES.spheres, params);
    if ( !params.paused ) {
        iter += 1;
        // let v = S.simu_getVelocity();
        // console.log(v);

        S.simu_step_forward(15);
        let t = S.simu_getTime();
        CGHANDLER.update_2d_cg_field(S, params);
        let forcing_velocity = 2*Math.PI*params.intruder.vibration.amplitude*params.intruder.vibration.frequency * Math.sin( 2*Math.PI*params.intruder.vibration.frequency * t );
        // console.log(vertical_forcing_velocity);
        if ( params.intruder.vibration.mode === 'Orthogonal') {
            S.simu_setVelocity(0,[params.intruder.velocity,forcing_velocity]);
            S.simu_setAngularVelocity(0,[0]);
            S.simu_setRadius(0,params.average_radius*params.intruder.size_ratio);
            SPHERES.update_radii(S);
        } else if ( params.intruder.vibration.mode === 'Parallel') {
            S.simu_setVelocity(0,[params.intruder.velocity + forcing_velocity, 0]);
            S.simu_setAngularVelocity(0,[0]);
            S.simu_setRadius(0,params.average_radius*params.intruder.size_ratio);
            SPHERES.update_radii(S);
        } else if ( params.intruder.vibration.mode === 'Rotation') {
            S.simu_setVelocity(0,[params.intruder.velocity, 0]);
            S.simu_setAngularVelocity(0,[forcing_velocity/params.average_radius]);
            S.simu_setRadius(0,params.average_radius*params.intruder.size_ratio);
            SPHERES.update_radii(S);
        } else if ( params.intruder.vibration.mode === 'Radial') {
            S.simu_setVelocity(0,[params.intruder.velocity, 0]);
            S.simu_setAngularVelocity(0,[0]);
            let amp = params.intruder.vibration.amplitude * Math.sin( 2*Math.PI*params.intruder.vibration.frequency * t );
            S.simu_setRadius(0,params.average_radius*params.intruder.size_ratio + amp);
            SPHERES.update_radii(S);
        }
        // S.simu_setFrozen(0);
        // if ( iter % 10 == 0 ) {
        // if ( t > 1e-2 ) {
            // params.current_pressure = CGHANDLER.get_mean_pressure(S, params);
            // }
            // WALLS.update_damped_wall(params, S, 5*1e-3/20., 0);
        // }
        // console.log(SPHERES.F[0]);
        // let P = WALLS.get_wall_pressure(S,SPHERES.F,1,n_wall+1,n_wall*2*params.average_radius,1);
        // console.log(P);
        if ( t > 1e-2 ) {
            let F = S.simu_getWallForce();
            if ( F.length === 4 ) {
                let mean_vert_wall_normal = (-F[2][1] + F[3][1])/2.
                let current_pressure = mean_vert_wall_normal/(2*params.L*params.aspect_ratio);
                WALLS.update_damped_wall(current_pressure, params.target_pressure, params, S, 1e-6, 1);
                let Fc = S.simu_getContactForce();
                let Fn = [0,0];
                for ( let i = 0; i < Fc.length; i++ ) {
                    if ( Fc[i][0] === 0 ) {
                        Fn[0] += Fc[i][2]
                        Fn[1] += Fc[i][3];
                    } else if ( Fc[i][1] === 0 ) {
                        Fn[0] -= Fc[i][2]
                        Fn[1] -= Fc[i][3];
                    }
                }
                update_graph(t,t,-Fn[0],Fn[1]);
            //     params.H += 1e-7*(mean_vert_wall_normal/(2*params.L*params.aspect_ratio) - params.target_pressure);
            //     S.simu_setBoundary(1, [-params.H, params.H]);
            //     console.log(mean_vert_wall_normal, params.target_pressure)
            // }
            }
        }
        

    }


    SPHERES.draw_force_network(S, params, scene);
    renderer.render( scene, camera );
}
function update_cg_params(S, params) {
    var cgparam ={} ;
    cgparam["file"]=[{"filename":"none", "content": "particles", "format":"interactive", "number":1}] ;
    cgparam["boxes"]=[params.cg_width,params.cg_height] ;
    // cgparam["boundaries"]=[[-params.L,-params.L,-params.L],[params.L,params.L,params.L]] ;
    cgparam["boundaries"]=[
        [-params.L*params.aspect_ratio,-params.L],
        [ params.L*params.aspect_ratio, params.L]] ;
    cgparam["window size"]=params.cg_window_size*params.average_radius ;
    cgparam["skip"]=0;
    cgparam["max time"]=1 ;
    cgparam["time average"]="None" ;
    cgparam["fields"]=["RHO", "VAVG", "TC","Pressure","ShearStress"] ;
    cgparam["periodicity"]=[false,false];
    cgparam["window"]="Lucy2D";
    cgparam["dimension"]=2;


    // console.log(JSON.stringify(cgparam)) ;
    S.cg_param_from_json_string(JSON.stringify(cgparam)) ;
    S.cg_setup_CG() ;
}

async function NDDEMCGPhysics() {

    if ( 'DEMCGND' in window === false ) {

        console.error( 'NDDEMPhysics: Couldn\'t find DEMCGND.js' );
        return;

    }

    await DEMCGND().then( (NDDEMCGLib) => {
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
    } );


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

        // console.log("boundary 0 MOVINGWALL -"+String(params.L)+" "+String(params.L) + " 0 " + String(2*params.L*params.shear_rate))
        // S.simu_interpret_command("boundary 0 MOVINGSIDEWAYSWALL -"+String(params.L)+" "+String(params.L) + " 0 " + String(-2*params.L*params.shear_rate));
        S.simu_interpret_command("boundary 0 PBC -"+String(params.aspect_ratio*params.L)+" "+String(params.aspect_ratio*params.L));
        S.simu_interpret_command("boundary 1 WALL -"+String(params.L)+" "+String(params.L));
        
        // S.simu_interpret_command("gravity 0 -1 ");

        S.simu_interpret_command("auto location randomdrop");

        let tc = 1e-3;
        let rest = 0.5; // super low restitution coeff to dampen out quickly
        let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, params.particle_mass)

        S.simu_interpret_command("set Kn " + String(vals.stiffness));
        S.simu_interpret_command("set Kt " + String(0.8*vals.stiffness));
        S.simu_interpret_command("set GammaN " + String(vals.dissipation));
        S.simu_interpret_command("set GammaT " + String(vals.dissipation));
        S.simu_interpret_command("set Mu 0.5");
        S.simu_interpret_command("set Mu_wall 1");
        S.simu_interpret_command("set damping 0.01");
        S.simu_interpret_command("set T 1000000");
        S.simu_interpret_command("set dt " + String(tc/20));
        S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
        // S.simu_interpret_command("auto skin");

        // now fix the intruder
        S.simu_interpret_command("location 0 0 0");
        update_intruder();

        // // now fix the wall roughness
        // n_wall = Math.ceil(2*params.aspect_ratio*params.L/(params.average_radius*2));
        // for ( var i=1; i<n_wall+1; i++ ) {
        //     let d = params.average_radius - params.aspect_ratio**params.L + i*params.average_radius*2; // location in x1 of these particles
        //     // console.log("radius " + String(i) + " " + String(params.average_radius));
        //     S.simu_interpret_command("location " + String(i) + " " + String(d) + " " + String(-params.L));

        //     S.simu_setRadius(i,params.average_radius);
        //     S.simu_setVelocity(i,[params.shear_rate*2*params.L,0]);
        //     S.simu_setFrozen(i);

        //     S.simu_interpret_command("location " + String(n_wall+i) + " " + String(d) + " " + String(params.L));
        //     S.simu_setFrozen(n_wall+i);
        //     S.simu_setRadius(n_wall+i,params.average_radius);
        // }

        S.simu_finalise_init () ;

        // let v = S.simu_getVelocity();
        // console.log(v);
        update_cg_params(S, params);

    }
}

function update_intruder() {
    S.simu_setRadius(0,params.average_radius*params.intruder.size_ratio);
    SPHERES.update_radii(S);
    // S.simu_setVelocity(0,[params.intruder.velocity,0]);
    S.simu_setFrozen(0);
    S.simu_interpret_command("auto mass");
    S.simu_interpret_command("auto inertia");
    S.simu_interpret_command("auto skin");
}