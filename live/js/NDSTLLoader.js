/**
 * @author aleeper / http://adamleeper.com/
 * @author mrdoob / http://mrdoob.com/
 * @author gero3 / https://github.com/gero3
 * @author Mugen87 / https://github.com/Mugen87
 * @author neverhood311 / https://github.com/neverhood311
 *
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
 *  var loader = new STLLoader();
 *  loader.load( './models/stl/slotted_disk.stl', function ( geometry ) {
 *    scene.add( new THREE.Mesh( geometry ) );
 *  });
 *
 * For binary STLs geometry might contain colors for vertices. To use it:
 *  // use the same code to load STL as above
 *  if (geometry.hasColors) {
 *    material = new THREE.MeshPhongMaterial({ opacity: geometry.alpha, vertexColors: true });
 *  } else { .... }
 *  var mesh = new THREE.Mesh( geometry, material );
 *
 * For ASCII STLs containing multiple solids, each solid is assigned to a different group.
 * Groups can be used to assign a different color by defining an array of materials with the same length of
 * geometry.groups and passing it to the Mesh constructor:
 *
 * var mesh = new THREE.Mesh( geometry, material );
 *
 * For example:
 *
 *  var materials = [];
 *  var nGeometryGroups = geometry.groups.length;
 *
 *  var colorMap = ...; // Some logic to index colors.
 *
 *  for (var i = 0; i < nGeometryGroups; i++) {
 *
 *		var material = new THREE.MeshPhongMaterial({
 *			color: colorMap[i],
 *			wireframe: false
 *		});
 *
 *  }
 *
 *  materials.push(material);
 *  var mesh = new THREE.Mesh(geometry, materials);
 */

import {
	BufferAttribute,
	BufferGeometry,
	FileLoader,
	Float32BufferAttribute,
	Loader,
	LoaderUtils,
	Vector3
} from "./three.module.js";


var STLLoader = function ( manager ) {

	Loader.call( this, manager );

};

STLLoader.prototype = Object.assign( Object.create( Loader.prototype ), {

	constructor: STLLoader,

	load: function ( params, onLoad, onProgress, onError ) {
        let url = params[0];
        let W = params[1];

		var scope = this;

		var loader = new FileLoader( scope.manager );
		loader.setPath( scope.path );
		loader.setResponseType( 'arraybuffer' );
		loader.load( url,  function ( text ) {

			try {

				onLoad( scope.parse( text, W ) );

			} catch ( e ) {

				if ( onError ) {

					onError( e );

				} else {

					console.error( e );

				}

				scope.manager.itemError( url );

			}

		}, onProgress, onError );

	},

	parse: function ( data, W ) {

		function isBinary( data ) {

			var expect, face_size, n_faces, reader;
			reader = new DataView( data );
			face_size = ( 32 / 8 * 3 ) + ( ( 32 / 8 * 3 ) * 3 ) + ( 16 / 8 );
			n_faces = reader.getUint32( 80, true );
			expect = 80 + ( 32 / 8 ) + ( n_faces * face_size );

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

			var solid = [ 115, 111, 108, 105, 100 ];

			for ( var off = 0; off < 5; off ++ ) {

				// If "solid" text is matched to the current offset, declare it to be an ASCII STL.

				if ( matchDataViewAt( solid, reader, off ) ) return false;

			}

			// Couldn't find "solid" text at the beginning; it is binary STL.

			return true;

		}

		function matchDataViewAt( query, reader, offset ) {

			// Check if each byte in query matches the corresponding byte from the current offset

			for ( var i = 0, il = query.length; i < il; i ++ ) {

				if ( query[ i ] !== reader.getUint8( offset + i, false ) ) return false;

			}

			return true;

		}

		function parseBinary( data ) {

			var reader = new DataView( data );
			var faces = reader.getUint32( 80, true );

			var r, g, b, hasColors = false, colors;
			var defaultR, defaultG, defaultB, alpha;

			// process STL header
			// check for default color in header ("COLOR=rgba" sequence).

			for ( var index = 0; index < 80 - 10; index ++ ) {

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

			var dataOffset = 84;
			var faceLength = 12 * 4 + 2;

			var geometry = new BufferGeometry();

			var vertices = new Float32Array( faces * 3 * 3 );
			var normals = new Float32Array( faces * 3 * 3 );

			for ( var face = 0; face < faces; face ++ ) {

				var start = dataOffset + face * faceLength;
				var normalX = reader.getFloat32( start, true );
				var normalY = reader.getFloat32( start + 4, true );
				var normalZ = reader.getFloat32( start + 8, true );

				if ( hasColors ) {

					var packedColor = reader.getUint16( start + 48, true );

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

				for ( var i = 1; i <= 3; i ++ ) {

					var vertexstart = start + i * 12;
					var componentIdx = ( face * 3 * 3 ) + ( ( i - 1 ) * 3 );

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

		function parseASCII( data, W ) {
            var N;

			var geometry = new BufferGeometry();
            var patternDimension = /solid ([0-9]+)/g ;
			var patternSolid = /solid([\s\S]*?)endsolid/g;
			var patternFace = /facet([\s\S]*?)endfacet/g;
			var faceCounter = 0;

            while ( ( result = patternDimension.exec( data ) ) !== null ) {
                N = parseInt(result[1]);
                console.log(N)
            }


			var patternFloat = /[\s]+([+-]?(?:\d*)(?:\.\d*)?(?:[eE][+-]?\d+)?)/.source;
            var patternFloats = patternFloat.repeat(N);
			var patternVertex = new RegExp( 'vertex' + patternFloats, 'g' );
			var patternNormal = new RegExp( 'normal' + patternFloats, 'g' );

			var vertices = [];
			var normals = [];

			// var normal = new Vector3(); // oh boy...

			var result;

			var groupCount = 0;
			var startVertex = 0;
			var endVertex = 0;



			while ( ( result = patternSolid.exec( data ) ) !== null ) {

				startVertex = endVertex;

				var solid = result[ 0 ];

				while ( ( result = patternFace.exec( solid ) ) !== null ) {

					var vertexCountPerFace = 0;

					var text = result[ 0 ];

					while ( ( result = patternVertex.exec( text ) ) !== null ) {
                        for ( var i=1; i<=N; i++ ) {
                            vertices.push( parseFloat(result[i]) );
                        }
					}

				}

				var start = startVertex;
				var count = endVertex - startVertex;

				geometry.addGroup( start, count, groupCount );
				groupCount ++;

			}
            let proj_vertices = [];
            let n_simplices = vertices.length/N;

            // let W = 0.5
            if ( N == 3 ) {
                proj_vertices = vertices;
            }
            else if ( N == 4 ) {
                for ( var i=0; i<n_simplices; i++ ) {
                    for ( var j=i+1; j<n_simplices; j++ ) {
                        let alpha = (W - vertices[i*N + 3])/(vertices[j*N + 3] - vertices[i*N + 3]);
                        if ( vertices[i*N + 3] == W && vertices[j*N + 3] == W ) {
                            // alpha is not defined, we are coincident with W, add both points
                            for ( var n=0; n<N; n++ ) {
                                proj_vertices.push( vertices[i*N + n] );
                            }
                            for ( var n=0; n<N; n++ ) {
                                proj_vertices.push( vertices[j*N + n] );
                            }
                        }
                        else if ( alpha > 0 && alpha < 1 ) {
                            // alpha is in range
                            for ( var n=0; n<N; n++ ) {
                                proj_vertices.push( vertices[i*N + n] + alpha*(vertices[j*N + n] - vertices[i*N + n]) );
                            }
                        }
                    }
                }
            }


			geometry.setAttribute( 'position', new Float32BufferAttribute( proj_vertices, 3 ) );
            console.log(geometry)
			// geometry.setAttribute( 'normal', new Float32BufferAttribute( normals, 3 ) );
			return geometry;

		}

		function ensureString( buffer ) {

			if ( typeof buffer !== 'string' ) {

				return LoaderUtils.decodeText( new Uint8Array( buffer ) );

			}

			return buffer;

		}

		function ensureBinary( buffer ) {

			if ( typeof buffer === 'string' ) {

				var array_buffer = new Uint8Array( buffer.length );
				for ( var i = 0; i < buffer.length; i ++ ) {

					array_buffer[ i ] = buffer.charCodeAt( i ) & 0xff; // implicitly assumes little-endian

				}

				return array_buffer.buffer || array_buffer;

			} else {

				return buffer;

			}

		}

		// start

        return parseASCII( ensureString( data ), W );
		// var binData = ensureBinary( data );

		// return isBinary( binData ) ? parseBinary( binData ) : parseASCII( ensureString( data ) );

	}

} );

export { STLLoader };
