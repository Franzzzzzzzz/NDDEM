// var THREE = require('three');

var container; // main div element
var camera, scene, controls, renderer; // UI elements
var controller1, controller2; // VR controllers
// var raycaster, intersected = []; // catching grains
// var tempMatrix = new THREE.Matrix4(); // catching grains
var particles, wristband1, wristband2, axesHelper, axesLabels; // groups of objects
var R, r; // parameters of torus
var N; // number of dimensions
var world = []; // properties that describe the domain
var ref_dim = { c: 1 }; //, 'x': 00, 'y': 1, 'z': 2}; // reference dimensions
var time = {
  cur: 0,
  frame: 0,
  prev_frame: 0,
  min: 0,
  max: 99,
  play: false,
  play_rate: 5.0,
  save_rate: 1000,
}; // temporal properties
var euler = { theta_1: 0, theta_2: 0, theta_3: 0 }; // rotations in higher dimensions!!!!!!!!!!
if (typeof window.autoplay !== "undefined") {
  time.play = window.autoplay === "true";
}
if (typeof window.rate !== "undefined") {
  time.play_rate = parseFloat(window.rate);
} // DEM time units/second
var axeslength, fontsize; // axis properties
var vr_scale = 0.5; // mapping from DEM units to VR units
var human_height = 0; // height of the human in m
var x_offset = 0; // NOT USED
var y_offset = 0; // NOT USED
// var human_height = 0.; // height of the human in m
var view_mode = window.view_mode; // options are: undefined (normal), catch_particle, rotations, velocity, rotation_rate
var velocity = { vmax: 1, omegamax: 100 }; // default GUI options
var roof; // top boundary
var bg; // background mesh with texture attached
var redraw_left = false; // force redrawing of particles from movement in left hand
var redraw_right = false; // force redrawing of particles from movement in right hand
var left_hand, right_hand; // store parameters for movement in higher dims via hand controls
var winning = false; // did you win the game?
var winning_texture; // texture to hold 'WINNING' sign for catch_particle mode
var clock = new THREE.Clock(); // global clock
if (typeof window.zoom !== "undefined") {
  var zoom = parseFloat(window.zoom);
} else {
  var zoom = 20;
} // default zoom level
if (typeof window.shadows !== "undefined") {
  var shadows = window.shadows == "true";
} else {
  var shadows = true;
}
if (typeof window.quality !== "undefined") {
  var quality = parseInt(window.quality);
} else {
  var quality = 5;
} // quality flag - 5 is default, 8 is ridiculous
if (typeof window.pinky !== "undefined") {
  var pinky = parseInt(window.pinky);
} else {
  var pinky = 100;
} // which particle to catch in catch_particle mode
var fname = window.fname; // which folder to load data from
if (fname.substr(-1) != "/") {
  fname += "/";
} // add trailing slash if required
var lut = new THREE.Lut("blackbody", 512); // options are rainbow, cooltowarm and blackbody
var arrow_material; // material used for arrows to show dimensions
if (typeof window.cache !== "undefined") {
  var cache = window.cache == "true";
} // should we use cached data or not
else {
  var cache = false;
}
if (typeof window.hard_mode !== "undefined") {
  var hard_mode = window.hard_mode == "true";
} // optional flag to not show wristbands if in catch_particle mode
else {
  var hard_mode = false;
}

if (typeof window.quasicrystal !== "undefined") {
  var quasicrystal = window.quasicrystal == "true";
} // optional flag to not show wristbands if in catch_particle mode

var root_dir = "http://localhost:54321/";
if (window.location.hostname.includes("benjymarks")) {
  root_dir = "http://www.benjymarks.com/nddem/";
  cache = true;
}

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

// window.mobileAndTabletcheck = function() {
//   var check = false;
//   (function(a){if(/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows ce|xda|xiino|android|ipad|playbook|silk/i.test(a)||/1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0,4))) check = true;})(navigator.userAgent||navigator.vendor||window.opera);
//   return check;
// };

let promise = new Promise(function (resolve, reject) {
  var request = new XMLHttpRequest();
  if (cache) {
    request.open("GET", root_dir + "Samples/" + fname + "in", true);
  } else {
    request.open(
      "GET",
      root_dir + "Samples/" + fname + "in?_=" + new Date().getTime(),
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
      lines = request.responseText.split("\n");
      for (i = 0; i < lines.length; i++) {
        // console.log(lines[i])
        line = lines[i].replace(/ {1,}/g, " "); // remove multiple spaces
        l = line.split(" ");
        if (l[0] == "dimensions") {
          N = parseInt(l[1]);
          for (j = 0; j < N; j++) {
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
          pinky = parseInt(l[1]);
        }
      }
      if (N == 1) {
        // just used for setting up cameras etc
        world.push({});
        world[1].min = 0;
        world[1].max = 0;
        world[1].cur = 0.5;
        world[1].prev = 0.5;
      }
      if (N < 3) {
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
      resolve("Loaded infile");
      // }
    }
  };
  // }
});

promise.then(
  function (result) {
    build_world();
    remove_everything(); // only runs on postMessage receive
    animate();
    setTimeout(function () {
      update_spheres_CSV(time.frame, true);
    }, 300); // for safety
    setTimeout(function () {
      update_spheres_CSV(time.frame, true);
    }, 1000); // for safety
    setTimeout(function () {
      update_spheres_CSV(time.frame, true);
    }, 2000); // for safety
    setTimeout(function () {
      update_spheres_CSV(time.frame, true);
    }, 5000); // for safety
  },
  function (error) {}
);

function build_world() {
  container = document.createElement("div");
  document.body.appendChild(container);

  N_tag = document.createElement("div");
  N_tag.setAttribute("id", "N_tag");
  N_tag.innerHTML = N + "D";
  container.appendChild(N_tag);

  scene = new THREE.Scene();
  scene.background = new THREE.Color(0x111111); // revealjs background colour
  // scene.background = new THREE.Color( 0xFFFFFF ); // white
  if (display_type === "VR") {
    var geometry = new THREE.SphereBufferGeometry(500, 60, 40);
    // invert the geometry on the x-axis so that all of the faces point inward
    geometry.scale(-1, 1, 1);
    var texture = new THREE.TextureLoader().load(
      root_dir + "visualise/resources/eso0932a.jpg"
    );
    winning_texture = new THREE.TextureLoader().load(
      root_dir + "visualise/resources/winning.png"
    );
    var material = new THREE.MeshBasicMaterial({ map: texture });
    bg = new THREE.Mesh(geometry, material);
    bg.rotation.z = Math.PI / 2;
    scene.add(bg);

    controller1 = new THREE.Object3D();
    controller2 = new THREE.Object3D();
  }

  make_camera();
  make_walls();
  if (!fname.includes("Spinner") && !quasicrystal) {
    make_axes();
  }
  make_lights();
  add_renderer();
  if (!fname.includes("Submarine")) {
    add_controllers();
  }
  // add_controllers();
  if (N > 3 && !fname.includes("Spinner") && !hard_mode) {
    add_torus();
  }
  // load_hyperspheres_VTK();
  if (view_mode === "rotations") {
    make_initial_sphere_texturing();
  } else {
    make_initial_spheres_CSV();
    update_spheres_CSV(0, false);
  }
  //update_spheres_CSV(0,false);
  add_gui();
  window.addEventListener("resize", onWindowResize, false);
  //if ( display_type === 'VR' ) { add_vive_models(); }
}

function update_higher_dims_left() {
  controller1.getWorldQuaternion(left_hand.current_direction);
  left_hand.diff = left_hand.current_direction
    .inverse()
    .multiply(left_hand.previous_direction);
  left_hand.diff_angle.setFromQuaternion(left_hand.diff); // + Math.PI;// between 0 and 2 Pi

  // move in D4 by rotations in z
  if (N > 3) {
    var new_orientation =
      left_hand.previous_torus_rotation_z + left_hand.diff_angle.z;
    if (new_orientation < 0) {
      new_orientation += 2 * Math.PI;
    } else if (new_orientation > 2 * Math.PI) {
      new_orientation -= 2 * Math.PI;
    }
    world[3].cur =
      (new_orientation * (world[3].max - world[3].min)) / Math.PI / 2;
  }
  // move in D5 by rotations in y
  if (N > 4) {
    var new_orientation =
      left_hand.previous_torus_rotation_x + 2 * left_hand.diff_angle.x; // double rotation in reality
    if (new_orientation < 0) {
      new_orientation += 2 * Math.PI;
    } else if (new_orientation > 2 * Math.PI) {
      new_orientation -= 2 * Math.PI;
    }
    world[4].cur =
      (new_orientation * (world[4].max - world[4].min)) / Math.PI / 2;
  }
}

function update_higher_dims_right() {
  controller2.getWorldQuaternion(right_hand.current_direction);
  right_hand.diff = right_hand.current_direction
    .inverse()
    .multiply(right_hand.previous_direction);
  right_hand.diff_angle.setFromQuaternion(right_hand.diff); // + Math.PI;// between 0 and 2 Pi

  // move in D4 by rotations in z
  if (N > 5) {
    var new_orientation =
      right_hand.previous_torus_rotation_z + right_hand.diff_angle.z;
    if (new_orientation < 0) {
      new_orientation += 2 * Math.PI;
    } else if (new_orientation > 2 * Math.PI) {
      new_orientation -= 2 * Math.PI;
    }
    world[5].cur =
      (new_orientation * (world[5].max - world[5].min)) / Math.PI / 2;
  }
  // move in D4 by rotations in y
  if (N > 6) {
    new_orientation =
      right_hand.previous_torus_rotation_x + 2 * right_hand.diff_angle.x;
    if (new_orientation < 0) {
      new_orientation += 2 * Math.PI;
    } else if (new_orientation > 2 * Math.PI) {
      new_orientation -= 2 * Math.PI;
    }
    world[6].cur =
      (new_orientation * (world[6].max - world[6].min)) / Math.PI / 2;
  }
}

// add_left_oculus_model(1);
// add_right_oculus_model(1);

function add_left_oculus_model(controller) {
  new THREE.MTLLoader()
    .setPath(root_dir + "visualise/resources/oculus/")
    .load("oculus-touch-controller-left.mtl", function (materials) {
      materials.preload();
      new THREE.OBJLoader()
        .setMaterials(materials)
        .setPath(root_dir + "visualise/resources/oculus/")
        .load("oculus-touch-controller-left.obj", function (object) {
          object.castShadow = true;
          object.receiveShadow = true;

          // Pause label
          var font_loader = new THREE.FontLoader();
          font_loader.load(
            root_dir +
              "visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json",
            function (font) {
              var fontsize = 0.005;
              var geometry = new THREE.TextBufferGeometry("  Play \nPause", {
                font: font,
                size: fontsize,
                height: fontsize / 5,
              });
              var textMaterial = new THREE.MeshPhongMaterial({
                color: 0xffffff,
              });
              var pause_label = new THREE.Mesh(geometry, textMaterial);
              pause_label.rotation.x = (-3 * Math.PI) / 4;
              pause_label.position.y = fontsize;
              pause_label.position.x = 0.05;
              pause_label.position.z = 0.052;
              object.add(pause_label);

              var geometry = new THREE.CylinderGeometry(
                0.001,
                0.001,
                0.03,
                16,
                16
              );
              var material = new THREE.MeshPhongMaterial({ color: 0xdddddd });
              var pause_line = new THREE.Mesh(geometry, material);
              pause_line.rotation.x = (-3 * Math.PI) / 4;
              pause_line.position.y = fontsize;
              pause_line.position.x = 0.034;
              pause_line.position.z = 0.052;
              pause_line.rotation.z = Math.PI / 2;
              object.add(pause_line);

              var geometry = new THREE.TextBufferGeometry("Menu", {
                font: font,
                size: fontsize,
                height: fontsize / 5,
              });
              var textMaterial = new THREE.MeshPhongMaterial({
                color: 0xffffff,
              });
              var menu_label = new THREE.Mesh(geometry, textMaterial);
              menu_label.rotation.x = (-3 * Math.PI) / 4;
              menu_label.position.y = -0.03;
              menu_label.position.x = 0.007;
              menu_label.position.z = 0.02;
              object.add(menu_label);

              var geometry = new THREE.CylinderGeometry(
                0.001,
                0.001,
                0.025,
                16,
                16
              );
              var material = new THREE.MeshPhongMaterial({ color: 0xdddddd });
              var menu_line = new THREE.Mesh(geometry, material);
              menu_line.rotation.x = (-3 * Math.PI) / 4;
              menu_line.position.y = -0.02;
              menu_line.position.x = 0.017;
              menu_line.position.z = 0.03;
              object.add(menu_line);

              controller.add(object);
              // object.position.y = -3
              // object.position.x = 3
              // object.position.z = -3
              // object.scale.set(20,20,20);
              // object.rotation.z = Math.PI;
              // scene.add( object );

              if (!hard_mode) {
                // Move label
                geometry = new THREE.TextBufferGeometry("Move", {
                  font: font,
                  size: fontsize,
                  height: fontsize / 5,
                });
                var move_label = new THREE.Mesh(geometry, textMaterial);
                move_label.rotation.x = (-1 * Math.PI) / 4;
                move_label.rotation.y = Math.PI;
                move_label.position.y = -0.035 - fontsize;
                move_label.position.x = 0.018;
                move_label.position.z = 0.045;
                if (N > 3) {
                  object.add(move_label);
                }
                // object.add(move_label);
              }
            }
          );
        });
    });
}

function add_right_oculus_model(controller) {
  new THREE.MTLLoader()
    .setPath(root_dir + "visualise/resources/oculus/")
    .load("oculus-touch-controller-right.mtl", function (materials) {
      materials.preload();
      new THREE.OBJLoader()
        .setMaterials(materials)
        .setPath(root_dir + "visualise/resources/oculus/")
        .load("oculus-touch-controller-right.obj", function (object) {
          object.castShadow = true;
          object.receiveShadow = true;

          // Pause label
          var font_loader = new THREE.FontLoader();
          font_loader.load(
            root_dir +
              "visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json",
            function (font) {
              var fontsize = 0.005;
              var geometry = new THREE.TextBufferGeometry("  Play \nPause", {
                font: font,
                size: fontsize,
                height: fontsize / 5,
              });
              var textMaterial = new THREE.MeshPhongMaterial({
                color: 0xffffff,
              });
              var pause_label = new THREE.Mesh(geometry, textMaterial);
              pause_label.rotation.x = (-3 * Math.PI) / 4;
              pause_label.position.y = fontsize;
              pause_label.position.x = 0.03;
              pause_label.position.z = 0.052;
              object.add(pause_label);

              var geometry = new THREE.CylinderGeometry(
                0.001,
                0.001,
                0.04,
                16,
                16
              );
              var material = new THREE.MeshPhongMaterial({ color: 0xdddddd });
              var pause_line = new THREE.Mesh(geometry, material);
              pause_line.rotation.x = (-3 * Math.PI) / 4;
              pause_line.position.y = fontsize;
              pause_line.position.x = 0.01;
              pause_line.position.z = 0.052;
              pause_line.rotation.z = Math.PI / 2;
              object.add(pause_line);

              var geometry = new THREE.TextBufferGeometry("Menu", {
                font: font,
                size: fontsize,
                height: fontsize / 5,
              });
              var textMaterial = new THREE.MeshPhongMaterial({
                color: 0xffffff,
              });
              var menu_label = new THREE.Mesh(geometry, textMaterial);
              menu_label.rotation.x = (-3 * Math.PI) / 4;
              menu_label.position.y = -0.03;
              menu_label.position.x = -0.03;
              menu_label.position.z = 0.02;
              object.add(menu_label);

              var geometry = new THREE.CylinderGeometry(
                0.001,
                0.001,
                0.025,
                16,
                16
              );
              var material = new THREE.MeshPhongMaterial({ color: 0xdddddd });
              var menu_line = new THREE.Mesh(geometry, material);
              menu_line.rotation.x = (-3 * Math.PI) / 4;
              menu_line.position.y = -0.02;
              menu_line.position.x = -0.02;
              menu_line.position.z = 0.03;
              object.add(menu_line);

              controller.add(object);
              // object.position.y = -3
              // object.position.x = 3
              // object.position.z = -3
              // object.scale.set(20,20,20);
              // // object.rotation.z = Math.PI;
              // scene.add( object );

              if (!hard_mode) {
                // Move label
                geometry = new THREE.TextBufferGeometry("Move", {
                  font: font,
                  size: fontsize,
                  height: fontsize / 5,
                });
                var move_label = new THREE.Mesh(geometry, textMaterial);
                move_label.rotation.x = (-1 * Math.PI) / 4;
                move_label.rotation.y = Math.PI;
                move_label.position.y = -0.035 - fontsize;
                move_label.position.x = 0.0;
                move_label.position.z = 0.045;
                if (N > 5) {
                  object.add(move_label);
                }
                // object.add(move_label);
              }
            }
          );
        });
    });
}

function add_vive_models() {
  var loader = new THREE.OBJLoader();
  loader.setPath(root_dir + "visualise/resources/vive/");
  loader.load("vr_controller_vive_1_5.obj", function (object) {
    var loader = new THREE.TextureLoader();
    loader.setPath(root_dir + "visualise/resources/vive/");
    var controller = object.children[0];
    controller.material.map = loader.load("onepointfive_texture.png");
    controller.material.specularMap = loader.load("onepointfive_spec.png");
    controller.castShadow = true;
    controller.receiveShadow = true;

    // Pause label
    var font_loader = new THREE.FontLoader();
    font_loader.load(
      root_dir +
        "visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json",
      function (font) {
        var fontsize = 0.005;
        var geometry = new THREE.TextBufferGeometry("  Play \nPause", {
          font: font,
          size: fontsize,
          height: fontsize / 5,
        });
        var textMaterial = new THREE.MeshPhongMaterial({ color: 0xffffff });
        var pause_label = new THREE.Mesh(geometry, textMaterial);
        pause_label.rotation.x = -Math.PI / 2;
        pause_label.position.y = fontsize;
        pause_label.position.x = -0.01;
        pause_label.position.z = 0.05;
        controller.add(pause_label);

        var geometry = new THREE.TextBufferGeometry("Menu", {
          font: font,
          size: fontsize,
          height: fontsize / 5,
        });
        var textMaterial = new THREE.MeshPhongMaterial({ color: 0xffffff });
        var menu_label = new THREE.Mesh(geometry, textMaterial);
        menu_label.rotation.x = -Math.PI / 2;
        menu_label.position.y = 2 * fontsize;
        menu_label.position.x = -0.008;
        menu_label.position.z = 0.023;
        controller.add(menu_label);

        controller1.add(controller.clone());
        controller2.add(controller.clone());

        if (!hard_mode) {
          // Move label
          geometry = new THREE.TextBufferGeometry("Move", {
            font: font,
            size: fontsize,
            height: fontsize / 5,
          });
          var move_label = new THREE.Mesh(geometry, textMaterial);
          move_label.rotation.x = -Math.PI / 2;
          move_label.rotation.y = Math.PI;
          move_label.position.y = -0.03 - fontsize;
          move_label.position.x = 0.01;
          move_label.position.z = 0.045;
          if (N > 3) {
            controller1.add(move_label);
          }
          if (N > 5) {
            controller2.add(move_label);
          }
        }
      }
    );
  });
}

function make_camera() {
  var aspect = window.innerWidth / window.innerHeight;
  if (N < 3) {
    camera = new THREE.OrthographicCamera(
      (-100 * aspect) / zoom,
      (100 * aspect) / zoom,
      100 / zoom,
      -100 / zoom,
      -1000,
      1000
    );
    camera.position.set(
      (world[0].min + world[0].max) / 2 / 2,
      (world[1].min + world[1].max) / 2,
      -1.0
    );
  } else {
    if (fname.includes("Spinner")) {
      camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000); // fov, aspect, near, far
      camera.position.set(0, 0, N);
    } else if (fname.includes("Lonely") || fname.includes("Drops")) {
      camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000); // fov, aspect, near, far
      camera.position.set(
        (world[0].min + world[0].max) / 2,
        (world[1].min + world[1].max) / 2,
        (-world[0].max / zoom) * 10
      );
    } else {
      if (display_type == "anaglyph") {
        camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000); // fov, aspect, near, far
        camera.position.set(
          (0.5 * world[0].max) / zoom,
          -world[0].max / zoom,
          -world[0].max / zoom
        );
        camera.focalLength = 3;
      } else {
        camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000); // fov, aspect, near, far
        camera.position.set(
          ((0.5 * world[0].max) / zoom) * 5,
          (-world[0].max / zoom) * 5,
          (-world[0].max / zoom) * 5
        );
      }
    }
  }
}
function add_renderer() {
  renderer = new THREE.WebGLRenderer({
    antialias: true,
    preserveDrawingBuffer: true,
  });
  renderer.setPixelRatio(window.devicePixelRatio);
  renderer.setSize(window.innerWidth, window.innerHeight);
  if (shadows) {
    renderer.shadowMap.enabled = true;
  }
  if (display_type == "VR") {
    renderer.vr.enabled = true;
    container.appendChild(WEBVR.createButton(renderer));
  }

  container.appendChild(renderer.domElement);
}

function add_gui() {
  if (display_type == "anaglyph" || display_type == "keyboard") {
    var gui = new dat.GUI();
    //gui.add( ref_dim, 'c').min(0).max(N-1).step(1).listen().name('Reference dimension').onChange( function( val ) { make_axes(); }) ;
    if (N > 3) {
      for (i = 3; i < N; i++) {
        if (view_mode === "rotations") {
          gui
            .add(world[i], "cur")
            .min(world[i].min)
            .max(world[i].max)
            .step(0.1)
            .name("x" + (i + 1));
        } else {
          gui
            .add(world[i], "cur")
            .min(world[i].min)
            .max(world[i].max)
            .step(0.01)
            .name("x" + (i + 1));
        }
      }
    }
    gui
      .add(time, "cur")
      .min(time.min)
      .max(time.max)
      .step(1)
      .listen()
      .name("Time");
    gui.add(time, "play_rate").min(0).max(10.0).name("Rate");
    gui
      .add(time, "play")
      .name("Autoplay")
      .onChange(function (flag) {
        time.play = flag;
        if (flag) recorder.start();
        else {
          recorder.stop();
          recorder.save();
        }
      });
    if (quasicrystal) {
      gui
        .add(euler, "theta_1")
        .name("Theta1")
        .min(0)
        .max(2 * Math.PI)
        .listen()
        .onChange(function () {
          update_spheres_CSV(time.frame, false);
        });
      gui
        .add(euler, "theta_2")
        .name("Theta2")
        .min(0)
        .max(2 * Math.PI)
        .listen()
        .onChange(function () {
          update_spheres_CSV(time.frame, false);
        });
      gui
        .add(euler, "theta_3")
        .name("Theta3")
        .min(0)
        .max(2 * Math.PI)
        .listen()
        .onChange(function () {
          update_spheres_CSV(time.frame, false);
        });
    }
    if (view_mode === "velocity") {
      gui
        .add(velocity, "vmax")
        .name("Max vel")
        .min(0)
        .max(2)
        .listen()
        .onChange(function () {
          update_spheres_CSV(time.frame, false);
        });
    }
    if (view_mode === "rotation_rate") {
      gui
        .add(velocity, "omegamax")
        .name("Max rot vel")
        .min(0)
        .max(100)
        .listen()
        .onChange(function () {
          update_spheres_CSV(time.frame, false);
        });
    }
    gui.open();
  }
  // else {
  //     var gui = dat.GUIVR.create('MuDEM');
  //     dat.GUIVR.enableMouse( camera, renderer );
  //     gui.add( ref_dim, 'c').min(0).max(N-1).step(1).listen().name('Reference dimension') ;
  //     if (N > 3) {
  //         for (i=3;i<N;i++) {
  //             gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).name('X'+i) ;
  //         }
  //     }
  //     gui.add( time, 'cur').min(time.min).max(time.max).step(1).listen().name('Time') ;
  //     gui.add( time, 'rate').min(0).max(1.0).name('Autoplay rate') ;
  //     gui.add( time, 'play').name('Autoplay').onChange( function(flag) { time.play = flag; })
  //
  //     gui.position.set(0,0,0.)
  //     gui.rotation.x = -Math.PI/3.;
  //     gui.scale.set(0.5,0.5,0.5);
  //     controller2.add( gui );
  //     var input1 = dat.GUIVR.addInputObject( controller1, renderer );
  //     //var input2 = dat.GUIVR.addInputObject( controller2 , renderer);
  //     scene.add( input1 );
  //     //scene.add( input2 );
  // }
}

function add_controllers() {
  if (display_type == "VR") {
    window.addEventListener("vr controller connected", function (event) {
      //  Here it is, your VR controller instance.
      //  It’s really a THREE.Object3D so you can just add it to your scene:
      var controller = event.detail;
      //console.log(controller)
      if (controller.gamepad.hand === "left") {
        if (controller.gamepad.id === "Oculus Touch (Left)") {
          add_left_oculus_model(controller);
        }
        controller.name = "vive_left_hand";
        controller.add(controller1);
        left_hand = new THREE.Object3D();
        left_hand.previous_torus_rotation_z = 0;
        left_hand.previous_torus_rotation_y = 0;
        left_hand.new_orientation = 0;
        left_hand.previous_direction = new THREE.Quaternion();
        left_hand.current_direction = new THREE.Quaternion();
        left_hand.diff = new THREE.Quaternion();
        left_hand.diff_angle = new THREE.Euler();
        console.log("Added left hand");
      } else if (controller.gamepad.hand === "right") {
        if (controller.gamepad.id === "Oculus Touch (Right)") {
          add_right_oculus_model(controller);
        }
        controller.add(controller2);
        controller.name = "vive_right_hand";
        right_hand = new THREE.Object3D();
        right_hand.previous_torus_rotation_z = 0;
        right_hand.previous_torus_rotation_y = 0;
        right_hand.new_orientation = 0;
        right_hand.previous_direction = new THREE.Quaternion();
        right_hand.current_direction = new THREE.Quaternion();
        right_hand.diff = new THREE.Quaternion();
        right_hand.diff_angle = new THREE.Euler();
        console.log("Added right hand");
      }
      scene.add(controller);
      controller.standingMatrix = renderer.vr.getStandingMatrix();
      controller.head = window.camera;

      //  Allow this controller to interact with DAT GUI.
      //var guiInputHelper = dat.GUIVR.addInputObject( controller )
      //scene.add( guiInputHelper )
      //  Button events. How easy is this?!
      //  We’ll just use the “primary” button -- whatever that might be ;)
      //  Check out the THREE.VRController.supported{} object to see
      //  all the named buttons we’ve already mapped for you!

      if (!hard_mode) {
        controller.addEventListener("primary press began", function (event) {
          if (controller.gamepad.hand === "left") {
            if (N > 3) {
              wristband1.material.emissive = new THREE.Color(0xe72564);
              redraw_left = true;
              controller1.getWorldQuaternion(left_hand.previous_direction);
              //left_hand.previous_torus_rotation_z = wristband1.rotation.z;
              left_hand.previous_torus_rotation_z =
                (world[3].cur / (world[3].max - world[3].min)) * Math.PI * 2;
            }
            if (N > 4) {
              left_hand.previous_torus_rotation_x =
                (world[4].cur / (world[4].max - world[4].min)) * 2 * Math.PI;
            }
          } else {
            if (N > 5) {
              wristband2.material.emissive = new THREE.Color(0xe72564);
              redraw_right = true;
              controller2.getWorldQuaternion(right_hand.previous_direction);
              right_hand.previous_torus_rotation_z =
                (world[5].cur / (world[5].max - world[5].min)) * Math.PI * 2;
            }
            if (N > 6) {
              right_hand.previous_torus_rotation_x =
                (world[6].cur / (world[6].max - world[6].min)) * 2 * Math.PI;
            }
          }
          //guiInputHelper.pressed( true )
        });
        controller.addEventListener("primary press ended", function (event) {
          if (controller.gamepad.hand === "left") {
            redraw_left = false;
            if (N > 3) {
              wristband1.material.emissive = new THREE.Color(0, 0, 0);
            }
          } else {
            redraw_right = false;
            if (N > 5) {
              wristband2.material.emissive = new THREE.Color(0, 0, 0);
            }
          }
          //guiInputHelper.pressed( false )
        });
      }
      controller.addEventListener("thumbpad press began", function (event) {
        // vive
        time.play = !time.play;
      });
      controller.addEventListener("A press began", function (event) {
        // oculus
        time.play = !time.play;
      });
      controller.addEventListener("X press began", function (event) {
        // oculus
        time.play = !time.play;
      });
      controller.addEventListener("menu press began", function (event) {
        // vive
        window.location.replace(root_dir + "visualise/vr-menu.html");
      });
      controller.addEventListener("B press began", function (event) {
        // oculus
        window.location.replace(root_dir + "visualise/vr-menu.html");
      });
      controller.addEventListener("Y press began", function (event) {
        // oculus
        window.location.replace(root_dir + "visualise/vr-menu.html");
      });
      controller.addEventListener("disconnected", function (event) {
        controller.parent.remove(controller);
      });
    });
    // built in THREEjs
    //controller1 = renderer.vr.getController( 0 ); // JUST HAS ONE BUTTON MAPPED! - SEE WebVRManager
    //controller2 = renderer.vr.getController( 1 );
    // THREEJS example file
    //controller1 = new THREE.ViveController(0);
    //controller2 = new THREE.ViveController(1);
    // Stewdio version from https://github.com/stewdio/THREE.VRController
    //controller1 =
    // var geometry = new THREE.SphereGeometry( 1, Math.pow(2,quality), Math.pow(2,quality) );
    // var material = new THREE.MeshPhongMaterial( { color: 0xdddddd } );
    // var sphere = new THREE.Mesh( geometry, material );
    //controller1.add( sphere );
    //scene.add( controller1 );
    //scene.add( controller2 );

    //if ( view_mode === 'catch_particle' ) {
    //controller1.addEventListener( 'selectstart', onSelectStart ); // left hand
    //controller1.addEventListener( 'selectend', onSelectEnd );
    // controller1.addEventListener( 'gripsdown', leftTorusGripDown );
    // controller1.addEventListener( 'gripsup', leftTorusGripUp );
    // controller2.addEventListener( 'triggerdown', pauseOnTrigger ); // right hand

    //controller2.addEventListener( 'selectend', onSelectEnd );
    //}
    //
    controls = new THREE.TrackballControls(camera, renderer.domElement);
    aim_camera();
    console.log("VR mode loaded");

    // if ( view_mode === 'catch_particle' ) {
    //     var geometry = new THREE.BufferGeometry().setFromPoints( [ new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, 0, - 1 ) ] );
    //
    //     var line = new THREE.Line( geometry );
    //     line.name = 'line';
    //     line.scale.z = 5;
    //
    //     if (display_type == "VR") {
    //         controller1.add( line.clone() );
    //         controller2.add( line.clone() );
    //         };
    //     raycaster = new THREE.Raycaster();
    // }
  } else if (display_type == "keyboard") {
    if (N < 3) {
      controls = new THREE.OrbitControls(camera, renderer.domElement);
      controls.target.set(
        (world[0].min + world[0].max) / 2 / 2,
        (world[1].min + world[1].max) / 2,
        0
      ); // view direction perpendicular to XY-plane. NOTE: VALUE OF 5 IS HARDCODED IN OTHER PLACES
      controls.enableRotate = false;
      camera.up.set(1, 0, 0);
    } else {
      // console.log(window.mobileAndTabletcheck);
      // if ( window.mobileAndTabletcheck ) {
      //     window.addEventListener("deviceorientation", handleOrientation, true);
      // }

      controls = new THREE.TrackballControls(camera, renderer.domElement);
      aim_camera();
    }

    // var gui = new dat.GUI();
    console.log("Keyboard mode loaded");
  } else if (display_type == "anaglyph") {
    controls = new THREE.TrackballControls(camera, renderer.domElement);
    aim_camera();
    effect = new THREE.AnaglyphEffect(renderer);
    effect.setSize(window.innerWidth, window.innerHeight);
    console.log("Anaglyph mode loaded");
  }
}
function aim_camera() {
  if (fname.includes("Lonely") || fname.includes("Drops")) {
    controls.target0.set(
      (world[0].min + world[0].max) / 2,
      (world[1].min + world[1].max) / 2,
      (world[2].min + world[2].max) / 2
    );
  } else if (N > 2) {
    controls.target0.set(
      (world[0].min + world[0].max) / 2 / 2, // NOTE: HARDCODED a FACTOR OF 2 BECAUSE OF CONSOLIDATION
      (world[1].min + world[1].max) / 2,
      (world[2].min + world[2].max) / 2
    );
  }
  if (fname.includes("Spinner")) {
    controls.up0.set(0, 1, 0);
  } // set x as up
  else {
    controls.up0.set(1, 0, 0);
  }
  controls.reset();
}

// function handleOrientation( event ) {
//     alert(event.absolute);
// }

function make_axes() {
  if (typeof axesLabels == "undefined") {
    axeslength = 5; // length of axes vectors
    fontsize = 0.5; // font size
    thickness = 0.1; // line thickness
    axesHelper = new THREE.Group();
    axesLabels = new THREE.Group();
    // axesHelper = new THREE.AxesHelper( axeslength ); // X - red, Y - green, Z - blue
    if (display_type === "VR") {
      axesHelper.position.set(0, -human_height, 0);
      axesLabels.position.set(0, -human_height, 0);
      axesHelper.scale.set(vr_scale, vr_scale, vr_scale);
      axesLabels.scale.set(vr_scale, vr_scale, vr_scale);
      axesLabels.rotation.z = Math.PI / 2;
      axesLabels.rotation.y = Math.PI / 2;
      thickness = 0.05; // line thickness
    }

    scene.add(axesHelper);
    scene.add(axesLabels);

    var arrow_body = new THREE.CylinderGeometry(
      thickness,
      thickness,
      axeslength,
      Math.pow(2, quality),
      Math.pow(2, quality)
    );
    var arrow_head = new THREE.CylinderGeometry(
      0,
      2 * thickness,
      4 * thickness,
      Math.pow(2, quality),
      Math.pow(2, quality)
    );
    arrow_material = new THREE.MeshPhongMaterial({ color: 0xdddddd });
    // var arrow_material_y = new THREE.MeshPhongMaterial( { color: 0x00ff00 } );
    // var arrow_material_z = new THREE.MeshPhongMaterial( { color: 0x0000ff } );
    var arrow_x = new THREE.Mesh(arrow_body, arrow_material);
    var arrow_y = new THREE.Mesh(arrow_body, arrow_material);
    var arrow_z = new THREE.Mesh(arrow_body, arrow_material);
    var arrow_head_x = new THREE.Mesh(arrow_head, arrow_material);
    var arrow_head_y = new THREE.Mesh(arrow_head, arrow_material);
    var arrow_head_z = new THREE.Mesh(arrow_head, arrow_material);

    arrow_x.position.x = axeslength / 2;
    arrow_x.rotation.z = -Math.PI / 2;
    arrow_head_x.position.x = axeslength + thickness;
    arrow_head_x.rotation.z = -Math.PI / 2;

    arrow_y.position.y = axeslength / 2;
    arrow_head_y.position.y = axeslength + thickness;

    arrow_z.position.z = axeslength / 2;
    arrow_z.rotation.x = -Math.PI / 2;
    arrow_head_z.position.z = axeslength + thickness;
    arrow_head_z.rotation.x = Math.PI / 2;

    if (N == 1) {
      arrow_x.position.y = -1.5;
      arrow_head_x.position.y = -1.5;
    }

    axesHelper.add(arrow_x);
    axesHelper.add(arrow_head_x);

    if (N > 1) {
      axesHelper.add(arrow_y);
      axesHelper.add(arrow_head_y);
    }
    if (N > 2) {
      axesHelper.add(arrow_z);
      axesHelper.add(arrow_head_z);
    }
  }

  if (ref_dim.c != ref_dim.x) {
    ref_dim.x = ref_dim.c;
    if (ref_dim.c < N - 1) {
      ref_dim.y = ref_dim.c + 1;
    } else {
      ref_dim.y = ref_dim.c + 1 - N;
    }
    if (ref_dim.c < N - 2) {
      ref_dim.z = ref_dim.c + 2;
    } else {
      ref_dim.z = ref_dim.c + 2 - N;
    }

    if (axesLabels.children.length > 0) {
      for (var i = axesLabels.children.length - 1; i >= 0; i--) {
        obj = axesLabels.children[i];
        axesLabels.remove(obj);
      }
    }
    var loader = new THREE.FontLoader();
    loader.load(
      root_dir +
        "visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json",
      function (font) {
        var textGeo_x = new THREE.TextBufferGeometry("x" + ref_dim.x, {
          font: font,
          size: fontsize,
          height: fontsize / 5,
        });
        var textMaterial_x = new THREE.MeshPhongMaterial({ color: 0xff0000 });
        var mesh_x = new THREE.Mesh(textGeo_x, arrow_material);
        mesh_x.position.x = axeslength; // - 1.5*fontsize;
        mesh_x.position.y = -0.3;
        mesh_x.position.z = fontsize / 4;
        mesh_x.rotation.z = Math.PI;
        mesh_x.rotation.y = Math.PI;
        if (N == 1) {
          mesh_x.position.y = -1.8;
        }
        axesLabels.add(mesh_x);

        if (N > 1) {
          var textGeo_y = new THREE.TextGeometry("x" + ref_dim.y, {
            font: font,
            size: fontsize,
            height: fontsize / 5,
          });
          var textMaterial_y = new THREE.MeshPhongMaterial({ color: 0x00ff00 });
          var mesh_y = new THREE.Mesh(textGeo_y, arrow_material);
          mesh_y.position.x = 0.3;
          mesh_y.position.y = axeslength; //-fontsize*1.5;
          mesh_y.position.z = fontsize / 4;
          mesh_y.rotation.z = -Math.PI / 2;
          mesh_y.rotation.x = Math.PI;
          axesLabels.add(mesh_y);
        }
        if (N > 2) {
          var textGeo_z = new THREE.TextGeometry("x" + ref_dim.z, {
            font: font,
            size: fontsize,
            height: fontsize / 5,
          });
          var textMaterial_z = new THREE.MeshPhongMaterial({ color: 0x0000ff });
          var mesh_z = new THREE.Mesh(textGeo_z, arrow_material);
          mesh_z.position.x = 0.3;
          mesh_z.position.y = fontsize / 4;
          mesh_z.position.z = axeslength + 1.5 * fontsize;
          mesh_z.rotation.z = -Math.PI / 2;
          mesh_z.rotation.x = Math.PI / 2;
          axesLabels.add(mesh_z);
        }
      }
    );
  }
}

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

  if (shadows) {
    light.castShadow = true;
    light.shadow.mapSize.set(4096, 4096);
  }

  scene.add(light);
  scene.add(light1);
}

function make_walls() {
  if (display_type === "VR") {
    var base_plain_geometry = new THREE.PlaneBufferGeometry(1, 1);
    var base_plain_material = new THREE.MeshStandardMaterial({
      color: 0x000000,
    });
    var base_plain = new THREE.Mesh(base_plain_geometry, base_plain_material);
    base_plain.rotation.x = -Math.PI / 2;
    base_plain.scale.set(10, 10, 10);
    base_plain.position.y = -0.1 - human_height;
    scene.add(base_plain);
  }

  var geometry = new THREE.PlaneBufferGeometry(1, 1);
  var material = new THREE.MeshStandardMaterial({
    color: 0xaaaaaa,
    // roughness: 1.0,
    // metalness: 1.0,
  });
  material.transparent = true;
  material.opacity = 0.5;
  // material.side = THREE.DoubleSide;

  // for ( var i=0;i<N;i++ ) {
  // if ( world[i].wall ) {
  // do it nicely
  // }
  // }

  if (world[0].wall) {
    var floor = new THREE.Mesh(geometry, material);
    if (display_type === "VR") {
      floor.scale.set(
        (world[2].max - world[2].min) * vr_scale,
        (world[1].max - world[1].min) * vr_scale,
        1
      );
      floor.rotation.x = +Math.PI / 2;
      floor.position.set(
        ((world[1].max - world[1].min) / 2) * vr_scale,
        world[0].min * vr_scale - human_height,
        ((world[2].max - world[2].min) * vr_scale) / 2
      );
      floor.material.side = THREE.DoubleSide;
    } else {
      floor.scale.set(
        world[2].max - world[2].min,
        world[1].max - world[1].min,
        1
      );
      floor.rotation.y = +Math.PI / 2;
      floor.position.set(
        world[0].min,
        (world[1].max - world[1].min) / 2,
        (world[2].max - world[2].min) / 2
      );
    }
    scene.add(floor);

    roof = new THREE.Mesh(geometry, material);
    if (display_type === "VR") {
      roof.scale.set(
        (world[2].max - world[2].min) * vr_scale,
        (world[1].max - world[1].min) * vr_scale,
        1
      );
      roof.rotation.x = -Math.PI / 2;
      roof.position.set(
        ((world[1].max - world[1].min) / 2) * vr_scale,
        world[0].max * vr_scale - human_height,
        ((world[2].max - world[2].min) / 2) * vr_scale
      );
      roof.material.side = THREE.DoubleSide;
    } else {
      roof.scale.set(
        world[2].max - world[2].min,
        world[1].max - world[1].min,
        1
      );
      roof.rotation.y = -Math.PI / 2;
      roof.position.set(
        world[0].max,
        (world[1].max - world[1].min) / 2,
        (world[2].max - world[2].min) / 2
      );
    }

    if (fname.includes("Uniaxial")) {
      roof.material.side = THREE.DoubleSide;
      roof.material.opacity = 0.9;
      floor.material.side = THREE.DoubleSide;
      floor.material.opacity = 0.9;
    }
    scene.add(roof);
  }

  if (world[1].wall) {
    var left_wall = new THREE.Mesh(geometry, material);
    if (display_type === "VR") {
      left_wall.scale.set(
        (world[2].max - world[2].min) * vr_scale,
        (world[0].max - world[0].min) * vr_scale,
        1
      );
      left_wall.rotation.y = -Math.PI / 2;
      left_wall.position.set(
        world[1].min * vr_scale,
        ((world[0].max - world[0].min) / 2) * vr_scale - human_height,
        ((world[2].max - world[2].min) / 2) * vr_scale
      );
      left_wall.material.side = THREE.DoubleSide;
    } else {
      left_wall.scale.set(
        world[2].max - world[2].min,
        world[0].max - world[0].min,
        1
      );
      left_wall.rotation.x = -Math.PI / 2;
      left_wall.position.set(
        (world[0].max - world[0].min) / 2,
        world[1].min,
        (world[2].max - world[2].min) / 2
      );
    }
    scene.add(left_wall);

    var right_wall = new THREE.Mesh(geometry, material);
    if (display_type === "VR") {
      right_wall.scale.set(
        (world[2].max - world[2].min) * vr_scale,
        (world[0].max - world[0].min) * vr_scale,
        1
      );
      right_wall.rotation.y = Math.PI / 2;
      right_wall.position.set(
        world[1].max * vr_scale,
        ((world[0].max - world[0].min) / 2) * vr_scale - human_height,
        ((world[2].max - world[2].min) / 2) * vr_scale
      );
      right_wall.material.side = THREE.DoubleSide;
    } else {
      right_wall.scale.set(
        world[2].max - world[2].min,
        world[0].max - world[0].min,
        1
      );
      right_wall.rotation.x = Math.PI / 2;
      right_wall.position.set(
        (world[0].max - world[0].min) / 2,
        world[1].max,
        (world[2].max - world[2].min) / 2
      );
    }
    scene.add(right_wall);
  }

  if (N > 2) {
    if (world[2].wall) {
      var front_wall = new THREE.Mesh(geometry, material);
      if (display_type === "VR") {
        front_wall.scale.set(
          (world[1].max - world[1].min) * vr_scale,
          (world[0].max - world[0].min) * vr_scale,
          1
        );
        front_wall.position.set(
          ((world[1].max - world[1].min) / 2) * vr_scale,
          ((world[0].max - world[0].min) / 2) * vr_scale - human_height,
          world[2].min * vr_scale
        );
        front_wall.material.side = THREE.DoubleSide;
      } else {
        front_wall.scale.set(
          world[0].max - world[0].min,
          world[1].max - world[1].min,
          1
        );
        front_wall.position.set(
          (world[0].max - world[0].min) / 2,
          (world[1].max - world[1].min) / 2,
          world[2].min
        );
      }
      scene.add(front_wall);

      var back_wall = new THREE.Mesh(geometry, material);
      if (display_type === "VR") {
        back_wall.scale.set(
          (world[1].max - world[1].min) * vr_scale,
          (world[0].max - world[0].min) * vr_scale,
          1
        );
        back_wall.rotation.x = Math.PI;
        back_wall.position.set(
          ((world[1].max - world[1].min) / 2) * vr_scale,
          ((world[0].max - world[0].min) / 2) * vr_scale - human_height,
          world[2].max * vr_scale
        );
        back_wall.material.side = THREE.DoubleSide;
      } else {
        back_wall.scale.set(
          world[0].max - world[0].min,
          world[1].max - world[1].min,
          1
        );
        back_wall.rotation.y = Math.PI;
        back_wall.position.set(
          (world[0].max - world[0].min) / 2,
          (world[1].max - world[1].min) / 2,
          world[2].max
        );
      }
      scene.add(back_wall);
    }
  }
}

function add_torus() {
  if (display_type == "VR") {
    R = 0.1;
  } else {
    R = 0.5;
  }
  r = R / 2;
  var geometry = new THREE.TorusBufferGeometry(
    R,
    r,
    Math.pow(2, quality + 1) * 2,
    Math.pow(2, quality + 1)
  );
  var material = new THREE.MeshPhongMaterial({
    color: 0xaaaaaa,
    // roughness: 0.7,
    // metalness: 0.5
  });

  wristband1 = new THREE.Mesh(geometry, material);
  if (shadows) {
    wristband1.castShadow = true;
    wristband1.receiveShadow = true;
  }

  var geometry = new THREE.TorusBufferGeometry(
    r + R - r / 6,
    r / 5,
    Math.pow(2, quality + 1) * 2,
    Math.pow(2, quality + 1)
  );
  var material = new THREE.MeshPhongMaterial({
    color: 0x000000,
    // roughness: 0.7,
  });
  wristband1_phi = new THREE.Mesh(geometry, material);

  var geometry = new THREE.TorusBufferGeometry(
    r,
    r / 10,
    Math.pow(2, quality + 1) * 2,
    Math.pow(2, quality + 1)
  );
  wristband1_theta = new THREE.Mesh(geometry, material);
  wristband1_theta.rotation.y = Math.PI / 2;

  if (display_type == "VR") {
    wristband1.position.set(0, 0, 0.2);
    wristband1.rotation.set(0, 0, Math.PI);
    wristband1_phi.position.set(0, 0, 0.2);
    wristband1_theta.position.set(0, R, 0.2);
    controller1.add(wristband1);
    controller1.add(wristband1_phi);
    controller1.add(wristband1_theta);
  } else {
    wristband1.position.set(2.5, -3 * R, 0.5);
    wristband1.rotation.set(0, 0, Math.PI);
    wristband1_phi.position.set(2.5, -3 * R, 0.5);
    wristband1_theta.position.set(2.5, -3 * R + R, 0.5);
    scene.add(wristband1);
    scene.add(wristband1_phi);
    scene.add(wristband1_theta);
  }

  if (N > 5) {
    var geometry = new THREE.TorusBufferGeometry(
      R,
      r,
      Math.pow(2, quality) * 2,
      Math.pow(2, quality)
    );
    var material = new THREE.MeshPhongMaterial({
      color: 0xaaaaaa,
      // roughness: 0.7,
      // metalness: 0.5
    });

    wristband2 = new THREE.Mesh(geometry, material);
    if (shadows) {
      wristband2.castShadow = true;
      wristband2.receiveShadow = true;
    }

    var geometry = new THREE.TorusBufferGeometry(
      r + R - r / 6,
      r / 5,
      Math.pow(2, quality) * 2,
      Math.pow(2, quality)
    );
    var material = new THREE.MeshPhongMaterial({
      color: 0x000000,
      // roughness: 0.7,
    });
    wristband2_phi = new THREE.Mesh(geometry, material);

    var geometry = new THREE.TorusBufferGeometry(
      r,
      r / 10,
      Math.pow(2, quality) * 2,
      Math.pow(2, quality)
    );
    wristband2_theta = new THREE.Mesh(geometry, material);
    wristband2_theta.rotation.y = Math.PI / 2;

    if (display_type == "VR") {
      wristband2.position.set(0, 0, 0.2);
      wristband2.rotation.set(0, 0, Math.PI);
      wristband2_phi.position.set(0, 0, 0.2);
      wristband2_theta.position.set(0, R, 0.2);
      controller2.add(wristband2);
      controller2.add(wristband2_phi);
      controller2.add(wristband2_theta);
    } else {
      wristband1.position.x -= 1.5;
      wristband1_phi.position.x -= 1.5;
      wristband1_theta.position.x -= 1.5;
      wristband2.position.set(4, -3 * R, 0.5);
      wristband2.rotation.set(0, 0, Math.PI);
      wristband2_phi.position.set(4, -3 * R, 0.5);
      wristband2_theta.position.set(4, -3 * R + R, 0.5);
      scene.add(wristband2);
      scene.add(wristband2_phi);
      scene.add(wristband2_theta);
    }
  }
}

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
    for (i = particles.children.length; (i = 0); i--) {
      var object = particles.children[i];
      object.geometry.dispose();
      object.material.dispose();
      if (view_mode === "rotations") {
        object.texture.dispose();
      }
    }
    if (N > 3) {
      for (i = wristband1.children.length; (i = 0); i--) {
        var object = controller1.children[i];
        object.geometry.dispose();
        object.material.dispose();
      }
    }
    if (N > 5) {
      for (i = wristband2.children.length; (i = 0); i--) {
        var object = controller1.children[i];
        object.geometry.dispose();
        object.material.dispose();
      }
    }
    if (display_type === "anaglyph") {
      effect.dispose();
    }
    renderer.vr.enabled = false;
    renderer.dispose();
    scene.dispose();
    console.log(
      "Killed everything. Remaining programs:" + renderer.info.programs.length
    );
  }
} // FG: Removed as I don't think we use that anymore

/*function load_hyperspheres_VTK() {
    var loader = new THREE.VTKLoader();
    loader.load(root_dir + "visualise/data/vtk//dump-00000.vtu", function ( geometry ) {
        console.log(geometry);
    } );
};*/ function make_initial_sphere_texturing() {
  var commandstring = "";
  for (i = 3; i < N; i++) {
    commandstring =
      commandstring + ("x" + (i + 1) + "=" + world[i].cur.toFixed(1));
    if (i < N - 1) commandstring += "&";
  }
  request = new XMLHttpRequest();
  /*request.open('POST', root_dir + "make_textures?" +
                 "arr=" + JSON.stringify(arr) +
                 "&N=" + N +
                 "&t=" + "00000" +
                 "&quality=" + quality +
                 "&fname=" + fname,
                 true);*/
  request.open(
    "GET",
    root_dir +
      "load?ND=" +
      N +
      "&path=" +
      fname +
      "&texturepath=../../Textures&resolution=" +
      quality,
    true
  );
  request.send(null);

  request.onload = function () {
    request.open("GET", root_dir + "render?ts=00000&" + commandstring, true);
    request.send(null);
    request.onload = function () {
      make_initial_spheres_CSV();
      update_spheres_CSV(0, false);
    };
  };
  // Let's do the first rendering as well
  //request.open('GET', root_dir + 'render?ts=00000&' + commandstring, true) ;
  //request.send(null);

  // request.onreadystatechange = function () {}
}

function make_initial_spheres_CSV() {
  if (cache) {
    var filename = root_dir + "Samples/" + fname + "dump-00000.csv";
  } else {
    var filename =
      root_dir +
      "Samples/" +
      fname +
      "dump-00000.csv" +
      "?_=" +
      new Date().getTime();
  }
  Papa.parse(filename, {
    download: true,
    dynamicTyping: true,
    header: true,
    complete: function (results) {
      particles = new THREE.Group();
      scene.add(particles);
      spheres = results.data;
      if (N == 1) {
        var geometry = new THREE.CylinderGeometry(
          1,
          1,
          2,
          Math.pow(2, quality),
          Math.pow(2, quality)
        );
      } else {
        var geometry = new THREE.SphereGeometry(
          1,
          Math.pow(2, quality),
          Math.pow(2, quality)
        );
      }
      var pointsGeometry = new THREE.SphereGeometry(
        1,
        Math.max(Math.pow(2, quality - 2), 4),
        Math.max(Math.pow(2, quality - 2), 4)
      );
      var scale = 20; // size of particles on tori
      for (var i = 0; i < spheres.length; i++) {
        if (N === 2) {
          var color = ((Math.random() + 0.25) / 1.5) * 0xffffff;
          var material = new THREE.PointsMaterial({
            color: color,
          });
        } else {
          if (view_mode === "catch_particle" || fname.includes("Lonely")) {
            if (i == pinky) {
              var color = 0xe72564;
            } else {
              var color = 0xaaaaaa;
            }
            var material = new THREE.MeshPhongMaterial({ color: color });
          } else {
            if (view_mode === "rotations") {
              texture_path = root_dir + "Textures/Texture-" + i + "-00000";
              for (var iiii = 3; iiii < N; iiii++) {
                texture_path += "-0.0";
              }
              var texture = new THREE.TextureLoader().load(
                texture_path + ".png"
              ); //TODO
              var material = new THREE.MeshBasicMaterial({ map: texture });
            } else {
              var color = ((Math.random() + 0.25) / 1.5) * 0xffffff;
              var material = new THREE.MeshPhongMaterial({ color: color });
            }
          }
        }
        var object = new THREE.Mesh(geometry, material);
        object.position.set(spheres[i].x0, spheres[i].x1, spheres[i].x2);
        object.rotation.z = Math.PI / 2;
        if (shadows) {
          object.castShadow = true;
          object.receiveShadow = true;
        }
        particles.add(object);
        if (N > 3 && !fname.includes("Spinner") && !hard_mode) {
          pointsMaterial = new THREE.PointsMaterial({ color: color });
          object2 = new THREE.Mesh(pointsGeometry, pointsMaterial);
          if (fname.includes("Lonely")) {
            object2.scale.set(
              (2 * R) / scale,
              (2 * R) / scale,
              (2 * R) / scale
            );
          } else {
            object2.scale.set(R / scale, R / scale, R / scale);
          }
          object2.position.set(0, 0, 0);
          wristband1.add(object2);
          if (N > 5) {
            object3 = object2.clone();
            wristband2.add(object3);
          }
        }
      }
      if (fname.includes("Submarine")) {
        camera.position.set(
          particles.children[pinky].position.x,
          particles.children[pinky].position.y,
          particles.children[pinky].position.z
        );
        console.log(camera.position);
      }
    },
  });
}

function load_textures(t, Viewpoint) {
  if (particles !== undefined) {
    var loader = new THREE.TextureLoader();
    for (ii = 0; ii < particles.children.length - 1; ii++) {
      if (cache) {
        var filename =
          root_dir + "Textures/" + "Texture-" + ii + "-" + Viewpoint + ".png";
      } else {
        var filename =
          root_dir +
          "Textures/" +
          "Texture-" +
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

function update_spheres_texturing(t) {
  if (true) {
    //TODO Do something better ...
    var commandstring = "";
    var Viewpoint = String(t * time.save_rate).padStart(5, "0");

    for (i = 3; i < N; i++) {
      commandstring =
        commandstring + "&" + ("x" + (i + 1) + "=" + world[i].cur.toFixed(1));
      Viewpoint = Viewpoint + "-" + world[i].cur.toFixed(1);
    }

    var request = new XMLHttpRequest();
    /*request.open('POST',
                       root_dir + "make_textures?" +
                       "arr=" + JSON.stringify(arr) +
                       "&N=" + N +
                       "&t=" + t + "0000" +
                       "&quality=" + quality +
                       "&fname=" + fname,
                       true);*/
    var runvalue = 0;
    if (time.play) runvalue = 1;
    request.open(
      "GET",
      root_dir +
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

function update_spheres_CSV(t, changed_higher_dim_view) {
  if (cache) {
    var filename =
      root_dir +
      "Samples/" +
      fname +
      "dump-" +
      String(t * time.save_rate).padStart(5, "0") +
      ".csv";
  } else {
    var filename =
      root_dir +
      "Samples/" +
      fname +
      "dump-" +
      String(t * time.save_rate).padStart(5, "0") +
      ".csv" +
      "?_=" +
      new Date().getTime();
  }
  Papa.parse(filename, {
    download: true,
    dynamicTyping: true,
    header: true,
    cache: cache,
    complete: function (results) {
      spheres = results.data;
      for (i = 0; i < spheres.length; i++) {
        var object = particles.children[i];
        if (N > 3) {
          x3_unrotated = spheres[i].x3;

          x0_temp =
            spheres[i].x0 * Math.cos(euler.theta_1) -
            spheres[i].x3 * Math.sin(euler.theta_1);
          x3_temp =
            spheres[i].x0 * Math.sin(euler.theta_1) +
            spheres[i].x3 * Math.cos(euler.theta_1);

          x1_temp =
            spheres[i].x1 * Math.cos(euler.theta_2) -
            x3_temp * Math.sin(euler.theta_2);
          x3_temp =
            spheres[i].x1 * Math.sin(euler.theta_2) +
            x3_temp * Math.cos(euler.theta_2);

          x2_temp =
            spheres[i].x2 * Math.cos(euler.theta_3) -
            x3_temp * Math.sin(euler.theta_3);
          x3_temp =
            spheres[i].x2 * Math.sin(euler.theta_3) +
            x3_temp * Math.cos(euler.theta_3);

          spheres[i].x0 = x0_temp;
          spheres[i].x1 = x1_temp;
          spheres[i].x2 = x2_temp;
          spheres[i].x3 = x3_temp;
        }

        if (N == 1) {
          spheres[i].x1 = 0;
        }
        if (N < 3) {
          spheres[i].x2 = 0;
        }
        if (N < 4) {
          var R_draw = spheres[i].R;
        } else if (N == 4) {
          var R_draw = Math.sqrt(
            Math.pow(spheres[i].R, 2) -
              Math.pow(world[3].cur - spheres[i].x3, 2)
          );

          //if ( (world[3].cur >  world[3].max-spheres[i].R ) // NOTE: IMPLEMENT THIS!!
        } else if (N == 5) {
          var R_draw = Math.sqrt(
            Math.pow(spheres[i].R, 2) -
              Math.pow(world[3].cur - spheres[i].x3, 2) -
              Math.pow(world[4].cur - spheres[i].x4, 2)
          );
        } else if (N == 6) {
          var R_draw = Math.sqrt(
            Math.pow(spheres[i].R, 2) -
              Math.pow(world[3].cur - spheres[i].x3, 2) -
              Math.pow(world[4].cur - spheres[i].x4, 2) -
              Math.pow(world[5].cur - spheres[i].x5, 2)
          );
        } else if (N == 7) {
          var R_draw = Math.sqrt(
            Math.pow(spheres[i].R, 2) -
              Math.pow(world[3].cur - spheres[i].x3, 2) -
              Math.pow(world[4].cur - spheres[i].x4, 2) -
              Math.pow(world[5].cur - spheres[i].x5, 2) -
              Math.pow(world[6].cur - spheres[i].x6, 2)
          );
        } else if (N == 8) {
          var R_draw = Math.sqrt(
            Math.pow(spheres[i].R, 2) -
              Math.pow(world[3].cur - spheres[i].x3, 2) -
              Math.pow(world[4].cur - spheres[i].x4, 2) -
              Math.pow(world[5].cur - spheres[i].x5, 2) -
              Math.pow(world[6].cur - spheres[i].x6, 2) -
              Math.pow(world[7].cur - spheres[i].x7, 2)
          );
        } else if (N == 10) {
          var R_draw = Math.sqrt(
            Math.pow(spheres[i].R, 2) -
              Math.pow(world[3].cur - spheres[i].x3, 2) -
              Math.pow(world[4].cur - spheres[i].x4, 2) -
              Math.pow(world[5].cur - spheres[i].x5, 2) -
              Math.pow(world[6].cur - spheres[i].x6, 2) -
              Math.pow(world[7].cur - spheres[i].x7, 2) -
              Math.pow(world[8].cur - spheres[i].x8, 2) -
              Math.pow(world[9].cur - spheres[i].x9, 2)
          );
        } else if (N == 30) {
          var R_draw = Math.sqrt(
            Math.pow(spheres[i].R, 2) -
              Math.pow(world[3].cur - spheres[i].x3, 2) -
              Math.pow(world[4].cur - spheres[i].x4, 2) -
              Math.pow(world[5].cur - spheres[i].x5, 2) -
              Math.pow(world[6].cur - spheres[i].x6, 2) -
              Math.pow(world[7].cur - spheres[i].x7, 2) -
              Math.pow(world[8].cur - spheres[i].x8, 2) -
              Math.pow(world[9].cur - spheres[i].x9, 2) -
              Math.pow(world[10].cur - spheres[i].x10, 2) -
              Math.pow(world[11].cur - spheres[i].x11, 2) -
              Math.pow(world[12].cur - spheres[i].x12, 2) -
              Math.pow(world[13].cur - spheres[i].x13, 2) -
              Math.pow(world[14].cur - spheres[i].x14, 2) -
              Math.pow(world[15].cur - spheres[i].x15, 2) -
              Math.pow(world[16].cur - spheres[i].x16, 2) -
              Math.pow(world[17].cur - spheres[i].x17, 2) -
              Math.pow(world[18].cur - spheres[i].x18, 2) -
              Math.pow(world[19].cur - spheres[i].x19, 2) -
              Math.pow(world[20].cur - spheres[i].x20, 2) -
              Math.pow(world[21].cur - spheres[i].x21, 2) -
              Math.pow(world[22].cur - spheres[i].x22, 2) -
              Math.pow(world[23].cur - spheres[i].x23, 2) -
              Math.pow(world[24].cur - spheres[i].x24, 2) -
              Math.pow(world[25].cur - spheres[i].x25, 2) -
              Math.pow(world[26].cur - spheres[i].x26, 2) -
              Math.pow(world[27].cur - spheres[i].x27, 2) -
              Math.pow(world[28].cur - spheres[i].x28, 2) -
              Math.pow(world[29].cur - spheres[i].x29, 2)
          );
        }
        if (isNaN(R_draw)) {
          object.visible = false;
        } else {
          if (fname.includes("Submarine") && i == pinky) {
            object.visible = false;
          } else {
            if (display_type === "VR") {
              R_draw = R_draw * vr_scale;
              object.position.set(
                spheres[i].x1 * vr_scale,
                spheres[i].x0 * vr_scale - human_height,
                spheres[i].x2 * vr_scale
              );
            } else {
              object.position.set(spheres[i].x0, spheres[i].x1, spheres[i].x2);
            }
            if (quasicrystal) {
              scale = 5;
              object.scale.set(
                spheres[i].R / scale,
                spheres[i].R / scale,
                spheres[i].R / scale
              );
            } else {
              object.scale.set(R_draw, R_draw, R_draw);
            }
            object.visible = true;
            if (view_mode === "velocity") {
              lut.setMin(0);
              lut.setMax(velocity.vmax);
              object.material.color = lut.getColor(spheres[i].Vmag);
            } else if (view_mode === "rotation_rate") {
              lut.setMin(0);
              lut.setMax(velocity.omegamax);
              object.material.color = lut.getColor(spheres[i].Omegamag);
            } else if (view_mode === "D4") {
              lut.setMin(world[3].min);
              lut.setMax(world[3].max);
              object.material.color = lut.getColor(x3_unrotated);
            }
          }
        }
        if (!hard_mode) {
          if (N == 4 && !fname.includes("Spinner")) {
            var object2 = wristband1.children[i];
            phi =
              (2 * Math.PI * (world[3].cur - spheres[i].x3)) /
                (world[3].max - world[3].min) -
              Math.PI / 2;
            x = (R + r) * Math.cos(phi);
            y = (R + r) * Math.sin(phi);
            z = 0;
            object2.position.set(x, y, z);
          }

          if (N > 4 && !fname.includes("Spinner") && !hard_mode) {
            var object2 = wristband1.children[i];
            phi =
              (2 * Math.PI * (world[3].cur - spheres[i].x3)) /
                (world[3].max - world[3].min) -
              Math.PI / 2;
            theta =
              (2 * Math.PI * (world[4].cur - spheres[i].x4)) /
              (world[4].max - world[4].min);
            x = (R + r * Math.cos(theta)) * Math.cos(phi);
            y = (R + r * Math.cos(theta)) * Math.sin(phi);
            z = r * Math.sin(theta);
            object2.position.set(x, y, z);
          }

          if (N == 6 && !fname.includes("Spinner")) {
            var object3 = wristband2.children[i];
            phi =
              (2 * Math.PI * (world[5].cur - spheres[i].x5)) /
                (world[5].max - world[5].min) -
              Math.PI / 2;
            x = (R + r) * Math.cos(phi);
            y = (R + r) * Math.sin(phi);
            z = 0;
            object3.position.set(x, y, z);
          }

          if (N >= 7 && !fname.includes("Spinner")) {
            var object3 = wristband2.children[i];
            phi =
              (2 * Math.PI * (world[5].cur - spheres[i].x5)) /
                (world[5].max - world[5].min) -
              Math.PI / 2;
            theta =
              (2 * Math.PI * (world[6].cur - spheres[i].x6)) /
              (world[6].max - world[6].min);
            x = (R + r * Math.cos(theta)) * Math.cos(phi);
            y = (R + r * Math.cos(theta)) * Math.sin(phi);
            z = r * Math.sin(theta);
            object3.position.set(x, y, z);
          }
        }
      }
    },
  });
}

function onWindowResize() {
  if (N < 3) {
    var aspect = window.innerWidth / window.innerHeight;
    // var zoom = 10.
    camera.left = -zoom * aspect;
    camera.right = zoom * aspect;
    camera.bottom = -zoom;
    camera.top = zoom;
  } else {
    camera.aspect = window.innerWidth / window.innerHeight;
  }

  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
  if (controls !== undefined) {
    controls.handleResize();
  }
  if (display_type == "anaglyph") {
    effect.setSize(window.innerWidth, window.innerHeight);
  }
  render();
}

// function onSelectStart( event ) {
//     var controller = event.target;
//     var intersections = getIntersections( controller );
//     if ( intersections.length > 0 ) {
//         var intersection = intersections[ 0 ];
//         tempMatrix.getInverse( controller.matrixWorld );
//         var object = intersection.object;
//         object.matrix.premultiply( tempMatrix );
//         object.matrix.decompose( object.position, object.quaternion, object.scale );
//         object.material.emissive.b = 1;
//         controller.add( object );
//         controller.userData.selected = object;
//     }
// }

// function onSelectEnd( event ) {
//     var controller = event.target;
//     if ( controller.userData.selected !== undefined ) {
//         var object = controller.userData.selected;
//         object.matrix.premultiply( controller.matrixWorld );
//         object.matrix.decompose( object.position, object.quaternion, object.scale );
//         object.material.emissive.b = 0;
//         particles.add( object );
//         controller.userData.selected = undefined;
//     }
// };
//
// function getIntersections( controller ) {
//
//     tempMatrix.identity().extractRotation( controller.matrixWorld );
//
//     raycaster.ray.origin.setFromMatrixPosition( controller.matrixWorld );
//     raycaster.ray.direction.set( 0, 0, - 1 ).applyMatrix4( tempMatrix );
//
//     if ( particles !== undefined ) { return raycaster.intersectObjects( particles.children ); }
//     else { return []; }
//
// }
//
// function intersectObjects( controller ) {
//
//     // Do not highlight when already selected
//
//     if ( controller.userData.selected !== undefined ) return;
//
//     var line = controller.getObjectByName( 'line' );
//     var intersections = getIntersections( controller );
//
//     if ( intersections.length > 0 ) {
//
//         var intersection = intersections[ 0 ];
//
//         var object = intersection.object;
//         object.material.emissive.r = 1;
//         intersected.push( object );
//
//         line.scale.z = intersection.distance;
//
//     } else {
//
//         line.scale.z = 5;
//
//     }
//
// }
//
// function cleanIntersected() {
//     while ( intersected.length ) {
//         var object = intersected.pop();
//         object.material.emissive.r = 0;
//     }
// };

function check_if_won() {
  if (particles !== undefined) {
    if (particles.children[pinky].visible === true) {
      var loc_left = new THREE.Vector3();
      var loc_right = new THREE.Vector3();
      if (
        scene.getObjectByName("vive_left_hand") &&
        scene.getObjectByName("vive_right_hand")
      ) {
        scene.getObjectByName("vive_left_hand").getWorldPosition(loc_left);
        scene.getObjectByName("vive_right_hand").getWorldPosition(loc_right);
        //console.log(controller1.position.distanceTo(particles.children[pinky].position));
        //console.log(controller2.position.distanceTo(particles.children[pinky].position));
        if (
          loc_left.distanceTo(particles.children[pinky].position) <
            particles.children[pinky].scale.x ||
          loc_right.distanceTo(particles.children[pinky].position) <
            particles.children[pinky].scale.x
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

function animate() {
  if (view_mode === "catch_particle") {
    check_if_won();
  }
  THREE.VRController.update();
  if (redraw_left) {
    update_higher_dims_left();
  }
  if (redraw_right) {
    update_higher_dims_right();
  }
  if (fname.includes("Uniaxial")) {
    if (display_type === "VR") {
      roof.position.y =
        (world[0].max - 5 - time.cur / 10) * vr_scale - human_height;
    } else {
      roof.position.x = world[0].max - 5 - time.cur / 10;
    }
  }
  if (N > 3) {
    for (iii = 3; iii < N; iii++) {
      if (world[iii].cur != world[iii].prev) {
        update_spheres_CSV(time.frame, true);
        if (view_mode === "rotations") {
          update_spheres_texturing(time.frame);
        }
        world[iii].prev = world[iii].cur;
      }
    }
  }
  delta = clock.getDelta();
  if (time.play) {
    time.cur += delta * time.play_rate;
  } // current time is in 'seconds'
  time.frame = Math.floor(time.cur * time.frames_per_second);
  if (display_type === "VR") {
    // if ( time.play ) { floor.material.emissive = new THREE.Color(0x555555); }
    // else { floor.material.emissive = new THREE.Color(0x333333); }
  }
  //if ( display_type === 'VR' ) { bg.rotation.x = time.cur/100.; } // rotate the background over time
  if (time.frame !== time.prev_frame) {
    update_spheres_CSV(time.frame, false);
    if (view_mode === "rotations") {
      update_spheres_texturing(time.frame);
    }
    time.prev_frame = time.frame;
  }
  if (time.cur > time.max) {
    time.cur = 0;
  }
  requestAnimationFrame(animate);
  if (controls !== undefined) {
    controls.update();
  }
  renderer.setAnimationLoop(render);
}

function render() {
  if (display_type == "anaglyph") {
    effect.render(scene, camera);
  } else {
    renderer.render(scene, camera);
  }
  recorder.capture(renderer.domElement);
  //requestAnimationFrame(render);
}
