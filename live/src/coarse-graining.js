import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';
import { Lut } from 'three/examples/jsm/math/Lut.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';
import * as RAYCAST from '../libs/RaycastHandler.js';

var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel, controls;
let gui;
let S;
let NDDEMCGLib;
let cg_mesh, colorbar_mesh;
let pointer;
let v, omegaMag;
let radii;
let particle_volume;

var params = {
    dimension: 3,
    L: 5, //system size
    N: 70,
    zoom: 15,
    // packing_fraction: 0.5,
    constant_volume: true,
    axial_strain: 0,
    volumetric_strain: 0,
    paused: false,
    g_mag: 1e3,
    theta: 0, // slope angle in DEGREES
    d4_cur:0,
    r_max: 0.501,
    r_min: 0.499,
    freq: 0.05,
    new_line: false,
    shear_rate: 10,
    lut: 'White',
    cg_field: 'Density',
    quality: 5,
    cg_width: 50,
    cg_height: 50,
    cg_opacity: 0.8,
    cg_window_size: 1.5,
    aspect_ratio: 1,
}

let rainbow    = new Lut("rainbow", 512); // options are rainbow, cooltowarm and blackbody
let cooltowarm = new Lut("cooltowarm", 512); // options are rainbow, cooltowarm and blackbody
let blackbody  = new Lut("blackbody", 512); // options are rainbow, cooltowarm and blackbody

params.average_radius = (params.r_min + params.r_max)/2.;
params.thickness = params.average_radius;

particle_volume = 4./3.*Math.PI*Math.pow(params.average_radius,3);
if ( urlParams.has('dimension') ) {
    params.dimension = parseInt(urlParams.get('dimension'));
}
if ( params.dimension === 4) {
    params.L = 2.5;
    params.N = 300
    particle_volume = Math.PI*Math.PI*Math.pow(params.average_radius,4)/2.;
}

if ( urlParams.has('quality') ) { params.quality = parseInt(urlParams.get('quality')); }

SPHERES.createNDParticleShader(params).then( init() );

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

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add( hemiLight );

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set( 5, -5, -5 );
    dirLight.castShadow = true;
    scene.add( dirLight );

    SPHERES.add_spheres(S,params,scene);

    WALLS.add_back(params, scene);
    WALLS.add_left(params, scene);
    WALLS.add_right(params, scene);
    WALLS.back.scale.y = params.thickness;//Math.PI/2.;
    var vert_walls = [WALLS.left,WALLS.right,WALLS.back];

    vert_walls.forEach( function(mesh) {
        mesh.scale.x = 2*params.L + 2*params.thickness;
        mesh.scale.z = 2*params.L + 2*params.thickness;
    });

    let geometry = new THREE.PlaneGeometry( 2*params.L, 2*params.L );
    let material = new THREE.MeshBasicMaterial( {color: 0xffff00, side: THREE.DoubleSide} );
    material.transparent = true;
    cg_mesh = new THREE.Mesh( geometry, material );
    cg_mesh.position.z = -1;
    scene.add( cg_mesh );

    // geometry = new THREE.PlaneGeometry( 2*params.L, 0.1*params.L );
    // material = new THREE.MeshBasicMaterial( {color: 0xffff00, side: THREE.DoubleSide} );
    // colorbar_mesh = new THREE.Mesh( geometry, material );
    // colorbar_mesh.position.y = -1.1*params.L;
    // scene.add( colorbar_mesh );


    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
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
    gui.add ( params, 'cg_opacity', 0, 1).name('Opacity').listen();
    gui.add ( params, 'cg_field', ['Density', 'Velocity', 'Pressure', 'Shear stress']).name('Field').listen();
    gui.add ( params, 'cg_window_size', 0.5, params.L/2.).name('Window size').listen().onChange( () => {
        update_cg_params(S, params);
    });
    // gui.add ( params, 'cg_width', 1, 100,1).name('Resolution').listen().onChange( () => {
        // params.cg_height = params.cg_width;
        // update_cg_params(S, params);
    // });
    // gui.add ( params, 'paused').name('Paused').listen();

    // const controls = new OrbitControls( camera, renderer.domElement );
    // controls.update();

    window.addEventListener( 'resize', onWindowResize, false );

    animate();
}

function onWindowResize(){

    // camera.aspect = window.innerWidth / window.innerHeight;
    // camera.updateProjectionMatrix();
    var aspect = window.innerWidth / window.innerHeight;
    // camera.left = -params.zoom * aspect;
    // camera.right = params.zoom * aspect;
    // camera.bottom = -params.zoom;
    // camera.top = params.zoom;

    renderer.setSize( window.innerWidth, window.innerHeight );
}

function animate() {
    requestAnimationFrame( animate );
    SPHERES.move_spheres(S,params);
    RAYCAST.animate_locked_particle(S, camera, SPHERES.spheres, params);
    if ( !params.paused ) {
        S.simu_step_forward(5);
        S.cg_param_read_timestep(0) ;
        S.cg_process_timestep(0,false) ;
        var grid = S.cg_get_gridinfo();
        const size = params.cg_width * params.cg_height;
        const data = new Uint8Array( 4 * size );
        const opacity = parseInt(255 * params.cg_opacity);
        let val;
        let lut;
        if ( params.cg_field === 'Density' ) {
            val = S.cg_get_result(0, "RHO", 0);
            lut = rainbow;
                                    let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(0);
            lut.setMax(1);
        }
        else if ( params.cg_field === 'Velocity' ) {
            val = S.cg_get_result(0, "VAVG", 1);
            lut = cooltowarm;
            let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(-maxVal);
            lut.setMax(maxVal);
        }
        else if ( params.cg_field === 'Pressure' ) {
            const stressTcxx=S.cg_get_result(0, "TC", 0) ;
            const stressTcyy=S.cg_get_result(0, "TC", 4) ;
            const stressTczz=S.cg_get_result(0, "TC", 8) ;
            val = new Array(stressTcxx.length);
            for (var i=0 ; i<stressTcxx.length ; i++)
            {
                val[i]=(stressTcxx[i]+stressTcyy[i]+stressTczz[i])/3. ;
            }
            lut = rainbow;
            let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(0);
            lut.setMax(0.9*maxVal);
        } else if ( params.cg_field === 'Shear stress' ) {
            val = S.cg_get_result(0, "TC", 1);
            lut = cooltowarm;
            let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(-maxVal);
            lut.setMax( maxVal);
        }




        for ( let i = 0; i < size; i ++ ) {
            var color = lut.getColor(val[i]);
            // console.log(lut)
            const r = Math.floor( color.r * 255 );
            const g = Math.floor( color.g * 255 );
            const b = Math.floor( color.b * 255 );
            const stride = i * 4;
            data[ stride     ] = r;//parseInt(val[i]/maxVal*255);
            data[ stride + 1 ] = g;
            data[ stride + 2 ] = b;
            if ( val[i] === 0 ) {
                data[ stride + 3 ] = 0;
            } else {
                data[ stride + 3 ] = opacity;
            }


        }
        const texture = new THREE.DataTexture( data, params.cg_width, params.cg_height );
        // texture.magFilter = THREE.LinearFilter; // smooth the data artifically
        texture.needsUpdate = true;
        cg_mesh.material.map = texture;
        // cg_mesh.material.opacity = parseInt(255*params.opacity);
    }
    renderer.render( scene, camera );
}
function update_cg_params(S, params) {
    var cgparam ={} ;
    cgparam["file"]=[{"filename":"none", "content": "particles", "format":"interactive", "number":1}] ;
    cgparam["boxes"]=[params.cg_width,params.cg_height,1] ;
    // cgparam["boundaries"]=[[-params.L,-params.L,-params.L],[params.L,params.L,params.L]] ;
    cgparam["boundaries"]=[
        [-params.L,-params.L,-1],
        [ params.L, params.L, 1]] ;
    cgparam["window size"]=params.cg_window_size ;
    cgparam["skip"]=0;
    cgparam["max time"]=1 ;
    cgparam["time average"]="None" ;
    cgparam["fields"]=["RHO", "VAVG", "TC"] ;
    cgparam["periodicity"]=[false,false,false];
    cgparam["window"]="Lucy3D";
    cgparam["dimension"]=3;


    // console.log(JSON.stringify(cgparam)) ;
    S.cg_param_from_json_string(JSON.stringify(cgparam)) ;
    S.cg_setup_CG() ;
}

async function NDDEMCGPhysics() {

    // if ( 'DEMCGND' in window === false ) {
    //     console.error( 'NDDEMCGPhysics: Couldn\'t find DEMCGND.js' );
    //     return;
    // }

    // NDDEMCGLib = await DEMCGND(); // eslint-disable-line no-undef
    await DEMCGND().then( (NDDEMCGLib) => {
        if ( params.dimension == 3 ) {
            S = await new NDDEMCGLib.DEMCGND (params.N);
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
        S.simu_interpret_command("mass -1 1");
        S.simu_interpret_command("auto rho");
        S.simu_interpret_command("auto radius uniform "+params.r_min+" "+params.r_max);
        S.simu_interpret_command("auto mass");
        S.simu_interpret_command("auto inertia");

        S.simu_interpret_command("boundary 0 WALL -"+String(params.L)+" "+String(params.L));
        S.simu_interpret_command("boundary 1 WALL -"+String(params.L)+" "+String(params.L));
        S.simu_interpret_command("boundary 2 WALL -0.51 0.51");
        if ( params.dimension == 4 ) {
            S.simu_interpret_command("boundary 3 PBC -"+String(params.L)+" "+String(params.L));
        }
        S.simu_interpret_command("gravity -1000 0 " + "0 ".repeat(params.dimension - 3))

        S.simu_interpret_command("auto location randomdrop");

        S.simu_interpret_command("set Kn 2e5");
        S.simu_interpret_command("set Kt 8e4");
        S.simu_interpret_command("set GammaN 75");
        S.simu_interpret_command("set GammaT 75");
        S.simu_interpret_command("set Mu 1");
        S.simu_interpret_command("set Mu_wall 0");
        S.simu_interpret_command("set T 150");
        S.simu_interpret_command("set dt 0.0002");
        S.simu_interpret_command("set tdump 10"); // how often to calculate wall forces
        S.simu_interpret_command("auto skin");
        S.simu_finalise_init () ;

        update_cg_params(S, params);

    }
}