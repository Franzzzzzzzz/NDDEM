let radii;
export let spheres;
let NDParticleShader;
let v, omegaMag;

export let total_particle_volume;

import { Lut } from "three/examples/jsm/math/Lut.js";
// import { Lut } from './js/Lut.js'
var lut = new Lut("blackbody", 512); // options are rainbow, cooltowarm and blackbody

import {
    Vector3,
    Matrix4,
    Group,
    Color,
    Mesh,
    SphereGeometry,
    CylinderGeometry,
    MeshStandardMaterial
} from "three";

let forces = new Group();

const cylinder_geometry = new CylinderGeometry( 1, 1, 1, 16 );
cylinder_geometry.applyMatrix( new Matrix4().makeRotationX( Math.PI / 2 ) ); // rotate the geometry to make the forces point in the right direction
const cylinder_material = new MeshStandardMaterial( {color: 0xffffff} );
cylinder_material.emissive = new Color( 0x0000ff );
cylinder_material.transparent = false;
const cylinder = new Mesh( cylinder_geometry, cylinder_material );

export async function createNDParticleShader(params) {
    import("./shaders/" + params.dimension + "DShader.js").then((module) => {
        NDParticleShader = module.NDDEMShader;
    });
}

export function add_spheres(S,params,scene) {
    radii = S.simu_getRadii();
    total_particle_volume = 0;
    for ( let i=0; i<radii.length; i++ ) {
        total_particle_volume += 4./3.*Math.PI*Math.pow(radii[i],3);
    }
    console.log('Actual particle volume: ' + total_particle_volume);
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
            object.material.transparent = true;
            // object.material.opacity = params.particle_opacity;
            object.material.uniforms.opacity.value = params.particle_opacity;
        }
    }
    else {
        for ( let i = 0; i < params.N; i ++ ) {
            var object = spheres.children[i];
            object.material = new MeshStandardMaterial();
            object.material.transparent = true;
            object.material.opacity = params.particle_opacity;
        }
    }
    if ( params.lut === 'Velocity' ) {
        lut.setMin(0);
        lut.setMax( params.vmax );
        // var min_el = lut_folder.add()
    } else if ( params.lut === 'Fluct Velocity') {
        lut.setMin(0);
        lut.setMax( params.vmax/2. );
    } else if ( params.lut === 'Size' ) {
        lut = new Lut("cooltowarm", 512);
        // lut.setMin(params.r_min);
        // lut.setMax(params.r_max);
        for ( let i = 0; i < params.N; i ++ ) {
            var object = spheres.children[i];
            // object.material.color = lut.getColor(radii[i]);
            object.material.color = lut.getColor( 1 - (radii[i] - params.r_min)/(params.r_max - params.r_min) )
        }
    } else if ( params.lut === 'White' ) {
        // do nothing, they're already white
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

    // if ( params.show_colorbar ) {
    //     let canvas = document.getElementById("canvas");
    //     let colorbar = document.createElement("canvas");
    //     canvas.appendChild(colorbar);
    //     colorbar.setAttribute("id", "colorbar");
    //     let colorbarCanvas = lut.createCanvas();
    //
    //     console.log(canvas)
    //     colorbar.width = canvas.offsetWidth;
    //     colorbar.height = 50;
    //
    //     //grab the context from your destination canvas
    //     var ctx = colorbar.getContext('2d');
    //
    //
    //     ctx.translate(ctx.width/2., ctx.height/2.);
    //
    //     // rotate around that point, converting our
    //     // angle from degrees to radians
    //     ctx.rotate(Math.PI/2.);
    //     ctx.translate(-ctx.width/2.,-ctx.height/2.);
    //
    //     // draw it up and to the left by half the width
    //     // and height of the image
    //     // ctx.drawImage(colorbarCanvas, -(colorbarCanvas.width/2), -(colorbarCanvas.height/2));
    //     ctx.drawImage(colorbarCanvas, 0, 0, ctx.width, ctx.height);
    // }
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

export function setCollisionTimeAndRestitutionCoefficient (tc, eps, mass) {
    // stolen from MercuryDPM
    // ONLY USE THIS FOR LINEAR SPRINGS
    // Sets k, disp such that it matches a given tc and eps for a collision of two copies of equal mass m.
    //
    // Parameters
    // [in]	tc	collision time
    // [in]	eps	restitution coefficient
    // [in]	mass	harmonic average particle mass, \(\frac{2}{1/m1+1/m2}\)
    let stiffness, dissipation
    if ( eps === 0.0 ) {
        stiffness = 0.5 * mass * Math.pow(Math.PI / tc, 2);
        dissipation = Math.sqrt(2.0 * mass * stiffness);
    } else {
        dissipation = -mass / tc * Math.log(eps);
        stiffness = .5 * mass * ( Math.pow(Math.PI / tc, 2) + Math.pow(dissipation / mass, 2) );
    }
    return { 'dissipation': dissipation, 'stiffness': stiffness }
}

export function getHertzCriticalTimestep (bulk_modulus, poisson_coefficient, radius, density) {
    // stolen from Burns et at 2019
    let beta = 0.8766 + 0.163*poisson_coefficient;
    let critical_timestep = Math.PI * radius / beta * Math.sqrt(density / bulk_modulus);

    return critical_timestep
}

export function randomise_particles( params, S ) {
    if ( S !== undefined ) {
        for ( let i = 0; i < params.N; i ++ ) {
            S.simu_fixParticle(i,[
                -params.L + Math.random()*2*params.L,
                -params.L + Math.random()*2*params.L,
                -params.L + Math.random()*2*params.L]);
        }
    }
}

export function randomise_particles_isotropic( params, S ) {
    if ( S !== undefined ) {
        for ( let i = 0; i < params.N; i ++ ) {
            S.simu_fixParticle(i,[
                -params.L + params.r_max + Math.random()*2*(params.L-params.r_max),
                -params.L + params.r_max + Math.random()*2*(params.L-params.r_max),
                -params.H + params.r_max + Math.random()*2*(params.H-params.r_max)]);
        }
    }
}


export function draw_force_network(S,params,scene) {
    if ( S !== undefined ) {
        if (params.particle_opacity < 1) {
            scene.remove( forces );
            forces = new Group();

            var F = S.simu_getParticleForce(); // very poorly named

            let width = radii[0]/2.;
            let F_mag_max = 1e0;

            for ( let i = 0; i < F.length; i ++ ) {
            // for ( let i = 0; i < 100; i ++ ) {
                let F_mag = Math.sqrt(
                    Math.pow(F[i][2],2) +
                    Math.pow(F[i][3],2) +
                    Math.pow(F[i][4],2)
                )
                if (F_mag > 0) {
                    let c = cylinder.clone();
                    let a = spheres.children[F[i][0]].position;
                    let b = spheres.children[F[i][1]].position;
                    let distance = a.distanceTo( b );
                    if ( distance < (radii[F[i][0]] + radii[F[i][1]]) ) { // ignore periodic boundaries
                        let mid_point = new Vector3();
                        mid_point.addVectors(a,b);
                        mid_point.divideScalar(2);
                        c.position.copy( mid_point );
                        c.scale.set(width*F_mag/F_mag_max,
                                    width*F_mag/F_mag_max,
                                    distance);
                        c.lookAt(a);

                        // c.material.emissiveIntensity = F_mag/F_mag_max;

                        forces.add( c );
                    }
                }
            }
            scene.add ( forces );
        }
        else {
            if ( forces.parent === scene ) { scene.remove(forces) }
        }
    }

}
