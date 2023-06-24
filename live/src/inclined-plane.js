import css from "../css/main.css";
import Plotly from "plotly.js-dist";

// import { DEMCGND } from "../../deploy/DEMCGND.js";
import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';

var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel, controls;
let physics, position;
let gui;
let floor;
let S;
let grid;
let particle_volume;
let NDsolids, material, STLFilename;
let meshes = new THREE.Group();
let density, vavg, stressTcxx, stressTcyy, stressTczz, stressTcxy;
let pressure=[], shearstress=[], xloc=[] ;
let show_stats = true;

let graph_fraction = 0.5;
document.getElementById("stats").style.width = String(100*graph_fraction) + '%';
document.getElementById("canvas").style.width = String(100*(1-graph_fraction)) + '%';

var params = {
    dimension: 3,
    L: 0.025, //system size
    N: 800,
    // packing_fraction: 0.5,
    paused: false,
    g_mag: 10,
    theta: 0, // slope angle in DEGREES
    d4: {cur:0},
    r_max: 0.0033,
    r_min: 0.0027,
    new_line: false,
    lut: 'None',
    quality: 5,
    vmax: 20, // max velocity to colour by
    omegamax: 20, // max rotation rate to colour by
    particle_opacity: 1.0,
    particle_density: 2700,
}

set_derived_properties();

function set_derived_properties() {
    params.average_radius = (params.r_min + params.r_max)/2.;
    params.particle_volume = 4./3.*Math.PI*Math.pow(params.average_radius,3);
    params.particle_mass = params.particle_volume * params.particle_density;

}

params.average_radius = (params.r_min + params.r_max)/2.;
let thickness = params.average_radius;

particle_volume = 4./3.*Math.PI*Math.pow(params.average_radius,3);
if ( urlParams.has('dimension') ) {
    params.dimension = parseInt(urlParams.get('dimension'));
}
if ( params.dimension === 4) {
    params.L = 2.5;
    params.N = 300
    particle_volume = Math.PI*Math.PI*Math.pow(params.average_radius,4)/2.;
}

SPHERES.createNDParticleShader(params).then( init );

async function init() {

    physics = await NDDEMPhysics();
    // physics.main(params.dimensions, params.N, inputfile)
    position = new THREE.Vector3();

    //

    camera = new THREE.PerspectiveCamera( 50, window.innerWidth*(1-graph_fraction) / window.innerHeight, 1e-5, 1000 );
    camera.position.set( 1*params.L, 0, -5*params.L );
    camera.up.set(1, 0, 0);

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

    // const wall_geometry = new THREE.BoxGeometry( params.L*2 + thickness*2, thickness, params.L*2 + thickness*2 );
    const wall_geometry = new THREE.BoxGeometry( 1, thickness, 1 );
    const wall_material = new THREE.MeshLambertMaterial();
    wall_material.wireframe = true;
    // const wall_material = new THREE.ShadowMaterial( )

    floor = new THREE.Mesh( wall_geometry, wall_material );
    floor.rotation.z = Math.PI/2.;
    // floor.position.x = -params.L - thickness/2.;// + params.r_min + params.r_max;
    floor.scale.x = 2*params.L + 2*thickness;
    floor.scale.z = 2*params.L + 2*thickness;
    scene.add( floor );

    SPHERES.add_spheres(S,params,scene);

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth*(1-graph_fraction), window.innerHeight );
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;

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
    gui.add ( params, 'theta', 0, 90, 0.1).name('Slope angle (deg) (W/S)').listen()
        .onChange( () => update_slope_angle() );
    gui.add ( params, 'particle_opacity',0,1).name('Particle opacity').listen().onChange( () => SPHERES.update_particle_material(params,
        // lut_folder
    ));
    gui.add ( params, 'lut', ['None', 'Velocity', 'Rotation Rate', 'Size']).name('Colour by')
        .onChange( () => SPHERES.update_particle_material(params,
            // lut_folder
        ) );
    gui.add ( params, 'paused').name('Paused (Enable to rotate graph)').listen();
    gui.add ( params, 'new_line').name('New loading path').listen()
        .onChange( () => {
            var data = [{
                          type: 'scatter3d',
                          mode: 'lines',
                          x: [], y: [], z: [],
                          line: { width: 5 },
                          // name: 'Load path'
                        }]
            Plotly.addTraces('stats', data);
            params.new_line = false;
        });
    controls = new OrbitControls( camera, container );
    controls.target.x = 1*params.L;
    controls.target.y = 0;
    controls.target.z = 0;
    controls.update();

    window.addEventListener( 'resize', onWindowResize, false );
    window.addEventListener( 'keypress', checkKeys, false );

    if ( show_stats ) { make_graph(); }
    update_slope_angle()

    animate();
}

function update_slope_angle() {
    S.simu_interpret_command("gravity " + String(-params.g_mag*Math.cos(params.theta*Math.PI/180.)) + " " + String(params.g_mag*Math.sin(params.theta*Math.PI/180.)) + " 0 " + "0 ".repeat(params.dimension - 3));
    camera.up.set(Math.cos(-params.theta*Math.PI/180.), Math.sin(-params.theta*Math.PI/180.), 0);
    if ( controls !== undefined) { controls.update(); }
    console.log(params.theta)
}

function checkKeys( event ) {
    if ( event.code === 'KeyW' ) {
        params.theta += 0.1;
        update_slope_angle();
        }
    if ( event.code === 'KeyS' ) {
        params.theta -= 0.1;
        update_slope_angle();
        }
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
    requestAnimationFrame( animate );
    SPHERES.move_spheres(S,params);
    if ( !params.paused ) {
        S.simu_step_forward(5);
        S.cg_param_read_timestep(0) ;
        S.cg_process_timestep(0,false) ;
        density=S.cg_get_result(0, "RHO", 0) ;
        vavg=S.cg_get_result(0, "VAVG", 1) ;
        stressTcxx=S.cg_get_result(0, "TC", 0) ;
        stressTcyy=S.cg_get_result(0, "TC", 4) ;
        stressTczz=S.cg_get_result(0, "TC", 8) ;
        stressTcxy=S.cg_get_result(0, "TC", 1) ;
        for (var i=0 ; i<stressTcxx.length ; i++)
        {
            xloc[i]=grid[0]+i*grid[3] ;
            pressure[i]=(stressTcxx[i]+stressTcyy[i]+stressTczz[i])/3. ;
            shearstress[i]=-stressTcxy[i] ;
        }

        // if ( counter % 3 == 0 ) { update_graph(); }
        // counter += 1;
        update_graph();
        SPHERES.draw_force_network(S, params, scene);
    }
    renderer.render( scene, camera );
}

async function NDDEMPhysics() {

    // if ( 'DEMCGND' in window === false ) {
    //
    //     console.error( 'NDDEMPhysics: Couldn\'t find DEMCGND.js' );
    //     return;
    //
    // }

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
}


function finish_setup() {
    S.simu_interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
    S.simu_interpret_command("radius -1 0.5");
    let m = 4./3.*Math.PI*0.5*0.5*0.5*params.particle_density;
    S.simu_interpret_command("mass -1 " + String(m));
    S.simu_interpret_command("auto rho");
    S.simu_interpret_command("auto radius uniform "+params.r_min+" "+params.r_max);
    S.simu_interpret_command("auto mass");
    S.simu_interpret_command("auto inertia");
    S.simu_interpret_command("auto skin");

    S.simu_interpret_command("boundary 0 WALL 0 "+String(10*params.L));
    S.simu_interpret_command("boundary 1 PBC -"+String(params.L)+" "+String(params.L));
    S.simu_interpret_command("boundary 2 PBC -"+String(params.L)+" "+String(params.L));
    if ( params.dimension == 4 ) {
        S.simu_interpret_command("boundary 3 PBC -"+String(params.L)+" "+String(params.L));
    }
    S.simu_interpret_command("gravity " + String(-params.g_mag*Math.cos(params.theta*Math.PI/180.)) + " " + String(-params.g_mag*Math.sin(params.theta*Math.PI/180.)) + " 0 " + "0 ".repeat(params.dimension - 3))

    S.simu_interpret_command("auto location roughinclineplane");
    // S.simu_interpret_command("auto location randomdrop");

    let tc = 5e-3;
    let rest = 0.2; // super low restitution coeff to dampen out quickly
    let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, params.particle_mass)

    S.simu_interpret_command("set Kn " + String(vals.stiffness));
    S.simu_interpret_command("set Kt " + String(0.8*vals.stiffness));
    S.simu_interpret_command("set GammaN " + String(vals.dissipation));
    S.simu_interpret_command("set GammaT " + String(vals.dissipation));
    S.simu_interpret_command("set Mu 0.5");
    S.simu_interpret_command("set T 150");
    S.simu_interpret_command("set dt " + String(tc/20));
    S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
    S.simu_interpret_command("auto skin");
    S.simu_finalise_init () ;

    var cgparam ={} ;
    cgparam["file"]=[{"filename":"none", "content": "particles", "format":"interactive", "number":1}] ;
    cgparam["boxes"]=Array(params.dimension).fill(1) ;
    cgparam["boxes"][0] = 20; // more in first dimension
    // cgparam["boundaries"]=[[-params.L,-params.L,-params.L],[params.L,params.L,params.L]] ;
    cgparam["boundaries"]=[
        Array(params.dimension).fill(-params.L+params.r_max),
        Array(params.dimension).fill( params.L-params.r_max)];
    cgparam["boundaries"][0][0] = params.r_max;
    cgparam["boundaries"][1][0] = 4*params.L;
    cgparam["window size"]=2*params.average_radius ;
    cgparam["skip"]=0;
    cgparam["max time"]=1 ;
    cgparam["time average"]="None" ;
    cgparam["fields"]=["RHO", "VAVG", "TC","Pressure","ShearStress"] ;
    cgparam["periodicity"]=Array(params.dimension).fill(true);
    cgparam["window"]="Lucy3D";
    cgparam["dimension"]=params.dimension;


    console.log(JSON.stringify(cgparam)) ;
    S.cg_param_from_json_string(JSON.stringify(cgparam)) ;
    S.cg_setup_CG() ;
    grid = S.cg_get_gridinfo();
}

function update_graph() {
    // density = params.packing_fraction*material_density;
    // vertical_stress_smooth = (vertical_stress + vertical_stress_smooth)/2.;
    // console.log(vertical_stress)
    // shear_time.push(shear);
    // density_time.push(density);

    // if (( Math.abs((pressure_time[pressure_time.length - 2] - pressure )/pressure) > 1e-2 ) || ( Math.abs((shear_time[shear_time.length - 2] - shear )/shear) > 1e-2 ) || ( Math.abs((density_time[density_time.length - 2] - density )/density) > 1e-2 )) {
    //console.log(density,vavg)
    var maxVelocity = vavg.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);

    Plotly.update('stats', {
            'y': [xloc],
            // 'y': [[shear]],
            'x': [vavg],
    }, {
        // xaxis: {
        //     range: [-maxVelocity, maxVelocity],
        //     title: 'Velocity (mm/s)',
        // },
    }, [0])

    Plotly.update('stats', {
            'y': [xloc],
            // 'y': [[shear]],
            'x': [pressure],
    }, {}, [1])

    Plotly.update('stats', {
            'y': [xloc],
            // 'y': [[shear]],
            'x': [shearstress],
    }, {}, [2])

    var update = {
        xaxis: {
            range: [-maxVelocity, maxVelocity],
            title: 'Velocity (mm/s)',
        },
    }
    Plotly.relayout('stats', update);

    /*Plotly.update('stats', {
            'x': [1,2,3,4,5,6,7,8,9,10],
            // 'y': [[shear]],
            'y': [pressure],
    }, [1])


    Plotly.update('stats', {
            'x': [1,2,3,4,5,6,7,8,9,10],
            // 'y': [[shear]],
            'y': [shearstress],
    }, [2])*/
// }
}

// function make_graph() {
//     Plotly.newPlot('stats', plotly_data_three_lines_2d, plotly_layout_three_lines_2d);
// }
//
// document.getElementById ("download_tag").addEventListener ("click", download_data, false);
// document.getElementById ("stats").addEventListener ("mouseenter",
//     () => {
//         document.getElementById("download_tag").classList.remove("hidden")
//         document.getElementById("download_tag").classList.add("visible")
// }, false);
// document.getElementById ("stats").addEventListener ("mouseleave",
//     () => {
//         document.getElementById("download_tag").classList.add("hidden")
//         document.getElementById("download_tag").classList.remove("visible")
// }, false);
//
// function download_data() {
//     let gd = document.getElementById('stats')
//     let data = gd.data;
//     let header = ['Velocity (mm/s)','Pressure (kPa)','Shear stress (kPa)'];
//     let csv = '';
//     let ix = 0;
//     data.forEach( trace => {
//         csv = csv + 'Position (mm),' + header[ix] + '\n' + trace.y.map((el, i) => [el, trace.x[i]].join(",")).join('\n') + '\n';
//         ix += 1;
//         });
//
//     var link = document.getElementById("download_tag");
//     link.setAttribute("href", encodeURI("data:text/csv;charset=utf-8,"+csv));
// }
function make_graph() {
    let { data, layout } = LAYOUT.plotly_two_xaxis_graph('Downslope velocity (m/s)','Stress (Pa)','Height (mm)','Velocity','Pressure (p)','Shear stress (𝜏)');
    var maxStress = 4*params.L*params.particle_density*params.g_mag*0.6;
    layout.xaxis2 = {
        range: [0,maxStress],
        title: 'Stress (Pa)',
        overlaying: 'x',
        side: 'top',
        rangemode: 'tozero',
        color: 'blue'
    }
    Plotly.newPlot('stats', data, layout);
}
