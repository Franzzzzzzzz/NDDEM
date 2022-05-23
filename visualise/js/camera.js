import * as THREE from "three";
import { TrackballControls } from "three/examples/jsm/controls/TrackballControls.js";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls.js";
import { AnaglyphEffect } from "three/examples/jsm/effects/AnaglyphEffect.js";
// import { VRButton } from "three/examples/jsm/webxr/VRButton.js";
import { VRButton } from 'three/examples/jsm/webxr/VRButton.js';
import { XRControllerModelFactory } from 'three/examples/jsm/webxr/XRControllerModelFactory.js';
import { OculusHandModel } from 'three/examples/jsm/webxr/OculusHandModel.js';

var camera, controls, effect;
/**
 * Make the camera and position it
 */
function make_camera(scene, params, world) {
  var aspect = window.innerWidth / window.innerHeight;
  if (params.N < 3) {
    camera = new THREE.OrthographicCamera(
      (-100 * aspect) / params.zoom,
      (100 * aspect) / params.zoom,
      100 / params.zoom,
      -100 / params.zoom,
      -1000,
      1000
    );
    camera.position.set(
      (world[0].min + world[0].max) / 2 / 2,
      (world[1].min + world[1].max) / 2,
      -1.0
    );
  } else {
    if (params.fname.includes("Spinner")) {
      camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000); // fov, aspect, near, far
      camera.position.set(0, 0, params.N);
    } else if (
      params.fname.includes("Lonely") ||
      params.fname.includes("Drops")
    ) {
      camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000); // fov, aspect, near, far
      camera.position.set(
        (world[0].min + world[0].max) / 2,
        (world[1].min + world[1].max) / 2,
        (-world[0].max / params.zoom) * 10
      );
    } else {
      if (params.display_type == "anaglyph") {
        camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000); // fov, aspect, near, far
        camera.position.set(
          (0.5 * world[0].max) / params.zoom,
          -world[0].max / params.zoom,
          -world[0].max / params.zoom
        );
        camera.focalLength = 3;
      } else {
        camera = new THREE.PerspectiveCamera(70, aspect, 0.0001, 1000); // fov, aspect, near, far
        camera.position.set(
          ((0.5 * world[0].max) / params.zoom) * 5,
          (-world[0].max / params.zoom) * 5,
          (-world[0].max / params.zoom) * 5
        );
      }
    }
  }
  if (typeof params.initial_camera_location !== "undefined") {
    var pos = params.initial_camera_location.split(",");
    camera.position.set(
      parseFloat(pos[0]),
      parseFloat(pos[1]),
      parseFloat(pos[2])
    );
    console.log("Set new camera position:");
  }
}

/**
 * If the current example requires a specific direction for the camera to face at all times (e.g. a 1D or 2D simulation) then set that
 */
function aim_camera(params, world) {
  if (typeof params.camera_target !== "undefined") {
    var pos = params.camera_target.split(",");
    controls.target0.set(
      parseFloat(pos[0]),
      parseFloat(pos[1]),
      parseFloat(pos[2])
    );
    console.log("Set default target");
  } else {
    if (params.fname.includes("Lonely") || params.fname.includes("Drops")) {
      controls.target0.set(
        (world[0].min + world[0].max) / 2,
        (world[1].min + world[1].max) / 2,
        (world[2].min + world[2].max) / 2
      );
    } else if (params.N > 2) {
      controls.target0.set(
        (world[0].min + world[0].max) / 2 / 2, // NOTE: HARDCODED a FACTOR OF 2 BECAUSE OF CONSOLIDATION
        (world[1].min + world[1].max) / 2,
        (world[2].min + world[2].max) / 2
      );
    }
  }
  if (params.fname.includes("Spinner")) {
    controls.up0.set(0, 1, 0);
  } // set x as up
  else {
    controls.up0.set(1, 0, 0);
  }
  controls.reset();
}

/**
 * Add the renderer and associated VR warnings if necessary
 */
function add_renderer(params, container) {
  var renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setPixelRatio(window.devicePixelRatio);
  renderer.setSize(window.innerWidth, window.innerHeight);
  if (params.shadows) {
    renderer.shadowMap.enabled = true;
  }
  if (params.display_type == "VR") {
    container.appendChild(VRButton.createButton(renderer));
    renderer.xr.enabled = true;
  }

  container.appendChild(renderer.domElement);
  return renderer;
}

/**
 * Update camera and renderer if window size changes
 */
function on_window_resize(params, scene, renderer) {
  if (params.N < 3) {
    var aspect = window.innerWidth / window.innerHeight;
    // var zoom = 10.
    camera.left = -params.zoom * aspect;
    camera.right = params.zoom * aspect;
    camera.bottom = -params.zoom;
    camera.top = params.zoom;
  } else {
    camera.aspect = window.innerWidth / window.innerHeight;
  }

  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
  if (controls !== undefined) {
    controls.handleResize();
  }
  if (params.display_type == "anaglyph") {
    effect.setSize(window.innerWidth, window.innerHeight);
  }
}

/**
 * Add the non-VR and/or VR controllers and associated buttons
 */
function add_controllers(scene, params, world, renderer) {
  if (params.display_type == "VR") {
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
      controller.head = camera;

      //  Allow this controller to interact with DAT GUI.
      //var guiInputHelper = dat.GUIVR.addInputObject( controller )
      //scene.add( guiInputHelper )
      //  Button events. How easy is this?!
      //  We’ll just use the “primary” button -- whatever that might be ;)
      //  Check out the THREE.VRController.supported{} object to see
      //  all the named buttons we’ve already mapped for you!

      if (!params.no_tori) {
        controller.addEventListener("primary press began", function (event) {
          if (controller.gamepad.hand === "left") {
            if (params.N > 3) {
              TORUS.wristband1.material.emissive = new THREE.Color(0xe72564);
              redraw_left = true;
              controller1.getWorldQuaternion(left_hand.previous_direction);
              //left_hand.previous_torus_rotation_z = TORUS.wristband1.rotation.z;
              left_hand.previous_torus_rotation_z =
                (world[3].cur / (world[3].max - world[3].min)) * Math.PI * 2;
            }
            if (params.N > 4) {
              left_hand.previous_torus_rotation_x =
                (world[4].cur / (world[4].max - world[4].min)) * 2 * Math.PI;
            }
          } else {
            if (params.N > 5) {
              TORUS.wristband2.material.emissive = new THREE.Color(0xe72564);
              redraw_right = true;
              controller2.getWorldQuaternion(right_hand.previous_direction);
              right_hand.previous_torus_rotation_z =
                (world[5].cur / (world[5].max - world[5].min)) * Math.PI * 2;
            }
            if (params.N > 6) {
              right_hand.previous_torus_rotation_x =
                (world[6].cur / (world[6].max - world[6].min)) * 2 * Math.PI;
            }
          }
          //guiInputHelper.pressed( true )
        });
        controller.addEventListener("primary press ended", function (event) {
          if (controller.gamepad.hand === "left") {
            redraw_left = false;
            if (params.N > 3) {
              TORUS.wristband1.material.emissive = new THREE.Color(0, 0, 0);
            }
          } else {
            redraw_right = false;
            if (params.N > 5) {
              TORUS.wristband2.material.emissive = new THREE.Color(0, 0, 0);
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
        window.location.replace(params.root_dir + "visualise/vr-menu.html");
      });
      controller.addEventListener("B press began", function (event) {
        // oculus
        window.location.replace(params.root_dir + "visualise/vr-menu.html");
      });
      controller.addEventListener("Y press began", function (event) {
        // oculus
        window.location.replace(params.root_dir + "visualise/vr-menu.html");
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
    // var geometry = new THREE.SphereGeometry( 1, Math.pow(2,params.quality), Math.pow(2,params.quality) );
    // var material = new THREE.MeshPhongMaterial( { color: 0xdddddd } );
    // var sphere = new THREE.Mesh( geometry, material );
    //controller1.add( sphere );
    //scene.add( controller1 );
    //scene.add( controller2 );

    //if ( params.view_mode === 'catch_particle' ) {
    //controller1.addEventListener( 'selectstart', onSelectStart ); // left hand
    //controller1.addEventListener( 'selectend', onSelectEnd );
    // controller1.addEventListener( 'gripsdown', leftTorusGripDown );
    // controller1.addEventListener( 'gripsup', leftTorusGripUp );
    // controller2.addEventListener( 'triggerdown', pauseOnTrigger ); // right hand

    //controller2.addEventListener( 'selectend', onSelectEnd );
    //}
    //
    controls = new TrackballControls(camera, renderer.domElement);
    aim_camera(params, world);
    console.log("VR mode loaded");

    // if ( params.view_mode === 'catch_particle' ) {
    //     var geometry = new THREE.BufferGeometry().setFromPoints( [ new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, 0, - 1 ) ] );
    //
    //     var line = new THREE.Line( geometry );
    //     line.name = 'line';
    //     line.scale.z = 5;
    //
    //     if (params.display_type == "VR") {
    //         controller1.add( line.clone() );
    //         controller2.add( line.clone() );
    //         };
    //     raycaster = new THREE.Raycaster();
    // }
  } else if (params.display_type == "keyboard") {
    if (params.N < 3) {
      controls = new OrbitControls(camera, renderer.domElement);
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
      controls = new TrackballControls(camera, renderer.domElement);
      aim_camera(params, world);
    }

    // var gui = new dat.GUI();
    console.log("Keyboard mode loaded");
  } else if (params.display_type == "anaglyph") {
    controls = new TrackballControls(camera, renderer.domElement);
    aim_camera(params, world);
    effect = new AnaglyphEffect(renderer);
    effect.setSize(window.innerWidth, window.innerHeight);
    console.log("Anaglyph mode loaded");
  }
}

export {
  make_camera,
  add_renderer,
  on_window_resize,
  camera,
  controls,
  add_controllers,
  effect,
};
