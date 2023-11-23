import css from "../css/main.css";

import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
import * as monaco from 'monaco-editor';

let code = document.createElement("div");
code.id = 'code';
code.style.height = '50%';
document.getElementById("stats").appendChild(code);
let logs = document.createElement("div");
logs.id = 'logs';
logs.style.overflow = 'auto';
logs.style.height = '50%';
logs.style.fontSize = 'smaller';
document.getElementById("stats").appendChild(logs);

let params = {
    quality : 6,
    lut : 'None',
    particle_opacity : 0.8,
};

let value = `S.simu_interpret_command("dimensions 3 1000");
S.simu_interpret_command("radius -1 0.5");
S.simu_interpret_command("mass -1 1");
S.simu_interpret_command("auto rho");
S.simu_interpret_command("auto radius uniform 0.1 0.2");
S.simu_interpret_command("auto mass");
S.simu_interpret_command("auto inertia");
S.simu_interpret_command("auto skin");

S.simu_interpret_command("boundary 0 PBC -1 1");
S.simu_interpret_command("boundary 1 PBC -1 1");
S.simu_interpret_command("boundary 2 WALL -2 2");
S.simu_interpret_command("gravity 0 -5 -5");

S.simu_interpret_command("auto location randomdrop");

S.simu_interpret_command("set Kn 75");
S.simu_interpret_command("set Kt " + String(0.8*75));
S.simu_interpret_command("set GammaN 0.1");
S.simu_interpret_command("set GammaT 0.1");
S.simu_interpret_command("set Mu 0.5");
S.simu_interpret_command("set T 150");
S.simu_interpret_command("set dt " + String(0.01/20));
S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
S.simu_interpret_command("auto skin");
S.simu_finalise_init () ;
`;

if (!localStorage.getItem("script")) {
    localStorage.setItem("script", value);
} else { 
    value = localStorage.getItem("script");
}

// Hover on each property to see its docs!
const editor = monaco.editor.create(document.getElementById("code"), {
	value,
	language: "javascript",
	automaticLayout: true,
    theme: "vs-dark",
});

editor.onDidChangeModelContent(update_from_text);

function update_from_text() {
    // reset everything
    logs.innerHTML = '';
    S = undefined;
    scene.remove(SPHERES.spheres)
    SPHERES.wipe(scene);

    let text = editor.getValue();
    localStorage.setItem("script", text);
    let first_line = text.trim().split('\n')[0];
    let list = first_line.split(' ');

    params.dimension = parseInt(list[1]);
    params.N = parseInt(list[2]);
    params.text = text;

    for ( let d=0; d<params.dimension; d++ ) {
        let D = text.split('\n').find(line => line.includes('boundary ' + String(d))).split(' ');
        params['boundary'+d] = {}
        params['boundary'+d].type = D[2];
        params['boundary'+d].min = parseFloat(D[3]);
        params['boundary'+d].max = parseFloat(D[4]);
    }
    for ( let d=params.dimension; d<3; d++ ) {
        params['boundary'+d] = {}
        params['boundary'+d].type = 'None';
        params['boundary'+d].min = 0;
        params['boundary'+d].max = 0;
    }
    controls.target.x = params.boundary0.min + (params.boundary0.max - params.boundary0.min)/2;
    controls.target.y = params.boundary1.min + (params.boundary1.max - params.boundary1.min)/2;
    controls.target.z = params.boundary2.min + (params.boundary2.max - params.boundary2.min)/2;

    camera.position.x = params.boundary0.max + (params.boundary0.max - params.boundary0.min);
    camera.position.y = params.boundary1.max + (params.boundary1.max - params.boundary1.min);
    camera.position.z = params.boundary2.max + (params.boundary2.max - params.boundary2.min);

    controls.update();
    // console.log(params)

    SPHERES.createNDParticleShader(params).then( () => {
        if ( params.dimension == 3 ) {
            S = new NDDEMCGLib.DEMCG3D (params.N);
        }
        else if ( params.dimension == 4 ) {
            S = new NDDEMCGLib.DEMCG4D (params.N);
        }
        else if ( params.dimension == 5 ) {
            S = new NDDEMCGLib.DEMCG5D (params.N);
        }
        eval(params.text);
        SPHERES.add_spheres(S,params,scene);
    });
}

// pipe console.log to the logs div
if (typeof console  != "undefined") 
    if (typeof console.log != 'undefined')
        console.olog = console.log;
    else
        console.olog = function() {};

console.log = function(message) {
    console.olog(message);
    logs.innerHTML += message + '<br>';
    logs.scrollTop = logs.scrollHeight; // scroll to bottom of logs
};
console.error = console.debug = console.info =  console.log


var urlParams = new URLSearchParams(window.location.search);

let camera, scene, renderer, stats, panel, controls;
let S;
let NDDEMCGLib;

let graph_fraction = 0.333;
document.getElementById("stats").style.width = String(100*graph_fraction) + '%';
document.getElementById("canvas").style.width = String(100*(1-graph_fraction)) + '%';

async function init() {

    camera = new THREE.PerspectiveCamera( 50, window.innerWidth*(1-graph_fraction) / window.innerHeight, 1e-5, 1000 );
    camera.up.set(0, 0, 1);

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x111 );

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add( hemiLight );

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth*(1-graph_fraction), window.innerHeight );

    var container = document.getElementById( 'canvas' );
    container.appendChild( renderer.domElement );
    
    controls = new OrbitControls( camera, container );

    window.addEventListener( 'resize', onWindowResize, false );

    animate();

    update_from_text();
}

function onWindowResize(){

    camera.aspect = window.innerWidth*(1-graph_fraction) / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth*(1-graph_fraction), window.innerHeight );

}

function animate() {
    requestAnimationFrame( animate );
    if ( S !== undefined ) {
        if ( SPHERES.spheres !== undefined ) {
            SPHERES.move_spheres(S,params);
            S.simu_step_forward(5);
            // SPHERES.draw_force_network(S, params, scene);
        }
    }
    renderer.render( scene, camera );

}

async function NDDEMPhysics() {
    await DEMCGND().then( (lib) => {
        NDDEMCGLib = lib;
    } );
}

NDDEMPhysics().then( () => {
    init();
});