console.log("Started")

let quat = [0, 0, 0, 0]

var socket = new WebSocket("ws://localhost:1237");

socket.onopen = () => {
  alert("Соединение установлено.")
  setInterval(()=>socket.send("k"), 10);
}



socket.onmessage = event => {
  try {
    let x = event.data.toString().split(separator=";").map(x=>parseFloat(x))
    quat[0] = x[0]
    quat[1] = x[1]
    quat[2] = x[2]
    quat[3] = x[3]
  } catch(e) {
    console.log(e)
    quat = [0, 0, 0, 0]
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
const material = new THREE.MeshPhongMaterial( { map: map, side: THREE.DoubleSide } );
const geometry = new THREE.BoxGeometry(4, 2, 1);
//const material = new THREE.MeshBasicMaterial( { color: 0x00ff00 } );
const cube = new THREE.Mesh( geometry, material );
scene.add( cube );

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

var t = 0;
const animate = function () {
  requestAnimationFrame( animate );
  
  cube.quaternion._w = quat[0]
  cube.quaternion._x = quat[1]
  cube.quaternion._y = quat[2]
  cube.quaternion._z = quat[3]
  //cube.quaternion.normalize()

  //cube.applyMatrix4(new THREE.Matrix4().makeRotationZ(Math.PI));
  cube.applyMatrix4(new THREE.Matrix4().makeRotationX(-Math.PI/2));
  //cube.applyMatrix4(new THREE.Matrix4().makeRotationX(Math.PI / 2));
  //cube.quaternion.normalize()
  //console.log(quat)
  renderer.render( scene, camera );
};


animate();