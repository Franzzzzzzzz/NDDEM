// TO DO LIST
// 1. Enable generation of scene by passing a variable rather than an infile
// 2. Pull out time marching so that individual loops can be run


async function NDDEMPhysics() {

	if ( 'DEMND' in window === false ) {

		console.error( 'NDDEMPhysics: Couldn\'t find DEMND.js' );
		return;

	}

	const NDDEMLib = await DEMND(); // eslint-disable-line no-undef
    const Dimensions = 3;
    const Num_grains = 50;
	const frameRate = 1000;

	// const world = new NDDEMLib.btDiscreteDynamicsWorld( dispatcher, broadphase, solver, collisionConfiguration );
	// world.setGravity( 0, - 9.8, 0 );

    // From François:
    // Simulation<3> S (1000) ; //For 3D, 1000 particles ...
    // S.interpret_command("set kn 10000") ;
    // S.interpret_command("set mu 0.5") ;  //etc. ...
    // S.step_forward (20) ; //Advance 20 timesteps
    // S.P.Boundaries[1][0]=0.1 ; // Set location of the bottom wall in dimension y to 0.1
    // S.finalise() ; // At the end, not really needed I guess


    // let S = await NDDEMLib.Simulation;
    let S = await new NDDEMLib.Simulation (Num_grains);
    // let a = S.main();
    // console.log(a)
    S.interpret_command("radius -1 0.5");
    S.interpret_command("mass -1 1");
    S.interpret_command("auto rho");
    S.interpret_command("auto inertia");

    S.interpret_command("boundary 0 WALL 0 20");
    S.interpret_command("boundary 1 PBC  0 4");
    S.interpret_command("boundary 2 PBC  0 4");
    S.interpret_command("auto location randomdrop");
    S.interpret_command("gravity -1 0 0");
    S.interpret_command("set Kn 2e5");
    S.interpret_command("set Kt 8e4");
    S.interpret_command("set GammaN 75");
    S.interpret_command("set GammaT 75");
    S.interpret_command("set Mu 0.5");
    S.interpret_command("set T 150");
    S.interpret_command("set dt 0.0001");

    // let x = new Float64Array(Num_grains*3);
    // console.log(x);
    // x = S.X;
    console.log(NDDEMLib);
    var pointer = NDDEMLib._X;
    // var pointer = 100;
    // var offset = NDDEMLib._malloc(NDDEMLib._X)
    console.log(pointer)
    window.x = NDDEMLib.HEAPF64.subarray(pointer, pointer + Dimensions*Num_grains);
    console.log(NDDEMLib.HEAPF64)
    // let x = new Float64Array(NDDEMLib._X);
    console.log(x)

    // console.log(S);
    // console.log(S.X);
    // S.finalise();

    // console.log(S);


    const walls = [];
	const meshes = [];
	const meshMap = new WeakMap();

	function addMesh( mesh, mass = 0 ) {

		const shape = getShape( mesh.geometry );

		if ( shape !== null ) {

			if ( mesh.isInstancedMesh ) {

				handleInstancedMesh( mesh, mass, shape );

			} else if ( mesh.isMesh ) {

				handleMesh( mesh, mass, shape );

			}

		}

	}

	function handleMesh( mesh, mass, shape ) {

		const position = mesh.position;
		const quaternion = mesh.quaternion;

		const transform = new NDDEMLib.btTransform();
		transform.setIdentity();
		transform.setOrigin( new NDDEMLib.btVector3( position.x, position.y, position.z ) );
		transform.setRotation( new NDDEMLib.btQuaternion( quaternion.x, quaternion.y, quaternion.z, quaternion.w ) );

		const motionState = new NDDEMLib.btDefaultMotionState( transform );

		const localInertia = new NDDEMLib.btVector3( 0, 0, 0 );
		shape.calculateLocalInertia( mass, localInertia );

		const rbInfo = new NDDEMLib.btRigidBodyConstructionInfo( mass, motionState, shape, localInertia );

		const body = new NDDEMLib.btRigidBody( rbInfo );


		world.addRigidBody( body );

		if ( mass > 0 ) {

			meshes.push( mesh );
			meshMap.set( mesh, body );

		}

        else{
            walls.push( body );
        }



	}

	function handleInstancedMesh( mesh, mass, shape ) {

		const array = mesh.instanceMatrix.array;

		const bodies = [];

		for ( let i = 0; i < mesh.count; i ++ ) {

			const index = i * 16;

			const transform = new NDDEMLib.btTransform();
			transform.setFromOpenGLMatrix( array.slice( index, index + 16 ) );

			const motionState = new NDDEMLib.btDefaultMotionState( transform );

			const localInertia = new NDDEMLib.btVector3( 0, 0, 0 );
			shape.calculateLocalInertia( mass, localInertia );

			const rbInfo = new NDDEMLib.btRigidBodyConstructionInfo( mass, motionState, shape, localInertia );
            // rbInfo.m_restitution = 0.1;

			const body = new NDDEMLib.btRigidBody( rbInfo );
            body.setFriction( 0.5 );
            body.setRestitution( 0.01 );
            // body.setDamping(0.5,0.5);
            body.setSleepingThresholds(0,0);
            body.setActivationState(4);
            body.activate();

			world.addRigidBody( body );

			bodies.push( body );

		}

		if ( mass > 0 ) {

			mesh.instanceMatrix.setUsage( 35048 ); // THREE.DynamicDrawUsage = 35048
			meshes.push( mesh );

			meshMap.set( mesh, bodies );

		}

	}

    function getWallForce( ) {
        // console.log('a')
		const body = walls[0]; // this is a btCollisionObject not a btRigidBody!!!!
        // const collisionObj = Ammo.wrapPointer(body, Ammo.btCollisionObject);
        const rigidBody = new NDDEMLib.btRigidBody;
        rigidBody.upcast(body);
        // console.log(body.getTotalForce())
        // console.log(collisionObj.getTotalForce())
        // console.log(rigidBody.getTotalForce())

        return rigidBody.getOldTotalForce()
    }

	//

	function setMeshPosition( mesh, position, index = 0 ) {

		if ( mesh.isInstancedMesh ) {

			const bodies = meshMap.get( mesh );
			const body = bodies[ index ];

			body.setAngularVelocity( new NDDEMLib.btVector3( 0, 0, 0 ) );
			body.setLinearVelocity( new NDDEMLib.btVector3( 0, 0, 0 ) );

			worldTransform.setIdentity();
			worldTransform.setOrigin( new NDDEMLib.btVector3( position.x, position.y, position.z ) );
			body.setWorldTransform( worldTransform );

		} else if ( mesh.isMesh ) {

			const body = meshMap.get( mesh );

			body.setAngularVelocity( new NDDEMLib.btVector3( 0, 0, 0 ) );
			body.setLinearVelocity( new NDDEMLib.btVector3( 0, 0, 0 ) );

			worldTransform.setIdentity();
			worldTransform.setOrigin( new NDDEMLib.btVector3( position.x, position.y, position.z ) );
			body.setWorldTransform( worldTransform );

		}

	}

	//

	let lastTime = 0;

	function step() {
        const force = 0;
		const time = performance.now();

		if ( lastTime > 0 ) {

			const delta = ( time - lastTime ) / 1000;
            S.step_forward(100);
            var pointer = NDDEMLib._X/64;
            // var offset = NDDEMLib._malloc(NDDEMLib._X)
            // console.log(offset)
            window.x = NDDEMLib.HEAPF64.subarray(pointer, pointer + Dimensions*Num_grains);
            // S.finalise();
            // S.step_forward(1);
			// console.time( 'world.step' );
			// world.stepSimulation( delta, 10 );
			// console.timeEnd( 'world.step' );
		}

		lastTime = time;

		//

		for ( let i = 0, l = meshes.length; i < l; i ++ ) {

			const mesh = meshes[ i ];
            // console.log(mesh)
			if ( mesh.isInstancedMesh ) {

				const array = mesh.instanceMatrix.array;
				const bodies = meshMap.get( mesh );

				for ( let j = 0; j < bodies.length; j ++ ) {

					const body = bodies[ j ];
                    // console.log(body)
					const motionState = body.getMotionState();
					motionState.getWorldTransform( worldTransform );

					const position = worldTransform.getOrigin();
					const quaternion = worldTransform.getRotation();

					compose( position, quaternion, array, j * 16 );

				}

				mesh.instanceMatrix.needsUpdate = true;

			} else if ( mesh.isMesh ) {

				const body = meshMap.get( mesh );

				const motionState = body.getMotionState();
				motionState.getWorldTransform( worldTransform );

				const position = worldTransform.getOrigin();
				const quaternion = worldTransform.getRotation();
				mesh.position.set( position.x(), position.y(), position.z() );
				mesh.quaternion.set( quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w() );

			}

		}

	}

	// animate

	setInterval( step, 1000 / frameRate );

	return {
		addMesh: addMesh,
		setMeshPosition: setMeshPosition,
        getWallForce: getWallForce
		// addCompoundMesh
	};

}

function compose( position, quaternion, array, index ) {

	const x = quaternion.x(), y = quaternion.y(), z = quaternion.z(), w = quaternion.w();
	const x2 = x + x, y2 = y + y, z2 = z + z;
	const xx = x * x2, xy = x * y2, xz = x * z2;
	const yy = y * y2, yz = y * z2, zz = z * z2;
	const wx = w * x2, wy = w * y2, wz = w * z2;

	array[ index + 0 ] = ( 1 - ( yy + zz ) );
	array[ index + 1 ] = ( xy + wz );
	array[ index + 2 ] = ( xz - wy );
	array[ index + 3 ] = 0;

	array[ index + 4 ] = ( xy - wz );
	array[ index + 5 ] = ( 1 - ( xx + zz ) );
	array[ index + 6 ] = ( yz + wx );
	array[ index + 7 ] = 0;

	array[ index + 8 ] = ( xz + wy );
	array[ index + 9 ] = ( yz - wx );
	array[ index + 10 ] = ( 1 - ( xx + yy ) );
	array[ index + 11 ] = 0;

	array[ index + 12 ] = position.x();
	array[ index + 13 ] = position.y();
	array[ index + 14 ] = position.z();
	array[ index + 15 ] = 1;

}

export { NDDEMPhysics };
