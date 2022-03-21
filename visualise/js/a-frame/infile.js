AFRAME.registerComponent("infile", {
  schema: {
    N: { type: "number", default: -1 },
    world: { type: "array", default: [] },
    tmax: { type: "number", default: 99 },
    pinky: { type: "number", default: 0 },
    fname: { type: "string", default: "D3/" },
  },

  init: function () {
    // Closure to access fresh `this.data` from event handler context.
    // var self = this;
    // this.N = data.
    // .init() is a good place to set up initial state and variables.
    // Store a reference to the handler so we can later remove it.
    // this.eventHandlerFn = function () { console.log(self.data.message); };
  },

  update: function () {
    // var data = this.data;
    // console.log(data);
    // var el = this.el;
    // var sel
    var request = new XMLHttpRequest();
    request.open(
      "GET",
      "http://localhost:8000/Samples/" +
        this.data.fname +
        "in?_=" +
        new Date().getTime(),
      true
    );
    // request.open('GET', "http://localhost:8000/Samples/" + this.data.fname + "in", true);
    request.send(null);
    request.onreadystatechange = function () {
      if (request.readyState === 4 && request.status === 200) {
        var data = document.querySelector("a-entity[infile]").components.infile
          .data;
        var type = request.getResponseHeader("Content-Type");
        if (type.indexOf("text") !== 1) {
          lines = request.responseText.split("\n");
          for (i = 0; i < lines.length; i++) {
            line = lines[i].replace(/ {1,}/g, " "); // remove multiple spaces
            l = line.split(" ");
            if (l[0] == "dimensions") {
              data.N = parseInt(l[1]);
              for (j = 0; j < data.N; j++) {
                data.world.push({});
                data.world[j].min = 0;
                data.world[j].max = 1;
                data.world[j].cur = 0.5;
                data.world[j].prev = 0.5;
                data.world[j].wall = false;
              }
            } else if (l[0] == "boundary") {
              if (l[2] == "WALL" || l[2] == "PBC") {
                data.world[l[1]].min = parseFloat(l[3]);
                data.world[l[1]].max = parseFloat(l[4]);
                data.world[l[1]].cur =
                  (3 * (data.world[l[1]].min + data.world[l[1]].max)) / 4;
                data.world[l[1]].prev = data.world[l[1]].cur;
              }
              if (l[2] == "WALL") {
                data.world[l[1]].wall = true;
              }
            } else if (l[0] == "set") {
              if (l[1] == "T") {
                data.tmax = parseInt(l[2]) - 1;
              }
            } else if (l[0] == "freeze") {
              data.pinky = parseInt(l[1]);
            }
          }
          if (data.N == 1) {
            // just used for setting up cameras etc
            data.world.push({});
            data.world[1].min = 0;
            data.world[1].max = 0;
            data.world[1].cur = 0.5;
            data.world[1].prev = 0.5;
          }
          if (data.N < 3) {
            // just used for setting up cameras etc
            data.world.push({});
            data.world[2].min = 0;
            data.world[2].max = 0;
            data.world[2].cur = 0.5;
            data.world[2].prev = 0.5;
          }
        }
      }
    };
  },
});
