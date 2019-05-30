AFRAME.registerComponent('torus', {
    schema: {
        hand: {type: 'string', default: ''},
    },

    init: function() {
        console.log('Adding torus to ' + this.data.hand + ' hand');
    },

    update: function() {
        // if (window.display_type == "VR") { R = 0.1; }
        // else { R = 0.5; }
        var R = 0.1;
        var r = R/2.;
        var N = document.querySelector("#particles").components.particles.data.N;
        var quality = document.querySelector("#particles").components.particles.data.quality;
        var shadows = document.querySelector("#particles").components.particles.data.shadows;
        var geometry = new THREE.TorusBufferGeometry( R, r, Math.pow(2,quality)*2, Math.pow(2,quality) );
        var material = new THREE.MeshPhongMaterial( {
            color: 0xffffff,
            // roughness: 0.7,
            // metalness: 0.5
        } );

        wristband = new THREE.Mesh( geometry, material );
        if ( shadows ) {
            wristband.castShadow = true;
            wristband.receiveShadow = true;
        }

        var geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., Math.pow(2,quality)*2, Math.pow(2,quality) );
        var material = new THREE.MeshPhongMaterial( {
            color: 0x000000,
            // roughness: 0.7,
        } );
        wristband_phi = new THREE.Mesh( geometry, material );

        var geometry = new THREE.TorusBufferGeometry( r, r/10., Math.pow(2,quality)*2, Math.pow(2,quality) );
        wristband_theta = new THREE.Mesh( geometry, material );
        wristband_theta.rotation.y = Math.PI/2;

        if ( N > 3 ) {
        // if (window.display_type == "VR") {
            wristband.position.set(0.,0.,0.);
            wristband_phi.position.set(0.,0.,0.);
            wristband_theta.position.set(0.,R,0.);
            // controller1.add( wristband );
            // controller1.add( wristband_phi );
            // controller1.add( wristband_theta );
            el.setObject3D('wristband_left', wristband)
            el.setObject3D('wristband_phi_left', wristband_phi)
            el.setObject3D('wristband_theta_left', wristband_theta)
        // }
        // else {
            // wristband.position.set(      2.5,-3*R,  0.5);
            // wristband_phi.position.set(  2.5,-3*R,  0.5);
            // wristband_theta.position.set(2.5,-3*R+R,0.5);
            // scene.add( wristband );
            // scene.add( wristband_phi );
            // scene.add( wristband_theta );
        // }
        }
        if ( N > 5 ) {
            var geometry = new THREE.TorusBufferGeometry( R, r, Math.pow(2,quality)*2, Math.pow(2,quality) );
            var material = new THREE.MeshPhongMaterial( {
                color: 0xffffff,
                // roughness: 0.7,
                // metalness: 0.5
            } );

            wristband1 = new THREE.Mesh( geometry, material );
            if ( shadows ) {
                wristband1.castShadow = true;
                wristband1.receiveShadow = true;
            }

            var geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., Math.pow(2,quality)*2, Math.pow(2,quality) );
            var material = new THREE.MeshPhongMaterial( {
                color: 0x000000,
                // roughness: 0.7,
            } );
            wristband1_phi = new THREE.Mesh( geometry, material );

            var geometry = new THREE.TorusBufferGeometry( r, r/10., Math.pow(2,quality)*2, Math.pow(2,quality) );
            wristband1_theta = new THREE.Mesh( geometry, material );
            wristband1_theta.rotation.y = Math.PI/2;


            // if (window.display_type == "VR") {
                wristband1.position.set(0.,0.,0.);
                wristband1_phi.position.set(0.,0.,0.);
                wristband1_theta.position.set(0.,R,0.);
                // controller2.add( wristband1 );
                // controller2.add( wristband1_phi );
                // controller2.add( wristband1_theta );
                el.setObject3D('wristband_right', wristband1)
                el.setObject3D('wristband_phi_right', wristband1_phi)
                el.setObject3D('wristband_theta_right', wristband1_theta)
            // }
            // else {
            //     wristband.position.x -= 1.5
            //     wristband_phi.position.x -= 1.5
            //     wristband_theta.position.x -= 1.5
            //     wristband1.position.set(      4,-3*R,  0.5);
            //     wristband1_phi.position.set(  4,-3*R,  0.5);
            //     wristband1_theta.position.set(4,-3*R+R,0.5);
            //     scene.add( wristband1 );
            //     scene.add( wristband1_phi );
            //     scene.add( wristband1_theta );
            // }
        }
    }
});
