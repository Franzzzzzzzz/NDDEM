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

let info_div = document.createElement("div")
info_div.innerHTML = "Click on a particle to grab it"
info_div.style.color = "white";
info_div.style.position = "absolute";
info_div.style.left = "20px";
info_div.style.top = "20px";
document.body.appendChild(info_div);

var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel, controls;
let gui;
let S;
let cg_mesh, colorbar_mesh;

var params = {
    dimension: 2,
    L: 16*0.004, //system size
    initial_density: 0.88,
    zoom: 1000,
    aspect_ratio: 2,
    // paused: false,
    // g_mag: 1e3,
    // theta: 0, // slope angle in DEGREES
    d4: {cur:0},
    r_max: 0.0045,
    r_min: 0.0035,
    particle_density: 2700,
    // freq: 0.05,
    // new_line: false,
    shear_rate: 2,
    // lut: 'None',
    lut: 'White',
    cg_field: 'Density',
    quality: 5,
    cg_width: 50,
    cg_height: 25,
    cg_opacity: 0.8,
    cg_window_size: 3,
    particle_opacity: 0.5,
    F_mag_max: 5e4,
    audio: false,
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

    let geometry = new THREE.PlaneGeometry( 2*params.L, 2*params.L );
    let material = new THREE.MeshBasicMaterial( {color: 0xffff00, side: THREE.DoubleSide} );
    material.transparent = true;
    cg_mesh = new THREE.Mesh( geometry, material );
    cg_mesh.position.z = -2*params.r_max;
    cg_mesh.scale.x = params.aspect_ratio;
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
    gui.add ( params, 'shear_rate', 0, 10, 0.1).name('Shear rate').listen().onChange( update_wall_particle_velocities );
    // gui.add ( params, 'cg_width', 1, 100,1).name('Resolution').listen().onChange( () => {
        // params.cg_height = params.cg_width;
        // update_cg_params(S, params);
    // });
    // gui.add ( params, 'paused').name('Paused').listen();

    // const controls = new OrbitControls( camera, renderer.domElement );
    // controls.update();

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
    if ( AUDIO.listener !== undefined ){
        SPHERES.update_fixed_sounds(S, params);
    }
    RAYCAST.animate_locked_particle(S, camera, SPHERES.spheres, params);
    if ( !params.paused ) {
        // let v = S.simu_getVelocity();
        // console.log(v);

        S.simu_step_forward(15);
        if ( params.cg_opacity > 0 ) {
            cg_mesh.visible = true;
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
                lut.setMax(params.particle_density*100);
            }
            else if ( params.cg_field === 'Velocity' ) {
                val = S.cg_get_result(0, "VAVG", 1);
                lut = cooltowarm;
                let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
                lut.setMin(-0.9*maxVal);
                lut.setMax( 0.9*maxVal);
            }
            else if ( params.cg_field === 'Pressure' ) {
                const stressTcxx=S.cg_get_result(0, "TC", 0) ;
                const stressTcyy=S.cg_get_result(0, "TC", 3) ;
                const stressTczz=S.cg_get_result(0, "TC", 6) ;
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
                lut.setMin(-0.9*maxVal);
                lut.setMax( 0.9*maxVal);
            }

            for ( let i = 0; i < size; i ++ ) {
                var color = lut.getColor(val[i]);
                // console.log(val[i])
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
        } else {
            cg_mesh.visible = false;
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
    cgparam["fields"]=["RHO", "VAVG", "TC"] ;
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
        
        S.simu_interpret_command("gravity 0 -1000 ");

        S.simu_interpret_command("auto location randomdrop");

        let tc = 1e-4;
        let rest = 0.5; // super low restitution coeff to dampen out quickly
        let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient (tc, rest, params.particle_mass)

        S.simu_interpret_command("set Kn " + String(vals.stiffness));
        S.simu_interpret_command("set Kt " + String(0.8*vals.stiffness));
        S.simu_interpret_command("set GammaN " + String(vals.dissipation));
        S.simu_interpret_command("set GammaT " + String(vals.dissipation));
        S.simu_interpret_command("set Mu 0.5");
        S.simu_interpret_command("set damping " + String(0.1/tc));
        S.simu_interpret_command("set T 1000000");
        S.simu_interpret_command("set dt " + String(tc/20));
        S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
        // S.simu_interpret_command("auto skin");

        // now fix the wall roughness
        let n_wall = Math.ceil(2*params.aspect_ratio*params.L/(params.average_radius*2));
        for ( var i=0; i<n_wall; i++ ) {
            let d = params.average_radius - params.aspect_ratio**params.L + i*params.average_radius*2; // location in x1 of these particles
            console.log("radius " + String(i) + " " + String(params.average_radius));
            S.simu_interpret_command("location " + String(i) + " " + String(d) + " " + String(-params.L));

            S.simu_setRadius(i,params.average_radius);
            S.simu_setVelocity(i,[params.shear_rate*2*params.L,0]);
            S.simu_setFrozen(i);

            S.simu_interpret_command("location " + String(n_wall+i) + " " + String(d) + " " + String(params.L));
            S.simu_setFrozen(n_wall+i);
            S.simu_setRadius(n_wall+i,params.average_radius);
        }

        S.simu_finalise_init () ;

        // let v = S.simu_getVelocity();
        // console.log(v);
        update_cg_params(S, params);

    }
}

function update_wall_particle_velocities() {
    let n_wall = Math.ceil(2*params.aspect_ratio*params.L/(params.average_radius*2));
        for ( var i=0; i<n_wall; i++ ) {
            S.simu_setVelocity(i,[params.shear_rate*2*params.L,0]);
        }
}
