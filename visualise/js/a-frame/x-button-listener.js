AFRAME.registerComponent("x-button-listener", {
  init: function () {
    var el = this.el;
    el.addEventListener("gripdown", function (evt) {
      el.setAttribute("visible", !el.getAttribute("visible"));
      console.log("X BUTTON PUSHED");
      var particles = document.querySelector("a-entity[particles]");
      var paused = particles.components.particles.data.paused;
      particles.components.particles.data.paused = 1 - paused;
    });
  },
});
