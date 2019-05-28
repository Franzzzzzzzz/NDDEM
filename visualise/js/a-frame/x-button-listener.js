AFRAME.registerComponent('x-button-listener', {
  init: function () {
    var el = this.el;
    el.addEventListener('xbuttondown', function (evt) {
      el.setAttribute('visible', !el.getAttribute('visible'));
      console.log('X BUTTON PUSHED');
      var particles = document.querySelector("a-entity[particles]")
      var paused = particles.getAttribute('paused');
      particles.setAttribute('paused', 1 - paused);

    });
  }
});
