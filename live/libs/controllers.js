import { XRControllerModelFactory } from 'three/examples/jsm/webxr/XRControllerModelFactory.js';

export let controller1, controller2, controllerGrip1, controllerGrip2;

function onSelectStart() {

}

function onSelectEnd() {

}

export function add_controllers(renderer, scene) {
    controller1 = renderer.xr.getController( 0 );
	controller1.addEventListener( 'selectstart', onSelectStart );
	controller1.addEventListener( 'selectend', onSelectEnd );
	scene.add( controller1 );

	controller2 = renderer.xr.getController( 1 );
	controller2.addEventListener( 'selectstart', onSelectStart );
	controller2.addEventListener( 'selectend', onSelectEnd );
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
