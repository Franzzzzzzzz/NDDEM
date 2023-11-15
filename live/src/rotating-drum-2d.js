import css from "../css/main.css";

// import * as THREE from "three";
// import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';
import * as SPHERES from "../libs/SphereHandler.js"
// import * as WALLS from "../libs/WallHandler.js"
// import * as LAYOUT from '../libs/Layout.js'
import * as CGHANDLER from '../libs/CGHandler.js';

let urlParams = new URLSearchParams(window.location.search);

if ( !urlParams.has('lut') ) { urlParams.set('lut','Size') }

// Your existing code unmodified...
let Fr_div = document.createElement('div');
Fr_div.style.cssText = 'position:absolute;color:white;top:10px;left:10px;font-size:24px;z-index:10000';
document.body.appendChild(Fr_div);

// import Stats from 'three/examples/jsm/libs/stats.module'
// const stats = new Stats()
// document.body.appendChild(stats.dom)

let camera, scene, renderer, panel, controls;
let gui;
let boundary;
let S;
let dt;

var params = {
    dimension: 2,
    // Fr : 0.5,
    R: 0.1, // drum radius
    // N: 500,
    // packing_fraction: 0.5,
    gravity: false,
    paused: false,
    // r_min: 0.002,
    r_max: 0.0035,
    omega: 12.1, // rotation rate
    lut: 'Size',
    cg_field: 'Size',
    quality: 5,
    cg_width: 50,
    cg_height: 50,
    cg_opacity: 0.8,
    cg_window_size: 3,
    vmax: 20, // max velocity to colour by
    omegamax: 20, // max rotation rate to colour by
    particle_density : 2700,
    zoom : 1000,
    mu_wall : 0.5,
    mu : 0.5,
    F_mag_max: 50,
    particle_opacity : 0.95,
    lifters: {
        number: 0,
        width: 0.05,
        prev_num: 0,
    },
    phi_s : 0.5, // volumetric concentration of small particles
    filling_fraction : 0.5, // how full the drum is
    size_ratio : 1.5, // ratio of small to large particle size
    density_ratio : 1, // ratio of small to large particle density
}

function setup() {
    let target_nu = 0.8;
    let V_solid = params.filling_fraction*target_nu*Math.PI*params.R*params.R;
    let V_small = params.phi_s*V_solid;
    let V_large = (1-params.phi_s)*V_solid;
    params.r_min = params.r_max/params.size_ratio;
    params.N_small = Math.floor(V_small/(Math.PI*params.r_min*params.r_min));
    params.N_large = Math.floor(V_large/(Math.PI*params.r_max*params.r_max));
    params.N = params.N_small + params.N_large;
    params.number_ratio = params.N_small/params.N;

    params.average_radius = (params.r_min + params.r_max)/2.;

    params.particle_volume = Math.PI*Math.pow(params.average_radius,2);
    params.particle_mass = params.particle_volume*params.particle_density

    params.F_mag_max = 50*params.density_ratio;
}

if ( urlParams.has('quality') ) { params.quality = parseInt(urlParams.get('quality')); }
if ( urlParams.has('cg_opacity') ) { params.cg_opacity = parseInt(urlParams.get('cg_opacity')); }
if ( urlParams.has('size_ratio') ) { params.size_ratio = parseFloat(urlParams.get('size_ratio')); }
if ( urlParams.has('density_ratio') ) { params.density_ratio = parseFloat(urlParams.get('density_ratio')); }

SPHERES.update_cylinder_colour( 0x000000 );
SPHERES.createNDParticleShader(params).then( init );

async function reset_particles() {
    setup();
    await NDDEMPhysics();
    scene.remove(SPHERES.spheres);
    SPHERES.wipe();
    SPHERES.add_spheres(S,params,scene);
}

async function init() {
    setup();

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
    scene.background = new THREE.Color( 0x000000 );

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
    const wall_material = new THREE.MeshLambertMaterial({color: 0xffffff, side: THREE.DoubleSide});
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

    gui.add( params, 'omega', 0,20)
        // .name( 'Froude number').listen()
        .name( 'Rotation rate').listen()
        .onChange( () => {
            // params.omega = Math.sqrt(2*params.Fr*9.81/(2*params.R));
            // S.simu_interpret_command("gravityrotate -9.81 " + params.omega + " 0 1")
            update_Fr();
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
    gui.add( params, 'filling_fraction', 0,1)
        .name( 'Filling Fraction').listen()
        .onChange( () => {
            reset_particles();
        } );
    gui.add( params, 'phi_s', 0,1)
        .name( 'Small particle conc').listen()
        .onChange( () => {
            reset_particles();
        } );
    gui.add( params, 'size_ratio', 1,3,0.01)
        .name( 'Size ratio').listen()
        .onChange( () => {
            reset_particles();
        } );
    gui.add( params, 'density_ratio', 0.1,10)
        .name( 'Density ratio').listen()
        .onChange( () => {
            reset_particles();
        } );
    

    // gui.add( params.lifters, 'number', 0,12,1)
    //     .name( 'Number of lifters').listen()
    //     .onChange( update_lifters);
    // gui.add( params.lifters, 'width', 0,params.R/2.)
    //     .name( 'Lifter width').listen()
    //     .onChange( update_lifters);

    gui.add ( params, 'cg_opacity', 0, 1).name('Coarse grain opacity').listen();
    gui.add ( params, 'cg_field', ['Density', 'Size', 'Velocity', 'Pressure', 'Shear stress','Kinetic Pressure']).name('Field').listen();
    // gui.add ( params, 'cg_window_size', 0.5, 6).name('Window size (radii)').listen().onChange( () => {
    //     update_cg_params(S, params);
    // });
    // controls = new OrbitControls( camera, renderer.domElement );
    // controls.update();

    window.addEventListener( 'resize', onWindowResize, false );

    update_Fr();
    // update_walls();
    animate();
}

function update_lifters(){
    // clear anything from previously
    for ( let i = 0; i<params.lifters.prev_num; i++){
        S.simu_interpret_command('mesh remove 0');
    }

    let vertices = [];
    let theta = 0;
    let dtheta = 2*Math.PI/params.lifters.number;
    for (let i=0;i<params.lifters.number;i++){
        vertices.push([[                       params.R*Math.cos(theta),                        params.R*Math.sin(theta)],
                       [(params.R-params.lifters.width)*Math.cos(theta), (params.R-params.lifters.width)*Math.sin(theta)]]);
        theta += dtheta;
    }
    let header = 'mesh string {"dimension":2,"objects":[';
    let footer = ']}';
    let body = '';
    for (let i=0;i<vertices.length;i++){
        body += '{"dimensionality":1,"vertices":' + JSON.stringify(vertices[i]) + '},';
    }

    // console.log(header + body.slice(0, -1) + footer);

    S.simu_interpret_command(header + body.slice(0, -1) + footer);
    params.lifters.prev_num = params.lifters.number;
    // S.simu_interpret_command('mesh string {"dimension":2,"objects":[{"dimensionality":1,"vertices":[['+String(params.D/2.)+','+String(-params.L)+'],['+String(params.W/2.)+','+String(params.H-params.L)+']]},{"dimensionality":1,"vertices":[['+String(-params.D/2.)+','+String(-params.L)+'],['+String(-params.W/2.)+','+String(params.H-params.L)+']]}]}');

}

function update_Fr() {
    // Fr = omega^2*R/g
    params.Fr  = params.omega*params.omega*params.R/9.81/2; // WHY IS THERE A FACTOR OF TWO HERE?!?!???
    Fr_div.innerHTML = 'Fr = ' + params.Fr.toPrecision(2);
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
    CGHANDLER.update_2d_cg_field(S,params);
    
    // let angle = -S.simu_getGravityAngle() + Math.PI/2.;
    // SPHERES.spheres.setRotationFromAxisAngle ( new THREE.Vector3(0,0,1), angle );
    // camera.up.set(0, 0, 0);
    // controls.update();
    S.simu_interpret_command('mesh rotate ' + String(-params.omega*25*dt) + " 0 0");
    SPHERES.draw_force_network(S, params, scene);
    renderer.render( scene, camera );
    put_particles_back();

    // stats.update()
}

function put_particles_back(){
    for ( let i = 0; i<params.N; i++){
        let dist = Math.sqrt(SPHERES.x[i][0]*SPHERES.x[i][0] + SPHERES.x[i][1]*SPHERES.x[i][1]);
        if ( dist > params.R ) { // if particle is outside of the circle 
            S.simu_interpret_command("location " + String(i) + " 0 0"); // put it back at the origin
        }
    }
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
    // S.simu_interpret_command("radius -1 0.5");
    // let m = Math.PI*0.5*0.5*params.particle_density;
    // S.simu_interpret_command("mass -1 " + String(m));
    // S.simu_interpret_command("auto rho");
    // S.simu_interpret_command("auto radius uniform "+params.r_min+" "+params.r_max);
    // console.log(number_ratio)
    // S.simu_interpret_command("auto radius bidisperse "+params.r_min+" "+params.r_max+" "+params.number_ratio);
    let m_small = Math.PI*params.r_min*params.r_min*params.particle_density*params.density_ratio;
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
    // S.simu_interpret_command("auto mass");
    S.simu_interpret_command("auto rho");
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

    let tc = 1e-3;
    let rest = 0.5; // super low restitution coeff to dampen out quickly
    let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, min_mass)
    dt = tc/10;

    S.simu_interpret_command("set Kn " + String(vals.stiffness));
    S.simu_interpret_command("set Kt " + String(0.8*vals.stiffness));
    S.simu_interpret_command("set GammaN " + String(vals.dissipation));
    S.simu_interpret_command("set GammaT " + String(vals.dissipation));
    S.simu_interpret_command("set Mu " + String(params.mu));
    S.simu_interpret_command("set Mu_wall " + String(params.mu_wall));
    // S.simu_interpret_command("set damping 0.001");
    S.simu_interpret_command("set T 150");
    S.simu_interpret_command("set dt " + String(dt));
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
