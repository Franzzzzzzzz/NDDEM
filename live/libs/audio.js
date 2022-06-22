// So how does audio work?
// 1. When particles collide (i.e. they weren't touching before and now they're touching), they ring like bells, with a wavelength related to the elastic wave speed and their size? --- sine wave?
// 2. When particles rub past each other ( i.e. they are in contact and they roll/translate ), they emit a scratching noise whose magnitude is proportional to the normal force being applied? --- sawtooth wave at low frequency?

import {
    AudioListener,
    PositionalAudio,
    AudioLoader,
    Mesh,
    SphereGeometry,
    CylinderGeometry,
    MeshStandardMaterial
} from "three";

let listener;
export let normal_oscillator, tangential_oscillator;

export function make_listener( target ) {
    // create an AudioListener and add it to the camera
    listener = new AudioListener();
    target.add( listener );

    normal_oscillator = listener.context.createOscillator();
	normal_oscillator.type = 'sine';
	normal_oscillator.frequency.setValueAtTime( 10000, listener.context.currentTime );
	normal_oscillator.start( );

    tangential_oscillator = listener.context.createOscillator();
	tangential_oscillator.type = 'sawtooth';
	tangential_oscillator.frequency.setValueAtTime( 1000, listener.context.currentTime );
	tangential_oscillator.start( );
    // console.log('made an object the audio listener')
}

export function add_normal_sound( target ) {
    let sound = new PositionalAudio( listener );
    sound.setNodeSource( normal_oscillator );
    // sound.setRefDistance( 20 );
    // sound.setVolume( 0.5 );

    target.add( sound )
}

export function add_tangential_sound( target ) {
    let sound = new PositionalAudio( listener );
    sound.setNodeSource( tangential_oscillator );
    sound.setRefDistance( 20 );
    sound.setVolume( 0.5 );

    target.add( sound )
}

export function add_impact_sound ( target ) {
    // create the PositionalAudio object (passing in the listener)
    const sound = new PositionalAudio( listener );


    // load a sound and set it as the PositionalAudio object's buffer
    const audioLoader = new AudioLoader();
    audioLoader.load( './resources/ping_pong.mp3', function( buffer ) {
    	sound.setBuffer( buffer );
    });



    // finally add the sound to the target
    target.add( sound );

    return target;
}
