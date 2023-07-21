import css from "../css/main.css";
import Plotly from "plotly.js-dist";

import * as THREE from "three";
// import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';
import { Lut } from 'three/examples/jsm/math/Lut.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';
import * as RAYCAST from '../libs/RaycastHandler.js';
import * as AUDIO from '../libs/audio.js';
import * as CGHANDLER from '../libs/CGHandler.js';


// let info_div = document.createElement("div")
// info_div.innerHTML = "Click on a particle to grab it"
// info_div.style.color = "white";
// info_div.style.position = "absolute";
// info_div.style.left = "20px";
// info_div.style.top = "20px";
// document.body.appendChild(info_div);

let graph_fraction = 0.5;
document.getElementById("stats").style.width = String(100*graph_fraction) + '%';
document.getElementById("canvas").style.width = String(100*(1-graph_fraction)) + '%';

let I_div = document.createElement('div');
I_div.style.cssText = 'position:absolute;color:white;bottom:10px;right:10px;font-size:24px;z-index:10000;';
document.body.appendChild(I_div);

var urlParams = new URLSearchParams(window.location.search);

let density, vavg, stressTcxx, stressTcyy, stressTczz, stressTcxy;
let pressure=[], shearstress=[], xloc=[] ;
let camera, scene, renderer, stats, panel, controls;
let gui;
let S;
// let count;

var params = {
    dimension: 2,
    L: 20*0.004, //system size
    initial_density: 0.75,
    zoom: 0.75,
    aspect_ratio: 1,
    // paused: false,
    // g_mag: 1e3,
    // theta: 0, // slope angle in DEGREES
    d4: {cur:0},
    r_max: 0.005,
    r_min: 0.003,
    particle_density: 2700,
    friction: 0.5,
    // freq: 0.05,
    // new_line: false,
    shear_rate: 1,
    // lut: 'None',
    lut: 'White',
    cg_field: 'Density',
    quality: 5,
    cg_width: 25,
    cg_height: 25,
    cg_opacity: 0,
    cg_window_size: 3,
    particle_opacity: 0.5,
    target_pressure: 1e4,
    F_mag_max: 2e1,
    audio: false,
    audio_sensitivity : 1,
    current_pressure: 0,
}

let rainbow    = new Lut("rainbow", 512); // options are rainbow, cooltowarm and blackbody
let cooltowarm = new Lut("cooltowarm", 512); // options are rainbow, cooltowarm and blackbody
let blackbody  = new Lut("blackbody", 512); // options are rainbow, cooltowarm and blackbody

params.average_radius = (params.r_min + params.r_max)/2.;
params.thickness = params.average_radius;


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
    var aspect = window.innerWidth / window.innerHeight /2.;
    camera = new THREE.OrthographicCamera(
       -params.L*params.aspect_ratio*aspect/params.zoom,
        params.L*params.aspect_ratio*aspect/params.zoom,
       -params.L*params.aspect_ratio/params.zoom,
        params.L*params.aspect_ratio/params.zoom,
       -10,
        10
    );
    camera.position.set( 0, 0, 1 );
    camera.rotateZ(Math.PI/2.);
    // camera.up.set(0, 0, 0);
    // camera.lookAt(0,0,0);
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
    SPHERES.update_contact_flags(0x80 | 0x100 | 0x200);

    // WALLS.add_back(params, scene);
    // WALLS.add_left(params, scene);
    // WALLS.add_right(params, scene);
    // WALLS.add_front(params, scene);
    // // WALLS.back.scale.y = params.thickness;//Math.PI/2.;
    // var vert_walls = [WALLS.left,WALLS.right,WALLS.back,WALLS.front];

    // vert_walls.forEach( function(mesh) {
    //     mesh.scale.x = 2*params.L + 2*params.thickness;
    //     mesh.scale.z = 2*params.L + 2*params.thickness;
    // });

    CGHANDLER.add_cg_mesh(2*params.L, 2*params.L*params.aspect_ratio, scene);

    // geometry = new THREE.PlaneGeometry( 2*params.L, 0.1*params.L );
    // material = new THREE.MeshBasicMaterial( {color: 0xffff00, side: THREE.DoubleSide} );
    // colorbar_mesh = new THREE.Mesh( geometry, material );
    // colorbar_mesh.position.y = -1.1*params.L;
    // scene.add( colorbar_mesh );


    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth/2., window.innerHeight );

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
    gui.add ( params, 'shear_rate', -1, 1).name('Shear rate').listen().onChange( update_shear_rate );
    // gui.add ( params, 'shear_rate', {Back : -1, Slow: 1e-1, Forward: 1}).name('Shear rate').listen().onChange( update_shear_rate );
    gui.add ( params, 'friction', 0,2).name('Interparticle friction').listen().onChange( () => {S.simu_interpret_command("set Mu " + String(params.friction))} );
    // gui.add ( params, 'target_pressure', 1e4, 1e6, 1e4).name('Target pressure (Pa)').listen().onChange(update_shear_rate);

    
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

    make_graph();
    animate();
    update_I();
}

function onWindowResize(){

    // camera.aspect = window.innerWidth / window.innerHeight;
    // camera.updateProjectionMatrix();
    var aspect = window.innerWidth / window.innerHeight /2.;
    camera.left = -params.L*params.aspect_ratio*aspect/params.zoom;
    camera.right = params.L*params.aspect_ratio*aspect/params.zoom;
    camera.bottom = -params.L*params.aspect_ratio/params.zoom;
    camera.top = params.L*params.aspect_ratio/params.zoom;

    renderer.setSize( window.innerWidth/2., window.innerHeight );
}

function animate() {
    requestAnimationFrame( animate );
    if ( !params.paused ) {
        if ( AUDIO.listener !== undefined ){
            SPHERES.update_fixed_sounds(S, params);
        }
        RAYCAST.animate_locked_particle(S, camera, SPHERES.spheres, params);
    
        // let v = S.simu_getVelocity();
        // console.log(v);
        SPHERES.move_spheres(S,params);
        S.simu_step_forward(5);
        CGHANDLER.update_2d_cg_field(S,params);
        SPHERES.draw_force_network(S, params, scene);
        update_pressure();
        update_graph();
        // console.log(S.simu_getTime())
        if ( S.simu_getTime() > 0.1 ) {
            WALLS.update_damped_wall(params.current_pressure, params.target_pressure, params, S, 15*1e-2/20.);
        }
        
    }
    
    renderer.render( scene, camera );
}

function update_pressure() {
    S.cg_param_read_timestep(0) ;
    S.cg_process_timestep(0,false) ;
    var grid = S.cg_get_gridinfo();
    density=S.cg_get_result(0, "RHO", 0) ;
    vavg=S.cg_get_result(0, "VAVG", 1) ;
    let normal_stresses = [];
    for (let i=0; i<params.dimension; i++) {
        let component = i*(params.dimension+1);
        let this_stress = S.cg_get_result(0, "TC", component) ;
        normal_stresses.push(this_stress);
    }
    // stressTcxx=S.cg_get_result(0, "TC", 0) ;
    // stressTcyy=S.cg_get_result(0, "TC", 4) ;
    // stressTczz=S.cg_get_result(0, "TC", 8) ;
    stressTcxy=S.cg_get_result(0, "TC", 1) ;
    for (var i=0 ; i<stressTcxy.length ; i++) // for all grid points
    {
        xloc[i]=grid[0]+i*grid[3] ;
        shearstress[i]=-stressTcxy[i] ;
        let this_pressure = 0;
        for ( let j=0; j<normal_stresses.length; j++) {
            this_pressure += normal_stresses[j][i];
        }
        pressure[i]=this_pressure / normal_stresses.length; // get isotropic pressure
    }
    
    // params.viscosity = 1e6;
    // params.inertial_number = 0.1;
    // params.target_pressure = Math.pow(params.shear_rate*params.average_radius/params.inertial_number,2)*params.particle_density;
    params.current_pressure = pressure.reduce((a, b) => a + b, 0) / pressure.length; // average vertical stress
    params.current_shearstress = shearstress.reduce((a, b) => a + b, 0) / shearstress.length; // average shear stress

    
    // let dt = 1e-3;
}

function update_cg_params(S, params) {
    var cgparam ={} ;
    cgparam["file"]=[{"filename":"none", "content": "particles", "format":"interactive", "number":1}] ;
    cgparam["boxes"]=[params.cg_width,params.cg_height] ;
    // cgparam["boundaries"]=[[-params.L,-params.L,-params.L],[params.L,params.L,params.L]] ;
    cgparam["boundaries"]=[
        [-params.L,-params.L*params.aspect_ratio],
        [ params.L, params.L*params.aspect_ratio]] ;
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
        let m = params.particle_density * SPHERES.get_particle_volume(params.dimension, 0.5);
        S.simu_interpret_command("mass -1 " + String(m));
        S.simu_interpret_command("auto rho");
        S.simu_interpret_command("auto radius uniform "+params.r_min+" "+params.r_max);
        S.simu_interpret_command("auto mass");
        S.simu_interpret_command("auto inertia");
        S.simu_interpret_command("auto skin");

        S.simu_interpret_command("boundary 0 PBCLE -"+String(params.L)+" "+String(params.L)+" "+String(params.shear_rate));
        S.simu_interpret_command("boundary 1 PBC -"+String(params.L*params.aspect_ratio)+" "+String(params.L*params.aspect_ratio));
        if ( params.dimension >= 3 ) {
            S.simu_interpret_command("boundary 2 PBC -"+String(params.L)+" "+String(params.L));
        }
        if ( params.dimension >= 4 ) {
            S.simu_interpret_command("boundary 3 PBC -"+String(params.L)+" "+String(params.L));
        }
        S.simu_interpret_command("gravity" + " 0".repeat(params.dimension))

        S.simu_interpret_command("auto location randomdrop");

        let tc = 1e-2;
        let rest = 0.2; // super low restitution coeff to dampen out quickly
        let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, params.particle_mass)

        S.simu_interpret_command("set Kn " + String(vals.stiffness));
        S.simu_interpret_command("set Kt " + String(0.8*vals.stiffness));
        S.simu_interpret_command("set GammaN " + String(vals.dissipation));
        S.simu_interpret_command("set GammaT " + String(vals.dissipation));
        S.simu_interpret_command("set Mu " + String(params.friction));
        S.simu_interpret_command("set damping 0.001");
        S.simu_interpret_command("set T 150");
        S.simu_interpret_command("set dt " + String(tc/20));
        S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
        S.simu_finalise_init () ;

        // let v = S.simu_getVelocity();
        // console.log(v);
        update_cg_params(S, params);

    }
}

// function update_wall_particle_velocities() {
//     console.log(params.shear_rate)
//     let n_wall = Math.ceil(2*params.aspect_ratio*params.L/(params.average_radius*2));
//         for ( var i=0; i<n_wall; i++ ) {
//             S.simu_setVelocity(i,[params.shear_rate*2*params.L,0]);
//         }
// }

function update_shear_rate() {
    update_I();
    S.simu_setBoundary(0, [-params.L,params.L,params.shear_rate]);
    params.vmax = 1.5*Math.abs(params.shear_rate)*params.L;
    params.omegamax = 1e3*Math.abs(params.shear_rate)*params.average_radius;
    SPHERES.update_particle_material(params);
}

function update_I() {
    // I = |gamma_dot| * d / sqrt(P/rho)
    params.I  = Math.abs(params.shear_rate) * params.average_radius / Math.sqrt(params.target_pressure/params.particle_density);
    I_div.innerHTML = 'I = ' + params.I.toPrecision(4);
}

async function update_graph() {
    let numBins = 15;
    let branches = [];
    let Fns = [];
    let Fts = [];

    for ( let c = 0; c < SPHERES.F.length; c++ ) {
        let i = SPHERES.F[c][0];
        let j = SPHERES.F[c][1];

        if ( SPHERES.F[c][2] !== 0 && SPHERES.F[c][3] !== 0 ) { // lots of results with zero force
            // branch_theta.push(Math.atan2(SPHERES.x[i][0] - SPHERES.x[j][0],SPHERES.x[i][1] - SPHERES.x[j][1]));
            // Fn_theta.push(Math.atan2(SPHERES.F[c][2],SPHERES.F[c][3]));
            // Ft_theta.push(Math.atan2(SPHERES.F[c][4],SPHERES.F[c][5]));
            branches.push([SPHERES.x[i][0] - SPHERES.x[j][0],SPHERES.x[i][1] - SPHERES.x[j][1]]);
            Fns.push([SPHERES.F[c][2],SPHERES.F[c][3]]);

            let sign = crossProductSign([SPHERES.x[i][0] - SPHERES.x[j][0],SPHERES.x[i][1] - SPHERES.x[j][1]],
                                        [SPHERES.F[c][4],SPHERES.F[c][5]]);
            Fts.push([SPHERES.F[c][4],SPHERES.F[c][5],sign]);
        }
    }

    // let branch_hist = calculateAngularAverage(branches, numBins);
    let branch_hist = calculateAngularCount(branches, numBins);
    let Fn_hist = calculateAngularAverage(Fns, numBins, false);
    

    branch_hist = branch_hist.concat(branch_hist); // double up data
    Fn_hist = Fn_hist.concat(Fn_hist); // double up data
    
    let theta_edge = linspace(0, 2*Math.PI, 2*numBins+1);
    let theta_center = edge_to_center(theta_edge);
    let dtheta = theta_edge[1] - theta_edge[0];
    
    let branch_sum = branch_hist.reduce( (a,b) => a+b );
    let f0 = Fn_hist.reduce( (a,b) => a+b ) / Fn_hist.length;
    // let Ft_mean = Ft_hist.reduce( (a,b) => a+b ) / Ft_hist.length;
    
    branch_hist = branch_hist.map( x => x/branch_sum/dtheta ); // WHY 1.1????
    Fn_hist = Fn_hist.map( x => x/f0 ); 
    
    let Ft_hist = calculateAngularAverage(Fts, numBins, true);
    Ft_hist = Ft_hist.concat(Ft_hist); // double up data
    Ft_hist = Ft_hist.map( x => x/f0 ); 

    // console.log(branch_hist.reduce( (a,b) => a+b*dtheta ))

    let [a,a_n,a_t,theta_a,theta_n,theta_t,pred_a,pred_n,pred_t] = fitCosineCyclic( theta_edge, branch_hist, Fn_hist, Ft_hist );
    // console.log(branches)
    // console.log(branch_hist)

    branch_hist = branch_hist.concat(branch_hist[0]); // make cyclic
    Fn_hist = Fn_hist.concat(Fn_hist[0]); // make cyclic
    Ft_hist = Ft_hist.concat(Ft_hist[0]); // make cyclic
    theta_center = theta_center.concat(theta_center[0]); // make cyclic

    // Fn_hist = Fn_hist.map( x => Math.abs(x) ); 
    Ft_hist = Ft_hist.map( x => Math.abs(x) );
    // pred_n = pred_n.map( x => Math.abs(x) ); 
    pred_t = pred_t.map( x => Math.abs(x) ); 
    // console.log(theta_center[0],theta_center[theta_center.length-1]);

    Plotly.update('stats', {
        'theta': [theta_center],
        'r': [branch_hist],
    }, {}, [0]);

    Plotly.update('stats', {
        'theta': [theta_center],
        'r': [pred_a],
    }, {}, [1]);

    Plotly.update('stats', {
        'theta': [theta_center],
        'r': [Fn_hist],
    }, {}, [2])

    Plotly.update('stats', {
        'theta': [theta_center],
        'r': [pred_n],
    }, {}, [3])

    Plotly.update('stats', {
        'theta': [theta_center],
        'r': [Ft_hist],
    }, {}, [4])

    Plotly.update('stats', {
        'theta': [theta_center],
        'r': [pred_t],
    }, {}, [5])

    // console.log(S.simu_getTime())
    // console.log(params.current_shearstress,params.current_pressure)
    let t = S.simu_getTime();
    Plotly.extendTraces('stats', {
        'x': [[t],[t],[t],[t],[t]],
        'y': [[Math.abs(params.current_shearstress/params.current_pressure)],[a],[a_n],[a_t],[0.5*(a+a_n+a_t)/(1+a*a_n/2.)]],
    }, [6,7,8,9,10])

}

function make_graph() {
    let { data, layout, config } = LAYOUT.plotly_2x2_graphs();
    Plotly.newPlot('stats', data, layout, config);
}

function histogram(numbers, binEdges) {
    const numBins = binEdges.length - 1;
    const H = new Array(numBins).fill(0);
  
    for (const number of numbers) {
      for (let i = 0; i < numBins; i++) {
        if (number >= binEdges[i] && number < binEdges[i + 1]) {
          H[i]++;
          break;
        }
      }
    }
  
    return H;
}

function linspace(start, stop, num) {
    const step = (stop - start) / (num - 1);
    const result = [];
  
    for (let i = 0; i < num; i++) {
      const value = start + step * i;
      result.push(value);
    }
 
    return result
  }

function edge_to_center(edge) {
    let result = [];
    for (let j = 0; j < edge.length-1; j++) {
        const value = (edge[j] + edge[j+1])/2.
        result.push(value);
    }
    return result
}

function calculateAngularCount(vectors, numBins) {
    // Create an array to store the sum and count of magnitudes in each bin
    // const binData = Array(numBins).fill().map(() => ({ sum: 0, count: 0 }));
    const binData = Array(numBins).fill(0);
  
    // Iterate through each vector
    for (const vector of vectors) {
        // Calculate the angle of the vector
        const angle = Math.PI/2. - Math.atan(vector[0]/vector[1]); // want it in range 0 to pi
        // console.log(vector)
        // Map the angle to the corresponding bin index
        const binIndex = Math.floor((angle / (Math.PI)) * numBins);
        // console.log(binIndex)

        binData[binIndex] += 1;
    }
    // console.log(binData);
    return binData
    
}

function calculateAngularAverage(vectors, numBins, shear=false) {
  // Create an array to store the sum and count of magnitudes in each bin
  const binData = Array(numBins).fill().map(() => ({ sum: 0, count: 0 }));

  // Iterate through each vector
  for (const vector of vectors) {
    // Calculate the angle of the vector
    const angle = Math.PI/2. - Math.atan(vector[0]/vector[1]); // want it in range 0 to pi

    // Map the angle to the corresponding bin index
    const binIndex = Math.floor((angle / (Math.PI)) * numBins);
    
    // Calculate the magnitude of the vector
    let magnitude = Math.sqrt(vector[0] ** 2 + vector[1] ** 2);
    
    if ( shear !== false ) {
        magnitude *= vector[2];
    }
    
    // Update the sum and count of magnitudes in the bin
    binData[binIndex].sum += magnitude;
    binData[binIndex].count++;
  }

  // Calculate the mean value of the magnitudes in each bin
  const binMeans = binData.map(bin => bin.count > 0 ? bin.sum / bin.count : 0);

  return binMeans;
}
 
// Function to fit a cosine function to the data with cyclic x values
function fitCosineCyclic(xValues,E,f_n,f_t) {
    let A_a = 0.0;
    let B_a = 0.0;
    let A_n = 0.0;
    let B_n = 0.0;
    let A_t = 0.0;
    let B_t = 0.0;
    let dx = xValues[1] - xValues[0];
  
    for (let i = 0; i < E.length; i++) {
        const x = xValues[i];
        const cos2x_dx = Math.cos(2*x)*dx;
        const sin2x_dx = Math.sin(2*x)*dx;
        A_a +=   E[i]*cos2x_dx;
        B_a +=   E[i]*sin2x_dx;
        A_n += f_n[i]*cos2x_dx;
        B_n += f_n[i]*sin2x_dx;
        A_t += f_t[i]*cos2x_dx;
        B_t += f_t[i]*sin2x_dx;
    }
  
    // Calculate the coefficients
    const theta_a = Math.atan(B_a/A_a)/2.;
    const a   = 2*A_a/Math.cos(2*theta_a);

    const theta_n = Math.atan(B_n/A_n)/2.;
    const a_n = A_n/Math.cos(2*theta_n)/Math.PI;

    const theta_t = Math.atan(-A_t/B_t)/2.;
    const a_t = -A_t/Math.sin(2*theta_t)/Math.PI;
  
    // Generate the predicted values using the fitted coefficients
    const pred_a = [];
    const pred_n = [];
    const pred_t = [];
    for (let i = 0; i < E.length; i++) {
      const x = xValues[i];
      pred_a.push( (1 +   a*Math.cos(2*(x - theta_a))) / (2*Math.PI) );
      pred_n.push(  1 + a_n*Math.cos(2*(x - theta_n))                );
      pred_t.push(      a_t*Math.sin(2*(x - theta_t))                );
    }
  
    // Return the fit results
    return [Math.abs(a),Math.abs(a_n),Math.abs(a_t),theta_a,theta_n,theta_t,pred_a,pred_n,pred_t];
}

function crossProductSign(vector1, vector2) {
    const crossProduct = vector1[0] * vector2[1] - vector1[1] * vector2[0];
    if (crossProduct > 0) {
        return 1; // Positive sign
    } else if (crossProduct < 0) {
        return -1; // Negative sign
    } else {
        return 0; // Zero sign (vectors are parallel or collinear)
    }
}