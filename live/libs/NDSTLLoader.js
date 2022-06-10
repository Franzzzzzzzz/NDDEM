import {
	BufferAttribute,
	BufferGeometry,
	FileLoader,
	Float32BufferAttribute,
	Loader,
	LoaderUtils,
	Vector3,
    Mesh,
    Group,
    Box3,
    BoxGeometry,
} from 'three';

import { ConvexGeometry } from "three/examples/jsm/geometries/ConvexGeometry.js";
import { VertexNormalsHelper } from 'three/examples/jsm/helpers/VertexNormalsHelper.js';


/**
 * Description: A THREE loader for STL ASCII files, as created by Solidworks and other CAD programs.
 *
 * Supports both binary and ASCII encoded files, with automatic detection of type.
 *
 * The loader returns a non-indexed buffer geometry.
 *
 * Limitations:
 *  Binary decoding supports "Magics" color format (http://en.wikipedia.org/wiki/STL_(file_format)#Color_in_binary_STL).
 *  There is perhaps some question as to how valid it is to always assume little-endian-ness.
 *  ASCII decoding assumes file is UTF-8.
 *
 * Usage:
 *  const loader = new STLLoader();
 *  loader.load( './models/stl/slotted_disk.stl', function ( geometry ) {
 *    scene.add( new THREE.Mesh( geometry ) );
 *  });
 *
 * For binary STLs geometry might contain colors for vertices. To use it:
 *  // use the same code to load STL as above
 *  if (geometry.hasColors) {
 *    material = new THREE.MeshPhongMaterial({ opacity: geometry.alpha, vertexColors: true });
 *  } else { .... }
 *  const mesh = new THREE.Mesh( geometry, material );
 *
 * For ASCII STLs containing multiple solids, each solid is assigned to a different group.
 * Groups can be used to assign a different color by defining an array of materials with the same length of
 * geometry.groups and passing it to the Mesh constructor:
 *
 * const mesh = new THREE.Mesh( geometry, material );
 *
 * For example:
 *
 *  const materials = [];
 *  const nGeometryGroups = geometry.groups.length;
 *
 *  const colorMap = ...; // Some logic to index colors.
 *
 *  for (let i = 0; i < nGeometryGroups; i++) {
 *
 *		const material = new THREE.MeshPhongMaterial({
 *			color: colorMap[i],
 *			wireframe: false
 *		});
 *
 *  }
 *
 *  materials.push(material);
 *  const mesh = new THREE.Mesh(geometry, materials);
 */


class NDSTLLoader extends Loader {

	constructor( manager ) {

		super( manager );

	}

	load( url, onLoad, onProgress, onError ) {

		const scope = this;

		const loader = new FileLoader( this.manager );
		loader.setPath( this.path );
		loader.setResponseType( 'arraybuffer' );
		loader.setRequestHeader( this.requestHeader );
		loader.setWithCredentials( this.withCredentials );

		loader.load( url, function ( text ) {

			try {

				onLoad( scope.parse( text ) );

			} catch ( e ) {

				if ( onError ) {

					onError( e );

				} else {

					console.error( e );

				}

				scope.manager.itemError( url );

			}

		}, onProgress, onError );

	}

	parse( data ) {

		function isBinary( data ) {

			const reader = new DataView( data );
			const face_size = ( 32 / 8 * 3 ) + ( ( 32 / 8 * 3 ) * 3 ) + ( 16 / 8 );
			const n_faces = reader.getUint32( 80, true );
			const expect = 80 + ( 32 / 8 ) + ( n_faces * face_size );

			if ( expect === reader.byteLength ) {

				return true;

			}

			// An ASCII STL data must begin with 'solid ' as the first six bytes.
			// However, ASCII STLs lacking the SPACE after the 'd' are known to be
			// plentiful.  So, check the first 5 bytes for 'solid'.

			// Several encodings, such as UTF-8, precede the text with up to 5 bytes:
			// https://en.wikipedia.org/wiki/Byte_order_mark#Byte_order_marks_by_encoding
			// Search for "solid" to start anywhere after those prefixes.

			// US-ASCII ordinal values for 's', 'o', 'l', 'i', 'd'

			const solid = [ 115, 111, 108, 105, 100 ];

			for ( let off = 0; off < 5; off ++ ) {

				// If "solid" text is matched to the current offset, declare it to be an ASCII STL.

				if ( matchDataViewAt( solid, reader, off ) ) return false;

			}

			// Couldn't find "solid" text at the beginning; it is binary STL.

			return true;

		}

		function matchDataViewAt( query, reader, offset ) {

			// Check if each byte in query matches the corresponding byte from the current offset

			for ( let i = 0, il = query.length; i < il; i ++ ) {

				if ( query[ i ] !== reader.getUint8( offset + i ) ) return false;

			}

			return true;

		}

		function parseBinary( data ) {

			const reader = new DataView( data );
			const faces = reader.getUint32( 80, true );

			let r, g, b, hasColors = false, colors;
			let defaultR, defaultG, defaultB, alpha;

			// process STL header
			// check for default color in header ("COLOR=rgba" sequence).

			for ( let index = 0; index < 80 - 10; index ++ ) {

				if ( ( reader.getUint32( index, false ) == 0x434F4C4F /*COLO*/ ) &&
					( reader.getUint8( index + 4 ) == 0x52 /*'R'*/ ) &&
					( reader.getUint8( index + 5 ) == 0x3D /*'='*/ ) ) {

					hasColors = true;
					colors = new Float32Array( faces * 3 * 3 );

					defaultR = reader.getUint8( index + 6 ) / 255;
					defaultG = reader.getUint8( index + 7 ) / 255;
					defaultB = reader.getUint8( index + 8 ) / 255;
					alpha = reader.getUint8( index + 9 ) / 255;

				}

			}

			const dataOffset = 84;
			const faceLength = 12 * 4 + 2;

			const geometry = new BufferGeometry();

			const vertices = new Float32Array( faces * 3 * 3 );
			const normals = new Float32Array( faces * 3 * 3 );

			for ( let face = 0; face < faces; face ++ ) {

				const start = dataOffset + face * faceLength;
				const normalX = reader.getFloat32( start, true );
				const normalY = reader.getFloat32( start + 4, true );
				const normalZ = reader.getFloat32( start + 8, true );

				if ( hasColors ) {

					const packedColor = reader.getUint16( start + 48, true );

					if ( ( packedColor & 0x8000 ) === 0 ) {

						// facet has its own unique color

						r = ( packedColor & 0x1F ) / 31;
						g = ( ( packedColor >> 5 ) & 0x1F ) / 31;
						b = ( ( packedColor >> 10 ) & 0x1F ) / 31;

					} else {

						r = defaultR;
						g = defaultG;
						b = defaultB;

					}

				}

				for ( let i = 1; i <= 3; i ++ ) {

					const vertexstart = start + i * 12;
					const componentIdx = ( face * 3 * 3 ) + ( ( i - 1 ) * 3 );

					vertices[ componentIdx ] = reader.getFloat32( vertexstart, true );
					vertices[ componentIdx + 1 ] = reader.getFloat32( vertexstart + 4, true );
					vertices[ componentIdx + 2 ] = reader.getFloat32( vertexstart + 8, true );

					normals[ componentIdx ] = normalX;
					normals[ componentIdx + 1 ] = normalY;
					normals[ componentIdx + 2 ] = normalZ;

					if ( hasColors ) {

						colors[ componentIdx ] = r;
						colors[ componentIdx + 1 ] = g;
						colors[ componentIdx + 2 ] = b;

					}

				}

			}

			geometry.setAttribute( 'position', new BufferAttribute( vertices, 3 ) );
			geometry.setAttribute( 'normal', new BufferAttribute( normals, 3 ) );

			if ( hasColors ) {

				geometry.setAttribute( 'color', new BufferAttribute( colors, 3 ) );
				geometry.hasColors = true;
				geometry.alpha = alpha;

			}

			return geometry;

		}

        function parseASCII( data ) {
            var N;

            var patternDimension = /solid ([0-9]+)/g ;
			var patternSolid = /solid([\s\S]*?)endsolid/g;
			var patternFace = /facet([\s\S]*?)endfacet/g;

            while ( ( result = patternDimension.exec( data ) ) !== null ) {
                N = parseInt(result[1]);
            }

			var patternFloat = /[\s]+([+-]?(?:\d*)(?:\.\d*)?(?:[eE][+-]?\d+)?)/.source;
            var patternFloats = patternFloat.repeat(N);
			var patternVertex = new RegExp( 'vertex' + patternFloats, 'g' );
			var patternNormal = new RegExp( 'normal' + patternFloats, 'g' );

			var result;

            var solids = [];

			while ( ( result = patternSolid.exec( data ) ) !== null ) { // for each solid

				var solid_text = result[ 0 ];
                var solid = [];

				while ( ( result = patternFace.exec( solid_text ) ) !== null ) { // for each facet

                    var facet_text = result[ 0 ];
                    var facet = [];
					while ( ( result = patternVertex.exec( facet_text ) ) !== null ) {  // get the N vertices
                        var vertex = [];
                        for ( var i=1; i<=N; i++ ) {
                            vertex.push( parseFloat(result[i]) );
                        }
                        facet.push(vertex);
					}
                    solid.push(facet);

				}
                solids.push(solid);
			}
			return solids;

		}

		function ensureString( buffer ) {

			if ( typeof buffer !== 'string' ) {

				return LoaderUtils.decodeText( new Uint8Array( buffer ) );

			}

			return buffer;

		}

		function ensureBinary( buffer ) {

			if ( typeof buffer === 'string' ) {

				const array_buffer = new Uint8Array( buffer.length );
				for ( let i = 0; i < buffer.length; i ++ ) {

					array_buffer[ i ] = buffer.charCodeAt( i ) & 0xff; // implicitly assumes little-endian

				}

				return array_buffer.buffer || array_buffer;

			} else {

				return buffer;

			}

		}

		// start

        return parseASCII( ensureString( data ) );
		// const binData = ensureBinary( data );

		// return isBinary( binData ) ? parseBinary( binData ) : parseASCII( ensureString( data ) );

	}

}

function pushUnique(arr, new_entry) {
    let added = false;
    let found = false;
    let tol = 1e-6;
    if ( arr.length === 0 ) { // add first entry
        arr.push( new_entry[0], new_entry[1], new_entry[2] );
        added = true;
    }
    else {
        for ( var i=0; i<arr.length/3; i++ ) {
            if ( Math.abs( arr[i*3]   - new_entry[0] ) < tol &&
                 Math.abs( arr[i*3+1] - new_entry[1] ) < tol &&
                 Math.abs( arr[i*3+2] - new_entry[2] ) < tol) {
                found = true;
            }
        }
        if ( found === false ) {
            arr.push ( new_entry[0], new_entry[1], new_entry[2] );
            added = true;
        }
    }
    return added
}

function calculateNormal(points) {
    let ux = points[1][0]-points[0][0];
    let uy = points[1][1]-points[0][1];
    let uz = points[1][2]-points[0][2];
    let vx = points[2][0]-points[0][0];
    let vy = points[2][1]-points[0][1];
    let vz = points[2][2]-points[0][2];

    let u_cross_v = [uy*vz-uz*vy, uz*vx-ux*vz, ux*vy-uy*vx] //cross product
    return u_cross_v;
}



function renderSTL( meshes, NDsolids, scene, material, x4 ) {

    var vertices, normals;
    var N = NDsolids[0][0][0].length; // get dimension from vertex length

    NDsolids.forEach((solid, i) => {
        var geometry = new BufferGeometry();
        vertices = [];
        normals = [];
        let tol = 1e-6;
        let added, alpha;
        solid.forEach((facet, j) => {
            var normal = calculateNormal(facet);
            facet.forEach((vertex, k) => {
                if ( N == 3 ) {
                    added = pushUnique(vertices, vertex);
                    if ( added ) { normals.push( normal[0], normal[1], normal[2]); }
                 }
                else if ( N == 4 ) {
                    // loop through all other vertices in facet
                    for (var l=k+1; l<N; l++) {
                        alpha = (x4 - vertex[N-1])/(facet[l][N-1] - vertex[N-1]);
                        if ( Math.abs(vertex[N-1] - x4) < tol && Math.abs(facet[l][N-1] - x4) < tol ) {
                            // alpha is not defined, we are coincident with x4, add both points
                            added = pushUnique(vertices, vertex);
                            if ( added ) { normals.push( normal[0], normal[1], normal[2]); }
                            added = pushUnique(vertices, facet[l]);
                            if ( added ) { normals.push( normal[0], normal[1], normal[2]); }
                        }
                        else if ( alpha >= 0 && alpha <= 1 ) { // alpha is in range
                            let sliced_vertex = [];
                            for ( var n=0; n<3; n++ ) {
                                sliced_vertex.push( vertex[n] + alpha*(facet[l][n] - vertex[n]) );
                            }
                            added = pushUnique(vertices, sliced_vertex);
                            if ( added ) { normals.push( normal[0], normal[1], normal[2]); }
                        }
                    }
                }
            });
        });

        geometry.setAttribute( 'position', new Float32BufferAttribute( vertices, 3 ) );
        geometry.setAttribute( 'normal', new Float32BufferAttribute( normals, 3 ) );

        let points = [];
        // for ( let i=0; i<vertices.length/3; i++ ) { points.push( new Vector3( vertices[i*3]   + Math.random()*1e-6,
        //                                                                       vertices[i*3+1] + Math.random()*1e-6,
        //                                                                       vertices[i*3+2] + Math.random()*1e-6
        //                                                                      ) ) };
        for ( let i=0; i<vertices.length/3; i++ ) {
            points.push( new Vector3( vertices[i*3],
                                      vertices[i*3+1],
                                      vertices[i*3+2]
                                    )
                        )
        };

        let min_val = 0.005;
        let WHD = new Vector3();
        let center = new Vector3();
        if ( points.length > 3 ) {
            // geometry = new ConvexGeometry( points ); // causing all kinds of problems for planes
            const box = new Box3();
            box.setFromPoints( points );
            box.getSize(WHD);
            box.getCenter(center);
            WHD.clampScalar( min_val, 1e4 ); // set a minimum width
            geometry = new BoxGeometry( WHD.x, WHD.y, WHD.z );
        }

        var this_mesh = new Mesh( geometry, material );
        this_mesh.position.set( center.x, center.y-min_val, center.z );
        this_mesh.castShadow = true;
        this_mesh.receiveShadow = true;

        meshes.add(this_mesh);
    });

    // meshes.castShadow = true;
    // meshes.receiveShadow = true;

    // console.log(meshes)

    // console.log(meshes)
    return meshes
}

export { NDSTLLoader, renderSTL };
