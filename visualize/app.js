console.log("Started")

const DEVICES = 2
let quat = []

var socket = new WebSocket("ws://localhost:1237");

socket.onopen = () => {
  setInterval(()=>socket.send("k"), 10);
}



socket.onmessage = event => {
  try {
    let x = event.data.toString().split(separator=";").map(x=>parseFloat(x))
    for (let i = 0; i < 4*DEVICES; i++) {
      quat[i] = x[i]
    }
  } catch(e) {
      quat = []
      for (let i = 0; i < DEVICES; i++)
        quat.push(0)
  }
  finally {}
}

socket.onclose = () => alert("Connection lost")
 
const scene = new THREE.Scene();

const renderer = new THREE.WebGLRenderer();
renderer.setSize( window.innerWidth, window.innerHeight );
document.body.appendChild( renderer.domElement );

const camera = new THREE.PerspectiveCamera (45, window.innerWidth / window.innerHeight, 1, 10000);
camera.position.x = 10;
camera.position.y = 10;
camera.position.z = 10;
camera.lookAt (new THREE.Vector3(0,0,0));
scene.add(camera);

const ambientLight = new THREE.AmbientLight( 0xcccccc, 0.4 );
scene.add( ambientLight );

const pointLight = new THREE.PointLight( 0xffffff, 0.8 );
camera.add(pointLight);

controls = new THREE.OrbitControls(camera, renderer.domElement);

scene.add(new THREE.GridHelper(10, 10));
scene.add(new THREE.AxesHelper(5));

const figures = []
const links = []
const joints = [{x:0, y:0, z:0}]

for (let i = 0; i < DEVICES; i++) {
    // new line
    const _points = [];
    _points.push(new THREE.Vector3( 0, 0, 0 ));
    _points.push(new THREE.Vector3( 0, 1, 0 ));
    const _geometry = new THREE.BufferGeometry().setFromPoints(_points);
    const _material = new THREE.LineBasicMaterial({color: 0xffffff});
    const _line = new THREE.Line(_geometry, _material);
    links.push(_line)
    joints.push({x:0, y:0, z:0})

    //new figure
    const map = new THREE.TextureLoader()
                    .load( 'https://raw.githubusercontent.com/mrdoob/three.js/master/examples/textures/uv_grid_opengl.jpg' );

	map.wrapS = map.wrapT = THREE.RepeatWrapping;
	map.anisotropy = 16;
  const material1 = new THREE.MeshPhongMaterial( { map: map, side: THREE.DoubleSide } );
  const geometry1 = new THREE.BoxGeometry(0.5, 0.75, 0.25);
  const _cube = new THREE.Mesh( geometry1, material1 );
  figures.push(_cube)
  
  scene.add(_cube)
  scene.add(_line)
}

const kQ = [ 0, Math.sqrt(0.5), Math.sqrt(0.5), 0]
const off = new THREE.Quaternion(0, 0, 0, 1)

const animate = function () {
    requestAnimationFrame( animate );
    for (let i = 0; i < DEVICES; i++) {
        //const q = new THREE.Quaternion(quat[4*i+3], quat[4*i],quat[4*i+1],quat[4*i+2])
        const q = new THREE.Quaternion(quat[4*i], -quat[4*i+1], -quat[4*i+3],quat[4*i+2])
        q.premultiply(off)

        links[i].position.x = joints[i].x
        links[i].position.y = joints[i].y
        links[i].position.z = joints[i].z

        links[i].quaternion._x = kQ[0]
        links[i].quaternion._y = kQ[1]
        links[i].quaternion._z = kQ[2]
        links[i].quaternion._w = kQ[3]

        links[i].applyQuaternion(q)
        
        joints[i+1].x = links[i].matrix.elements[4] + joints[i].x
        joints[i+1].y = links[i].matrix.elements[5] + joints[i].y
        joints[i+1].z = links[i].matrix.elements[6] + joints[i].z
        
        figures[i].position.x = (joints[i].x + joints[i+1].x) / 2
        figures[i].position.y = (joints[i].y + joints[i+1].y) / 2 
        figures[i].position.z = (joints[i].z + joints[i+1].z) / 2
        
        figures[i].quaternion._x = kQ[0]
        figures[i].quaternion._y = kQ[1]
        figures[i].quaternion._z = kQ[2]
        figures[i].quaternion._w = kQ[3]


        figures[i].applyQuaternion(q)
    }

    renderer.render( scene, camera );
};


animate();
