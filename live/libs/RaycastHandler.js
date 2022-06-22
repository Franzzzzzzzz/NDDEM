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

window.addEventListener( 'mousemove', onMouseMove, false );
window.addEventListener( 'mousedown', (e) => { onSelectParticle(e,camera) }, false );
// window.addEventListener( 'keypress', (e) => { onSelectParticle(e,camera) }, false );

function onMouseMove( event ) {

    // calculate mouse position in normalized device coordinates
    // (-1 to +1) for both components

    mouse.x = ( event.clientX / window.innerWidth ) * 2 - 1;
    mouse.y = - ( event.clientY / window.innerHeight ) * 2 + 1;

    // console.log(x,y)
}

export function animate_locked_particle(S, c, spheres, params) {
    camera = c
    if ( locked_particle !== null ) {
        raycaster.ray.intersectPlane( intersection_plane, ref_location);
        if ( 'aspect_ratio' in params ) {
            ref_location.clamp( new Vector3( -params.L*params.aspect_ratio,-params.L, 0 ),
                                new Vector3(  params.L*params.aspect_ratio, params.L, 0 ) );

        } else {
            ref_location.clamp( new Vector3( -params.L, -params.L, 0),
                                new Vector3(  params.L,  params.L, 0) );
        }
        S.simu_fixParticle(locked_particle.NDDEM_ID,[ref_location.x, ref_location.y, ref_location.z]);
    }
    calculate_intersection(camera, spheres, params);
}

function onSelectParticle( event, camera ) {
    // console.log(camera.getWorldDirection() )
    // if ( event.code === 'Enter' ) {
        if ( locked_particle === null ) {
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

function calculate_intersection(camera, spheres, params) {
    // update the picking ray with the camera and mouse position
    raycaster.setFromCamera( mouse, camera );

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
}
