// import * as THREE from '../../node_modules/three/build/three.module.js';
import Papa from "papaparse";

var spheres;
var orientations;

async function load_world(params, time, world) {
  let promise = new Promise(function (resolve, reject) {
    var request = new XMLHttpRequest();
    if (params.cache) {
      request.open(
        "GET",
        params.data_dir + "Samples/" + params.fname + "in",
        true
      );
    } else {
      request.open(
        "GET",
        params.data_dir +
          "Samples/" +
          params.fname +
          "in?_=" +
          new Date().getTime(),
        true
      );
    }
    request.send(null);
    request.onreadystatechange = function () {
      if (
        request.readyState === 4 &&
        (request.status === 200 || request.status === 304)
      ) {
        // fully loaded and ( fresh or cached )
        // var type = request.getResponseHeader('Content-Type');
        // if (type.indexOf("text") !== 1) {
        var lines = request.responseText.split("\n");
        for (var i = 0; i < lines.length; i++) {
          // console.log(lines[i])
          var line = lines[i].replace(/ {1,}/g, " "); // remove multiple spaces
          var l = line.split(" ");
          if (l[0] == "dimensions") {
            params.N = parseInt(l[1]);
            params.num_particles = parseInt(l[2]);
            for (var j = 0; j < params.N; j++) {
              world.push({});
              world[j].min = 0;
              world[j].max = 1;
              world[j].cur = 0.5;
              world[j].prev = 0.5;
              world[j].wall = false;
            }
          } else if (l[0] == "boundary") {
            if (l[2] == "WALL" || l[2] == "PBC") {
              world[l[1]].min = parseFloat(l[3]);
              world[l[1]].max = parseFloat(l[4]);
              world[l[1]].cur = (world[l[1]].min + world[l[1]].max) / 2;
              world[l[1]].prev = world[l[1]].cur;
            }
            if (l[2] == "WALL") {
              world[l[1]].wall = true;
            }
          } else if (l[0] == "set") {
            if (l[1] == "T") {
              time.max = parseInt(l[2]) - 1;
            } else if (l[1] === "tdump") {
              time.save_rate = parseInt(l[2]);
            } else if (l[1] === "dt") {
              time.dt_dem = parseFloat(l[2]);
            }
          } else if (l[0] == "freeze") {
            params.pinky = parseInt(l[1]);
          }
        }
        if (params.N == 1) {
          // just used for setting up cameras etc
          world.push({});
          world[1].min = 0;
          world[1].max = 0;
          world[1].cur = 0.5;
          world[1].prev = 0.5;
        }
        if (params.N < 3) {
          // just used for setting up cameras etc
          world.push({});
          world[2].min = 0;
          world[2].max = 0;
          world[2].cur = 0.5;
          world[2].prev = 0.5;
        }
        time.frames_per_second = 1 / (time.save_rate * time.dt_dem); // time between DEM frames in seconds
        time.nt = time.max * time.frames_per_second; // total number of saved frames
        // build_world();
        // remove_everything(); // only runs on postMessage receive
        // animate();
        resolve([params, time, world]);
        // }
      }
    };
  });
  var output = await promise.then(function (result) {
    return result;
  });
  return output;
}

/**
 * Make the initial particles
 */
async function load_initial_spheres(params, time) {
  if (params.cache) {
    var filename =
      params.data_dir +
      "Samples/" +
      params.fname +
      "dump-" +
      String(time.cur * time.save_rate).padStart(5, "0") +
      ".csv";
  } else {
    var filename =
      params.data_dir +
      "Samples/" +
      params.fname +
      "dump-" +
      String(time.cur * time.save_rate).padStart(5, "0") +
      ".csv" +
      "?_=" +
      new Date().getTime();
  }
  console.log(filename);
  let promise = new Promise(function (resolve, reject) {
    Papa.parse(filename, {
      download: true,
      dynamicTyping: true,
      header: false,
      complete: function (results) {
        spheres = results.data.slice(1); // skip header
        resolve(spheres);
      },
    });
  });
  spheres = await promise.then(function (result) {
    return result;
  });
  return spheres;
}

/**
 * Update sphere locations
 * @param {number} t timestep
 * @param {number} changed_higher_dim_view flag to determine if we have changed which dimensions we are representing --- NOTE: CURRENTLY NOT DOING ANYTHING
 */
async function load_current_spheres(params, time, changed_higher_dim_view) {
  if (params.cache) {
    var filename =
      params.data_dir +
      "Samples/" +
      params.fname +
      "dump-" +
      String(time.frame * time.save_rate).padStart(5, "0") +
      ".csv";
  } else {
    var filename =
      params.data_dir +
      "Samples/" +
      params.fname +
      "dump-" +
      String(time.frame * time.save_rate).padStart(5, "0") +
      ".csv" +
      "?_=" +
      new Date().getTime();
  }
  let promise = new Promise(function (resolve, reject) {
    Papa.parse(filename, {
      download: true,
      dynamicTyping: true,
      header: false,
      cache: params.cache,
      complete: function (results) {
        resolve(results.data.slice(1)); // skip header
      },
    });
  });
  spheres = await promise.then(function (result) {
    return result;
  });
  return spheres;
}

async function load_current_orientation(params, time, changed_higher_dim_view) {
  if (params.cache) {
    var filename =
      params.data_dir +
      "Samples/" +
      params.fname +
      "dumpA-" +
      String(time.frame * time.save_rate).padStart(5, "0") +
      ".csv";
  } else {
    var filename =
      params.data_dir +
      "Samples/" +
      params.fname +
      "dumpA-" +
      String(time.frame * time.save_rate).padStart(5, "0") +
      ".csv" +
      "?_=" +
      new Date().getTime();
  }
  let promise = new Promise(function (resolve, reject) {
    Papa.parse(filename, {
      download: true,
      dynamicTyping: true,
      header: false,
      cache: params.cache,
      complete: function (results) {
        resolve(results.data.slice(1));
      },
    });
  });
  orientations = await promise.then(function (result) {
    return result;
  });
  return orientations;
}

export {
  load_world,
  load_initial_spheres,
  load_current_spheres,
  load_current_orientation,
};
