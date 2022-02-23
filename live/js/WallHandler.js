export let left, right, floor, roof, front, back;

import {
    BoxGeometry,
    MeshLambertMaterial,
    Mesh,
} from "./three.module.js";

import { PIDcontroller } from './PIDcontroller.js'

let p_controller = new PIDcontroller(5e-4,1e-5,0);
let q_controller = new PIDcontroller(5e-4,1e-5,0);
// let radial_controller = new PIDcontroller(1e-3,0,0);
// let y_controller = new PIDcontroller(1e-3,0,0);
// let z_controller = new PIDcontroller(1e-3,0,0);

// if ( params.dimension == 3 ) {
// p_controller = new PIDcontroller(1e-3,1e-5,1e-5);
// q_controller = new PIDcontroller(1e-3,1e-5,1e-5);
// }
// else {
    // p_controller = new PIDcontroller(1e-7,1e-6,0);
    // q_controller = new PIDcontroller(1e-6,1e-5,0);
// }

const wall_geometry = new BoxGeometry( 1, 1, 1 );
const wall_material = new MeshLambertMaterial();
// wall_material.wireframe = true;


export function add_left(params, scene) {
    left = new Mesh( wall_geometry, wall_material );
    left.scale.y = params.thickness;
    left.position.y = - params.L - params.thickness/2.;
    // floor.receiveShadow = true;
    scene.add( left );
}

export function add_right(params, scene) {
    right = new Mesh( wall_geometry, wall_material );
    right.scale.y = params.thickness;
    right.position.y = params.L + params.thickness/2.;
    // top.receiveShadow = true;
    scene.add( right );
}

export function add_floor(params, scene) {
    floor = new Mesh( wall_geometry, wall_material );
    floor.scale.y = params.thickness;
    floor.rotation.x = Math.PI/2.;
    floor.position.z = - params.L*params.aspect_ratio - params.thickness/2.;
    // left.receiveShadow = true;
    scene.add( floor );
}

export function add_roof(params, scene) {
    roof = new Mesh( wall_geometry, wall_material );
    roof.rotation.x = Math.PI/2.;
    roof.position.z = params.L*params.aspect_ratio + params.thickness/2.;
    // right.receiveShadow = true;
    scene.add( roof );
}

export function add_front(params, scene) {
    front = new Mesh( wall_geometry, wall_material );
    front.rotation.z = Math.PI/2.;
    front.position.x = params.L + params.thickness/2.;
    // back.receiveShadow = true;
    scene.add( front );
}

export function add_back(params, scene) {
    back = new Mesh( wall_geometry, wall_material );
    back.rotation.z = Math.PI/2.;
    back.position.x = -params.L - params.thickness/2.;
    // front.receiveShadow = true;
    scene.add( back );
}

export function add_cuboid_walls(params, scene) {

    // const wall_geometry = new THREE.BoxGeometry( params.L*2 + params.thickness*2, params.thickness, params.L*2 + params.thickness*2 );
    // const wall_material = new THREE.ShadowMaterial( )

    add_left(params,scene);
    add_right(params,scene);
    add_floor(params, scene);
    add_roof(params, scene);
    add_front(params, scene);
    add_back(params, scene);

}

export function update_walls(params, S, dt=0.001) {
    params.packing_fraction = (params.N*params.particle_volume)/Math.pow(params.L_cur-params.W_cur,params.dimension-1)/(params.L_cur - params.H_cur)/Math.pow(2,params.dimension);
    // console.log(params.packing_fraction) // NOTE: STILL A BIT BUGGY!!!!

    if ( params.loading_method == 'strain_controlled') {
        if ( params.constant_volume ) {
            params.L_cur = params.L*(1-params.volumetric_strain);
            params.H_cur = params.L*params.axial_strain;
            params.W_cur = -( -Math.sqrt(params.L*params.L*params.L*(params.L-params.H_cur)) - params.H_cur*params.L + params.L*params.L ) / ( params.H_cur - params.L );
            // console.log(params.L_cur, params.H_cur, params.W_cur);

        }
        else {
            params.L_cur =  params.L*(1-params.volumetric_strain);
            params.H_cur =  params.L*params.axial_strain;
            params.W_cur = 0;
        }


    }
    else if ( params.loading_method == 'stress_controlled' ) {
        let delta_p = p_controller.update(params.pressure_set_pt,pressure,dt);
        let delta_q = q_controller.update(params.deviatoric_set_pt,shear,dt)
        // console.log(pressure)
        params.L_cur -= delta_p;
        params.H_cur += delta_q;

    }
    params.front =  params.L_cur - params.W_cur;
    params.back  = -params.L_cur + params.W_cur;
    params.left  = -params.L_cur + params.W_cur;
    params.right =  params.L_cur - params.W_cur;
    params.floor = -params.L_cur + params.H_cur;
    params.roof  =  params.L_cur - params.H_cur;

    S.simu_setBoundary(0, [params.back,params.front]) ; // Set location of the walls in x
    S.simu_setBoundary(1, [params.left,params.right]) ; // Set location of the walls in y
    S.simu_setBoundary(2, [params.floor,params.roof]) ; // Set location of the walls in z
    for (var j = 0; j < params.dimension - 3; j++) {
        S.simu_setBoundary(j + 3, [-params.L_cur,params.L_cur]) ; // Set location of the walls in z
    }
    back.position.x = params.back - params.thickness/2.;
    front.position.x = params.front + params.thickness/2.;
    left.position.y = params.left - params.thickness/2.;
    right.position.y = params.right + params.thickness/2.;
    floor.position.z = params.floor - params.thickness/2.;
    roof.position.z = params.roof + params.thickness/2.;

    var horiz_walls = [floor,roof];
    var vert_walls = [left,right,front,back];

    vert_walls.forEach( function(mesh) {
        mesh.scale.x = 2*params.L_cur + 2*params.thickness;
        mesh.scale.z = 2*(params.L_cur-params.H_cur) + 2*params.thickness;
    });

    horiz_walls.forEach( function(mesh) {
        mesh.scale.x = 2*params.L_cur + 2*params.thickness;
        mesh.scale.z = 2*params.L_cur + 2*params.thickness;
    });

}

export function update_triaxial_walls(params, S, dt=1) {
    params.packing_fraction = (params.N*params.particle_volume)/Math.pow(params.L_cur-params.W_cur,params.dimension-1)/(params.L_cur*params.aspect_ratio - params.H_cur)/Math.pow(2,params.dimension);
    // console.log(params.packing_fraction) // NOTE: STILL A BIT BUGGY!!!!

    if ( params.consolidate_active ) {
        let delta_p = p_controller.update(params.pressure_set_pt,params.current_pressure,dt);
        params.L_cur -= delta_p;

        // let delta_q = q_controller.update(0,params.current_shear,dt) // keep zero axial
        // params.H_cur += delta_q;
    }
    if ( params.shear_active ) {
        if ( params.constant_volume ) {
            params.H_cur += params.loading_rate*dt
            params.W_cur = params.L_cur - Math.sqrt(params.V_const/(params.L_cur - params.H_cur));
            // console.log(params.W_cur)
        } else {
            // constant pressure
            let delta_p = p_controller.update(params.pressure_set_pt,params.current_pressure,dt);
            params.W_cur -= delta_p;
            // strain controlled loading axially
            params.H_cur += params.loading_rate*dt
        }
    }

    // if ( params.consolidate_active || params.shear_active ) {
        params.front =  params.L_cur - params.W_cur;
        params.back  = -params.L_cur + params.W_cur;
        params.left  = -params.L_cur + params.W_cur;
        params.right =  params.L_cur - params.W_cur;
        params.floor = -params.L_cur*params.aspect_ratio + params.H_cur*params.aspect_ratio;
        params.roof  =  params.L_cur*params.aspect_ratio - params.H_cur*params.aspect_ratio;

        S.simu_setBoundary(0, [params.back,params.front]) ; // Set location of the walls in x
        S.simu_setBoundary(1, [params.left,params.right]) ; // Set location of the walls in y
        S.simu_setBoundary(2, [params.floor,params.roof]) ; // Set location of the walls in z
        for (var j = 0; j < params.dimension - 3; j++) {
            S.simu_setBoundary(j + 3, [-params.L_cur,params.L_cur]) ; // Set location of the walls in z
        }

        // and now tidy things up on the threejs side
        back.position.x = params.back - params.thickness/2.;
        front.position.x = params.front + params.thickness/2.;
        left.position.y = params.left - params.thickness/2.;
        right.position.y = params.right + params.thickness/2.;
        floor.position.z = params.floor - params.thickness/2.;
        roof.position.z = params.roof + params.thickness/2.;

        var horiz_walls = [floor,roof];
        var vert_walls = [left,right,front,back];

        vert_walls.forEach( function(mesh) {
            mesh.scale.x = 2*params.L_cur + 2*params.thickness;
            mesh.scale.z = 2*(params.L_cur*params.aspect_ratio-params.H_cur) + 2*params.thickness;
        });

        horiz_walls.forEach( function(mesh) {
            mesh.scale.x = 2*params.L_cur + 2*params.thickness;
            mesh.scale.z = 2*params.L_cur + 2*params.thickness;
        });
    // }

}

export function update_top_wall(params, S, dt=0.001) {
    params.packing_fraction = (params.N*params.particle_volume)/Math.pow(params.L,params.dimension-1)/(params.L_cur)/Math.pow(2,params.dimension)*2;
    // console.log(params.packing_fraction) // NOTE: STILL A BIT BUGGY!!!!

    params.L_cur =  params.L*(1-2*params.vertical_strain);
    params.roof  =  params.L_cur;// - params.H_cur;

    S.simu_setBoundary(2, [-params.L,params.roof]) ; // Set location of the walls in z
    roof.position.z = params.roof + params.thickness/2.;

    var horiz_walls = [floor,roof];
    var vert_walls = [left,right,front,back];

    vert_walls.forEach( function(mesh) {
        mesh.scale.x = 2*params.L + 2*params.thickness;
        mesh.scale.z = 2*(params.L) + 2*params.thickness;
    });

    horiz_walls.forEach( function(mesh) {
        mesh.scale.x = 2*params.L + 2*params.thickness;
        mesh.scale.z = 2*params.L + 2*params.thickness;
    });

}
