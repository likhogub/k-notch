console.log("Started")

let quat = [0, 0, 0, 0]

var socket = new WebSocket("ws://localhost:1237");

socket.onopen = () => {
  setInterval(()=>socket.send("k"), 20);
}



socket.onmessage = event => {
  try {
    let x = event.data.toString().split(separator=";").map(x=>parseFloat(x))
    for (let i = 0; i < 4*3; i++) {
      quat[i] = x[i]
    }
  } catch(e) {
    console.log(e)
    quat = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
  }
  finally {}
}

socket.onclose = () => alert("Connection lost")

const scene = new THREE.Scene();

const renderer = new THREE.WebGLRenderer();
renderer.setSize( window.innerWidth, window.innerHeight );
document.body.appendChild( renderer.domElement );
const map = new THREE.TextureLoader().load( 'https://raw.githubusercontent.com/mrdoob/three.js/master/examples/textures/uv_grid_opengl.jpg' );
				map.wrapS = map.wrapT = THREE.RepeatWrapping;
				map.anisotropy = 16;
const material1 = new THREE.MeshPhongMaterial( { map: map, side: THREE.DoubleSide } );
const geometry1 = new THREE.BoxGeometry(0.25, 0.75, 0.25);

const cube = new THREE.Mesh( geometry1, material1 );
const cube2 = new THREE.Mesh( geometry1, material1 );
const cube3 = new THREE.Mesh( geometry1, material1 );

const V = new THREE.Mesh(geometry1, material1)

scene.add( cube )
scene.add( cube2 )
scene.add( cube3 )



const camera = new THREE.PerspectiveCamera (45, window.innerWidth / window.innerHeight, 1, 10000);
camera.position.x = 10;
camera.position.y = 10;
camera.position.z = 10;
camera.lookAt (new THREE.Vector3(0,0,0));

const ambientLight = new THREE.AmbientLight( 0xcccccc, 0.4 );
scene.add( ambientLight );

const pointLight = new THREE.PointLight( 0xffffff, 0.8 );
camera.add( pointLight );
scene.add( camera );


const gridHelper = new THREE.GridHelper( 10, 10 );
scene.add( gridHelper );

controls = new THREE.OrbitControls (camera, renderer.domElement);
document.cube = cube;

const axesHelper = new THREE.AxesHelper( 5 );
scene.add( axesHelper );


document.V = V

const points = [];
points.push( new THREE.Vector3( 0, 0, 0 ) );
points.push( new THREE.Vector3( 0, 1, 0 ) );

const geometry = new THREE.BufferGeometry().setFromPoints( points );
const material = new THREE.LineBasicMaterial( { color: 0xffffff } );
const line = new THREE.Line( geometry, material );
scene.add( line );

const geometry3 = new THREE.BufferGeometry().setFromPoints( points );
const material3 = new THREE.LineBasicMaterial( { color: 0xffffff } );
const line2 = new THREE.Line( geometry3, material3 );
scene.add( line2 );

const geometry4 = new THREE.BufferGeometry().setFromPoints( points );
const material4 = new THREE.LineBasicMaterial( { color: 0xffffff } );
const line3 = new THREE.Line( geometry4, material4 );
scene.add( line3 );


const ZZZ = new THREE.Vector3(0,0,0)
const animate = function () {
  requestAnimationFrame( animate );
  line.quaternion._w = quat[0]
  line.quaternion._x = quat[1]
  line.quaternion._y = quat[2]
  line.quaternion._z = quat[3]

  line2.position.x = line.matrix.elements[4]
  line2.position.y = line.matrix.elements[5]
  line2.position.z = line.matrix.elements[6]

  line2.quaternion._w = quat[4]
  line2.quaternion._x = quat[5]
  line2.quaternion._y = quat[6]
  line2.quaternion._z = quat[7]

  line3.position.x = line2.matrix.elements[4] + line2.position.x
  line3.position.y = line2.matrix.elements[5] + line2.position.y
  line3.position.z = line2.matrix.elements[6] + line2.position.z

  line3.quaternion._w = quat[8]
  line3.quaternion._x = quat[9]
  line3.quaternion._y = quat[10]
  line3.quaternion._z = quat[11]

  cube.position.x = line.matrix.elements[4] / 2
  cube.position.y = line.matrix.elements[5] / 2
  cube.position.z = line.matrix.elements[6] / 2
  cube.quaternion._w = quat[0]
  cube.quaternion._x = quat[1]
  cube.quaternion._y = quat[2]
  cube.quaternion._z = quat[3]


  cube2.position.x = (line2.matrix.elements[4] + line.matrix.elements[4] + line.matrix.elements[4]) / 2
  cube2.position.y = (line2.matrix.elements[5] + line.matrix.elements[5] + line.matrix.elements[5]) / 2
  cube2.position.z = (line2.matrix.elements[6] + line.matrix.elements[6] + line.matrix.elements[6]) / 2
  cube2.quaternion._w = quat[4]
  cube2.quaternion._x = quat[5]
  cube2.quaternion._y = quat[6]
  cube2.quaternion._z = quat[7]
  
  cube3.position.x = (line3.matrix.elements[4] + 2*(line2.matrix.elements[4] + line.matrix.elements[4])) / 2
  cube3.position.y = (line3.matrix.elements[5] + 2*(line2.matrix.elements[5] + line.matrix.elements[5])) / 2
  cube3.position.z = (line3.matrix.elements[6] + 2*(line2.matrix.elements[6] + line.matrix.elements[6])) / 2
  cube3.quaternion._w = quat[8]
  cube3.quaternion._x = quat[9]
  cube3.quaternion._y = quat[10]
  cube3.quaternion._z = quat[11]



  renderer.render( scene, camera );
};


animate();
