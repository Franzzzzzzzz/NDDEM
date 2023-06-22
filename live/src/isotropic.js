import css from "../css/main.css";
import Plotly from "plotly.js-dist";

import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';

var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();
var startTime = clock.getElapsedTime()

let camera, scene, renderer, stats, panel;
let S;
let pressure = 0;
let sigma_xx = 0;
let sigma_yy = 0;
let sigma_zz = 0;
let density = 0;
let started = false;
let old_time = 0;
let new_time = 0;
let NDsolids, material, STLFilename;
let meshes = new THREE.Group();
let loading_direction = 1;

let graph_fraction = 0.5;
document.getElementById("stats").style.width = String(100*graph_fraction) + '%';
document.getElementById("canvas").style.width = String(100*(1-graph_fraction)) + '%';

var params = {
    dimension: 3,
    // L: 4, //system size
    L:0.025,
    H: 0.05,
    boxratio: 1.5,
    initial_packing_fraction: 0.4,
    N: 300,
    epsilonv: 0,
    gravity: false,
    paused: false,
    H_cur: 0,
    pressure_set_pt: 1e4,
    deviatoric_set_pt: 0,
    d4: {cur:0},
    r_max: 0.0033,
    r_min: 0.0027,
    freq: 0.05,
    new_line: false,
    loading_rate: 0.05,
    // max_vertical_strain: 0.3,
    target_stress: 5e3,
    unloading_stress:100,
    lut: 'None',
    quality: 5,
    vmax: 20, // max velocity to colour by
    omegamax: 20, // max rotation rate to colour by
    loading_active: false,
    particle_density: 2700, // kg/m^3
    particle_opacity: 1.0,
    hideaxes: false,
}
set_derived_properties();

function set_derived_properties() {
    params.average_radius = (params.r_min + params.r_max)/2.;
    params.thickness = 0.0001;//params.average_radius;

    params.particle_volume = 4./3.*Math.PI*Math.pow(params.average_radius,3);
    console.log('estimate of particle volume: ' + params.particle_volume*params.N)
    params.particle_mass = params.particle_volume * params.particle_density;
    params.L = Math.pow(params.particle_volume*params.N/params.initial_packing_fraction/params.boxratio, 1./3.)/2.;
    params.H = params.L*params.boxratio ;

    params.L_cur = params.L;
    params.H_cur = params.H;
    // params.packing_fraction = params.N*params.particle_volume/Math.pow(2*params.L,3);
    // params.back = -params.L;
    // params.front = params.L;
    // params.left = -params.L;
    // params.right = params.L;
    // params.floor = -params.L;
    // params.roof = params.L;
}

function reset_particles() {
    params.loading_active = false;
    params.vertical_displacement = 0;
    loading_direction = 1;
    set_derived_properties();
    SPHERES.randomise_particles_isotropic(params, S);
    WALLS.add_cuboid_walls(params,scene);
    WALLS.update_isotropic_wall(params, S, scene);
    setup_CG();
    started = false;
    startTime = clock.getElapsedTime()
}

if ( urlParams.has('dimension') ) {
    params.dimension = parseInt(urlParams.get('dimension'));
}
if ( params.dimension === 4) {
    params.L = 2.5;
    params.N = 300
    params.particle_volume = Math.PI*Math.PI*Math.pow(params.average_radius,4)/2.;
}
if ( urlParams.has('quality') ) { params.quality = parseInt(urlParams.get('quality')); }

SPHERES.createNDParticleShader(params).then( init );

async function init() {

    await NDDEMPhysics();
    camera = new THREE.PerspectiveCamera( 50, window.innerWidth*(1-graph_fraction) / window.innerHeight, 1e-5, 1000 );
    camera.position.set( 3*params.L, 3*params.L, 1.5*params.L );
    camera.up.set(0, 0, 1);
    camera.lookAt( 0, 0, 0 );

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x111 );

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add( hemiLight );

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set( 5, 5, 5 );
    dirLight.castShadow = true;
    dirLight.shadow.camera.zoom = 2;
    scene.add( dirLight );

    WALLS.add_cuboid_walls(params,scene);
    WALLS.update_isotropic_wall(params, S, scene);
    WALLS.add_scale(params, scene);

    SPHERES.add_spheres(S,params,scene);

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth*(1-graph_fraction), window.innerHeight );
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;

    var container = document.getElementById( 'canvas' );
    container.appendChild( renderer.domElement );

    let gui = new GUI();
    gui.width = 450;

    gui.add( params, 'initial_packing_fraction', 0.35, 0.55, 0.01 )
        .name( 'Initial solids fraction' ).listen().onChange( reset_particles );
    gui.add( params, 'loading_rate', 0.001, 0.1, 0.001).name( 'Volumetric strainrate (1/s)' );
    gui.add( params, 'target_stress', 0, 1e4).name( 'Target stress - loading' );
    gui.add( params, 'unloading_stress', 0, 1e4).name( 'Target stress - unloading' );
    if ( params.dimension == 4 ) {
        gui.add( params.d4, 'cur', -params.L,params.L, 0.001)
            .name( 'D4 location').listen()
            // .onChange( function () { WALLS.update_top_wall(params, S); } );
            .onChange( function () {
                if ( urlParams.has('stl') ) {
                    meshes = renderSTL( meshes, NDsolids, scene, material, params.d4.cur );
                }
            });
    }
    gui.add ( params, 'particle_opacity',0,1).name('Particle opacity').listen().onChange( () => SPHERES.update_particle_material(params,
        // lut_folder
    ));
    gui.add ( params, 'lut', ['None', 'Velocity', 'Rotation Rate' ]).name('Colour by')
        .onChange( () => SPHERES.update_particle_material(params,
            // lut_folder
        ) );
    /*gui.add ( params, 'gravity').name('Gravity').listen()
        .onChange( function() {
            if ( params.gravity === true ) {
                S.simu_interpret_command("gravity 0 0 -10 " + "0 ".repeat(params.dimension - 3)) }
            else {
                S.simu_interpret_command("gravity 0 0 0 " + "0 ".repeat(params.dimension - 3)) }
            });*/
    gui.add ( params, 'new_line').name('New loading path').listen().onChange( new_load_path );
    //gui.add ( params, 'paused').name('Paused').listen();
    gui.add(params, 'hideaxes').name("Static axes (allow many cycles)").listen() ;
    gui.add( params, 'loading_active').name( 'Loading active' ).listen();

    const controls = new OrbitControls( camera, container );
    controls.update();

    window.addEventListener( 'resize', onWindowResize, false );

    make_graph();
    WALLS.update_isotropic_wall(params, S, scene);
    animate();
}

function new_load_path() {
    WALLS.update_isotropic_wall(params, S, scene);
    var data = [{
                  type: 'scatter',
                  mode: 'lines',
                  x: [], y: [],
                  line: { width: 5 },
                  name: 'Load path ' + String(document.getElementById('stats').data.length+1)
                }]
    Plotly.addTraces('stats', data);
    params.new_line = false;
}

function onWindowResize(){

    camera.aspect = window.innerWidth*(1-graph_fraction) / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth*(1-graph_fraction), window.innerHeight );

    var update = {
        width: window.innerWidth*graph_fraction,
        height: window.innerHeight
        };
    Plotly.relayout('stats', update);

}

function animate() {
    if ( clock.getElapsedTime() - startTime > 3 ) { started = true; }
    requestAnimationFrame( animate );
    SPHERES.move_spheres(S,params);
    new_time = clock.getElapsedTime() - startTime;
    if ( !params.paused ) {
        if ( started ) {
            if ( params.loading_active) {
                var dt = new_time - old_time;
                params.epsilonv += loading_direction*params.loading_rate*dt;
                if ( (pressure >= params.target_stress) && (loading_direction === 1) ) { // just run this once
                    window.setTimeout(() => { loading_direction = -1}, 3000) // wait then reverse
                }
                if ( (pressure >= params.target_stress) && (loading_direction > 0) ) {
                    loading_direction *= 0.5; // slow down gradually
                }
                if ( (params.epsilonv <= 1e-4 || pressure<params.unloading_stress) && (loading_direction === -1) ) { // just run this once
                    window.setTimeout(() => { loading_direction = 1; new_load_path();}, 3000) // wait then reverse
                }
                if ( (params.epsilonv <= 1e-4 || pressure<params.unloading_stress) && (loading_direction < 0) ) {
                    loading_direction *= 0.5; // slow down gradually
                }
                WALLS.update_isotropic_wall(params, S, scene);
                update_graph();
                SPHERES.draw_force_network(S, params, scene);

            }
        }

        S.simu_step_forward(5);
        S.cg_param_read_timestep(0) ;
        S.cg_process_timestep(0,false) ;
        var grid = S.cg_get_gridinfo();
        sigma_xx = S.cg_get_result(0, "TC", 0)[0];
        sigma_yy = S.cg_get_result(0, "TC", 4)[0];
        sigma_zz = S.cg_get_result(0, "TC", 8)[0];
        pressure=(sigma_xx+sigma_yy+sigma_zz)/3
        density = S.cg_get_result(0, "RHO", 0)[0];
    }

    renderer.render( scene, camera );

    old_time = new_time;

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
        setup_NDDEM();
        setup_CG();
    } );

}

function setup_NDDEM() {
    S.simu_interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
    S.simu_interpret_command("radius -1 0.5");
    // now need to find the mass of a particle with diameter 1
    let m = 4./3.*Math.PI*0.5*0.5*0.5*params.particle_density;
    S.simu_interpret_command("mass -1 " + String(m));
    S.simu_interpret_command("auto rho");
    S.simu_interpret_command("auto radius uniform "+params.r_min+" "+params.r_max);
    S.simu_interpret_command("auto mass");
    S.simu_interpret_command("auto inertia");
    S.simu_interpret_command("auto skin");
    console.log(params.L, params.H)
    S.simu_interpret_command("boundary 0 WALL -"+String(params.L)+" "+String(params.L));
    S.simu_interpret_command("boundary 1 WALL -"+String(params.L)+" "+String(params.L));
    S.simu_interpret_command("boundary 2 WALL -"+String(params.H)+" "+String(params.H));
    if ( params.gravity === true ) {
        S.simu_interpret_command("gravity 0 0 " + String(-9.81) + "0 ".repeat(params.dimension - 3)) }
    else {
        S.simu_interpret_command("gravity 0 0 0 " + "0 ".repeat(params.dimension - 3)) }

    // S.simu_interpret_command("auto location randomsquare");
    S.simu_interpret_command("auto location randomdrop");

    let tc = 1e-3;
    let rest = 0.2; // super low restitution coeff to dampen out quickly
    let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, params.particle_mass)

    S.simu_interpret_command("set Kn 250000"); //+ String(vals.stiffness));
    S.simu_interpret_command("set Kt 250000"); //+ String(0.8*vals.stiffness));
    S.simu_interpret_command("set GammaN 0.2"); //+ String(vals.dissipation));
    S.simu_interpret_command("set GammaT 0.2"); //+ String(vals.dissipation));
    S.simu_interpret_command("set Mu 0.5");
    S.simu_interpret_command("set Mu_wall 0");
    S.simu_interpret_command("set T 150");
    S.simu_interpret_command("set dt " + String(tc/20));
    S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
    S.simu_interpret_command("ContactModel Hertz") ;
    S.simu_finalise_init () ;
}

function setup_CG() {
    var cgparam ={} ;
    cgparam["file"]=[{"filename":"none", "content": "particles", "format":"interactive", "number":1}] ;
    cgparam["boxes"]=[1,1,1] ;
    // cgparam["boundaries"]=[[-params.L,-params.L,-params.L],[params.L,params.L,params.L]] ;
    cgparam["boundaries"]=[
        [-params.L/2.,-params.L/2.,-params.L/2.],
        [ params.L/2., params.L/2., params.L/2.]] ;
        // [-params.L+params.r_max,-params.L+params.r_max,-params.L+params.r_max],
        // [ params.L-params.r_max, params.L-params.r_max, params.L-params.r_max]] ;

    cgparam["window size"]=params.L/2. ;
    cgparam["skip"]=0;
    cgparam["max time"]=1 ;
    cgparam["time average"]="None" ;
    cgparam["fields"]=["RHO", "TC","Pressure","ShearStress"] ;
    cgparam["periodicity"]=[false,false,false];
    cgparam["window"]="Lucy3D";
    cgparam["dimension"]=3;


    // console.log(JSON.stringify(cgparam)) ;
    S.cg_param_from_json_string(JSON.stringify(cgparam)) ;
    S.cg_setup_CG() ;
}

var decimate=0 ;
function update_graph() {
    Plotly.extendTraces('stats', {
        'x': [[params.epsilonv*1e2]], // convert to %
        'y': [[pressure]],

        // 'y': [[1./(density/params.particle_density) - 1.]], // void ratio
        // 'x': [[Math.log(vertical_stress)]], // pressure
    }, [-1])
    //console.log(data.length) ;
}

function make_graph() {
    let { data, layout } = LAYOUT.plotly_graph('Volumetric strain (%)','Pressure (Pa)');
    // layout.yaxis.range = [0,1e5];
    // layout.yaxis.autorange = false;
    Plotly.newPlot('stats', data, layout);
}
