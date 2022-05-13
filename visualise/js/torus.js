import * as THREE from "three";

import { MTLLoader } from "three/examples/jsm/loaders/MTLLoader.js";
import { OBJLoader } from "three/examples/jsm/loaders/OBJLoader.js";
import { TextGeometry } from "three/examples/jsm/geometries/TextGeometry.js";

var R, r; // parameters of torus
var wristband1, wristband2;
var wristband1_phi, wristband1_theta;
var wristband2_phi, wristband2_theta;
var controller1, controller2;
controller1 = new THREE.Object3D();
controller2 = new THREE.Object3D();

/**
 * Add the torus(es) as necessary
 */
function add_torus(scene, params, world, particles) {
  if (params.display_type == "VR") {
    R = 0.1;
  } else {
    R = 0.5;
  }
  r = R / 2;
  if (params.colour_scheme === "inverted") {
    var torus_colour = 0x111111;
    var wristband_colour = 0xeeeeee;
  } else {
    var torus_colour = 0xaaaaaa;
    var wristband_colour = 0x000000;
  }
  var geometry = new THREE.TorusBufferGeometry(
    R,
    r,
    Math.pow(2, params.quality + 1) * 2,
    Math.pow(2, params.quality + 1)
  );
  var material = new THREE.MeshPhongMaterial({
    color: torus_colour,
    // roughness: 0.7,
    // metalness: 0.5
  });

  wristband1 = new THREE.Mesh(geometry, material);
  if (params.shadows) {
    wristband1.castShadow = true;
    wristband1.receiveShadow = true;
  }

  var geometry = new THREE.TorusBufferGeometry(
    r + R - r / 6,
    r / 5,
    Math.pow(2, params.quality + 1) * 2,
    Math.pow(2, params.quality + 1)
  );
  var material = new THREE.MeshPhongMaterial({
    color: wristband_colour,
    // roughness: 0.7,
  });
  wristband1_phi = new THREE.Mesh(geometry, material);

  var geometry = new THREE.TorusBufferGeometry(
    r,
    r / 10,
    Math.pow(2, params.quality + 1) * 2,
    Math.pow(2, params.quality + 1)
  );
  wristband1_theta = new THREE.Mesh(geometry, material);
  wristband1_theta.rotation.y = Math.PI / 2;

  if (params.display_type == "VR") {
    wristband1.position.set(0, 0, 0.2);
    wristband1.rotation.set(0, 0, Math.PI);
    wristband1_phi.position.set(0, 0, 0.2);
    wristband1_theta.position.set(0, R, 0.2);
    controller1.add(wristband1);
    controller1.add(wristband1_phi);
    controller1.add(wristband1_theta);
  } else {
    controller1 = new THREE.Object3D();
    scene.add(controller1);

    wristband1.rotation.set(0, 0, Math.PI);
    wristband1_theta.position.set(0, R, 0);

    controller1.add(wristband1);
    controller1.add(wristband1_phi);
    controller1.add(wristband1_theta);
    controller1.position.set(2.5, -3 * R, 0.5);
  }

  controller1.rotation.x += (params.rotate_torus / 180) * Math.PI;

  if (params.N > 5) {
    var geometry = new THREE.TorusBufferGeometry(
      R,
      r,
      Math.pow(2, params.quality) * 2,
      Math.pow(2, params.quality)
    );
    var material = new THREE.MeshPhongMaterial({
      color: 0xaaaaaa,
      // roughness: 0.7,
      // metalness: 0.5
    });

    wristband2 = new THREE.Mesh(geometry, material);
    if (params.shadows) {
      wristband2.castShadow = true;
      wristband2.receiveShadow = true;
    }

    var geometry = new THREE.TorusBufferGeometry(
      r + R - r / 6,
      r / 5,
      Math.pow(2, params.quality) * 2,
      Math.pow(2, params.quality)
    );
    var material = new THREE.MeshPhongMaterial({
      color: 0x000000,
      // roughness: 0.7,
    });
    wristband2_phi = new THREE.Mesh(geometry, material);

    var geometry = new THREE.TorusBufferGeometry(
      r,
      r / 10,
      Math.pow(2, params.quality) * 2,
      Math.pow(2, params.quality)
    );
    wristband2_theta = new THREE.Mesh(geometry, material);
    wristband2_theta.rotation.y = Math.PI / 2;

    if (params.display_type == "VR") {
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

/**
 * Get the current orientation of the left hand controller and set world coordinates appropriately
 */
function update_higher_dims_left() {
  controller1.getWorldQuaternion(left_hand.current_direction);
  left_hand.diff = left_hand.current_direction
    .inverse()
    .multiply(left_hand.previous_direction);
  left_hand.diff_angle.setFromQuaternion(left_hand.diff); // + Math.PI;// between 0 and 2 Pi

  // move in D4 by rotations in z
  if (params.N > 3) {
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
  if (params.N > 4) {
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

/**
 * Get the current orientation of the right hand controller and set world coordinates appropriately
 */
function update_higher_dims_right() {
  controller2.getWorldQuaternion(right_hand.current_direction);
  right_hand.diff = right_hand.current_direction
    .inverse()
    .multiply(right_hand.previous_direction);
  right_hand.diff_angle.setFromQuaternion(right_hand.diff); // + Math.PI;// between 0 and 2 Pi

  // move in D4 by rotations in z
  if (params.N > 5) {
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
  if (params.N > 6) {
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

/**
 * Add the left oculus controller
 * @param {number} controller controller number (0 or 1)
 */
function add_left_oculus_model(controller) {
  new MTLLoader()
    .setPath(params.root_dir + "visualise/resources/oculus/")
    .load("oculus-touch-controller-left.mtl", function (materials) {
      materials.preload();
      new OBJLoader()
        .setMaterials(materials)
        .setPath(params.root_dir + "visualise/resources/oculus/")
        .load("oculus-touch-controller-left.obj", function (object) {
          object.castShadow = true;
          object.receiveShadow = true;

          // Pause label
          var font_loader = new THREE.FontLoader();
          font_loader.load(
            params.root_dir +
              "visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json",
            function (font) {
              var fontsize = 0.005;
              var geometry = new TextGeometry("  Play \nPause", {
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

              var geometry = new TextGeometry("Menu", {
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

              if (!params.no_tori) {
                // Move label
                geometry = new TextGeometry("Move", {
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
                if (params.N > 3) {
                  object.add(move_label);
                }
                // object.add(move_label);
              }
            }
          );
        });
    });
}

/**
 * Add the right oculus controller
 * @param {number} controller controller number (0 or 1)
 */
function add_right_oculus_model(controller) {
  new MTLLoader()
    .setPath(params.root_dir + "visualise/resources/oculus/")
    .load("oculus-touch-controller-right.mtl", function (materials) {
      materials.preload();
      new OBJLoader()
        .setMaterials(materials)
        .setPath(params.root_dir + "visualise/resources/oculus/")
        .load("oculus-touch-controller-right.obj", function (object) {
          object.castShadow = true;
          object.receiveShadow = true;

          // Pause label
          var font_loader = new THREE.FontLoader();
          font_loader.load(
            params.root_dir +
              "visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json",
            function (font) {
              var fontsize = 0.005;
              var geometry = new TextGeometry("  Play \nPause", {
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

              var geometry = new TextGeometry("Menu", {
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

              if (!params.no_tori) {
                // Move label
                geometry = new TextGeometry("Move", {
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
                if (params.N > 5) {
                  object.add(move_label);
                }
                // object.add(move_label);
              }
            }
          );
        });
    });
}
/**
 * Add the two vive controllers
 */
function add_vive_models(scene, params, world) {
  var loader = new OBJLoader();
  loader.setPath(params.root_dir + "visualise/resources/vive/");
  loader.load("vr_controller_vive_1_5.obj", function (object) {
    var loader = new THREE.TextureLoader();
    loader.setPath(params.root_dir + "visualise/resources/vive/");
    var controller = object.children[0];
    controller.material.map = loader.load("onepointfive_texture.png");
    controller.material.specularMap = loader.load("onepointfive_spec.png");
    controller.castShadow = true;
    controller.receiveShadow = true;

    // Pause label
    var font_loader = new THREE.FontLoader();
    font_loader.load(
      params.root_dir +
        "visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json",
      function (font) {
        var fontsize = 0.005;
        var geometry = new TextGeometry("  Play \nPause", {
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

        var geometry = new TextGeometry("Menu", {
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

        if (!params.no_tori) {
          // Move label
          geometry = new TextGeometry("Move", {
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
          if (params.N > 3) {
            controller1.add(move_label);
          }
          if (params.N > 5) {
            controller2.add(move_label);
          }
        }
      }
    );
  });
}

function delete_everything(params) {
  if (params.N > 3) {
    for (var i = wristband1.children.length; (i = 0); i--) {
      var object = controller1.children[i];
      object.geometry.dispose();
      object.material.dispose();
    }
  }
  if (params.N > 5) {
    for (var i = wristband2.children.length; (i = 0); i--) {
      var object = controller1.children[i];
      object.geometry.dispose();
      object.material.dispose();
    }
  }
}

export {
  wristband1,
  wristband2,
  R,
  r,
  add_torus,
  add_vive_models,
  add_right_oculus_model,
  add_left_oculus_model,
  delete_everything,
};
