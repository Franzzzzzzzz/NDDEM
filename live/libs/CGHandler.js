import { Lut } from './Lut.js';
import * as COLORBAR from './colorbar.js';

export let cg_mesh;

let sequential = new Lut('inferno', 512);
let divergent  = new Lut('bkr', 512);
let grainsize  = new Lut('grainsize', 512);
let angular  = new Lut('virino', 512);

export function add_cg_mesh(width, height, scene) {
    let geometry = new THREE.PlaneGeometry( width, height );
    let material = new THREE.MeshBasicMaterial( {color: 0xffffff, side: THREE.DoubleSide} );
    material.transparent = true;
    cg_mesh = new THREE.Mesh( geometry, material );
    cg_mesh.position.z = -1;
    scene.add( cg_mesh );
}

export function update_2d_cg_field(S, params) {
    if ( params.cg_opacity > 0 ) { 
        S.cg_param_read_timestep(0) ;
        S.cg_process_timestep(0,false) ;
        var grid = S.cg_get_gridinfo();
        const size = params.cg_width * params.cg_height;
        const data = new Uint8Array( 4 * size );
        const opacity = parseInt(255 * params.cg_opacity);
        let val;
        let lut;
        let alpha; // variable opacity level
        if ( params.cg_field === 'Density' ) {
            val = S.cg_get_result(0, "RHO", 0);
            lut = sequential;
            // val = val.map(x => x/60); // HACK!!!!
            // console.log(val)
            // let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(0);
            lut.setMax(params.particle_density);
            lut.units = 'Density (kg/m<sup>'+String(params.dimension)+'</sup>)';
        } else if ( params.cg_field === 'Size' ) {
            val = S.cg_get_result(0, "RADIUS", 0);
            lut = grainsize;
            // let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(params.r_min);
            lut.setMax(params.r_max);
            lut.units = 'Average particle size (m)';
        }
        else if ( params.cg_field === 'Velocity' ) {
            let vx = S.cg_get_result(0, "VAVG", 0);
            let vy = S.cg_get_result(0, "VAVG", 1);
            val = atan2vec(vx,vy);
            let U = norm(vx,vy);
            lut = angular;
            let maxU = U.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            alpha = divide_vec(U,maxU); // let it saturate a bit
            // lut.setMin(-0.9*maxVal);
            // lut.setMax( 0.9*maxVal);
            lut.setMin(-Math.PI);
            lut.setMax( Math.PI);
            lut.units = 'Velocity direction (rad)';
        } else if ( params.cg_field === 'Pressure' || params.cg_field === 'DEM Pressure' ) {
            val=S.cg_get_result(0, "Pressure", 0) ;
            lut = sequential;
            let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(0);
            lut.setMax( 0.9*maxVal);
            lut.units = 'Pressure (N/m<sup>'+String(params.dimension-1)+'</sup>)';
        } else if ( params.cg_field === 'Total Pressure' ) {
            val=S.cg_get_result(0, "Pressure", 0) ;
            // now loop through and any positions below p.water_table, add the weight of the water, i.e. p += p.water_density * p.g_mag * p.x
            let xmin = grid[0];
            // let ymin = grid[1];
            let dx = grid[3];
            // let dy = grid[4];
            let nx = grid[6];
            let ny = grid[7];
            for (let i = 0; i < nx; i++) {
                for (let j = 0; j < ny; j++) {
                    let index = i + j * nx;
                    let x = xmin + (i + 0.5) * dx;
                    // let y = ymin + (j + 0.5) * dy;
                    if (params.dimension === 2) {
                        if (x < params.water_table) {
                            val[index] += params.water_density * params.g_mag * (params.water_table - x);
                        }
                        
                    } else if (params.dimension === 3) {
                        if (z < params.water_table) {
                            val[index] += params.water_density * params.g_mag * (params.water_table - z);
                        }
                    }
                }
            }
            
            lut = sequential;
            let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(0);
            lut.setMax( 0.9*maxVal);
            lut.units = 'Pressure (N/m<sup>'+String(params.dimension-1)+'</sup>)';
        } else if ( params.cg_field === 'Hydrostatic Pressure' ) {
            val=S.cg_get_result(0, "Pressure", 0) ;
            let xmin = grid[0];
            let dx = grid[3];
            // let dy = grid[4];
            let nx = grid[6];
            let ny = grid[7];
            for (let i = 0; i < nx; i++) {
                for (let j = 0; j < ny; j++) {
                    let index = i + j * nx;
                    let x = xmin + (i + 0.5) * dx;
                    // let y = grid.ymin + (j + 0.5) * grid.dy;
                    if (params.dimension === 2) {
                        if (x < params.water_table) {
                            val[index] = params.water_density * params.g_mag * (params.water_table - x);
                        }
                        
                    } else if (params.dimension === 3) {
                        if (z < params.water_table) {
                            val[index] = params.water_density * params.g_mag * (params.water_table - z);
                        }
                    }
                }
            }
            lut = sequential;
            let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(0);
            lut.setMax( 0.9*maxVal);
            lut.units = 'Pressure (N/m<sup>'+String(params.dimension-1)+'</sup>)';
        } else if ( params.cg_field === 'Kinetic Pressure' ) {
            val=S.cg_get_result(0, "KineticPressure", 0) ;
            lut = divergent;
            let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(-0.9*maxVal);
            lut.setMax( 0.9*maxVal);
            lut.units = 'Kinetic pressure (N/m<sup>'+String(params.dimension-1)+'</sup>)';
        } else if ( params.cg_field === 'Shear stress' ) {
            val = S.cg_get_result(0, "TC", 1);
            lut = divergent;
            let maxVal = val.reduce(function(a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);
            lut.setMin(-0.9*maxVal);
            lut.setMax( 0.9*maxVal);
            lut.units = 'Shear stress (N/m<sup>'+String(params.dimension-1)+'</sup>)';
        }
        
        // update colorbar
        lut.opacity = params.cg_opacity;
        COLORBAR.renderColorbar(lut);

        for ( let i = 0; i < size; i ++ ) {
            var color = lut.getColor(val[i]);
            
            const r = Math.floor( color.r * 255 );
            const g = Math.floor( color.g * 255 );
            const b = Math.floor( color.b * 255 );
            const stride = i * 4;
            data[ stride     ] = r;//parseInt(val[i]/maxVal*255);
            data[ stride + 1 ] = g;
            data[ stride + 2 ] = b;
            if ( val[i] === 0 ) {
                data[ stride + 3 ] = 0;
            } else {
                if ( alpha === undefined ) {
                    data[ stride + 3 ] = opacity;
                } else {
                    // console.log(alpha)
                    // console.log(alpha[i]);
                    data[ stride + 3 ] = parseInt(255 * Math.pow(alpha[i]*params.cg_opacity,0.2));
                }
            }


        }
        const texture = new THREE.DataTexture( data, params.cg_width, params.cg_height );
        // texture.magFilter = THREE.LinearFilter; // smooth the data artifically
        texture.needsUpdate = true;
        cg_mesh.material.map = texture;
        // cg_mesh.material.opacity = parseInt(255*params.opacity);
    }
    else { 
        // show nothing
        const size = params.cg_width * params.cg_height;
        const data = new Uint8Array( 4 * size );
        const texture = new THREE.DataTexture( data, params.cg_width, params.cg_height );

        texture.needsUpdate = true;
        cg_mesh.material.map = texture;

        COLORBAR.hideColorbar();
    }
}

export function get_mean_pressure(S, params) {
    if ( params.cg_opacity === 0 ) {
        S.cg_param_read_timestep(0) ;
        S.cg_process_timestep(0,false) ;
    }
    let p =S.cg_get_result(0, "Pressure", 0) ;
    // console.log(p);
    // let mean_p = nanmean(p);
    let mean_p = p.reduce((a, b) => a + b, 0) / p.length;
    return mean_p;
}

function addvector(a,b){
    return a.map((e,i) => e + b[i]);
}
function norm(a,b){
    return a.map((e,i) => Math.sqrt(e*e * b[i]*b[i]));
}
function atan2vec(a,b){
    return a.map((e,i) => Math.atan2(e,b[i]));
}
function divide_vec(a,b){
    return a.map((e,i) => e/b);
}
// function nanmean(array) {
//     // Filter out NaN values
//     const filteredArray = array.filter((value) => !isNaN(value));
  
//     // Calculate the sum of filtered values
//     const sum = filteredArray.reduce((acc, value) => acc + value, 0);
  
//     // Calculate the mean
//     const mean = sum / filteredArray.length;
  
//     return mean;
//   }
