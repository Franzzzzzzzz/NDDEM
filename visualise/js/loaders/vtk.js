import * as THREE from "three";
import { VTKLoader } from 'three/examples/jsm/loaders/VTKLoader.js';

var all_locs;

async function load_world(params, time, world) {
  if (params.fname.substr(-1) === "/") {
    var fname = params.fname.slice(0, -1);
  } // remove trailing slash (which we just added) ; //
  if (params.cache) {
    var filename = params.data_dir + "Samples/" + fname;
  } else {
    var filename =
      params.data_dir + "Samples/" + fname + "?_=" + new Date().getTime();
  }
  let promise = new Promise(function (resolve, reject) {
    const loader = new VTKLoader();
    loader.load( filename, function ( geometry ) {

    	geometry.computeVertexNormals();
    	geometry.center();

        world.push({});
        world[0].min = geometry.boundingBox.min[0];
        world[0].max = geometry.boundingBox.max[0];
        world[0].cur = (world[0].min + world[0].max) / 2;
        world[0].prev = world[0].cur;
        world.push({});
        world[1].min = geometry.boundingBox.min[1];
        world[1].max = geometry.boundingBox.max[1];
        world[1].cur = (world[1].min + world[1].max) / 2;
        world[1].prev = world[1].cur;
        world.push({});
        world[2].min = geometry.boundingBox.min[2];
        world[2].max = geometry.boundingBox.max[2];
        world[2].cur = (world[2].min + world[2].max) / 2;
        world[2].prev = world[2].cur;
    } );
    // Papa.parse(filename, {
    //   download: true,
    //   dynamicTyping: true,
    //   header: false,
    //   preview: 1, // just load one line!
    //   complete: function (results) {
    //     params.N = 3; // HACK: ASSUMED!!!
    //     var first_row = results.data[0][0].split(" ");
    //     // console.log(first_row);
    //     params.num_particles = first_row[0];
    //     // console.log(params.num_particles);
    //     world.push({});
    //     world[0].min = first_row[2];
    //     world[0].max = first_row[5];
    //     world[0].cur = (world[0].min + world[0].max) / 2;
    //     world[0].prev = world[0].cur;
    //     world.push({});
    //     world[1].min = first_row[3];
    //     world[1].max = first_row[6];
    //     world[1].cur = (world[1].min + world[1].max) / 2;
    //     world[1].prev = world[1].cur;
    //     world.push({});
    //     world[2].min = first_row[4];
    //     world[2].max = first_row[7];
    //     world[2].cur = (world[2].min + world[2].max) / 2;
    //     world[2].prev = world[2].cur;
    //
    //     // HACK: STILL NOT FINDING TIME STEP OR NUMBER OF TIME STEPS!!!!
    //     time.frames_per_second = 1;
    //
    //     resolve([params, time, world]);
    //   },
    // });
  });
  var output = await promise.then(function (result) {
    return result;
  });
  return output;
}

async function load_initial_spheres(params, time) {
  let promise = new Promise(function (resolve, reject) {
    var request = new XMLHttpRequest();
    if (params.fname.substr(-1) === "/") {
      var fname = params.fname.slice(0, -1);
    } // remove trailing slash (which we just added) ; //
    if (params.cache) {
      var filename = params.data_dir + "Samples/" + fname;
    } else {
      var filename =
        params.data_dir + "Samples/" + fname + "?_=" + new Date().getTime();
    }
    request.open("GET", filename, true);
    request.send(null);
    request.onreadystatechange = function () {
      if (
        request.readyState === 4 &&
        (request.status === 200 || request.status === 304)
      ) {
        // fully loaded and ( fresh or cached )
        all_locs = [];
        var lines = request.responseText.split("\n");
        for (var i = 0; i < lines.length; i++) {
          var l = lines[i].split(" ");
          if (l[0] == params.num_particles) {
            all_locs.push([]);
            if (all_locs.length == 2) {
              time.save_rate = l[1];
            }
          } else {
            all_locs[all_locs.length - 1].push([parseFloat(l[0]),
                                                parseFloat(l[1]),
                                                parseFloat(l[2]),
                                                parseFloat(l[6])]);
          }
        }
        time.max = all_locs.length - 1;
        // console.log(all_locs);
        resolve(all_locs);
      }
    };
  });
  var output = await promise.then(function (result) {
    return result[0];
  });
  return output;
}

async function load_current_spheres(params, time, changed_higher_dim_view) {
  return all_locs[time.frame];
}

async function load_current_orientation(params, time, changed_higher_dim_view) {
  console.warn("Rotations not implemented for MercuryDPM loader!");
}

export {
  load_world,
  load_initial_spheres,
  load_current_spheres,
  load_current_orientation,
};
