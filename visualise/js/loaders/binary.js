// import * as THREE from '../../node_modules/three/build/three.module.js';

var all_locs, all_rots;

import { load_world } from "./default.js";

async function load_initial_spheres(params, time) {
  var locfilename = params.data_dir + "Samples/" + params.fname + "loc.bin";
  let promise = new Promise(function (resolve, reject) {
    var lReq = new XMLHttpRequest();
    lReq.open("GET", locfilename, true);
    lReq.responseType = "arraybuffer";
    lReq.onload = function (oEvent) {
      if (lReq.status == 404) {
        const loadingText = document.getElementById("loading-text");
        loadingText.innerHTML = "File not found!";
      } else {
        var arrayBuffer = lReq.response;
        var dataview = new DataView(arrayBuffer);
        var num_data_pts = arrayBuffer.byteLength / 4;
        var nt = num_data_pts / params.num_particles / (params.N + 4);

        var a = new Array(nt);
        for (var i = 0; i < nt; i++) {
          a[i] = new Array(params.num_particles);
          for (var j = 0; j < params.num_particles; j++) {
            a[i][j] = new Array(params.N + 4);
            for (var k = 0; k < params.N + 4; k++) {
              a[i][j][k] = dataview.getFloat32(
                4 * (k + (params.N + 4) * (j + params.num_particles * i)),
                true
              );
            }
          }
        }
        resolve(a);
        // make_initial_spheres(all_locs[0])
        // update_spheres(all_locs[0],true);
        // remove_loading_screen();
      }
    };
    lReq.send(null);
  });

  if (params.view_mode === "rotations2") {
    var rotfilename = params.data_dir + "Samples/" + params.fname + "rot.bin";
    var rReq = new XMLHttpRequest();
    rReq.open("GET", rotfilename, true);
    rReq.responseType = "arraybuffer";
    rReq.onload = function (oEvent) {
      var arrayBuffer = rReq.response;
      var dataview = new DataView(arrayBuffer);
      var num_data_pts = arrayBuffer.byteLength / 4;
      var nt = num_data_pts / params.num_particles / (params.N * params.N);

      all_rots = new Array(nt);
      for (var i = 0; i < nt; i++) {
        all_rots[i] = new Array(params.num_particles);
        for (var j = 0; j < params.num_particles; j++) {
          all_rots[i][j] = new Array(params.N * params.N);
          for (var k = 0; k < params.N * params.N; k++) {
            all_rots[i][j][k] = dataview.getFloat32(
              4 * (k + params.N * params.N * (j + params.num_particles * i)),
              true
            );
          }
        }
      }
      // console.log(all_locs)
    };
    rReq.send(null);
  }
  return await promise.then(function (result) {
    all_locs = result;
    return result[time.frame];
  });
}

async function load_current_spheres(params, time, changed_higher_dim_view) {
  // console.log(all_locs[time.frame]);
  return all_locs[time.frame];
}

async function load_current_orientation(params, time, changed_higher_dim_view) {
  return all_rots[time.frame];
}

export {
  load_world,
  load_initial_spheres,
  load_current_spheres,
  load_current_orientation,
};
