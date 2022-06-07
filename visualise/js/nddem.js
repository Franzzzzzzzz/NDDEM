import * as THREE from "three";
console.log("Using threejs version " + THREE.REVISION);
// import { LoaderSupport } from "../node_modules/three/examples/js/loaders/LoaderSupport.js";
import { OBJLoader } from "three/examples/jsm/loaders/OBJLoader.js";
// import { MTLLoader } from "../node_modules/three/examples/js/loaders/MTLLoader.js";
import { VRController } from "../js/VRControllerModule.js";
import { Lut } from "three/examples/jsm/math/Lut.js";
import Stats from "three/examples/jsm/libs/stats.module.js";

import * as PARAMS from "./params.js";
import * as CAMERA from "./camera.js";
import * as AXES from "./axes.js";
import * as TORUS from "./torus.js";
import * as GUI from "./gui.js";

const recorder = new CCapture({
  verbose: true,
  display: true,
  framerate: 10,
  quality: 100,
  format: "png",
  timeLimit: 100,
  frameLimit: 0,
  autoSaveTime: 0,
});

var container; // main div element
var scene, renderer, particles, stats; // UI elements
var world = []; // properties that describe the domain
world.ref_dim = { c: 1 }; //, 'x': 00, 'y': 1, 'z': 2}; // reference dimensions
var time = {
  cur: 0,
  frame: 0,
  prev_frame: 0,
  min: 0,
  max: 99,
  play: false,
  play_rate: 5.0,
  save_rate: 1000,
  snapshot: false,
}; // temporal properties
var bg; // background mesh with texture attached
var redraw_left = false; // force redrawing of particles from movement in left hand
var redraw_right = false; // force redrawing of particles from movement in right hand
var left_hand, right_hand; // store parameters for movement in higher dims via hand controls
var winning = false; // did you win the game?
var winning_texture; // texture to hold 'WINNING' sign for catch_particle mode
var clock = new THREE.Clock(); // global clock
var lut = new Lut("blackbody", 512); // options are rainbow, cooltowarm and blackbody
var LOADER;
var NDParticleShader;
var texture_path ;

var params = PARAMS.process_params(time);

import("./loaders/" + params.data_type + ".js").then((module) => {
  // console.log(time);
  LOADER = module;
  LOADER.load_world(params, time, world).then((output) => {
    params = output[0];
    time = output[1];
    world = output[2];
    // console.log(params)
    import("../../live/libs/shaders/" + params.N + "DShader.js").then((module) => {
      NDParticleShader = module.NDDEMShader;
      build_world();
      remove_everything(); // only runs on postMessage receive
      animate();
    });
  });
});

/**
 * Initialise the threejs scene, adding everything necessary, such as camera, controls, lighting etc.
 */
function build_world() {
  var s = LOADER.load_initial_spheres(params, time).then((s) => {
    make_initial_spheres(s);
    remove_loading_screen();
    update_spheres(s);
    return s;
  });

  container = document.createElement("div");
  document.body.appendChild(container);

  var N_tag = document.createElement("div");
  N_tag.setAttribute("id", "N_tag");
  N_tag.innerHTML = params.N + "D";
  container.appendChild(N_tag);

  scene = new THREE.Scene();
  if (params.colour_scheme === "inverted") {
    scene.background = new THREE.Color(0xffffff); // white
    N_tag.style.color = "black";
  } else {
    scene.background = new THREE.Color(0x111111); // revealjs background colour
  }
  if (params.display_type === "VR") {
    var geometry = new THREE.SphereBufferGeometry(500, 60, 40);
    // invert the geometry on the x-axis so that all of the faces point inward
    geometry.scale(-1, 1, 1);
    var texture = new THREE.TextureLoader().load(
      params.root_dir + "visualise/resources/eso0932a.jpg"
    );
    winning_texture = new THREE.TextureLoader().load(
      params.root_dir + "visualise/resources/winning.png"
    );
    var material = new THREE.MeshBasicMaterial({ map: texture });
    bg = new THREE.Mesh(geometry, material);
    bg.rotation.z = Math.PI / 2;
    scene.add(bg);
  }

  CAMERA.make_camera(scene, params, world);
  if (!params.no_walls) {
    AXES.make_walls(scene, params, world);
  }
  if (!params.no_axes && !params.quasicrystal) {
    AXES.make_axes(scene, params, world);
  }
  make_lights();
  renderer = CAMERA.add_renderer(params, container);
  if (!params.fname.includes("Submarine")) {
    CAMERA.add_controllers(scene, params, world, renderer);
  }
  // add_controllers();
  if (params.N > 3 && !params.fname.includes("Spinner") && !params.no_tori) {
    TORUS.add_torus(scene, params, world, particles);
  }

  GUI.add_gui(params, world, time, recorder, LOADER);
  window.addEventListener(
    "resize",
    function () {
      CAMERA.on_window_resize(params, scene, renderer);
      render(params, scene);
    },
    false
  );

  if ( params.stats ) {
      stats = new Stats();
      document.body.appendChild( stats.dom );
  }
  //if ( params.display_type === 'VR' ) { add_vive_models(); }
}

/**
 * Make the scene lighting
 */
function make_lights() {
  var background_light = new THREE.AmbientLight(0x909090);
  scene.add(background_light);

  var light = new THREE.DirectionalLight(0xffffff);
  light.position.set(
    world[0].max,
    -world[0].max,
    (world[2].min + world[2].max) / 2
  );
  light.lookAt(
    (world[2].min + world[2].max) / 2,
    (world[2].min + world[2].max) / 2,
    (world[2].min + world[2].max) / 2
  );

  var light1 = new THREE.DirectionalLight(0xffffff);
  light1.position.set(
    world[0].max,
    world[0].max,
    (world[2].min + world[2].max) / 2
  );
  light1.lookAt(
    (world[2].min + world[2].max) / 2,
    (world[2].min + world[2].max) / 2,
    (world[2].min + world[2].max) / 2
  );

  if (params.shadows) {
    light.castShadow = true;
    light.shadow.mapSize.set(4096, 4096);
  }

  scene.add(light);
  scene.add(light1);
}

/**
 * Remove everything from scene - very useful for presentation mode when we don't want to kill the computer by loading multiple scenes simultaneously
 */
function remove_everything() {
  window.addEventListener("message", receiveMessage, false);
  function receiveMessage(event) {
    console.log(
      "Closing renderer. Current number of programs:" +
        renderer.info.programs.length
    );
    // console.log(particles);
    // console.log(wristband);
    // console.log(wristband1);
    // remove all particles
    for (var i = particles.children.length; (i = 0); i--) {
      var object = particles.children[i];
      object.geometry.dispose();
      object.material.dispose();
      if (params.view_mode === "rotations") {
        object.texture.dispose();
      }
    }
    TORUS.delete_everything(params);
    if (params.display_type === "anaglyph") {
      CAMERA.effect.dispose();
    }
    renderer.xr.enabled = false;
    renderer.dispose();
    scene.dispose();
    console.log(
      "Killed everything. Remaining programs:" + renderer.info.programs.length
    );
  }
}

/**
 * Make the initial texturing if showing rotations
 */
function make_initial_sphere_texturing() {
  var commandstring = "";
  for (var i = 3; i < params.N; i++) {
    commandstring =
      commandstring + ("x" + (i + 1) + "=" + world[i].cur.toFixed(1));
    if (i < params.N - 1) commandstring += "&";
  }
  request = new XMLHttpRequest();
  /*request.open('POST', params.root_dir + "make_textures?" +
                 "arr=" + JSON.stringify(arr) +
                 "&N=" + params.N +
                 "&t=" + "00000" +
                 "&quality=" + params.quality +
                 "&fname=" + params.fname,
                 true);*/
  request.open(
    "GET",
    params.root_dir +
      "load?ND=" +
      params.N +
      "&path=" +
      params.fname +
      "&texturepath=../" +
      params.texture_dir +
      "&resolution=" +
      params.quality,
    true
  );
  console.log(request)
  request.send(null);

  request.onload = function () {
    request.open(
      "GET",
      params.root_dir + "render?ts=00000&" + commandstring,
      true
    );
    request.send(null);
    request.onload = function () {
      LOADER.load_initial_spheres(params, time).then(() => {
        make_initial_spheres(LOADER.spheres).then(() => {
          update_spheres(LOADER.spheres);
        });
      });
    };
  };
  // Let's do the first rendering as well
  //request.open('GET', params.root_dir + 'render?ts=00000&' + commandstring, true) ;
  //request.send(null);

  // request.onreadystatechange = function () {}
}

/**
 * Load particles from MercuryDPM file format - NOTE: THIS IS NOT WORKING YET
 */

function make_initial_spheres(spheres) {
  particles = new THREE.Group();
  scene.add(particles);
  if (params.N == 1) {
    var geometry = new THREE.CylinderGeometry(
      1,
      1,
      2,
      Math.pow(2, params.quality),
      Math.pow(2, params.quality)
    );
  } else {
    // var geometry = new THREE.SphereGeometry( 1, Math.pow(2,params.quality), Math.pow(2,params.quality) );
    var geometry = new THREE.SphereGeometry(
        1,
        Math.pow(2, params.quality),
        Math.pow(2, params.quality)
    );
  }
  var pointsGeometry = new THREE.SphereGeometry(
    1,
    Math.max(Math.pow(2, params.quality - 2), 4),
    Math.max(Math.pow(2, params.quality - 2), 4)
  );
  var scale = 20; // size of particles on tori
  // if ( params.view_mode === 'rotations2' ) {
  //     var uniforms = {
  //         N: { value: params.N },
  //         N_lines: { value: 5.0 },
  //         //A: { value: new THREE.Matrix4() },
  //         A: {value: []}, // Size N*N
  //         xview: {value: []}, //Size N-3
  //         xpart: {value: []}, //Size N-3
  //         x4: { value: 0 },
  //         x4p: { value: 0 },
  //         R: { value: 1 },
  //     };
  //     for (var ij=0 ; ij<params.N-3 ; ij++)
  //     {
  //         uniforms.xview.value[ij] = world[ij].cur ;
  //         uniforms.xpart.value[ij] = 0. ;
  //     }
  //     if ( params.N > 3 ) { uniforms.x4.value = world[3].cur; }
  //     for (var ij=0 ; ij<params.N*params.N ; ij++)
  //     {
  //         if (ij%params.N == Math.floor(ij/params.N))
  //             uniforms.A.value[ij] = 1 ;
  //         else
  //             uniforms.A.value[ij] = 0 ;
  //     }
  //     var NDParticleShader = new THREE.ShaderMaterial( {
  //         uniforms: uniforms,
  //         vertexShader: document.getElementById( 'vertexshader-'+String(uniforms.N.value)+'D' ).textContent,
  //         fragmentShader: document.getElementById( 'fragmentshader' ).textContent
  //     } );
  // }
  if (params.view_mode === "size") {
    params.minRadius = 999999;
    params.maxRadius = 0;
    for (var j = 0; j < params.num_particles; j++) {
      // console.log(spheres[j][params.N]);
      if (spheres[j][params.N] < params.minRadius) {
        params.minRadius = spheres[j][params.N];
      }
      if (spheres[j][params.N] > params.maxRadius) {
        params.maxRadius = spheres[j][params.N];
      }
    }
    console.log(params.minRadius, params.maxRadius);
    lut.setMin(params.minRadius);
    lut.setMax(params.maxRadius);
  }
  // var sphere_mesh_instaced = new THREE.InstancedMesh(geometry, material, params.num_particles); // TODO: TRY THIS OUT TO INCREASE SPEED
  for (var i = 0; i < spheres.length; i++) {
    if (params.N < 3) {
      var color = ((Math.random() + 0.25) / 1.5) * 0xffffff;
      var material = new THREE.PointsMaterial({
        color: color,
      });
    } else {
      if (
        params.view_mode === "catch_particle" ||
        params.fname.includes("Lonely")
      ) {
        if (i == params.pinky) {
          var color = 0xe72564;
        } else {
          var color = 0xaaaaaa;
        }
        var material = new THREE.MeshPhongMaterial({ color: color });
      } else if (params.view_mode === "rotations2") {
        var material = NDParticleShader.clone();
        if (params.colour_scheme === "inverted") {
          var color = 0xdddddd; // for torus
        } else {
          var color = 0.222222; // for torus
        }
      } else if (params.view_mode === "size") {
        // console.log(spheres[i][params.N])
        var color = lut.getColor(spheres[i][params.N]);
        var material = new THREE.MeshPhongMaterial({ color: color });
        // console.log(color)
      } else {
        if (params.view_mode === "rotations") {
          texture_path =
            params.data_dir + params.texture_dir + "/Texture-" + i + "-00000";
          for (var iiii = 3; iiii < params.N; iiii++) {
            texture_path += "-0.0";
          }
          var texture = new THREE.TextureLoader().load(texture_path + ".png"); //TODO
          var material = new THREE.MeshBasicMaterial({ map: texture });
        } else {
          var color = ((Math.random() + 0.25) / 1.5) * 0xffffff;
          var material = new THREE.MeshPhongMaterial({ color: color });
        }
      }
    }
    var object = new THREE.Mesh(geometry, material);

    object.position.set(spheres[i][0], spheres[i][1], spheres[i][2]);
    object.rotation.z = Math.PI / 2;
    // if ( params.fname.includes('Coll') || params.fname.includes('Roll') ) { object.rotation.x = Math.PI/2.; }
    if (params.shadows) {
      object.castShadow = true;
      object.receiveShadow = true;
    }
    particles.add(object);
    if (params.N > 3 && !params.fname.includes("Spinner") && !params.no_tori) {
      var pointsMaterial = new THREE.PointsMaterial({ color: color });
      var object2 = new THREE.Mesh(pointsGeometry, pointsMaterial);
      if (params.fname.includes("Lonely")) {
        object2.scale.set(
          (2 * TORUS.R) / scale,
          (2 * TORUS.R) / scale,
          (2 * TORUS.R) / scale
        );
      } else {
        object2.scale.set(TORUS.R / scale, TORUS.R / scale, TORUS.R / scale);
      }
      object2.position.set(0, 0, 0);
      TORUS.wristband1.add(object2);
      if (params.N > 5) {
        var object3 = object2.clone();
        TORUS.wristband2.add(object3);
      }
    }
  }
  if (params.fname.includes("Submarine")) {
    CAMERA.camera.position.set(
      particles.children[params.pinky].position.x,
      particles.children[params.pinky].position.y,
      particles.children[params.pinky].position.z
    );
    console.log(CAMERA.camera.position);
  }
}

function update_orientation(spheres) {
  for (var i = 0; i < spheres.length; i++) {
    // skip header
    var object = particles.children[i];
    object.material.uniforms.A.value = spheres[i];
  }
}

/**
 * Load textures from TexturingServer
 * @param {number} t timestep
 * @param {number} Viewpoint where we are in D>3 space
 */
function load_textures(t, Viewpoint) {
  if (particles !== undefined) {
    var loader = new THREE.TextureLoader();
    for (var ii = 0; ii < particles.children.length - 1; ii++) {
      if (params.cache) {
        var filename =
          params.data_dir +
          params.texture_dir +
          "/Texture-" +
          ii +
          "-" +
          Viewpoint +
          ".png";
      } else {
        var filename =
          params.data_dir +
          params.texture_dir +
          "/Texture-" +
          ii +
          "-" +
          Viewpoint +
          ".png" +
          "?_=" +
          new Date().getTime();
      }
      loader.load(filename, function (texture) {
        //TODO not sure why not working ... ...
        //var myRe = /-[0-9]+.png/g
        //var res=myRe.exec(texture.image.currentSrc)
        //var myRe2 = /[0-9]+/
        //var iii = myRe2.exec(texture.image.currentSrc)[0]
        var iii = texture.image.currentSrc.split("-")[1];
        console.log(texture.image.currentSrc);
        console.log(iii);
        var o = particles.children[iii];
        o.material.map = texture;
        o.material.map.needsUpdate = true;
      });
    }
  }
}

/**
 * Update textures from TexturingServer
 * @param {number} t timestep
 */
function update_spheres_texturing(t) {
  if (true) {
    //TODO Do something better ...
    var commandstring = "";
    var Viewpoint = String(t * time.save_rate).padStart(5, "0");

    for (var i = 3; i < params.N; i++) {
      commandstring =
        commandstring + "&" + ("x" + (i + 1) + "=" + world[i].cur.toFixed(1));
      Viewpoint = Viewpoint + "-" + world[i].cur.toFixed(1);
    }

    var request = new XMLHttpRequest();
    /*request.open('POST',
                       params.root_dir + "make_textures?" +
                       "arr=" + JSON.stringify(arr) +
                       "&N=" + params.N +
                       "&t=" + t + "0000" +
                       "&quality=" + params.quality +
                       "&fname=" + params.fname,
                       true);*/
    var runvalue = 0;
    if (time.play) runvalue = 1;
    request.open(
      "GET",
      params.data_dir +
        "render?ts=" +
        String(t * time.save_rate).padStart(5, "0") +
        commandstring +
        "&running=" +
        runvalue,
      true
    );

    request.onload = function () {
      load_textures(t, Viewpoint);
    };
    request.send("");
  } else {
    load_textures(t, Viewpoint);
  }
}

function update_spheres(spheres) {
  for (var i = 0; i < spheres.length; i++) {
    var object = particles.children[i];

    if (params.N > 3) {
      var x3_unrotated = spheres[i][3];

      var x0_temp =
        spheres[i][0] * Math.cos(params.euler.theta_1) -
        spheres[i][3] * Math.sin(params.euler.theta_1);
      var x3_temp =
        spheres[i][0] * Math.sin(params.euler.theta_1) +
        spheres[i][3] * Math.cos(params.euler.theta_1);

      var x1_temp =
        spheres[i][1] * Math.cos(params.euler.theta_2) -
        x3_temp * Math.sin(params.euler.theta_2);
      var x3_temp =
        spheres[i][1] * Math.sin(params.euler.theta_2) +
        x3_temp * Math.cos(params.euler.theta_2);

      var x2_temp =
        spheres[i][2] * Math.cos(params.euler.theta_3) -
        x3_temp * Math.sin(params.euler.theta_3);
      var x3_temp =
        spheres[i][2] * Math.sin(params.euler.theta_3) +
        x3_temp * Math.cos(params.euler.theta_3);

      spheres[i][0] = x0_temp;
      spheres[i][1] = x1_temp;
      spheres[i][2] = x2_temp;
      spheres[i][3] = x3_temp;
    }
    // if (params.N == 1) {
    //   spheres[i][1] = 0;
    // }
    // if (params.N < 3) {
    //   spheres[i][2] = 0;
    // }
    if (params.N < 4) {
      var R_draw = spheres[i][params.N];
    } else if (params.N == 4) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2)
      );

      //if ( (world[3].cur >  world[3].max-spheres[i][params.N] ) // NOTE: IMPLEMENT THIS!!
    } else if (params.N == 5) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2)
      );
    } else if (params.N == 6) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2)
      );
    } else if (params.N == 7) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2) -
          Math.pow(world[6].cur - spheres[i][6], 2)
      );
    } else if (params.N == 8) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2) -
          Math.pow(world[6].cur - spheres[i][6], 2) -
          Math.pow(world[7].cur - spheres[i][7], 2)
      );
    } else if (params.N == 10) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2) -
          Math.pow(world[6].cur - spheres[i][6], 2) -
          Math.pow(world[7].cur - spheres[i][7], 2) -
          Math.pow(world[8].cur - spheres[i][8], 2) -
          Math.pow(world[9].cur - spheres[i][9], 2)
      );
    } else if (params.N == 30) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2) -
          Math.pow(world[6].cur - spheres[i][6], 2) -
          Math.pow(world[7].cur - spheres[i][7], 2) -
          Math.pow(world[8].cur - spheres[i][8], 2) -
          Math.pow(world[9].cur - spheres[i][9], 2) -
          Math.pow(world[10].cur - spheres[i][10], 2) -
          Math.pow(world[11].cur - spheres[i][11], 2) -
          Math.pow(world[12].cur - spheres[i][12], 2) -
          Math.pow(world[13].cur - spheres[i][13], 2) -
          Math.pow(world[14].cur - spheres[i][14], 2) -
          Math.pow(world[15].cur - spheres[i][15], 2) -
          Math.pow(world[16].cur - spheres[i][16], 2) -
          Math.pow(world[17].cur - spheres[i][17], 2) -
          Math.pow(world[18].cur - spheres[i][18], 2) -
          Math.pow(world[19].cur - spheres[i][19], 2) -
          Math.pow(world[20].cur - spheres[i][20], 2) -
          Math.pow(world[21].cur - spheres[i][21], 2) -
          Math.pow(world[22].cur - spheres[i][22], 2) -
          Math.pow(world[23].cur - spheres[i][23], 2) -
          Math.pow(world[24].cur - spheres[i][24], 2) -
          Math.pow(world[25].cur - spheres[i][25], 2) -
          Math.pow(world[26].cur - spheres[i][26], 2) -
          Math.pow(world[27].cur - spheres[i][27], 2) -
          Math.pow(world[28].cur - spheres[i][28], 2) -
          Math.pow(world[29].cur - spheres[i][29], 2)
      );
    }
    if (isNaN(R_draw)) {
      object.visible = false;
      if (params.view_mode === "D4" || params.view_mode === "D5") {
        // if ( params.colour_scheme === 'inverted' ) {
        TORUS.wristband1.children[i].material.color = new THREE.Color(0x777777);
        // }
        // else {
        // TORUS.wristband1.children[i].material.color = new THREE.Color( 0x111111 );
        // }
      }
    } else {
      if (params.fname.includes("Submarine") && i == params.pinky) {
        object.visible = false;
      } else {
        if (params.display_type === "VR") {
          R_draw = R_draw * params.vr_scale;
          object.position.set(
            spheres[i][1] * params.vr_scale,
            spheres[i][0] * params.vr_scale - params.human_height,
            spheres[i][2] * params.vr_scale
          );
        } else {
          object.position.set(spheres[i][0], spheres[i][1], spheres[i][2]);
          // console.log(object.position)
        }
        if (params.quasicrystal) {
          var scale = 5;
          object.scale.set(
            spheres[i][params.N] / scale,
            spheres[i][params.N] / scale,
            spheres[i][params.N] / scale
          );
        } else {
          object.scale.set(R_draw, R_draw, R_draw);
        }
        object.visible = true;
        if (params.view_mode === "velocity") {
          lut.setMin(0);
          lut.setMax(params.velocity.vmax);
          object.material.color = lut.getColor(spheres[i][params.N + 2]);
        } else if (params.view_mode === "rotation_rate") {
          lut.setMin(0);
          lut.setMax(params.velocity.omegamax);
          object.material.color = lut.getColor(spheres[i][params.N + 3]);
        } else if (params.view_mode === "rotations2") {
          for (var j = 0; j < params.N - 3; j++) {
            particles.children[i].material.uniforms.xview.value[j] =
              world[j + 3].cur;
            particles.children[i].material.uniforms.xpart.value[j] =
              spheres[i][j + 3];
          }
          if (params.N > 3) {
            object.material.uniforms.x4p.value = spheres[i][3];
            object.material.uniforms.x4.value = world[3].cur;
          } else {
            object.material.uniforms.x4p.value = 0.0;
          }
          //if (object.material.uniforms.xpart.value[0] != object.material.uniforms.x4p.value)
          //{console.log(object.material.uniforms.xpart.value[0]) ; console.log(object.material.uniforms.x4p.value) ; }
          // object.material.uniforms.xp.value = new THREE.Vector4(spheres[i][1],spheres[i][2],spheres[i][3],spheres[i][4])
          // object.material.uniforms.R.value = R_draw;
        } else if (params.view_mode === "D4") {
          //lut.setMin(world[3].min);
          //lut.setMax(world[3].max);
          lut.setMin(world[3].cur - 2 * TORUS.r);
          lut.setMax(world[3].cur + 2 * TORUS.r);
          object.material.color = lut.getColor(x3_unrotated);
          TORUS.wristband1.children[i].material.color = lut.getColor(
            x3_unrotated
          );
        } else if (params.view_mode === "D5") {
          //lut.setMin(world[4].min);
          //lut.setMax(world[4].max);
          lut.setMin(world[4].cur - 2 * TORUS.r);
          lut.setMax(world[4].cur + 2 * TORUS.r);
          object.material.color = lut.getColor(spheres[i][4]);
          TORUS.wristband1.children[i].material.color = lut.getColor(
            spheres[i][4]
          );
        } else if (params.view_mode === "size") {
          // lut.setMin(params.minRadius);
          // lut.setMax(params.maxRadius);
          // object.material.color = lut.getColor(spheres[i][params.N]);
        }
      }
      // else { object.mateial.color = '#ff00ff'}
    }
    if (!params.no_tori) {
      if (params.N == 4 && !params.fname.includes("Spinner")) {
        var object2 = TORUS.wristband1.children[i];
        var phi =
          (2 * Math.PI * (world[3].cur - spheres[i][3])) /
            (world[3].max - world[3].min) -
          Math.PI / 2;
        var x = (TORUS.R + TORUS.r) * Math.cos(phi);
        var y = (TORUS.R + TORUS.r) * Math.sin(phi);
        var z = 0;
        object2.position.set(x, y, z);
      }

      if (
        params.N > 4 &&
        !params.fname.includes("Spinner") &&
        !params.no_tori
      ) {
        var object2 = TORUS.wristband1.children[i];
        var phi =
          (2 * Math.PI * (world[3].cur - spheres[i][3])) /
            (world[3].max - world[3].min) -
          Math.PI / 2;
        var theta =
          (2 * Math.PI * (world[4].cur - spheres[i][4])) /
          (world[4].max - world[4].min);
        var x = (TORUS.R + TORUS.r * Math.cos(theta)) * Math.cos(phi);
        var y = (TORUS.R + TORUS.r * Math.cos(theta)) * Math.sin(phi);
        var z = TORUS.r * Math.sin(theta);
        object2.position.set(x, y, z);
      }

      if (params.N == 6 && !params.fname.includes("Spinner")) {
        var object3 = TORUS.wristband2.children[i];
        var phi =
          (2 * Math.PI * (world[5].cur - spheres[i][5])) /
            (world[5].max - world[5].min) -
          Math.PI / 2;
        var x = (TORUS.R + TORUS.r) * Math.cos(phi);
        var y = (TORUS.R + TORUS.r) * Math.sin(phi);
        var z = 0;
        object3.position.set(x, y, z);
      }

      if (params.N >= 7 && !params.fname.includes("Spinner")) {
        var object3 = TORUS.wristband2.children[i];
        var phi =
          (2 * Math.PI * (world[5].cur - spheres[i][5])) /
            (world[5].max - world[5].min) -
          Math.PI / 2;
        var theta =
          (2 * Math.PI * (world[6].cur - spheres[i][6])) /
          (world[6].max - world[6].min);
        var x = (TORUS.R + TORUS.r * Math.cos(theta)) * Math.cos(phi);
        var y = (TORUS.R + TORUS.r * Math.cos(theta)) * Math.sin(phi);
        var z = TORUS.r * Math.sin(theta);
        object3.position.set(x, y, z);
      }
    }
  }
}

/**
 * In catch_mode, let the user know if they found the pink ball
 */
function check_if_won() {
  if (particles !== undefined) {
    if (particles.children[params.pinky].visible === true) {
      var loc_left = new THREE.Vector3();
      var loc_right = new THREE.Vector3();
      if (
        scene.getObjectByName("vive_left_hand") &&
        scene.getObjectByName("vive_right_hand")
      ) {
        scene.getObjectByName("vive_left_hand").getWorldPosition(loc_left);
        scene.getObjectByName("vive_right_hand").getWorldPosition(loc_right);
        //console.log(controller1.position.distanceTo(particles.children[params.pinky].position));
        //console.log(controller2.position.distanceTo(particles.children[params.pinky].position));
        if (
          loc_left.distanceTo(particles.children[params.pinky].position) <
            particles.children[params.pinky].scale.x ||
          loc_right.distanceTo(particles.children[params.pinky].position) <
            particles.children[params.pinky].scale.x
        ) {
          winning = true;
          bg.material.map = winning_texture;
          bg.rotation.x = 0;
          bg.rotation.y = 0;
          bg.rotation.z = 0;
        }
      }
    }
  }
}

/**
 * Animation loop that runs every frame
 */
function animate() {
  if (params.view_mode === "catch_particle") {
    check_if_won();
  }
  VRController.update();
  if (redraw_left) {
    update_higher_dims_left();
  }
  if (redraw_right) {
    update_higher_dims_right();
  }
  if (params.fname.includes("Uniaxial")) {
    if (params.display_type === "VR") {
      roof.position.y =
        (world[0].max - 5 - time.cur / 10) * params.vr_scale -
        params.human_height;
    } else {
      roof.position.x = world[0].max - 5 - time.cur / 10;
    }
  }
  if (params.N > 3) {
    for (var iii = 3; iii < params.N; iii++) {
      if (world[iii].cur != world[iii].prev) {
        LOADER.load_current_spheres(params, time, true).then((s) => {
          update_spheres(s);
        });
        if (params.view_mode === "rotations2") {
          LOADER.load_current_orientation(params, time, true).then((s) => {
            update_orientation(s);
          });
        }
        world[iii].prev = world[iii].cur;
      }
    }
  }
  var delta = clock.getDelta();
  if (time.play) {
    time.cur += delta * time.play_rate;
  } // current time is in 'seconds'
  time.frame = Math.floor(time.cur * time.frames_per_second);
  if (params.display_type === "VR") {
    // if ( time.play ) { floor.material.emissive = new THREE.Color(0x555555); }
    // else { floor.material.emissive = new THREE.Color(0x333333); }
  }
  //if ( params.display_type === 'VR' ) { bg.rotation.x = time.cur/100.; } // rotate the background over time
  if (time.frame !== time.prev_frame) {
    LOADER.load_current_spheres(params, time, true).then((s) => {
      update_spheres(s);
    });

    if (params.view_mode === "rotations") {
      update_spheres_texturing(time.frame);
    } else if (params.view_mode === "rotations2") {
      LOADER.load_current_orientation(params, time, true).then((s) => {
        update_orientation(s);
      });
    }
    time.prev_frame = time.frame;
  }
  if (time.cur > time.max) {
    time.cur = 0;
  }
  requestAnimationFrame(animate);
  if (CAMERA.controls !== undefined) {
    CAMERA.controls.update();
  }
  renderer.setAnimationLoop(render);
  if ( params.stats ) { stats.update(); }
}

/**
 * Do the actual rendering
 */
function render() {
  if (params.display_type == "anaglyph") {
    CAMERA.effect.render(scene, CAMERA.camera);
  } else {
    renderer.render(scene, CAMERA.camera);
  }
  if (params.record) {
    // console.log(CAMERA.recorder)
    recorder.capture(renderer.domElement);
    if (time.snapshot) {
      setTimeout(() => {
        time.snapshot = false;
        recorder.stop();
        recorder.save();
      }, 30);
    }
  }
}

/**
 * Remove loading screen
 */
function remove_loading_screen() {
  const loadingScreen = document.getElementById("loading-screen");
  loadingScreen.classList.add("fade-out");
  loadingScreen.remove();
}
function onTransitionEnd(event) {
  const element = event.target;
  element.remove();
}

export { update_spheres };
