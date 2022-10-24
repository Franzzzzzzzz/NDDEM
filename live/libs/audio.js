// So how does audio work?
// 1. When particles collide (i.e. they weren't touching before and now they're touching), they ring like bells, with a wavelength related to the elastic wave speed and their size? --- sine wave?
// 2. When particles rub past each other ( i.e. they are in contact and they roll/translate ), they emit a scratching noise whose magnitude is proportional to the normal force being applied? --- sawtooth wave at low frequency?

import {
    AudioListener,
    PositionalAudio,
    AudioLoader,
    Mesh,
    Group,
    SphereGeometry,
    CylinderGeometry,
    MeshStandardMaterial
} from "three";

export let listener;
export let fixed_sound_source = new Group();
export let normal_oscillator, tangential_oscillator;

export function make_listener( target ) {
    // create an AudioListener and add it to the camera
    listener = new AudioListener();
    target.add( listener );

    // normal_oscillator = listener.context.createOscillator();
	// normal_oscillator.type = 'sine';
	// normal_oscillator.frequency.setValueAtTime( 256, listener.context.currentTime );
	// normal_oscillator.start( );

    // tangential_oscillator = listener.context.createOscillator();
	// tangential_oscillator.type = 'sawtooth';
	// tangential_oscillator.frequency.setValueAtTime( 1000, listener.context.currentTime );
	// tangential_oscillator.start( );
    // console.log('made an object the audio listener')
}

export function remove_listener( target ) {
    for ( let i = 0; i < target.children.length; i ++ ) {
        if ( target.children[i].type === "AudioListener" ) {
            target.children[i].remove(); // doesn't work...
        }
    }
}

export function add_fixed_sound_source( loc ) {
    let sound = new PositionalAudio( listener );
    let oscillator = listener.context.createOscillator();
	oscillator.type = 'square';

    oscillator.frequency.setValueAtTime( 700 , listener.context.currentTime);
	oscillator.start( );

    // sound.setNodeSource( normal_oscillator );
    sound.setNodeSource( oscillator );

    sound.gain.gain.value = 0;
    // sound.setRefDistance( 20 );

    fixed_sound_source.add( sound )
    fixed_sound_source.position.set( ...loc );
}

export function add_normal_sound( target ) {
    let sound = new PositionalAudio( listener );
    let oscillator = listener.context.createOscillator();
	oscillator.type = 'square';
    // every particle is unique!
	// oscillator.frequency.setValueAtTime( 100 + 900*Math.random(), listener.context.currentTime );
    oscillator.frequency.setValueAtTime( 700 , listener.context.currentTime);
	oscillator.start( );

    // sound.setNodeSource( normal_oscillator );
    sound.setNodeSource( oscillator );

    sound.gain.gain.value = 0;
    // sound.setRefDistance( 20 );

    target.add( sound )
}

// export function add_tangential_sound( target ) {
//     let sound = new PositionalAudio( listener );
//     sound.setNodeSource( tangential_oscillator );
//     sound.setRefDistance( 20 );
//     sound.setVolume( 0.5 );

//     target.add( sound )
// }

// export function add_impact_sound ( target ) {
//     // create the PositionalAudio object (passing in the listener)
//     const sound = new PositionalAudio( listener );


//     // load a sound and set it as the PositionalAudio object's buffer
//     const audioLoader = new AudioLoader();
//     audioLoader.load( './resources/ping_pong.mp3', function( buffer ) {
//     	sound.setBuffer( buffer );
//     });



//     // finally add the sound to the target
//     target.add( sound );

//     return target;
// }
