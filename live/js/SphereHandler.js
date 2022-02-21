let radii;
let spheres;
let NDParticleShader;
let v, omegaMag;

import { Lut } from "../../visualise/node_modules/three/examples/jsm/math/Lut.js";
// import { Lut } from './js/Lut.js'
var lut = new Lut("blackbody", 512); // options are rainbow, cooltowarm and blackbody

import {
    Group,
    Color,
    Mesh,
    SphereGeometry,
    MeshPhongMaterial
} from "./three.module.js";

export async function createNDParticleShader(params) {
    import("../../visualise/js/shaders/" + params.dimension + "DShader.js").then((module) => {
        NDParticleShader = module.NDDEMShader;
    });
}

export function add_spheres(S,params,scene) {
    radii = S.simu_getRadii();
    spheres = new Group();
    scene.add(spheres);
    // const material = new THREE.MeshStandardMaterial();


    // const matrix = new THREE.Matrix4();
    const color = new Color();

    const geometrySphere = new SphereGeometry( 0.5, Math.pow(2,params.quality), Math.pow(2,params.quality) );
    // const geometrySphere = new THREE.BufferGeometry().fromGeometry(
    //   new THREE.SphereGeometry(
    //     1,
    //     Math.pow(2, params.quality),
    //     Math.pow(2, params.quality)
    //   )
    // );
    // spheres = new THREE.InstancedMesh( geometrySphere, material, params.N );
    // spheres.castShadow = true;
    // spheres.receiveShadow = true;
    // scene.add( spheres );

    for ( let i = 0; i < params.N; i ++ ) {
        const material = NDParticleShader.clone();
        var object = new Mesh(geometrySphere, material);
        object.position.set(0,0,0);
        object.rotation.z = Math.PI / 2;
        object.NDDEM_ID = i;
        spheres.add(object);
        // spheres.setMatrixAt( i, matrix );
        // spheres.setColorAt( i, color.setHex( 0xffffff * Math.random() ) );

    }
    var lut_folder;
    update_particle_material(params, lut_folder)
}

export function update_particle_material(params, lut_folder) {
    if ( params.lut === 'None' ) {
        for ( let i = 0; i < params.N; i ++ ) {
            var object = spheres.children[i];
            object.material = NDParticleShader.clone();
        }
    }
    else {
        for ( let i = 0; i < params.N; i ++ ) {
            var object = spheres.children[i];
            object.material = new MeshPhongMaterial();
        }
    }
    if ( params.lut === 'Velocity' ) {
        lut.setMin(0);
        lut.setMax(params.vmax);
        // var min_el = lut_folder.add()
    } else if ( params.lut === 'Fluct Velocity') {
        lut.setMin(-params.vmax);
        lut.setMax( params.vmax);
    } else if ( params.lut === 'Size' ) {
        lut = new Lut("cooltowarm", 512);
        // lut.setMin(params.r_min);
        // lut.setMax(params.r_max);
        for ( let i = 0; i < params.N; i ++ ) {
            var object = spheres.children[i];
            // object.material.color = lut.getColor(radii[i]);
            object.material.color = lut.getColor( 1 - (radii[i] - params.r_min)/(params.r_max - params.r_min) )
        }
    } else if (params.lut === "Rotation Rate") {
      lut.setMin(0);
      lut.setMax(params.omegamax);
    } else if (params.view_mode === "D4") {
      lut.setMin(world[3].cur - 2 * TORUS.r);
      lut.setMax(world[3].cur + 2 * TORUS.r);
      // object.material.color = lut.getColor(x3_unrotated);
      // TORUS.wristband1.children[i].material.color = lut.getColor(
        // x3_unrotated
      // );
    } else if (params.view_mode === "D5") {
      lut.setMin(world[4].cur - 2 * TORUS.r);
      lut.setMax(world[4].cur + 2 * TORUS.r);
      // object.material.color = lut.getColor(spheres[i][4]);
      // TORUS.wristband1.children[i].material.color = lut.getColor(
        // spheres[i][4]
      // );
    }
}

export function move_spheres(S,params) {
    var x = S.simu_getX();
    var orientation = S.simu_getOrientation();
    if ( params.lut === 'Velocity' || params.lut === 'Fluct Velocity' ) {
        v = S.simu_getVelocity();
    }
    else if ( params.lut === 'Rotation Rate' ) {
        omegaMag = S.simu_getRotationRate();
    }
    else if ( params.lut === 'Particle Stress' ) {
        // forceMag = S.simu_getParticleStress(); // NOTE: NOT IMPLEMENTED YET
        console.warn('PARTICLE STRESSES NOT IMPLEMENTED YET')
    }
    for ( let i = 0; i < params.N; i ++ ) {
        var object = spheres.children[i];

        // const matrix = new THREE.Matrix4();
        // matrix.setPosition( x[i][0], x[i][1], x[i][2] );
        if ( params.dimension == 3 ) {
            var D_draw = 2*radii[i];
            object.scale.set(D_draw, D_draw, D_draw);
        }
        else if ( params.dimension == 4 ) {
            var D_draw = 2*Math.sqrt(
              Math.pow(radii[i], 2) - Math.pow(params.d4_cur - x[i][3], 2)
            );
            object.scale.set(D_draw, D_draw, D_draw);
            // matrix.scale( new THREE.Vector3(D_draw,D_draw,D_draw) );
        }
        // spheres.setMatrixAt( i, matrix );
        object.position.set( x[i][0], x[i][1], x[i][2] );
        if ( params.lut === 'Velocity' ) {
            // update brightness of textured particle
            // object.material.uniforms.ambient.value = 0.5 + 1e-3*( Math.pow(v[i][0],2) + Math.pow(v[i][1],2) + Math.pow(v[i][2],2) );
            // use LUT to set an actual colour
            let vel_mag = Math.sqrt(Math.pow(v[i][0],2) + Math.pow(v[i][1],2) + Math.pow(v[i][2],2));
            object.material.color = lut.getColor(vel_mag);
        } else if ( params.lut === 'Fluct Velocity') {
            let vel_mag = Math.sqrt(Math.pow(v[i][0],2) + Math.pow(v[i][1]- params.shear_rate*x[i][0],2) + Math.pow(v[i][2],2));
            object.material.color = lut.getColor(vel_mag);
        }
        if ( params.lut === 'Rotation Rate' ) {
            // console.log(omegaMag[i])
            // object.material.uniforms.ambient.value = 0.5 + 0.1*omegaMag[i];
            object.material.color = lut.getColor(omegaMag[i]);
        } else if ( params.lut === 'None' ) {
            for (var j = 0; j < params.N - 3; j++) {
              object.material.uniforms.xview.value[j] =
                params.d4_cur;
              object.material.uniforms.xpart.value[j] =
                x[i][j + 3];
            }
            object.material.uniforms.A.value = orientation[i];
        }
        // if (params.dimension > 3) {
        //   object.material.uniforms.x4p.value = x[i][j + 3];
        //   object.material.uniforms.x4.value = params.d4_cur;
        // } else {
        //   object.material.uniforms.x4p.value = 0.0;
        // }
    }
    // spheres.instanceMatrix.needsUpdate = true;
    // console.log(orientation[0])
}
