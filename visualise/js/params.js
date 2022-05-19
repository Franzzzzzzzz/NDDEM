function process_params(time) {
  const urlParams = new URLSearchParams(window.location.search);
  var params = {};

  if (urlParams.has("fname")) {
    params.fname = urlParams.get("fname");
    if (params.fname.substr(-1) != "/") {
      params.fname += "/";
    } // add trailing slash if required
  } else {
    params.fname = "D4/";
  }
  if (urlParams.has("display_type")) {
    params.display_type = urlParams.get("display_type"); // VR, anaglyph or keyboard (default)
  } else {
    params.display_type = "keyboard";
  }
  if (urlParams.has("view_mode")) {
    params.view_mode = urlParams.get("view_mode"); // options are: undefined (normal), catch_particle, rotations, velocity, rotation_rate, D4, D5, size, rotations, rotations2
  } else {
    params.view_mode = "normal";
  }
  if (urlParams.has("autoplay")) {
    time.play = urlParams.get("autoplay") === "true";
  }
  if (urlParams.has("rate")) {
    var rate = urlParams.get("rate");
    time.play_rate = parseFloat(rate);
  }
  if (urlParams.has("shadows")) {
    params.shadows = true;
  } else {
    params.shadows = false;
  }
  if (urlParams.has("quality")) {
    // quality flag - 5 is default, 8 is ridiculous
    params.quality = parseInt(urlParams.get("quality"));
  } else {
    params.quality = 5;
  }
  if (urlParams.has("zoom")) {
    params.zoom = parseFloat(urlParams.get("zoom"));
  } else {
    params.zoom = 20;
  }
  if (urlParams.has("pinky")) {
    params.pinky = parseInt(urlParams.get("pinky"));
  } else {
    params.pinky = 100;
  }
  if (urlParams.has("cache")) {
    params.cache = true;
  } else {
    params.cache = false;
  }
  if (urlParams.has("hard_mode")) {
    // optional flag to not show wristbands if in catch_particle mode
    params.no_tori = true;
  } else {
    params.no_tori = false;
  }
  if (urlParams.has("quasicrystal")) {
    params.quasicrystal = true;
  } else {
    params.quasicrystal = false;
  }
  if (urlParams.has("data_type")) {
    params.data_type = urlParams.get("data_type");
  } else {
    params.data_type = "default";
  }
  if (urlParams.has("colour_scheme")) {
    params.colour_scheme = urlParams.get("colour_scheme");
  } else {
    params.colour_scheme = "dark";
  }
  if (urlParams.has("rotate_torus")) {
    params.rotate_torus = urlParams.get("rotate_torus");
  } else {
    params.rotate_torus = 0;
  }
  if (urlParams.has("initial_camera_location")) {
    params.initial_camera_location = urlParams.get("initial_camera_location");
  }
  if (urlParams.has("camera_target")) {
    params.camera_target = urlParams.get("camera_target");
  }
  if (urlParams.has("record")) {
    params.record = true;
  } else {
    params.record = false;
  }
  if (urlParams.has("t0")) {
    // first timestep to load at
    time.cur = parseFloat(urlParams.get("t0"));
  }
  if (urlParams.has("texture_path")) {
    params.texture_dir = urlParams.get("texture_path");
  } else {
    params.texture_dir = "Textures/";
  }
  if (urlParams.has("no_walls")) {
    params.no_walls = true;
  } else {
    params.no_walls = false;
  }
  if (urlParams.has("no_axes")) {
    params.no_axes = true;
  } else {
    params.no_axes = false;
  }
  if ( urlParams.has("stats")) {
      params.stats = true;
  } else {
      params.stats = false;
  }

  params.euler = { theta_1: 0, theta_2: 0, theta_3: 0 }; // rotations in higher dimensions!!!!!!!!!!
  params.velocity = { vmax: 1, omegamax: 1 }; // default GUI options

  params.vr_scale = 0.1; // mapping from DEM units to VR units
  params.human_height = 0; // height of the human in m

  // params.root_dir = "http://localhost:54321/";
  params.root_dir = window.location.origin + '/';
  params.data_dir = params.root_dir;
  if (window.location.hostname.includes("benjymarks")) {
    params.root_dir = "https://franzzzzzzzz.github.io/NDDEM/"; //window.location.href;
    params.data_dir = "https://www.benjymarks.com/nddem/"; //params.root_dir;
    params.cache = true;
  } else if (window.location.hostname.includes("github")) {
    params.root_dir = "https://franzzzzzzzz.github.io/NDDEM/";
    params.data_dir = "https://www.benjymarks.com/nddem/";
    params.cache = true;
  }

  return params;
}

export { process_params };
