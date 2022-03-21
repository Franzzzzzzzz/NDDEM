import * as THREE from "three";
import { update_spheres } from "./nddem.js";
var gui;
/**
 * Add the non-VR GUI and set all sliders
 */
function add_gui(params, world, time, recorder, LOADER) {
  if (params.display_type == "anaglyph" || params.display_type == "keyboard") {
    gui = new dat.GUI();
    //gui.add( ref_dim, 'c').min(0).max(params.N-1).step(1).listen().name('Reference dimension').onChange( function( val ) { make_axes(); }) ;
    if (params.N > 3) {
      for (var i = 3; i < params.N; i++) {
        if (params.view_mode === "rotations") {
          gui
            .add(world[i], "cur")
            .min(world[i].min)
            .max(world[i].max)
            .step(0.1)
            .name("x<sub>" + (i + 1) + "</sub>");
        } else {
          gui
            .add(world[i], "cur")
            .min(world[i].min)
            .max(world[i].max)
            .step(0.01)
            .name("x<sub>" + (i + 1) + "</sub>");
        }
      }
    }
    gui
      .add(time, "cur")
      .min(time.min)
      .max(time.max)
      .step(0.1)
      .listen()
      .name("Time");
    gui.add(time, "play_rate").min(0).max(10.0).name("Rate");
    // gui.add( time, 'play').name('Autoplay').onChange( function(flag) { time.play = flag; })
    gui
      .add(time, "play")
      .name("Play")
      .onChange(function (flag) {
        time.play = flag;
        if (flag && params.record) {
          recorder.start();
          console.log("Recording");
        } else if (params.record) {
          recorder.stop();
          recorder.save();
          console.log("saving recording");
        }
      });
    if (params.quasicrystal) {
      gui
        .add(params.euler, "theta_1")
        .name("&theta;<sub>1</sub>")
        .min(0)
        .max(2 * Math.PI)
        .listen()
        .onChange(function () {
          LOADER.load_current_spheres(params, time, true).then((s) => {
            update_spheres(s);
          });
        });
      gui
        .add(params.euler, "theta_2")
        .name("&theta;<sub>2</sub>")
        .min(0)
        .max(2 * Math.PI)
        .listen()
        .onChange(function () {
          LOADER.load_current_spheres(params, time, true).then((s) => {
            update_spheres(s);
          });
        });
      gui
        .add(params.euler, "theta_3")
        .name("&theta;<sub>3</sub>")
        .min(0)
        .max(2 * Math.PI)
        .listen()
        .onChange(function () {
          LOADER.load_current_spheres(params, time, true).then((s) => {
            update_spheres(s);
          });
        });
    }
    if (params.view_mode === "velocity") {
      gui
        .add(params.velocity, "vmax")
        .name("Max vel")
        .min(0)
        .max(2)
        .listen()
        .onChange(function () {
          LOADER.load_current_spheres(params, time, true).then((s) => {
            update_spheres(s);
          });
        });
    }
    if (params.view_mode === "rotation_rate") {
      gui
        .add(params.velocity, "omegamax")
        .name("Max rot vel")
        .min(0)
        .max(10)
        .step(0.01)
        .listen()
        .onChange(function () {
          LOADER.load_current_spheres(params, time, true).then((s) => {
            update_spheres(s);
          });
        });
    }
    if (params.record) {
      gui
        .add(time, "snapshot")
        .name("Snapshot")
        .listen()
        .onChange(function (flag) {
          if (flag) {
            recorder.start();
          }
        });
    }
    gui.open();
  }
}

export { add_gui };
