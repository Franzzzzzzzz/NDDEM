export let pool_cue, small_end_radius, small_sphere;

import {
    Vector3,
    Matrix4,
    Group,
    Mesh,
    SphereGeometry,
    CylinderGeometry,
    MeshStandardMaterial
} from "three";

export function add_pool_cue( target ) {
    pool_cue = new Group();

    small_end_radius = 0.02; // any smaller and things go very poorly (at least using the webxr fake platform with apparently large step sizes)
    let large_end_radius = 0.06;
    let length = 1.4;

    const cylinder_geometry = new CylinderGeometry( small_end_radius, large_end_radius, length, 16 );
    cylinder_geometry.applyMatrix4( new Matrix4().makeRotationX( -Math.PI / 2 ) ); // rotate the geometry to make the forces point in the right direction
    const wood_material = new MeshStandardMaterial( {color: 0x5d2906} ); // wood colour
    const cylinder = new Mesh( cylinder_geometry, wood_material );
    pool_cue.add(cylinder);

    const small_sphere_geometry = new SphereGeometry( small_end_radius, 32, 16 );
    const chalk = new MeshStandardMaterial( {color: 0xffffff} ); // chalk colour
    small_sphere = new Mesh( small_sphere_geometry, chalk );

    pool_cue.add(small_sphere);
    small_sphere.position.z = -length/2.;

    const large_sphere_geometry = new SphereGeometry( large_end_radius, 32, 16 );
    const large_sphere = new Mesh( large_sphere_geometry, wood_material );

    pool_cue.add(large_sphere);
    large_sphere.position.z = length/2.;

    pool_cue.position.z = -length/4.

    target.add( pool_cue );
}
