import { XRControllerModelFactory } from 'three/examples/jsm/webxr/XRControllerModelFactory.js';

import {
    Mesh,
    Group,
    Quaternion,
    Euler,
    TorusGeometry,
    MeshStandardMaterial
} from "three";

export let controller1, controller2
let controllerGrip1, controllerGrip2;
export let torus1, torus2;
let torus_axis1_phi, torus_axis2_phi;
let torus_axis1_theta, torus_axis2_theta;

export let R = 0.1; // radius of torus
export let r = R / 2; // inner radius of torus
let torus_colour = 0xaaaaaa;
let torus_axis_colour = 0x000000; // the axis

let urlParams = new URLSearchParams(window.location.search);

function onSelectStart()  { this.userData.isSelecting = true; }
function onSelectEnd()    { this.userData.isSelecting = false; }

function onSqueezeStart() { this.userData.isSqueezing = true; }
function onSqueezeEnd()   { this.userData.isSqueezing = false; }

export function add_controllers(renderer, scene, params) {
    controller1 = renderer.xr.getController( 0 );
	controller1.addEventListener( 'selectstart',  onSelectStart );
	controller1.addEventListener( 'selectend',    onSelectEnd );
    controller1.addEventListener( 'squeezestart', onSqueezeStart );
    controller1.addEventListener( 'squeezeend',   onSqueezeEnd );
    controller1.userData.previous_torus_rotation_z = 0;
    controller1.userData.previous_torus_rotation_y = 0;
    controller1.userData.new_orientation = 0;
    controller1.userData.previous_direction = new Quaternion();
    controller1.userData.current_direction = new Quaternion();
    controller1.userData.diff = new Quaternion();
    controller1.userData.diff_angle = new Euler();
	scene.add( controller1 );

	controller2 = renderer.xr.getController( 1 );
	controller2.addEventListener( 'selectstart', onSelectStart );
	controller2.addEventListener( 'selectend', onSelectEnd );
    controller2.addEventListener( 'squeezestart', onSqueezeStart );
    controller2.addEventListener( 'squeezeend',   onSqueezeEnd );
    controller2.userData.previous_torus_rotation_z = 0;
    controller2.userData.previous_torus_rotation_y = 0;
    controller2.userData.new_orientation = 0;
    controller2.userData.previous_direction = new Quaternion();
    controller2.userData.current_direction = new Quaternion();
    controller2.userData.diff = new Quaternion();
    controller2.userData.diff_angle = new Euler();
	scene.add( controller2 );

	const controllerModelFactory = new XRControllerModelFactory();

	let controllerGrip1 = renderer.xr.getControllerGrip( 0 );
	controllerGrip1.add( controllerModelFactory.createControllerModel( controllerGrip1 ) );
	scene.add( controllerGrip1 );

	let controllerGrip2 = renderer.xr.getControllerGrip( 1 );
	controllerGrip2.add( controllerModelFactory.createControllerModel( controllerGrip2 ) );
	scene.add( controllerGrip2 );

    // let controllerHand0 = renderer.xr.getHand( 0 );
	// scene.add( controllerHand0 );
}

export async function add_torus( target, params ) {
    console.log('adding torus');

    var geometry = new TorusGeometry(
        R,
        r,
        Math.pow(2, params.quality + 1) * 2,
        Math.pow(2, params.quality + 1)
      );
    var material = new MeshStandardMaterial({
        color: torus_colour,
    });

    torus1 = new Mesh(geometry, material);

    var geometry = new TorusGeometry(
      r + R - r / 6,
      r / 5,
      Math.pow(2, params.quality + 1) * 2,
      Math.pow(2, params.quality + 1)
    );
    var material = new MeshStandardMaterial({
      color: torus_axis_colour,
      // roughness: 0.7,
    });
    torus_axis1_phi = new Mesh(geometry, material);

    var geometry = new TorusGeometry(
      r,
      r / 10,
      Math.pow(2, params.quality + 1) * 2,
      Math.pow(2, params.quality + 1)
    );
    torus_axis1_theta = new Mesh(geometry, material);
    torus_axis1_theta.rotation.y = Math.PI / 2;

    if ( params.vr ) {
      torus1.position.set(0, 0, 0.2);
      torus1.rotation.set(0, 0, Math.PI);
      torus_axis1_phi.position.set(0, 0, 0.2);
      torus_axis1_theta.position.set(0, R, 0.2);
      target.add(torus1);
      target.add(torus_axis1_phi);
      target.add(torus_axis1_theta);
    } else {
      controllerGrip1 = new Group();
      scene.add(controller1);

      torus1.rotation.set(0, 0, Math.PI);
      torus_axis1_theta.position.set(0, R, 0);

      target.add(torus1);
      target.add(torus_axis1_phi);
      target.add(torus_axis1_theta);
      target.position.set(2.5, -3 * R, 0.5);
    }
}

/**
 * Get the current orientation of the left hand controller and set world coordinates appropriately
 */
export function update_higher_dims(target, params) {
    let tt = target.userData;

    target.getWorldQuaternion(tt.current_direction);

    tt.diff = tt.current_direction.invert().multiply(tt.previous_direction);
    tt.diff_angle.setFromQuaternion(tt.diff); // + Math.PI;// between 0 and 2 Pi

    // move in D4 by rotations in z
    if (params.dimension >= 3) {
        let new_orientation = tt.previous_torus_rotation_z + tt.diff_angle.z;
        if (new_orientation < 0) { new_orientation += 2 * Math.PI; }
        else if (new_orientation > 2 * Math.PI) { new_orientation -= 2 * Math.PI; }
        params.d4.cur = (new_orientation * (params.d4.max - params.d4.min)) / Math.PI / 2;
        // console.log(params.d4.cur)
    }
    // move in D5 by rotations in y
    if (params.dimension > 4) {
        let new_orientation = tt.previous_torus_rotation_x + 2 * tt.diff_angle.x; // double rotation in reality
        if (new_orientation < 0) { new_orientation += 2 * Math.PI; }
        else if (new_orientation > 2 * Math.PI) { new_orientation -= 2 * Math.PI; }
        params.d5.cur = (new_orientation * (params.d5.max - params.d5.min)) / Math.PI / 2;
    }
    return params;
}
