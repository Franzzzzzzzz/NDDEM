import css from "../css/main.css";

import * as THREE from "three";
// import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
// import * as WALLS from "../libs/WallHandler.js"
// import * as LAYOUT from '../libs/Layout.js'
import * as CGHANDLER from '../libs/CGHandler.js';


var urlParams = new URLSearchParams(window.location.search);

if ( !urlParams.has('lut') ) { urlParams.set('lut','Size') }


let camera, scene, renderer, stats, panel, controls;
let gui;
let boundary;
let S;

var params = {
    dimension: 2,
    // Fr : 0.5,
    R: 0.1, // drum radius
    N: 400,
    // packing_fraction: 0.5,
    gravity: false,
    paused: false,
    r_min: 0.001,
    r_max: 0.005,
    omega: 10, // rotation rate
    lut: 'White',
    cg_field: 'Size',
    quality: 5,
    cg_width: 50,
    cg_height: 50,
    cg_opacity: 0.8,
    cg_window_size: 5,
    vmax: 20, // max velocity to colour by
    omegamax: 20, // max rotation rate to colour by
    particle_density : 2700,
    zoom : 1000,
    mu_wall : 0.5,
    mu : 0.5,
    F_mag_max: 50,
    particle_opacity : 0.95,
}

params.average_radius = (params.r_min + params.r_max)/2.;

params.particle_volume = Math.PI*Math.pow(params.average_radius,2);
params.particle_mass = params.particle_volume*params.particle_density

if ( urlParams.has('quality') ) { params.quality = parseInt(urlParams.get('quality')); }


SPHERES.createNDParticleShader(params).then( init() );

async function init() {
    await NDDEMPhysics();

    var aspect = window.innerWidth / window.innerHeight;
    camera = new THREE.OrthographicCamera(
        (-100 * aspect) / params.zoom,
        (100 * aspect) / params.zoom,
        100 / params.zoom,
        -100 / params.zoom,
        -1000,
        1000
    );
    camera.position.set( 0, 0, -5*params.R );
    camera.up.set(1, 0, 0);
    camera.lookAt(0,0,0);

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0xffffff );

    // const axesHelper = new THREE.AxesHelper( 50 );
    // scene.add( axesHelper );

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add( hemiLight );

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set( 5, -5, -5 );
    dirLight.castShadow = true;
    scene.add( dirLight );
    scene.add( dirLight );

    const wall_geometry = new THREE.CircleGeometry( params.R, 100 );
    const wall_material = new THREE.MeshLambertMaterial({color: 0x000000, side: THREE.DoubleSide});
    // wall_material.wireframe = true;

    boundary = new THREE.Mesh( wall_geometry, wall_material );
    scene.add( boundary );

    CGHANDLER.add_cg_mesh(params.R*2, params.R*2, scene);


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

    gui.add( params, 'omega', 0,25)
        // .name( 'Froude number').listen()
        .name( 'Rotation rate').listen()
        .onChange( () => {
            // params.omega = Math.sqrt(2*params.Fr*9.81/(2*params.R));
            // S.simu_interpret_command("gravityrotate -9.81 " + params.omega + " 0 1")
            S.simu_interpret_command("boundary "+String(params.dimension)+" ROTATINGSPHERE "+String(params.R)+" 0 0 " + String(-params.omega) + " 0 0"); // add a sphere!
        } );
    gui.add( params, 'mu', 0,1)
        .name( 'Particle Friction').listen()
        .onChange( () => {
            S.simu_interpret_command("set Mu " + String(params.mu));
        } );
    gui.add( params, 'mu_wall', 0,1)
        .name( 'Wall Friction').listen()
        .onChange( () => {
            S.simu_interpret_command("set Mu_wall " + String(params.mu_wall));
        } );

    gui.add ( params, 'cg_opacity', 0, 1).name('Coarse grain opacity').listen();
    gui.add ( params, 'cg_field', ['Density', 'Size', 'Velocity', 'Pressure', 'Shear stress','Kinetic Pressure']).name('Field').listen();
    // gui.add ( params, 'cg_window_size', 0.5, 6).name('Window size (radii)').listen().onChange( () => {
    //     update_cg_params(S, params);
    // });
    // controls = new OrbitControls( camera, renderer.domElement );
    // controls.update();

    window.addEventListener( 'resize', onWindowResize, false );


    // update_walls();
    animate();
}


function onWindowResize(){

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth, window.innerHeight );

}

function animate() {
    requestAnimationFrame( animate );
    SPHERES.move_spheres(S,params);
    
    S.simu_step_forward(25);
    CGHANDLER.update_2d_cg_field(S,params);
    
    // let angle = -S.simu_getGravityAngle() + Math.PI/2.;
    // SPHERES.spheres.setRotationFromAxisAngle ( new THREE.Vector3(0,0,1), angle );
    // camera.up.set(0, 0, 0);
    // controls.update();
    SPHERES.draw_force_network(S, params, scene);
    renderer.render( scene, camera );
}

async function NDDEMPhysics() {

    if ( 'DEMCGND' in window === false ) {

        console.error( 'NDDEMPhysics: Couldn\'t find DEMCGND.js' );
        return;

    }

    await DEMCGND().then( (NDDEMCGLib) => {
        S = new NDDEMCGLib.DEMCG2D (params.N);
        finish_setup();
    } );
}

function finish_setup() {
    S.simu_interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
    S.simu_interpret_command("radius -1 0.5");
    let m = Math.PI*0.5*0.5*params.particle_density;
    S.simu_interpret_command("mass -1 " + String(m));
    S.simu_interpret_command("auto rho");
    S.simu_interpret_command("auto radius uniform "+params.r_min+" "+params.r_max);
    // S.simu_interpret_command("auto radius bidisperse "+params.r_min+" "+params.r_max+" 0.5");
    S.simu_interpret_command("auto mass");
    S.simu_interpret_command("auto inertia");
    S.simu_interpret_command("auto skin");

    for ( let i=0;i<params.dimension;i++ ) {
        S.simu_interpret_command("boundary "+String(i)+" WALL -"+4*String(params.R)+" "+4*String(params.R));
    }

    S.simu_interpret_command("boundary "+String(params.dimension)+" ROTATINGSPHERE "+String(params.R)+" 0 0 " + String(-params.omega) + " 0 0"); // add a sphere!

    // S.simu_interpret_command("auto location randomsquare");
    // S.simu_interpret_command("auto location randomdrop");
    S.simu_interpret_command("auto location insphere");
    // S.simu_interpret_command("gravityrotate -9.81 " + params.omega + " 0 1"); // intensity, omega, rotdim0, rotdim1
    S.simu_interpret_command("gravity -9.81 0"); // intensity, omega, rotdim0, rotdim1

    let tc = 2e-3;
    let rest = 0.5; // super low restitution coeff to dampen out quickly
    let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, params.particle_mass)

    S.simu_interpret_command("set Kn " + String(vals.stiffness));
    S.simu_interpret_command("set Kt " + String(0.8*vals.stiffness));
    S.simu_interpret_command("set GammaN " + String(vals.dissipation));
    S.simu_interpret_command("set GammaT " + String(vals.dissipation));
    S.simu_interpret_command("set Mu " + String(params.mu));
    S.simu_interpret_command("set Mu_wall " + String(params.mu_wall));
    // S.simu_interpret_command("set damping 0.001");
    S.simu_interpret_command("set T 150");
    S.simu_interpret_command("set dt " + String(tc/10));
    S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
    S.simu_interpret_command("auto skin");
    S.simu_finalise_init () ;

    update_cg_params(S, params);
}

function update_cg_params(S, params) {
    var cgparam ={} ;
    cgparam["file"]=[{"filename":"none", "content": "particles", "format":"interactive", "number":1}] ;
    cgparam["boxes"]=[params.cg_width,params.cg_height] ;
    // cgparam["boundaries"]=[[-params.L,-params.L,-params.L],[params.L,params.L,params.L]] ;
    cgparam["boundaries"]=[
        [-params.R,-params.R],
        [ params.R, params.R]] ;
    cgparam["window size"]=params.cg_window_size*params.average_radius ;
    cgparam["skip"]=0;
    cgparam["max time"]=1 ;
    cgparam["time average"]="None" ;
    cgparam["fields"]=["RHO", "VAVG", "TC", "Pressure", "KineticPressure","RADIUS"] ;
    cgparam["periodicity"]=[false,false];
    cgparam["window"]="Lucy2D";
    cgparam["dimension"]=2;


    // console.log(JSON.stringify(cgparam)) ;
    S.cg_param_from_json_string(JSON.stringify(cgparam)) ;
    S.cg_setup_CG() ;
}
