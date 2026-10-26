import '../css/main.css';

import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';
import * as SPHERES from '../libs/SphereHandler.js';
import * as WALLS from '../libs/WallHandler.js';
import * as CGHANDLER from '../libs/CGHandler.js';
import * as COLORBAR from '../libs/colorbar.js';
import { EarthPressureSimulation } from '../libs/EarthPressureSimulation.js';

const params = {
    dimension: 2,
    quality: 5,
    N: 0,
    particle_count: 1000,
    aspect_ratio: 2, // Initial bed width / height; gravity changes the settled height.
    radius_ratio: 0.67,
    L: 0.33,
    thickness: 0.01,
    particle_density: 2700,
    lut: 'None',
    particle_opacity: 0.5,
    F_mag_max: 1000,
    cg_field: 'Density',
    cg_vertical_axis: 1, // y is vertical in this experiment.
    cg_opacity: 0.8,
    cg_width: 50,
    cg_height: 35,
    cg_window_size: 3,
    displacement: 0,
    actual: 0,
    thrust: 0,
    paused: false,
    status: 'Loading physics',
};

let model, camera, scene, renderer, controls, gui;
let actionControls;
let dragging = false;
const pointer = new THREE.Vector3();
const info = document.createElement('div');
info.id = 'info_div';
info.textContent = 'Active: move the wall left. Passive: move the wall right.';
document.body.appendChild(info);

function setTarget(mm) {
    if (!model?.ready) return;
    model.setTarget(mm / 1000);
    params.displacement = model.target * 1000;
}

async function resetBed() {
    actionControls.forEach(control => control.disable());
    params.status = 'Loading physics';
    params.paused = false;
    params.displacement = 0;
    params.actual = 0;
    params.thrust = 0;
    CGHANDLER.cg_mesh.visible = false;
    COLORBAR.hideColorbar();
    if (model) {
        model.S.delete();
        model = undefined;
    }
    try {
        // A fresh module also clears the engine's shared internal state.
        const library = await window.DEMCGND();
        model = new EarthPressureSimulation(library, {
            particleCount: params.particle_count,
            aspectRatio: params.aspect_ratio,
            radiusRatio: params.radius_ratio,
        });
        params.N = model.count;
        params.r_min = model.rMin;
        params.r_max = model.rMax;
        params.average_radius = (model.rMin + model.rMax) / 2;
        if (SPHERES.spheres) {
            const geometries = new Set();
            SPHERES.spheres.children.forEach(particle => {
                geometries.add(particle.geometry);
                particle.material.dispose();
            });
            geometries.forEach(geometry => geometry.dispose());
        }
        await SPHERES.add_spheres(model.S, params, scene);
        updateCGParameters();
        CGHANDLER.cg_mesh.scale.set(model.width + model.travel, model.wallHeight, 1);
        CGHANDLER.cg_mesh.position.set((model.width - model.travel) / 2, model.wallHeight / 2, -1);
        updateWalls();
        camera.position.set(model.width * 0.7, model.wallHeight * 0.47, 5);
        controls.target.set(model.width * 0.7, model.wallHeight * 0.47, 0);
        camera.zoom = 1;
        resize();
    } catch (error) {
        console.error(error);
        params.status = 'Physics failed to load';
        info.textContent = 'Could not load physics. Check deploy/DEMCGND.js.';
    }
}

function updateCGParameters() {
    if (!model) return;
    model.S.cg_param_from_json_string(JSON.stringify({
        file: [{ filename: 'none', content: 'particles', format: 'interactive', number: 1 }],
        boxes: [params.cg_width, params.cg_height],
        // Keep the grid fixed over the entire wall travel so the field stays aligned.
        boundaries: [[-model.travel, 0], [model.width, model.wallHeight]],
        'window size': params.cg_window_size * params.average_radius,
        skip: 0,
        'max time': 1,
        'time average': 'None',
        fields: ['RHO', 'VAVG', 'TC', 'Pressure', 'ShearStress'],
        periodicity: [false, false],
        window: 'LucyND',
        dimension: 2,
    }));
    model.S.cg_setup_CG();
}

function updateWalls() {
    WALLS.back.position.set(model.wall - params.thickness / 2, model.wallHeight / 2, 0);
    WALLS.front.position.set(model.width + params.thickness / 2, model.wallHeight / 2, 0);
    [WALLS.back, WALLS.front].forEach(wall => {
        wall.scale.set(model.wallHeight, params.thickness, params.thickness);
    });
    WALLS.left.position.set((model.width - model.travel) / 2, -params.thickness / 2, 0);
    WALLS.left.scale.set(model.width + model.travel + 2 * params.thickness,
        params.thickness, params.thickness);
}

function resize() {
    const aspect = window.innerWidth / window.innerHeight;
    const height = Math.max(1.35 * (model?.wallHeight ?? 0.43), 1.3 / aspect);
    camera.left = -height * aspect / 2;
    camera.right = height * aspect / 2;
    camera.top = height / 2;
    camera.bottom = -height / 2;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function worldPosition(event) {
    const bounds = renderer.domElement.getBoundingClientRect();
    pointer.set(2 * (event.clientX - bounds.left) / bounds.width - 1,
        1 - 2 * (event.clientY - bounds.top) / bounds.height, 0).unproject(camera);
    return pointer;
}

function addWallDragging() {
    const canvas = renderer.domElement;
    canvas.style.touchAction = 'none';
    canvas.addEventListener('pointerdown', event => {
        if (!model?.ready) return;
        const point = worldPosition(event);
        if (Math.abs(point.x - model.wall) < 0.025 && point.y >= 0 && point.y <= model.wallHeight) {
            dragging = true;
            controls.enabled = false;
            canvas.setPointerCapture(event.pointerId);
            event.preventDefault();
            event.stopImmediatePropagation();
        }
    }, true);
    canvas.addEventListener('pointermove', event => {
        if (!model?.ready) return;
        const point = worldPosition(event);
        if (dragging) setTarget(Math.round(point.x * 1000));
        canvas.style.cursor = dragging ? 'grabbing' :
            Math.abs(point.x - model.wall) < 0.025 && point.y >= 0 && point.y <= model.wallHeight ? 'ew-resize' : '';
    });
    const release = () => { dragging = false; controls.enabled = true; };
    canvas.addEventListener('pointerup', release);
    canvas.addEventListener('pointercancel', release);
}

function animate() {
    requestAnimationFrame(animate);
    if (model) {
        if (!params.paused && !document.hidden) model.advance();
        params.actual = model.wall * 1000;
        params.thrust = model.thrust;
        if (!model.ready) {
            params.status = `Settling ${Math.floor(100 * model.time / model.settlingTime)}%`;
        } else {
            actionControls.forEach(control => control.enable());
            params.status = params.paused ? 'Paused' :
                model.wall < -0.001 ? 'Active displacement' :
                model.wall > 0.001 ? 'Passive displacement' : 'Reference position';
        }
        SPHERES.move_spheres(model.S, params);
        SPHERES.draw_force_network(model.S, params, scene);
        if (model.ready) {
            CGHANDLER.cg_mesh.visible = params.cg_opacity > 0;
            // The shared handler replaces the field texture on each update.
            CGHANDLER.cg_mesh.material.map?.dispose();
            CGHANDLER.update_2d_cg_field(model.S, params);
        }
        updateWalls();
    }
    controls.update();
    renderer.render(scene, camera);
}

async function init() {
    await SPHERES.createNDParticleShader(params);
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x111);
    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add(hemiLight);
    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set(5, -5, 5);
    scene.add(dirLight);

    camera = new THREE.OrthographicCamera(-1, 1, 1, -1, 0.01, 100);
    camera.position.set(0.46, 0.2, 5);
    camera.lookAt(0.46, 0.2, 0);
    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    document.getElementById('canvas').appendChild(renderer.domElement);
    controls = new OrbitControls(camera, renderer.domElement);
    controls.enableRotate = false;
    controls.target.set(0.46, 0.2, 0);
    controls.update();

    WALLS.add_back(params, scene);
    WALLS.add_front(params, scene);
    WALLS.add_left(params, scene);
    WALLS.wall_material.wireframe = false;
    CGHANDLER.add_cg_mesh(1, 1, scene);
    CGHANDLER.cg_mesh.visible = false;

    gui = new GUI();
    gui.width = 320;
    const wallControl = gui.add(params, 'displacement', -65, 65, 1)
        .name('Wall displacement (mm)').listen().onChange(setTarget);
    const actions = {
        active: () => setTarget(-45),
        passive: () => setTarget(45),
        origin: () => setTarget(0),
        reset: resetBed,
    };
    actionControls = [wallControl,
        gui.add(params, 'particle_count', 100, 5000, 1).name('Number of particles').listen()
            .onFinishChange(resetBed),
        gui.add(params, 'aspect_ratio', 0.5, 4, 0.1).name('Initial bed width / height').listen()
            .onFinishChange(resetBed),
        gui.add(actions, 'active').name('Active: move left'),
        gui.add(actions, 'passive').name('Passive: move right'),
        gui.add(actions, 'origin').name('Return to zero'),
        gui.add(params, 'paused').name('Paused').listen(),
        gui.add(actions, 'reset').name('Reset bed')];
    gui.add(params, 'cg_opacity', 0, 1).name('Coarse grain opacity').listen();
    gui.add(params, 'cg_field', ['Density', 'Velocity', 'Pressure', 'Horizontal Stress', 'Vertical Stress', 'Shear stress'])
        .name('Field').listen();
    gui.add(params, 'cg_window_size', 0.5, 6).name('Window size (radii)').listen()
        .onChange(updateCGParameters);
    gui.add(params, 'particle_opacity', 0, 1).name('Particle opacity').listen()
        .onChange(() => { if (model) SPHERES.update_particle_material(params); });
    gui.add(params, 'actual').name('Actual position (mm)').decimals(1).listen().disable();
    gui.add(params, 'thrust').name('Wall thrust (N/m)').decimals(0).listen().disable();
    gui.add(params, 'status').name('State').listen().disable();

    addWallDragging();
    resize();
    window.addEventListener('resize', resize);
    await resetBed();
    animate();
}

init().catch(error => {
    console.error(error);
    info.textContent = 'Could not initialise the earth-pressure example.';
});
