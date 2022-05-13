import * as THREE from "three";
// import { VTKLoader } from '../../node_modules/three/examples/jsm/loaders/VTKLoader.js'

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
    Papa.parse(filename, {
      download: true,
      dynamicTyping: true,
      header: false,
      preview: 8, // just load eight lines
      complete: function (results) {
        params.N = 3; // HACK: ASSUMED!!!
        params.num_particles = results.data[3][0];

        var x1 = results.data[5][0].split(" ");
        var x2 = results.data[6][0].split(" ");
        var x3 = results.data[7][0].split(" ");
        world.push({});
        world[0].min = parseFloat(x1[0]);
        world[0].max = parseFloat(x1[1]);
        world[0].cur = (world[0].min + world[0].max) / 2;
        world[0].prev = world[0].cur;
        world.push({});
        world[1].min = parseFloat(x2[0]);
        world[1].max = parseFloat(x2[1]);
        world[1].cur = (world[1].min + world[1].max) / 2;
        world[1].prev = world[1].cur;
        world.push({});
        world[2].min = parseFloat(x3[0]);
        world[2].max = parseFloat(x3[1]);
        world[2].cur = (world[2].min + world[2].max) / 2;
        world[2].prev = world[2].cur;
        // console.log(world);
        // HACK: STILL NOT FINDING TIME STEP OR NUMBER OF TIME STEPS!!!!
        time.frames_per_second = 1;

        resolve([params, time, world]);
      },
    });
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
          if (lines[i] == "ITEM: TIMESTEP") {
            all_locs.push(new Array(params.num_particles));
            if (all_locs.length == 1) {
              var m = lines[i + 8].split(" ");
              var x1_index = m.indexOf("x") - 2;
              var x2_index = m.indexOf("y") - 2;
              var x3_index = m.indexOf("z") - 2;
              var radius_index = m.indexOf("radius") - 2;
            }
            if (all_locs.length == 2) {
              time.save_rate = lines[i + 1];
            }
            i += 8;
          } else {
            all_locs[all_locs.length - 1][l[0] - 1] = [
              parseFloat(l[x1_index]),
              parseFloat(l[x2_index]),
              parseFloat(l[x3_index]),
              parseFloat(l[radius_index]),
            ];
          }
        }
        time.max = all_locs.length - 1;
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
  console.warn("Rotations not implemented for LIGGGHTS loader!");
}

export {
  load_world,
  load_initial_spheres,
  load_current_spheres,
  load_current_orientation,
};
