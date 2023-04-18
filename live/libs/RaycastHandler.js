import {
    Vector2,
    Vector3,
    Raycaster,
    Plane,
    Color
} from "three";

const raycaster = new Raycaster();
const mouse = new Vector2();
let intersection_plane = new Plane();
let camera_direction = new Vector3();

let INTERSECTED = null;
let last_intersection = null;
let locked_particle = null;
let ref_location;
let camera;
let vel = [];
let last_time = Date.now();

let data_points

window.addEventListener( 'mousemove', onMouseMove, false );
window.addEventListener( 'mousedown', (e) => { onSelectParticle(e,camera) }, false );
window.addEventListener( 'mouseup', (e) => { onDeselectParticle() }, false );

window.addEventListener( 'touchmove', onTouchMove, false );
window.addEventListener( 'touchstart', (e) => { onSelectParticle(e,camera) }, false );
window.addEventListener( 'touchend', (e) => { onDeselectParticle() }, false );

// window.addEventListener( 'keypress', (e) => { onSelectParticle(e,camera) }, false );

function onMouseMove( event ) {

    // calculate mouse position in normalized device coordinates
    // (-1 to +1) for both components
    let new_x = ( event.clientX / window.innerWidth ) * 2 - 1;
    let new_y = - ( event.clientY / window.innerHeight ) * 2 + 1;
    let dt = Date.now() - last_time;
    vel = [200*(new_y-mouse.y)/dt,200*(new_x - mouse.x)/dt]; // NEED TO SCALE FROM PIXELS TO METERS
    mouse.x = new_x;
    mouse.y = new_y;
    last_time = Date.now();
}

function onTouchMove( event ) {

    // calculate mouse position in normalized device coordinates
    // (-1 to +1) for both components
    let new_x = ( event.touches[0].clientX / window.innerWidth ) * 2 - 1;
    let new_y = - ( event.touches[0].clientY / window.innerHeight ) * 2 + 1;
    let dt = Date.now() - last_time;
    vel = [200*(new_y-mouse.y)/dt,200*(new_x - mouse.x)/dt]; // NEED TO SCALE FROM PIXELS TO METERS
    mouse.x = new_x;
    mouse.y = new_y;
    last_time = Date.now();
}

export function animate_locked_particle(S, c, spheres, params) {
    camera = c
    calculate_intersection(camera, spheres, params);
    if ( locked_particle !== null ) {
        raycaster.ray.intersectPlane( intersection_plane, ref_location);
        if ( 'aspect_ratio' in params ) {
            ref_location.clamp( new Vector3( -params.L*params.aspect_ratio,-params.L, 0 ),
                                new Vector3(  params.L*params.aspect_ratio, params.L, 0 ) );

        } else {
            ref_location.clamp( new Vector3( -params.L, -params.L, 0),
                                new Vector3(  params.L,  params.L, 0) );
        }
        // vel = [ref_location.x - locked_particle.position.x,ref_location.x - locked_particle.position.x];
        // console.log(vel);
        S.simu_fixParticle(locked_particle.NDDEM_ID,[ref_location.x, ref_location.y, ref_location.z]);
        if ( vel.length > 0 ) { S.simu_setVelocity(locked_particle.NDDEM_ID,vel); }

    }
    
}

function onSelectParticle( event, camera ) {
    
    // console.log(camera.getWorldDirection() )
    // if ( event.code === 'Enter' ) {
        if ( locked_particle === null  && INTERSECTED !== null ) {
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
    locked_particle = null;
}

function calculate_intersection(camera, spheres, params) {
    // update the picking ray with the camera and mouse position
    raycaster.setFromCamera( mouse, camera );

    const intersects = raycaster.intersectObjects( spheres.children );
    if ( intersects.length > 0 ) { // if found something
        // console.log(intersects);
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
}

export function add_ghosts(scene, N=1000, radius=0.005, color=0xeeeeee) {
    data_points = new THREE.Group();
    data_points.nchildren = N;
    data_points.last_updated = 0;
    
    // let fg_mat = new THREE.PointsMaterial({ color: 0xeeeeee });
    let fg_mat = new THREE.MeshStandardMaterial({ color: color, side: THREE.DoubleSide });
    let fg_geom = new THREE.CircleGeometry(radius, 8);
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
        }
        // onDeselectParticle();
    }
    // INTERSECTED = 0;
}


export function update_ghosts() {
    if ( ref_location !== undefined && locked_particle === null) {
        data_points.children[data_points.last_updated].position.x = ref_location.x;
        data_points.children[data_points.last_updated].position.y = ref_location.y;

        data_points.last_updated += 1;
        if (data_points.last_updated == data_points.nchildren - 1) { data_points.last_updated = 0; }
    }
}