import {
    Vector2,
    Vector3,
    Raycaster,
    Plane,
    Color
} from "three";

import { spheres } from "./SphereHandler";

const raycaster = new Raycaster();
const mouse = new Vector2();
let intersection_plane = new Plane();
let camera_direction = new Vector3();

let INTERSECTED = null;
let last_intersection = null;
let locked_particle = null;
let ref_location, old_ref_location;
let S, camera, params;
let vel = [];
let last_time = Date.now();

let data_points

export function update_world(s, c, p) {
    S = s;
    camera = c;
    params = p;
}

// export function add_raycaster_listeners() {
    window.addEventListener( 'mousemove', onMouseMove, false );
    window.addEventListener( 'mousedown', (e) => { onSelectParticleMouse(e) }, false );
    window.addEventListener( 'mouseup', (e) => { onDeselectParticle() }, false );

    window.addEventListener( 'touchmove', onTouchMove, false );
    window.addEventListener( 'touchstart', (e) => { onSelectParticleTouch(e) }, false );
    window.addEventListener( 'touchend', (e) => { onDeselectParticle() }, false );
// }

// window.addEventListener( 'keypress', (e) => { onSelectParticle(e,camera) }, false );

function onMouseMove( event ) {
    // event.preventDefault();

    // calculate mouse position in normalized device coordinates
    // (-1 to +1) for both components
    let new_x = ( event.clientX / window.innerWidth ) * 2 - 1;
    let new_y = - ( event.clientY / window.innerHeight ) * 2 + 1;
    let dt = Date.now() - last_time;
    vel = [200*(mouse.x - new_x)/dt,200*(new_y - mouse.y)/dt]; // NEED TO SCALE FROM PIXELS TO METERS
    mouse.x = new_x;
    mouse.y = new_y;
    last_time = Date.now();

    if ( camera !== undefined ) {
        calculate_intersection(mouse);
        animate_locked_particle();
    }
}

function onTouchMove( event ) {
    // event.preventDefault();
    // console.debug('touch move')

    // calculate mouse position in normalized device coordinates
    // (-1 to +1) for both components
    
    let new_x = ( event.touches[0].clientX / window.innerWidth ) * 2 - 1;
    let new_y = - ( event.touches[0].clientY / window.innerHeight ) * 2 + 1;
    let dt = Date.now() - last_time;
    vel = [200*(mouse.x - new_x)/dt,200*(new_y - mouse.y)/dt]; // HACK: NEED TO SCALE FROM PIXELS TO METERS
    mouse.x = new_x;
    mouse.y = new_y;
    last_time = Date.now();

    calculate_intersection(mouse);

    animate_locked_particle();
}

export function animate_locked_particle() {

    if ( locked_particle !== null ) {
        raycaster.ray.intersectPlane( intersection_plane, ref_location);
        if ( params.boundary === undefined ) { // lazy version
            if ( 'aspect_ratio' in params ) {
                ref_location.clamp( new Vector3( -params.L*params.aspect_ratio+params.r_max,-params.L+params.r_max, 0 ),
                                    new Vector3(  params.L*params.aspect_ratio-params.r_max, params.L-params.r_max, 0 ) );

            } else {
                ref_location.clamp( new Vector3( -params.L+params.r_max, -params.L+params.r_max, 0),
                                    new Vector3(  params.L-params.r_max,  params.L-params.r_max, 0) );
            }
        } else { // if explicit boundary is defined
            if ( params.boundary === 'Square' ) {
                ref_location.clamp( new Vector3( -params.L+params.r_max, -params.L+params.r_max, 0),
                                    new Vector3(  params.L-params.r_max,  params.L-params.r_max, 0) );
            } else if ( params.boundary === 'Triangle' ) {
                ref_location.y = THREE.MathUtils.clamp(ref_location.y, -params.H/2.+params.r_max, params.H/2.-params.r_max );
                // let x_max = (params.L - ref_location.y)/2.0;
                let x_max = -params.L*ref_location.y/params.H + params.L/2.;
                ref_location.x = THREE.MathUtils.clamp(ref_location.x, -x_max+params.r_max, x_max-params.r_max );
                // console.log(ref_location);
            } else if ( params.boundary === 'Circle' ) {
                let r_cur = Math.sqrt(ref_location.x*ref_location.x + ref_location.y*ref_location.y);
                if ( r_cur > params.R - params.r_max ) {
                    ref_location.x *= (params.R - params.r_max)/r_cur;
                    ref_location.y *= (params.R - params.r_max)/r_cur;
                }
            } else if ( params.boundary === 'Ellipse' ) {
                let l1 = params.R - params.r_max;
                let l2 = params.R*params.ellipse_ratio - params.r_max;
                let l_cur = Math.sqrt(ref_location.x*ref_location.x/l1/l1 + ref_location.y*ref_location.y/l2/l2);
                if ( l_cur > 1 ) {
                    ref_location.x /= l_cur;///l1;
                    ref_location.y /= l_cur;//l2;
                }
            }
        }
        // vel = [ref_location.x - locked_particle.position.x,ref_location.x - locked_particle.position.x];
        // console.log(vel);
        S.simu_fixParticle(locked_particle.NDDEM_ID,[ref_location.x, ref_location.y, ref_location.z]);
        if ( vel.length > 0 ) {
            let max_mag = 5; // HACK: TOTALLY ARBITRARY
            let vel_mag = Math.sqrt(vel[0]*vel[0] + vel[1]*vel[1]);
            if ( vel_mag > 0.0 ) {
                let limited_vel_mag = Math.min(vel_mag, max_mag);
                let limited_vel = [limited_vel_mag*vel[0]/vel_mag, limited_vel_mag*vel[1]/vel_mag];
                S.simu_setVelocity(locked_particle.NDDEM_ID,limited_vel);
                // console.log(limited_vel)
            }
        }

    }
    
}

function onSelectParticleMouse( event ) {
    // console.debug('select particle');
    // console.debug(locked_particle, INTERSECTED)


    // if no particle is currently caught but I AM intersecting with something
    if ( locked_particle === null  && INTERSECTED !== null ) {
        // console.log('CAUGHT')
        reset_ghosts();
        locked_particle = INTERSECTED;
        // console.log(locked_particle);
        ref_location = locked_particle.position;

        camera.getWorldDirection( camera_direction ); // update camera direction
        // set the plane for the particle to move along to be orthogonal to the camera
        intersection_plane.setFromNormalAndCoplanarPoint( camera_direction,
                                                            locked_particle.position );
    }
    else {
        locked_particle = null;
    }
        
    // }
}

function onSelectParticleTouch( event ) {
    // console.debug('select particle by touch');
    // console.debug(locked_particle, INTERSECTED)
    mouse.x = ( event.touches[0].clientX / window.innerWidth ) * 2 - 1;
    mouse.y = - ( event.touches[0].clientY / window.innerHeight ) * 2 + 1;
    calculate_intersection(mouse)
    // if no particle is currently caught but I AM intersecting with something
    if ( locked_particle === null  && INTERSECTED !== null ) {
        // console.log('CAUGHT')
        reset_ghosts();
        locked_particle = INTERSECTED;
        // console.log(locked_particle);
        ref_location = locked_particle.position;

        camera.getWorldDirection( camera_direction ); // update camera direction
        // set the plane for the particle to move along to be orthogonal to the camera
        intersection_plane.setFromNormalAndCoplanarPoint( camera_direction,
                                                            locked_particle.position );
    }
    else {
        locked_particle = null;
    }
        
    // }
}

function onDeselectParticle( ) {
    // console.debug('deselect particle');
    locked_particle = null;
}

function calculate_intersection(coords) {
    // update the picking ray with the camera and mouse position
    raycaster.setFromCamera( coords, camera );

    const intersects = raycaster.intersectObjects( spheres.children );
    if ( intersects.length > 0 ) { // if found something
        
        if ( INTERSECTED != intersects[ 0 ].object ) { // if not the same as last time
                if ( INTERSECTED !== null ) {
                    // INTERSECTED.material.uniforms.ambient.value = 1.0;
                    if ( params.lut === 'None' ) {
                        INTERSECTED.material.uniforms.ambient.value = 1.0;
                    } else {
                        INTERSECTED.material.emissive = new Color( 0x000000 );
                        // console.log(INTERSECTED)
                    }
                }
                INTERSECTED = intersects[ 0 ].object;
                if ( params.lut === 'None' ) {
                    INTERSECTED.material.uniforms.ambient.value = 5.0;
                } else {
                    INTERSECTED.material.emissive = new Color( 0xffffff );
                    // console.log(INTERSECTED)
                }

        }
    }
    else { // didn't find anything
        if ( INTERSECTED !== null ) { // there was something before
            if ( params.lut === 'None' ) {
                INTERSECTED.material.uniforms.ambient.value = 1.0;
            } else {
                INTERSECTED.material.emissive = new Color( 0x000000 );
            }
            INTERSECTED = null;
        }
    }
    // console.debug(INTERSECTED);
}

export function add_ghosts(scene, N=1000, radius=0.005, color=0xeeeeee) {
    data_points = new THREE.Group();
    data_points.nchildren = N;
    data_points.last_updated = 0;
    data_points.prev_updated = 0;
    
    let fg_mat = new THREE.PointsMaterial({ color: color, side: THREE.DoubleSide });
    // let fg_mat = new THREE.MeshStandardMaterial({ color: color, side: THREE.DoubleSide });
    // let fg_geom = new THREE.CircleGeometry(radius, 8);
    let fg_geom = new THREE.CylinderGeometry(radius/5., radius/5., 1, 4);
    fg_geom.applyMatrix4( new THREE.Matrix4().makeRotationX( Math.PI / 2 ) ); // rotate the geometry to make the forces point in the right direction

    let data_point = new THREE.Mesh(fg_geom, fg_mat);
    data_point.position.set(1e10, 1e10, 0); // don't show to begin with

    for (let i = 0; i < data_points.nchildren; i++) {
        data_points.add(data_point.clone());
    }
    scene.add(data_points);
}

export function reset_ghosts(){
    if ( data_points !== undefined ) {
        for (let i = 0; i < data_points.nchildren; i++) {
            data_points.children[i].position.set(1e10,1e10,0);
            data_points.children[i].scale.z = 0;
        }
        // onDeselectParticle();
    }
    ref_location = undefined;
    old_ref_location = undefined;
    // INTERSECTED = 0;
}


export function update_ghosts() {
    if ( ref_location !== undefined && locked_particle === null) {
        if ( old_ref_location === undefined) { old_ref_location = ref_location.clone(); }
        else { 
            data_points.children[data_points.last_updated].position.x = (ref_location.x + old_ref_location.x)/2.;
            data_points.children[data_points.last_updated].position.y = (ref_location.y + old_ref_location.y)/2.;

            let l = ref_location.distanceTo(old_ref_location);
            // console.log(l);
            data_points.children[data_points.last_updated].scale.z = l;
            data_points.children[data_points.last_updated].lookAt(ref_location);

            // console.log(data_points.children[data_points.last_updated]);

            data_points.prev_updated = data_points.last_updated;
            data_points.last_updated += 1;

            old_ref_location = ref_location.clone();
            if (data_points.last_updated == data_points.nchildren - 1) { data_points.last_updated = 0; }
        }
    }
}