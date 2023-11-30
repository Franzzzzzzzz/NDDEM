import css from "../css/main.css";
import css1 from "../css/code.css";

import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
import * as monaco from 'monaco-editor';

monaco.languages.register({ id: 'infile'});
monaco.languages.setMonarchTokensProvider('infile', {
    tokenizer: {
        root: [
            [/#.*$/, 'comment'],
            [/\b(dimensions|radius|mass|auto|boundary|gravity|location|set|dt|tdump|finalise)\b/, 'keyword'],
            [/\b(uniform|randomdrop)\b/, 'keyword'],
            [/\b(PBC|WALL)\b/, 'keyword'],
            [/\b(0|1|2|3|4|5|6|7|8|9)\b/, 'number'],
            [/\b([a-zA-Z]+)\b/, 'identifier'],
        ]
    }
});

let params = {
    quality : 6,
    lut : 'None',
    particle_opacity : 0.8,
    graph_fraction : 0.333,
    boundary0 : {type: 'None', min: 0, max: 0},
    boundary1 : {type: 'None', min: 0, max: 0},
    boundary2 : {type: 'None', min: 0, max: 0},
};

const resizer = document.getElementById('divider');
const leftDiv = document.getElementById('container');
const rightDiv = document.getElementById('canvas');
let isDragging = false;

const resizer2 = document.getElementById('row-divider');
const topDiv = document.getElementById('code');
const botDiv = document.getElementById('logs');
let isDragging2 = false;

resizer.addEventListener('mousedown', function(e) {
    isDragging = true;
    e.preventDefault(); // This prevents unwanted behaviors
});

document.addEventListener('mousemove', function(e) {
    if (isDragging) {
        const containerOffsetLeft = leftDiv.offsetTop;
        const containerWidth = leftDiv.offsetWidth + rightDiv.offsetWidth;
        const leftWidth = e.clientX - containerOffsetLeft;
        const rightWidth = containerWidth - leftWidth;

        leftDiv.style.width = `${leftWidth}px`;
        rightDiv.style.width = `${rightWidth}px`;

        params.graph_fraction = leftWidth/containerWidth;
        // move the scigem tag and make the font white
        let c = document.getElementById("scigem_tag");
        c.style.left = 'calc('+String(100*params.graph_fraction)+'% + 5px)';

        onWindowResize();
    }
    if (isDragging2) {
        const containerOffset = topDiv.offsetTop;
        const containerHeight = topDiv.offsetHeight + botDiv.offsetHeight;
        const topHeight = e.clientY - containerOffset;
        const bottomHeight = containerHeight - topHeight;

        topDiv.style.height = `${topHeight}px`;
        botDiv.style.height = `${bottomHeight}px`;
    }
});


resizer2.addEventListener('mousedown', function(e) {
    isDragging2 = true;
    e.preventDefault(); // This prevents unwanted behaviors
});

document.addEventListener('mouseup', function() {
    isDragging = false;
    isDragging2 = false;
});

let urlParams = new URLSearchParams(window.location.search);
let script_type = 'infile';

const toggleSwitch= document.getElementById("toggle-switch");

toggleSwitch.addEventListener("click", function() {
    script_type = (script_type === "javascript") ? "infile" : "javascript";
    let text = localStorage.getItem(script_type + "-script")
    editor.getModel().setValue(text);
    monaco.editor.setModelLanguage(editor.getModel(), script_type)
    console.log("Script type is now: " + script_type);
    update_from_text();
});

let default_scripts = {}
default_scripts['javascript'] = `S.simu_interpret_command("dimensions 2 30");
S.simu_interpret_command("radius -1 0.5");
S.simu_interpret_command("mass -1 1");
S.simu_interpret_command("auto rho");
S.simu_interpret_command("auto radius uniform 0.1 0.2");
S.simu_interpret_command("auto mass");
S.simu_interpret_command("auto inertia");
S.simu_interpret_command("auto skin");

S.simu_interpret_command("boundary 0 PBC -1 1");
S.simu_interpret_command("boundary 1 WALL -1 1");
// S.simu_interpret_command("boundary 2 WALL -2 2");
S.simu_interpret_command("gravity 0 -5 -5");

S.simu_interpret_command("auto location randomdrop");

S.simu_interpret_command("set Kn 75");
S.simu_interpret_command("set Kt " + String(0.8*75));
S.simu_interpret_command("set GammaN 0.1");
S.simu_interpret_command("set GammaT 0.1");
S.simu_interpret_command("set Mu 0.5");
S.simu_interpret_command("set T 150");
S.simu_interpret_command("set dt " + String(0.01/20));
S.simu_interpret_command("set tdump 1000");
S.simu_interpret_command("auto skin");
S.simu_finalise_init();`;

default_scripts['infile'] = `dimensions 3 100
radius -1 0.5
mass -1 1
auto rho
auto radius uniform 0.1 0.2
auto mass
auto inertia
auto skin

boundary 0 PBC -1 1
boundary 1 PBC -1 1
boundary 2 WALL -2 2
gravity 0 -5 -5

auto location randomdrop

set Kn 75
set Kt 60
set GammaN 0.1
set GammaT 0.1
set Mu 0.5
set T 150
set dt 0.0005
set tdump 1000
auto skin`;

let script_types = ['javascript', 'infile'];
script_types.forEach( (s) => {
    // console.log("Checking for default " + s + " script");
    if (localStorage.getItem(s + "-script")) { // it is 'truthy'
        // console.log("Found default " + s + " script: ", localStorage.getItem(s + "-script"));
    } else {
        localStorage.setItem(s + "-script", default_scripts[s]);
        // console.log("Setting default " + s + " script")
    }
});

let current_value = localStorage.getItem(script_type + "-script"); //default_scripts[script_type];
// console.log('EDITOR DEFAULT IS: ', current_value);
const editor = monaco.editor.create(document.getElementById("code"), {
	current_value,
	language: "javascript",
	automaticLayout: true,
    theme: "vs-dark",
});
editor.setValue(current_value); // not sure why
monaco.editor.setModelLanguage(editor.getModel(), script_type)

editor.onDidChangeModelContent(() => {
    update_from_text();
});

// move the scigem tag and make the font white
let c = document.getElementById("scigem_tag");
c.style.color = 'white';
c.style.left = 'calc('+String(100*params.graph_fraction)+'% + 5px)';
document.querySelectorAll('#scigem_tag a').forEach(function(a) {
    a.style.color = 'white';
});


function update_from_text() {
    // reset everything
    logs.innerHTML = '';
    S = undefined;
    scene.remove(SPHERES.spheres)
    SPHERES.wipe(scene);

    let text = editor.getValue();
    localStorage.setItem(script_type + "-script", text);

    if ( script_type === 'infile' ) {
        let prependString = 'S.simu_interpret_command("';
        let appendString = '");';
    
        text = text.split('\n')
        // .map(line => prependString + line + appendString)
        .map(line => line.trim() === '' ? line : prependString + line + appendString)
        .join('\n');
        text += "\nS.simu_finalise_init () ;";
    }

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

    if ( params.dimension < 3 ) {
        camera.position.x = (params.boundary0.max + params.boundary0.min)/2.;
        camera.position.y = (params.boundary1.max + params.boundary1.min)/2.;
        camera.position.z = (params.boundary0.max - params.boundary0.min)*2.;
        
    }

    controls.update();

    SPHERES.createNDParticleShader(params).then( () => {
        if ( params.dimension == 1 ) {
            S = new NDDEMCGLib.DEMCG1D (params.N);
        } else if ( params.dimension == 2 ) {
            S = new NDDEMCGLib.DEMCG2D (params.N);
        } else if ( params.dimension == 3 ) {
            S = new NDDEMCGLib.DEMCG3D (params.N);
        } else if ( params.dimension == 4 ) {
            S = new NDDEMCGLib.DEMCG4D (params.N);
        } else if ( params.dimension == 5 ) {
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

let camera, scene, renderer, controls;
let S;
let NDDEMCGLib;

document.getElementById("container").style.width = String(100*params.graph_fraction) + '%';
document.getElementById("canvas").style.width = String(100*(1-params.graph_fraction)) + '%';

async function init() {

    camera = new THREE.PerspectiveCamera( 50, window.innerWidth*(1-params.graph_fraction) / window.innerHeight, 1e-5, 1000 );
    camera.up.set(0, 0, 1);

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x222222 );

    const light = new THREE.AmbientLight();
    scene.add( light );

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth*(1-params.graph_fraction), window.innerHeight );

    var container = document.getElementById( 'canvas' );
    container.appendChild( renderer.domElement );
    
    controls = new OrbitControls( camera, container );

    window.addEventListener( 'resize', onWindowResize, false );

    WALLS.createWalls(scene);

    animate();

    update_from_text();
}

function onWindowResize(){

    camera.aspect = window.innerWidth*(1-params.graph_fraction) / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth*(1-params.graph_fraction), window.innerHeight );

}

function animate() {
    requestAnimationFrame( animate );
    if ( S !== undefined ) {
        if ( SPHERES.spheres !== undefined ) {
            SPHERES.move_spheres(S,params);
            WALLS.update(params);
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