let radii;
export let spheres;
export let NDParticleShader;
let v, omegaMag;
export let ray;
export let total_particle_volume;
export let x;
export let F;
let F_mag_max;

import { Lut } from "../libs/Lut.js";
import { r, R } from "./controllers.js"
import * as AUDIO from '../libs/audio.js';

// import { Lut } from './js/Lut.js'
var lut = new Lut("blackbody", 512); // options are rainbow, cooltowarm and blackbody

let contact_flags = 0x80 | 0x100; // IDs and normal forces by default

export function update_contact_flags(flags) {
    contact_flags = flags;
}

let forces = new THREE.Group();

let debug_sound = false;

const cylinder_geometry = new THREE.CylinderGeometry( 1, 1, 1, 16 );
cylinder_geometry.applyMatrix4( new THREE.Matrix4().makeRotationX( Math.PI / 2 ) ); // rotate the geometry to make the forces point in the right direction
const cylinder_material = new THREE.MeshStandardMaterial( {color: 0xffffff} );
// cylinder_material.emissive = new Color( 0x0000ff );
cylinder_material.transparent = false;
const cylinder = new THREE.Mesh( cylinder_geometry, cylinder_material );

ray = new THREE.Line(
    new THREE.BufferGeometry().setFromPoints([
        new THREE.Vector3(0,-3,0),
        new THREE.Vector3(0,0,0),
    ]),
    new THREE.LineBasicMaterial( { color: 0xffffff })
);

export function wipe() {
    radii = undefined;
    spheres = undefined;
    x = undefined;
}

export function update_radii(S) {
    radii = S.simu_getRadii();
}

// export async function createNDParticleShader(params) {
//     import("./shaders/" + params.dimension + "DShader.js").then((module) => {
//         NDParticleShader = module.NDDEMShader;
//     });
// }

export async function createNDParticleShader(params) {
    return new Promise((resolve) => {
      import("./shaders/" + params.dimension + "DShader.js").then((module) => {
        NDParticleShader = module.NDDEMShader;
        // console.log('SET ND PARTICLE SHADER');
        resolve();
      });
    });
  }


export function update_cylinder_colour( colour ) {
    cylinder.material.color = new THREE.Color( colour );
}

export async function add_spheres(S,params,scene) {
    // console.log('Adding spheres to scene');
    await add_actual_spheres(S,params,scene);
    update_particle_material(params);
}

function add_actual_spheres(S,params,scene) {
    return new Promise((resolve) => {
        radii = S.simu_getRadii();
        total_particle_volume = 0;
        for ( let i=0; i<radii.length; i++ ) {
            // total_particle_volume += 4./3.*Math.PI*Math.pow(radii[i],3);
            total_particle_volume += get_particle_volume(params.dimension, radii[i]);
        }
        // console.log('Actual particle volume (assuming 3D particles): ' + total_particle_volume);
        if ( spheres === undefined ) {
            spheres = new THREE.Group();
            scene.add(spheres);
        } else {
            spheres.clear();
        }
        // const material = new THREE.MeshStandardMaterial();


        // const matrix = new THREE.Matrix4();
        const color = new THREE.Color();
        let geometrySphere;
        if ( params.dimension < 3 ) {
            geometrySphere = new THREE.CircleGeometry( 0.5, Math.pow(2,params.quality) );
            geometrySphere.applyMatrix4( new THREE.Matrix4().makeRotationZ( Math.PI / 2 ) ); //
            geometrySphere.applyMatrix4( new THREE.Matrix4().makeRotationX( Math.PI ) ); // rotate the geometry to make the forces point in the right direction
            // geometrySphere = new CylinderGeometry( 0.5, Math.pow(2,params.quality) );
            // geometrySphere = new SphereGeometry( 0.5, Math.pow(2,params.quality), Math.pow(2,params.quality) );
        }
        else {
            geometrySphere = new THREE.SphereGeometry( 0.5, Math.pow(2,params.quality), Math.pow(2,params.quality) );
        }

        for ( let i = 0; i < params.N; i ++ ) {
            // const material = NDParticleShader.clone();
            const material = new THREE.MeshStandardMaterial();
            material.side = THREE.DoubleSide;
            var object = new THREE.Mesh(geometrySphere, material);
            object.position.set(0,0,0);
            object.rotation.z = Math.PI / 2;
            object.NDDEM_ID = i;

            spheres.add(object);
            // spheres.setMatrixAt( i, matrix );
            // spheres.setColorAt( i, color.setHex( 0xffffff * Math.random() ) );

        }
        resolve();
    })
}

export function add_normal_sound_to_all_spheres() {
    if ( spheres.children.length > 100 ) { 
        for ( let i = 0; i < spheres.children.length; i ++ ) {

            if ( i >= 100 && i < 164 ) { // :(
                AUDIO.add_normal_sound( spheres.children[i] );
            }
            if ( debug_sound ) {
                spheres.children[i].material.emissive = new THREE.Color( 0x0000FF );
                spheres.children[i].material.color = new THREE.Color( 0xFF0000 );
            }
        }
    } else {
        for ( let i = 0; i < spheres.children.length; i ++ ) {
            AUDIO.add_normal_sound( spheres.children[i] );
        }
    }
}

export function mute_sounds() {
    // wipe anything from previous timestep
    active_sound_particles.forEach((id, index, arr) => {
        let ob = spheres.children[id];
        // update the gain node
        for ( let j = 0; j<ob.children.length; j ++ ) {
            if ( debug_sound) { ob.material.emissiveIntensity = 0; }

            if ( ob.children[j].type === 'Audio' ) {
                ob.children[j].gain.gain.value = 0.0;
            }
        }
    });
}

let active_sound_particles = [];

export function update_sounds(S, params) {
    // wipe anything from previous timestep
    active_sound_particles.forEach((id, index, arr) => {
        let ob = spheres.children[id];
        // update the gain node
        for ( let j = 0; j<ob.children.length; j ++ ) {
            if ( debug_sound) { ob.material.emissiveIntensity = 0; }

            if ( ob.children[j].type === 'Audio' ) {
                ob.children[j].gain.gain.value = 0.0;
            }
        }
    });

    active_sound_particles = [];

    let contact_info = S.simu_getContactInfos(0x80 | 0x8000);
    // console.log(contact_info)

    for ( let i = 0; i < contact_info.length; i ++ ) {
        let row = contact_info[i];
        let object_ids = [row[0], row[1]];

        let dissipation;
        if ( params.dimension === 2 ) {
            dissipation = Math.sqrt( row[2]*row[2] + row[3]*row[3] );
        } else if ( params.dimension === 3 ) {
            dissipation = Math.sqrt( row[2]*row[2] + row[3]*row[3] + row[4]*row[4] );
        } else if ( params.dimension === 4 ) {
            dissipation = Math.sqrt( row[2]*row[2] + row[3]*row[3] + row[4]*row[4] + row[5]*row[5] );
        }

        dissipation = Math.log10(dissipation)/1e3;
        dissipation = isFinite(dissipation) ? dissipation : 0.0; // remove non-finite values
        // console.log(dissipation)

        object_ids.forEach((id, index, arr) => {
            let ob = spheres.children[id];
            if ( ob.visible ) {
                // store a list of all particles that are being excited
                if (active_sound_particles.indexOf(id) === -1) {
                    active_sound_particles.push(id);
                }
                // update the gain node
                for ( let j = 0; j<ob.children.length; j ++ ) {
                    if ( ob.children[j].type === 'Audio' ) {
                        if ( debug_sound ) {
                            if ( ob.material.type === 'ShaderMaterial' ) {
                                ob.material.uniforms.ambient.value = dissipation;
                            }
                        }

                        ob.children[j].gain.gain.value = dissipation;
                        // console.log(ob.children[j].gain.gain.value)
                    }
                }
            }    
        });
    }
    
    // for ( let i = 0; i < spheres.children.length; i ++ ) {
    //     let ob = spheres.children[i];
    //     for ( let j = 0; j<ob.children.length; j ++ ) {
    //         if ( ob.children[j].type === 'Audio' ) {
    //             if ( ob.visible ) {
    //                 ob.children[j].gain.gain.value = v[i][0]; 
    //                 // ob.children[j].gain.gain.value = i/spheres.children.length;
    //                 // ob.children[j].gain.gain.value = dissipation[i];
    //             }
    //             else {
    //                 ob.children[j].gain.gain.value = 0;
    //             }
    //         }
    //     }
    // }
}

export function update_fixed_sounds(S, params) {
    if ( params.audio ) {
        let contact_info = S.simu_getContactInfos(0x80 | 0x20000);
        let total_dissipation = 0;
        for ( let i = 0; i< params.N; i++ ) {
            if ( spheres.children[i].material.emissiveIntensity !== 0 ) {
                spheres.children[i].material.emissiveIntensity = 0;
                spheres.children[i].material.needsUpdate = true;
            }
        }
        for ( let i = 0; i < contact_info.length; i ++ ) {
            let row = contact_info[i];
            let object_ids = [row[0], row[1]];

            let dissipation;
            if ( params.dimension === 2 ) {
                dissipation = Math.sqrt( row[2]*row[2] + row[3]*row[3] );
            } else if ( params.dimension === 3 ) {
                dissipation = Math.sqrt( row[2]*row[2] + row[3]*row[3] + row[4]*row[4] );
            } else if ( params.dimension === 4 ) {
                dissipation = Math.sqrt( row[2]*row[2] + row[3]*row[3] + row[4]*row[4] + row[5]*row[5] );
            }
            // dissipation = Math.log10(dissipation)/5e3;
            // dissipation = isFinite(dissipation) ? dissipation : 0.0; // remove non-finite values
            // let cutoff = 2e-2;
            if ( dissipation > 1./params.audio_sensitivity ) {
                spheres.children[row[0]].material.emissiveIntensity += dissipation*params.audio_sensitivity; // make them glow
                total_dissipation += dissipation;
            }
            
        }
        // console.log(total_dissipation/params.N/1e5);
        
        AUDIO.fixed_sound_source.children[0].gain.gain.value = total_dissipation/params.N;
    }
    else { 
        if ( AUDIO.fixed_sound_source.children[0].gain.gain.value !== 0 ) {
            AUDIO.fixed_sound_source.children[0].gain.gain.value = 0;
            for ( let i = 0; i< params.N; i++ ) {
                spheres.children[i].material.emissiveIntensity = 0;
                spheres.children[i].material.needsUpdate = true;
            }
        }
    }
}

export function add_pool_spheres(S,params,scene) {
    radii = S.simu_getRadii();

    spheres = new THREE.Group();
    scene.add(spheres);

    const geometrySphere = new THREE.SphereGeometry( 0.5, Math.pow(2,params.quality), Math.pow(2,params.quality) );

    for ( let i = 0; i < params.N; i ++ ) {
        if ( i == 0 ) {
            var material = new THREE.MeshStandardMaterial( {
                color: 0xaaaaaa });
        }
        else if ( i === 11 ) {
            var material = new THREE.MeshStandardMaterial( {
                color: 0x060606 });
        }
        else {
            var material = NDParticleShader.clone();
            material.uniforms.R.value = params.radius;
            material.uniforms.banding.value = 1 + 2*(i%3);
            if ( params.dimension === 2) {
                material.side = THREE.DoubleSide;
            }
            // material.uniforms.opacity.value = 1;
        }
        var object = new THREE.Mesh(geometrySphere, material);
        object.position.set(0,0,0);
        object.rotation.z = Math.PI / 2;
        object.NDDEM_ID = i;
        object.castShadow = true;
        object.receiveShadow = true;
        // AUDIO.add_normal_sound( object );
        spheres.add(object);
    }

    // display white ball
    // spheres.children[0].material.uniforms.banding.value = 1.;
    // spheres.children[0].material.uniforms.ambient.value = 5.;

    // display black ball
    // spheres.children[11].material.uniforms.banding.value = 0.;
    // spheres.children[11].material.uniforms.ambient.value = 1.;

    if ( !params.vr ) {
        spheres.children[0].add(ray); // add line
    }

    // add_spheres_to_torus(params,controller1,controller2);
}

export function add_spheres_to_torus(params,target) {
    const pointsGeometry = new THREE.SphereGeometry(
        1,
        Math.max(Math.pow(2, params.quality - 2), 4),
        Math.max(Math.pow(2, params.quality - 2), 4)
    );

    var scale = 20; // size of particles on tori
    let group = new THREE.Group();

    for ( let i = 0; i < params.N; i ++ ) {
        let color;
        if ( i == 0 ) { color = 0xaaaaaa; }
        else if ( i === 11 ) { color = 0x060606 }
        else if ( i%3 ) { color = 0x00ff00 }
        else { color = 0xff0000 }
        var pointsMaterial = new THREE.PointsMaterial({ color: color });
        var object = new THREE.Mesh(pointsGeometry, pointsMaterial);

        object.scale.set(R / scale, R / scale, R / scale);

        group.add(object);
    }
    target.add(group);
}

export function move_spheres_on_torus(params,target) {
    // console.log(target.children[0]);
    let real_target = target.children[0];
    if ( params.dimension === 4 ) {
        for ( let i = 0; i < params.N; i ++ ) {
            var object = real_target.children[i];
            var phi = (2 * Math.PI * (params.d4.cur - x[i][3])) / (params.d4.max - params.d4.min) - Math.PI / 2;
            var x_obj = (R + r) * Math.cos(phi);
            var y_obj = (R + r) * Math.sin(phi);
            var z_obj = 0;
            object.position.set(x_obj, y_obj, z_obj);
        }
    }
    else if ( params.dimension > 4 ) {
        console.log('trying both torus axes')
        for ( let i = 0; i < params.N; i ++ ) {
            var object = real_target.children[i];
            var phi =   (2 * Math.PI * (params.d4.cur - x[i][3])) / (params.d4.max - params.d4.min) - Math.PI / 2;
            var theta = (2 * Math.PI * (params.d5.cur - x[i][4])) / (params.d5.max - params.d5.min);
            var x_obj = (R + r * Math.cos(theta)) * Math.cos(phi);
            var y_obj = (R + r * Math.cos(theta)) * Math.sin(phi);
            var z_obj = r * Math.sin(theta);
            object.position.set(x_obj, y_obj, z_obj);
        }
    }
}

export function update_particle_material(params) {
    if ( params.particle_opacity === undefined ) { params.particle_opacity = 1; }
    console.log('Lut is ' + params.lut);
    if ( params.lut === 'None' ) {
        for ( let i = 0; i < params.N; i ++ ) {
            var object = spheres.children[i];
            // console.log(NDParticleShader)
            object.material = NDParticleShader.clone();
            if ( params.dimension === 2 ) { object.material.side = THREE.DoubleSide; }
            if ( params.particle_opacity < 1 ) { object.material.transparent = true; }
            // object.material.opacity = params.particle_opacity;
            object.material.uniforms.opacity.value = params.particle_opacity;
            object.material.uniforms.R.value = radii[i];
            // console.log(radii[i])
        }
    }
    else {
        for ( let i = 0; i < params.N; i ++ ) {
            var object = spheres.children[i];
            object.material = new THREE.MeshStandardMaterial({ color : 0xaaaaaa });
            object.material.emissiveIntensity = 0;
            object.material.transparent = true;
            object.material.opacity = params.particle_opacity;
            object.material.emissive = new THREE.Color( 0xFF0000 );
            if ( params.dimension === 2 ) { object.material.side = THREE.DoubleSide; }
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
        lut = new Lut("grainsize", 512);
        lut.setMin(params.r_min);
        lut.setMax(params.r_max);
        for ( let i = 0; i < params.N; i ++ ) {
            var object = spheres.children[i];
            object.material.color = lut.getColor(radii[i]);
            // object.material.color = lut.getColor( 1 - (radii[i] - params.r_min)/(params.r_max - params.r_min) )
        }
    } else if ( params.lut === 'White' ) {
        // do nothing, they're already white
    } else if ( params.lut === 'Black' ) {
        let black = new THREE.Color( 0x000000 );
        for ( let i = 0; i < params.N; i ++ ) {
            var object = spheres.children[i];
            object.material.color = black;
        }
    } else if (params.lut === "Rotation Rate") {
      lut.setMin(0);
      lut.setMax(params.omegamax);
    } else if (params.view_mode === "D4") {
      lut.setMin(params.d4.cur - 2 * r);
      lut.setMax(params.d4.cur + 2 * r);
      // object.material.color = lut.getColor(x3_unrotated);
      // TORUS.wristband1.children[i].material.color = lut.getColor(
        // x3_unrotated
      // );
    } else if (params.view_mode === "D5") {
      lut.setMin(params.d5.cur - 2 * r);
      lut.setMax(params.d5.cur + 2 * r);
      // object.material.color = lut.getColor(x[i][4]);
      // TORUS.wristband1.children[i].material.color = lut.getColor(
        // x[i][4]
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

export function move_spheres(S,params,controller1,controller2) {
    x = S.simu_getX();

    let orientation = S.simu_getOrientation();
    if ( params.lut === 'Velocity' || params.lut === 'Fluct Velocity' || params.audio ) {
        v = S.simu_getVelocity();
    }
    else if ( params.lut === 'Rotation Rate' ) {
        omegaMag = S.simu_getRotationRate();
    }
    else if ( params.lut === 'Particle Stress' ) {
        // forceMag = S.simu_getParticleStress(); // NOTE: NOT IMPLEMENTED YET
        console.warn('PARTICLE STRESSES NOT IMPLEMENTED YET')
    }
    let R_draw;
    for ( let i = 0; i < params.N; i ++ ) {
        let object = spheres.children[i];
        if ( params.dimension <= 3 ) {
            R_draw = radii[i];
        }
        else if ( params.dimension == 4 ) {
            R_draw = Math.sqrt(
              Math.pow(radii[i], 2) - Math.pow(params.d4.cur - x[i][3], 2)
            );
        } else if (params.dimension == 5) {
          R_draw = Math.sqrt(
            Math.pow(radii[i], 2) -
              Math.pow(params.d4.cur - x[i][3], 2) -
              Math.pow(params.d5.cur - x[i][4], 2)
          );
        } else if (params.dimension == 6) {
          R_draw = Math.sqrt(
            Math.pow(radii[i], 2) -
              Math.pow(params.d4.cur - x[i][3], 2) -
              Math.pow(params.d5.cur - x[i][4], 2) -
              Math.pow(params.d6.cur - x[i][5], 2)
          );
        } else if (params.dimension == 7) {
          R_draw = Math.sqrt(
            Math.pow(radii[i], 2) -
              Math.pow(params.d4.cur - x[i][3], 2) -
              Math.pow(params.d5.cur - x[i][4], 2) -
              Math.pow(params.d6.cur - x[i][5], 2) -
              Math.pow(params.d7.cur - x[i][6], 2)
          );
        } else if (params.dimension == 8) {
          R_draw = Math.sqrt(
            Math.pow(radii[i], 2) -
              Math.pow(params.d4.cur - x[i][3], 2) -
              Math.pow(params.d5.cur - x[i][4], 2) -
              Math.pow(params.d6.cur - x[i][5], 2) -
              Math.pow(params.d7.cur - x[i][6], 2) -
              Math.pow(params.d8.cur - x[i][7], 2)
          );
        } else if (params.dimension == 10) {
          R_draw = Math.sqrt(
            Math.pow(radii[i], 2) -
              Math.pow(params.d4.cur - x[i][3], 2) -
              Math.pow(params.d5.cur - x[i][4], 2) -
              Math.pow(params.d6.cur - x[i][5], 2) -
              Math.pow(params.d7.cur - x[i][6], 2) -
              Math.pow(params.d8.cur - x[i][7], 2) -
              Math.pow(params.d9.cur - x[i][8], 2) -
              Math.pow(params.d10.cur - x[i][9], 2)
          );
        } else if (params.dimension == 30) {
          R_draw = Math.sqrt(
            Math.pow(radii[i], 2) -
              Math.pow(params.d4.cur - x[i][3], 2) -
              Math.pow(params.d5.cur - x[i][4], 2) -
              Math.pow(params.d6.cur - x[i][5], 2) -
              Math.pow(params.d7.cur - x[i][6], 2) -
              Math.pow(params.d8.cur - x[i][7], 2) -
              Math.pow(params.d9.cur - x[i][8], 2) -
              Math.pow(params.d10.cur - x[i][9], 2) -
              Math.pow(params.d11.cur - x[i][10], 2) -
              Math.pow(params.d12.cur - x[i][11], 2) -
              Math.pow(params.d13.cur - x[i][12], 2) -
              Math.pow(params.d14.cur - x[i][13], 2) -
              Math.pow(params.d15.cur - x[i][14], 2) -
              Math.pow(params.d16.cur - x[i][15], 2) -
              Math.pow(params.d17.cur - x[i][16], 2) -
              Math.pow(params.d18.cur - x[i][17], 2) -
              Math.pow(params.d19.cur - x[i][18], 2) -
              Math.pow(params.d20.cur - x[i][19], 2) -
              Math.pow(params.d21.cur - x[i][20], 2) -
              Math.pow(params.d22.cur - x[i][21], 2) -
              Math.pow(params.d23.cur - x[i][22], 2) -
              Math.pow(params.d24.cur - x[i][23], 2) -
              Math.pow(params.d25.cur - x[i][24], 2) -
              Math.pow(params.d26.cur - x[i][25], 2) -
              Math.pow(params.d27.cur - x[i][26], 2) -
              Math.pow(params.d28.cur - x[i][27], 2) -
              Math.pow(params.d29.cur - x[i][28], 2) -
              Math.pow(params.d30.cur - x[i][29], 2)
          );
        }
        if (isNaN(R_draw)) {
            object.visible = false;
        } else {
            object.visible = true;
            object.scale.setScalar(2*R_draw);
            // spheres.setMatrixAt( i, matrix );
            object.position.set( x[i][0], x[i][1], x[i][2] );
        }
        if ( object.material.type === 'ShaderMaterial' ) { // found a custom shader material
            for (var j = 0; j < params.dimension - 3; j++) {
              object.material.uniforms.xview.value[j] =
                params.d4.cur;
              object.material.uniforms.xpart.value[j] =
                x[i][j + 3];
            }
            object.material.uniforms.A.value = orientation[i];
        } else if ( params.lut === 'Velocity' ) {
            // update brightness of textured particle
            // object.material.uniforms.ambient.value = 0.5 + 1e-3*( Math.pow(v[i][0],2) + Math.pow(v[i][1],2) + Math.pow(v[i][2],2) );
            // use LUT to set an actual colour
            // let vel_mag = Math.sqrt(Math.pow(v[i][0],2) + Math.pow(v[i][1],2) + Math.pow(v[i][2],2));
            let vel_mag = Math.sqrt(v[i].reduce((sum, element) => sum + (element * element), 0));
            // console.log(vel_mag)
            object.material.color = lut.getColor(vel_mag);
        } else if ( params.lut === 'Fluct Velocity') {
            let vel_mag;
            if ( params.dimension == 2) {
                vel_mag = Math.sqrt(Math.pow(v[i][0],2) + Math.pow(v[i][1]- params.shear_rate*x[i][0],2));
            } else if ( params.dimension == 3) {
                vel_mag = Math.sqrt(Math.pow(v[i][0],2) + Math.pow(v[i][1]- params.shear_rate*x[i][0],2) + Math.pow(v[i][2],2));
            }
            object.material.color = lut.getColor(vel_mag);
        } else if ( params.lut === 'Rotation Rate' ) {
            // console.log(omegaMag[i])
            // object.material.uniforms.ambient.value = 0.5 + 0.1*omegaMag[i];
            object.material.color = lut.getColor(omegaMag[i]);
        }

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
        stiffness = 0.5 * mass * ( Math.pow(Math.PI / tc, 2) + Math.pow(dissipation / mass, 2) );
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
            for (var i=0; i<forces.children.length; i++ ) {
                forces.children[i].geometry.dispose();
                forces.children[i].material.dispose();
            }
            scene.remove( forces );
            forces = new THREE.Group();

            //var F = S.simu_getContactForce(); // very poorly named
            F = S.simu_getContactInfos(contact_flags);
            
            let width = radii[0]/2.;
            if ( 'F_mag_max' in params ) {
                F_mag_max = params.F_mag_max;
            } else {
                F_mag_max = 1e0;
            }

            for ( let i = 0; i < F.length; i ++ ) {
            // for ( let i = 0; i < 100; i ++ ) {
                let F_mag;
                if ( params.dimension === 2) {
                    F_mag = Math.sqrt(
                        Math.pow(F[i][2],2) +
                        Math.pow(F[i][3],2)
                    )
                }
                else if ( params.dimension === 3) {
                    F_mag = Math.sqrt(
                        Math.pow(F[i][2],2) +
                        Math.pow(F[i][3],2) +
                        Math.pow(F[i][4],2)
                    )
                }
                if (F_mag > 0) {
                    let c = cylinder.clone();
                    let a = spheres.children[F[i][0]].position;
                    let b = spheres.children[F[i][1]].position;
                    let distance = a.distanceTo( b );
                    if (spheres.children[F[i][0]].visible && spheres.children[F[i][1]].visible) {
                        if ( distance < (radii[F[i][0]] + radii[F[i][1]]) ) { // ignore periodic boundaries
                            let mid_point = new THREE.Vector3();
                            mid_point.addVectors(a,b);
                            mid_point.divideScalar(2);
                            c.position.copy( mid_point );
                            c.scale.set(width*F_mag/F_mag_max,
                                        width*F_mag/F_mag_max,
                                        distance);
                            c.lookAt(a);
                            // AUDIO.add_normal_sound( c );

                            // c.material.emissiveIntensity = F_mag/F_mag_max;

                            forces.add( c );
                        }
                    }
                }
            }
            scene.add ( forces );
        }
        else {
            if ( forces.parent === scene ) {
                for (var i=0; i<forces.children.length; i++ ) {
                    forces.children[i].geometry.dispose();
                    forces.children[i].material.dispose();
                }
                scene.remove(forces);
            }
        }
    }

}

export function get_particle_volume(dimension, radius) {
    let volume;
    if ( dimension === 1 ) {
        volume = 2.*radius;
    } else if ( dimension === 2 ) {
        volume = Math.PI*Math.pow(radius,2);
    } else if ( dimension === 3 ) {
        volume = 4./3.*Math.PI*Math.pow(radius,3);
    } else if ( dimension === 4 ) {
        volume = Math.PI*Math.PI*Math.pow(radius,4)/2.;
    } else if ( dimension === 5) {
        volume = 8./15.*Math.PI*Math.PI*Math.pow(radius,5); // copilot suggestion, unvalidated
    }
    return volume;
}