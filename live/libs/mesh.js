import { ADDITION, SUBTRACTION, Brush, Evaluator } from 'three-bvh-csg';
import { STLExporter } from 'three/addons/exporters/STLExporter.js';
import * as WALLS from './WallHandler.js';

const link = document.createElement('a');
link.style.display = 'none';
document.body.appendChild(link);

export async function addition(a, b, mat) {
    const evaluator = new Evaluator();
    // evaluator.attributes = ['position', 'normal'];

    let brush1 = new Brush(a, mat);
    brush1.updateMatrixWorld();

    let brush2 = new Brush(b, mat);
    brush2.updateMatrixWorld();

    let result = await evaluator.evaluate(brush1, brush2, ADDITION);

    return result;
}

export async function subtraction(a, b, mat) {
    const evaluator = new Evaluator();
    // evaluator.attributes = ['position', 'normal'];

    let brush1 = new Brush(a, mat);
    brush1.updateMatrixWorld();

    let brush2 = new Brush(b, mat);
    brush2.updateMatrixWorld();

    let result = await evaluator.evaluate(brush1, brush2, SUBTRACTION);

    return result;
}

export function map_to_plane(spheres, params) {
    let ring = new THREE.Group();
    const vertex = new THREE.Vector3();

    let scale = params.R_0 * 2 / params.boundary[0].range;

    for (let i = 0; i < spheres.children.length; i++) {
        let obj = spheres.children[i];
        let new_obj = obj.clone();

        obj.getWorldPosition(vertex);

        let x = (vertex.x - params.boundary[0].min) * scale;
        let y = (vertex.y - params.boundary[1].min) * scale;// + params.shank_height;
        let z = (vertex.z - params.boundary[2].min) * scale;
        new_obj.position.set(x, y, z);
        // let offset_scale = 2 * r / (params.R_0 + params.R_1);
        let s = obj.scale.x * params.dilate * scale;// * params.T / params.boundary[2].range;
        // let s = obj.scale.x * params.dilate;// * offset_scale;
        // console.log(s)
        // console.log(x, y, z)
        new_obj.scale.set(s, s, s);

        ring.add(new_obj);
    }
    ring.position.set(-params.R_0, 0, 0);


    return ring
}

export function map_to_torus(spheres, params) {
    let ring = new THREE.Group();
    const vertex = new THREE.Vector3();

    for (let i = 0; i < spheres.children.length; i++) {
        let obj = spheres.children[i];
        let new_obj = obj.clone();

        if (params.mesh_mode === 'vertices') {
            // new_obj.geometry = obj.geometry.clone();

            let positionAttribute = new_obj.geometry.getAttribute('position');

            for (let j = 0; j < positionAttribute.count; j++) {
                vertex.fromBufferAttribute(positionAttribute, j);
                vertex.applyMatrix4(new_obj.matrixWorld);
                // console.log(vertex);

                // convert to cylindrical coordinates of ring
                let theta = (vertex.x - params.boundary[0].min) / params.boundary[0].range * params.theta_rad - params.theta_rad / 2 - 3 * Math.PI / 2.;
                let r = params.R_0 + (vertex.y - params.boundary[1].min) / params.boundary[1].range * (params.R_1 - params.R_0);
                let z = (vertex.z - params.boundary[2].min) / params.boundary[2].range * params.T;
                // console.log(theta / 2 / Math.PI);
                // convert back to cartesian coordinates
                let x = r * Math.cos(theta)
                let y = r * Math.sin(theta)

                // console.log(r, params.R_0, R_1)

                let vec = obj.worldToLocal(new THREE.Vector3(x, y, z));

                positionAttribute.setXYZ(j, vec.x, vec.y, vec.z);
            }
            let s = obj.scale.x * params.dilate;
            new_obj.scale.set(s, s, s);
        } else {
            obj.getWorldPosition(vertex);
            let theta = (vertex.x - params.boundary[0].min) / params.boundary[0].range * params.theta_rad - params.theta_rad / 2 - 3 * Math.PI / 2.;
            let r = params.R_0 + (vertex.y - params.boundary[1].min) / params.boundary[1].range * (params.R_1 - params.R_0);
            let z = (vertex.z - params.boundary[2].min) / params.boundary[2].range * params.T;
            // console.log(theta / 2 / Math.PI);
            // convert back to cartesian coordinates
            let x = r * Math.cos(theta)
            let y = r * Math.sin(theta)

            new_obj.position.set(x, y, z);
            let offset_scale = 2 * r / (params.R_0 + params.R_1);
            let s = obj.scale.x * params.dilate * offset_scale * params.T / params.boundary[2].range;
            // let s = obj.scale.x * params.dilate;// * offset_scale;
            // console.log(s)
            new_obj.scale.set(s, s, s);
        }

        ring.add(new_obj);
    }

    return ring

}

export function make_stl(filename, object, params) {

    // let ring = map_to_torus(object, params);
    let copy = object.clone();
    if (params.wall) {
        copy.add(WALLS.ring.clone());
    }

    const exporter = new STLExporter();
    const options = { binary: true }

    // Parse the input and generate the STL encoded output
    const result = exporter.parse(copy, options);

    if (options.binary) {
        saveArrayBuffer(result, filename);
    }
    else {
        saveString(result, filename);
    }

}

function save(blob, filename) {

    link.href = URL.createObjectURL(blob);
    link.download = filename;
    link.click();

}

function saveString(text, filename) {

    save(new Blob([text], { type: 'text/plain' }), filename);

}

function saveArrayBuffer(buffer, filename) {

    save(new Blob([buffer], { type: 'application/octet-stream' }), filename);

}