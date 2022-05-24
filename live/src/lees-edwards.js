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
let position;
let gui;
let spheres;
let floor;
let S;
let NDDEMCGLib;
let pointer;
let v, omegaMag;
let radii;
let NDsolids, material, STLFilename;
let meshes = new THREE.Group();
let density, vavg, stressTcxx, stressTcyy, stressTczz, stressTcxy;
let pressure=[], shearstress=[], xloc=[] ;
let show_stats = true;

const raycaster = new THREE.Raycaster();
const mouse = new THREE.Vector2();
let intersection_plane = new THREE.Plane();
let camera_direction = new THREE.Vector3();

let graph_fraction = 0.5;
document.getElementById("stats").style.width = String(100*graph_fraction) + '%';
document.getElementById("canvas").style.width = String(100*(1-graph_fraction)) + '%';

var params = {
    dimension: 3,
    initial_packing_fraction: 0.6,
    // L: 0.025, //system size
    N: 500,
    // packing_fraction: 0.5,
    constant_volume: true,
    axial_strain: 0,
    volumetric_strain: 0,
    paused: false,
    g_mag: 1e3,
    theta: 0, // slope angle in DEGREES
    d4_cur:0,
    r_max: 0.0033,
    r_min: 0.0027,
    freq: 0.05,
    new_line: false,
    shear_rate: 10,
    lut: 'None',
    quality: 5,
    vmax: 50, // max velocity to colour by
    omegamax: 50, // max rotation rate to colour by
    particle_density: 2700,
    particle_opacity: 1.0,
}

set_derived_properties();

function set_derived_properties() {
    params.average_radius = (params.r_min + params.r_max)/2.;
    params.particle_volume = 4./3.*Math.PI*Math.pow(params.average_radius,3);
    console.log('estimate of particle volume: ' + params.particle_volume*params.N)
    params.particle_mass = params.particle_volume * params.particle_density;
    params.L = Math.pow(params.particle_volume*params.N/params.initial_packing_fraction, 1./3.)/2.;
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

SPHERES.createNDParticleShader(params).then( init() );

async function init() {

    await NDDEMCGPhysics();
    // physics.main(params.dimensions, params.N, inputfile)
    position = new THREE.Vector3();

    //

    camera = new THREE.PerspectiveCamera( 50, window.innerWidth*(1-graph_fraction) / window.innerHeight, 1e-5, 1000 );
    camera.position.set( 0, 0, -5*params.L );
    camera.up.set(1, 0, 0);

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
        gui.add( params, 'd4_cur', -params.L,params.L, 0.001)
            .name( 'D4 location').listen()
            .onChange( function () {
                if ( urlParams.has('stl') ) {
                    meshes = renderSTL( meshes, NDsolids, scene, material, params.d4_cur );
                }
            });
    }
    gui.add ( params, 'shear_rate', 0, 100, 0.1).name('Shear rate (1/s) (W/S)').listen()
        .onChange( () => update_shear_rate() );
    gui.add ( params, 'particle_opacity',0,1).name('Particle opacity').listen().onChange( () => SPHERES.update_particle_material(params,
        // lut_folder
    ));
    gui.add ( params, 'lut', ['None', 'Velocity', 'Fluct Velocity', 'Rotation Rate' ]).name('Colour by')
            .onChange( () => SPHERES.update_particle_material(params,
                // lut_folder
            ) );
    gui.add ( params, 'paused').name('Paused').listen();
    // gui.add ( params, 'new_line').name('New loading path').listen()
    //     .onChange( () => {
    //         var data = [{
    //                       type: 'scatter',
    //                       mode: 'lines',
    //                       x: [], y: [],
    //                       line: { width: 5 },
    //                       name: 'Load path ' + String(document.getElementById('stats').data.length+1)
    //                     }]
    //         Plotly.addTraces('stats', data);
    //         params.new_line = false;
    //     });
    controls = new OrbitControls( camera, container );
    controls.update();

    window.addEventListener( 'resize', onWindowResize, false );
    window.addEventListener( 'keypress', checkKeys, false );

    if ( show_stats ) { make_graph(); }

    animate();
}

function update_shear_rate() {
    S.simu_setBoundary(0, [-params.L,params.L,params.shear_rate]);
    params.vmax = 1.5*params.shear_rate*params.L;
}

function checkKeys( event ) {
    if ( event.code === 'KeyW' ) {
        params.shear_rate += 0.1;
        update_shear_rate();
        }
    if ( event.code === 'KeyS' ) {
        params.shear_rate -= 0.1;
        update_shear_rate();
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
        var grid = S.cg_get_gridinfo();
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

        update_graph();
        SPHERES.draw_force_network(S, params, scene);
    }
    renderer.render( scene, camera );
}


// function add_spheres() {
//     radii = S.simu_getRadii();
//     spheres = new THREE.Group();
//     scene.add(spheres);
//
//     const color = new THREE.Color();
//
//     const geometrySphere = new THREE.SphereGeometry( 0.5, Math.pow(2,quality), Math.pow(2,quality) );
//
//     for ( let i = 0; i < params.N; i ++ ) {
//         const material = NDParticleShader.clone();
//         var object = new THREE.Mesh(geometrySphere, material);
//         object.position.set(0,0,0);
//         object.rotation.z = Math.PI / 2;
//         object.NDDEM_ID = i;
//         spheres.add(object);
//     }
// }
//
// function move_spheres() {
//     var x = S.simu_getX();
//     var orientation = S.simu_getOrientation();
//     if ( params.lut === 'Velocity' || params.lut === 'Fluct Velocity' ) {
//         v = S.simu_getVelocity();
//     }
//     else if ( params.lut === 'Rotation Rate' ) {
//         omegaMag = S.simu_getRotationRate();
//     }
//     else if ( params.lut === 'Force' ) {
//         forceMag = S.simu_getParticleStress(); // NOTE: NOT IMPLEMENTED YET
//     }
//
//     for ( let i = 0; i < params.N; i ++ ) {
//         var object = spheres.children[i];
//         if ( params.dimension == 3 ) {
//             var D_draw = 2*radii[i];
//             object.scale.set(D_draw, D_draw, D_draw);
//         }
//         else if ( params.dimension == 4 ) {
//             var D_draw = 2*Math.sqrt(
//               Math.pow(radii[i], 2) - Math.pow(params.d4_cur - x[i][3], 2)
//             );
//             object.scale.set(D_draw, D_draw, D_draw);
//         }
//         object.position.set( x[i][0], x[i][1], x[i][2] );
//         object.material.uniforms.R.value = radii[i];
//         if ( params.lut === 'Velocity' ) {
//             object.material.uniforms.ambient.value = 0.5 + 1e-4*( Math.pow(v[i][0],2) + Math.pow(v[i][1],2) + Math.pow(v[i][2],2) );
//         }
//         if ( params.lut === 'Fluct Velocity' ) {
//             object.material.uniforms.ambient.value = 0.5 + 1e-3*( Math.pow(v[i][0],2) + Math.pow(v[i][1] - params.shear_rate*x[i][0],2) + Math.pow(v[i][2],2) );
//             // object.material.uniforms.ambient.value = params.shear_rate*x[i][0];
//         }
//         if ( params.lut === 'Rotation Rate' ) {
//             // console.log(omegaMag[i])
//             object.material.uniforms.ambient.value = 0.5 + 0.01*omegaMag[i];
//         }
//         for (var j = 0; j < params.N - 3; j++) {
//           object.material.uniforms.xview.value[j] =
//             params.d4_cur;
//           object.material.uniforms.xpart.value[j] =
//             x[i][j + 3];
//         }
//         object.material.uniforms.A.value = orientation[i];
//     }
// }

async function NDDEMCGPhysics() {

    // if ( 'DEMCGND' in window === false ) {
    //     console.error( 'NDDEMCGPhysics: Couldn\'t find DEMCGND.js' );
    //     return;
    // }

    // NDDEMCGLib = await DEMCGND(); // eslint-disable-line no-undef
    await DEMCGND().then( (NDDEMCGLib) => {
        if ( params.dimension == 3 ) {
            S = new NDDEMCGLib.DEMCGND (params.N);
            finish_setup();
        }
        else if ( params.dimension > 3 ) {
            console.log("D>3 not available") ;
            /*S = await new NDDEMLib.Simulation4 (params.N);
            finish_setup();*/
        }
    });



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

        S.simu_interpret_command("boundary 0 PBCLE -"+String(params.L)+" "+String(params.L)+" "+String(params.shear_rate));
        S.simu_interpret_command("boundary 1 PBC -"+String(params.L)+" "+String(params.L));
        S.simu_interpret_command("boundary 2 PBC -"+String(params.L)+" "+String(params.L));
        if ( params.dimension == 4 ) {
            S.simu_interpret_command("boundary 3 PBC -"+String(params.L)+" "+String(params.L));
        }
        S.simu_interpret_command("gravity 0 0 " + "0 ".repeat(params.dimension - 3))

        S.simu_interpret_command("auto location randomdrop");

        let tc = 1e-3;
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
        S.simu_finalise_init () ;

        var cgparam ={} ;
        cgparam["file"]=[{"filename":"none", "content": "particles", "format":"interactive", "number":1}] ;
        cgparam["boxes"]=[20,1,1] ;
        // cgparam["boundaries"]=[[-params.L,-params.L,-params.L],[params.L,params.L,params.L]] ;
        cgparam["boundaries"]=[
            [-params.L+params.r_max,-params.L+params.r_max,-params.L+params.r_max],
            [ params.L-params.r_max, params.L-params.r_max, params.L-params.r_max]] ;
        cgparam["window size"]=2*params.average_radius ;
        cgparam["skip"]=0;
        cgparam["max time"]=1 ;
        cgparam["time average"]="None" ;
        cgparam["fields"]=["RHO", "VAVG", "TC"] ;
        cgparam["periodicity"]=[true,true,true];
        cgparam["window"]="Lucy3D";
        cgparam["dimension"]=3;


        console.log(JSON.stringify(cgparam)) ;
        S.cg_param_from_json_string(JSON.stringify(cgparam)) ;
        S.cg_setup_CG() ;

    }
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
        xaxis: {
            range: [-maxVelocity, maxVelocity],
            title: 'Velocity (mm/s)',
        }
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
//     var data = [{
//       type: 'scatter',
//       mode: 'lines',
//       x: [],
//       y: [],
//       name: 'Velocity',
//       // opacity: 1,
//       line: {
//         width: 5,
//         color: "black",
//         // reversescale: false
//       },
//     }, {
//       type: 'scatter',
//       mode: 'lines',
//       x: [],
//       y: [],
//       name: 'Pressure (p)',
//       // opacity: 1,
//       line: {
//         width: 5,
//         color: "blue",
//         // reversescale: false
//       },
//       xaxis: 'x2'
//     }, {
//       type: 'scatter',
//       mode: 'lines',
//       x: [],
//       y: [],
//       name: 'Shear stress (q)',
//       // opacity: 1,
//       line: {
//         // dash: 'dash',
//         dash: "8px,8px",
//         width: 5,
//         color: "blue",
//         // reversescale: false
//       },
//       xaxis: 'x2'
//     }]
//     var layout = {
//           // height: 300,
//           // width: 500,
//           xaxis: {
//             // linecolor: 'white',
//             autotick: true,
//             // autorange: true,
//             // range: [-maxVelocity, maxVelocity],
//             // range: [-1,1],
//             automargin: true,
//             title: 'Average velocity (m/s)',
//             side: 'bottom'
//             // title: 'Vertical displacement (mm)'
//         },
//           yaxis: {
//             // linecolor: 'white',
//             autotick: true,
//             autorange: true,
//             automargin: true,
//             title: 'Location (mm)',
//             // color: 'black',
//         },
//         xaxis2: {
//             autotick: true,
//             autorange: true,
//             automargin: true,
//             title: 'Stress (kPa)',
//             overlaying: 'x',
//             side: 'top',
//             rangemode: 'tozero',
//             color: 'blue'
//             },
//         legend: {
//             x: 1,
//             xanchor: 'right',
//             y: 1,
//             // bgcolor: "rgba(0,0,0,0.01)"
//             // opacity: 0.5,
//         },
//         margin: {
//             b: 100,
//         },
//         font: {
//             family: 'Montserrat, Open sans',
//         }
//     }
//     Plotly.newPlot('stats', data, layout);
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
    let { data, layout } = LAYOUT.plotly_two_xaxis_graph('Average velocity (m/s)','Stress (Pa)','Location (mm)','Velocity','Pressure (p)','Shear stress (𝜏)');
    Plotly.newPlot('stats', data, layout);
}
