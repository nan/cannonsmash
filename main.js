// main.js - Main script for the CannonSmash Web application

// --- Module Imports ---
import * as THREE from 'three';     
import { Ball } from './Ball.js';     
import { Player } from './Player.js'; 

// ... (all other global variables and GameState as before) ...
let scene, camera, renderer;            
let table, net, floor;                  
let gameBall;                           
let player1, player2;                   
let player1ScoreElement, player2ScoreElement;
const GameState = {
    PRE_SERVE: 'PRE_SERVE',            
    SERVE_IN_MOTION: 'SERVE_IN_MOTION', 
    RALLY: 'RALLY',                    
    POINT_SCORED: 'POINT_SCORED',      
    GAME_OVER: 'GAME_OVER'             
};
let currentGameState = GameState.PRE_SERVE; 
let servingPlayer = 1;                      
let score = { player1: 0, player2: 0 };     
const TABLE_LENGTH = 2.74; 
const TABLE_WIDTH = 1.525; 
const TABLE_HEIGHT = 0.76; 
const NET_POS_Z = 0;   


// --- Initialization Function (`init`) ---
function init() {
    // 1. Scoreboard UI Setup
    player1ScoreElement = document.getElementById('player1Score');
    player2ScoreElement = document.getElementById('player2Score');
    updateScoreDisplay(); 

    // 2. Three.js Scene Creation
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0xaaaaaa); 

    // 3. Camera Setup
    camera = new THREE.PerspectiveCamera(
        75,                                     
        window.innerWidth / window.innerHeight, 
        0.1,                                    
        1000                                    
    );
    camera.position.set(0, 2.0, 3.0); 
    camera.lookAt(0, 0.5, 0);         

    // 4. WebGL Renderer Setup
    // Wrap renderer creation in a try-catch to handle potential WebGL context issues
    try {
        renderer = new THREE.WebGLRenderer({ antialias: true }); 
        renderer.setSize(window.innerWidth, window.innerHeight); 
        document.body.appendChild(renderer.domElement); 
    } catch (e) {
        console.error("Three.js renderer initialization failed:", e);
        const initMessageElement = document.getElementById('initializationMessage');
        if (initMessageElement) {
            initMessageElement.textContent = "Error: Could not initialize WebGL. Please use a modern browser with WebGL enabled, and ensure hardware acceleration is active.";
            initMessageElement.style.color = 'red';
        }
        throw e; // Re-throw to stop further execution if renderer fails
    }


    // 5. Lighting Setup
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.7); 
    scene.add(ambientLight);
    const directionalLight = new THREE.DirectionalLight(0xffffff, 1.0); 
    directionalLight.position.set(-4, 6, 4); 
    directionalLight.lookAt(0,0,0);          
    scene.add(directionalLight);

    // 6. Material Definitions
    const tableMaterial = new THREE.MeshStandardMaterial({ color: 0x006400, roughness: 0.8, metalness: 0.2 }); 
    const netMaterial = new THREE.MeshStandardMaterial({ color: 0x333333, transparent: true, opacity: 0.8, roughness: 0.9 }); 
    const floorMaterial = new THREE.MeshStandardMaterial({ color: 0x808080, roughness: 0.9 }); 
    const ballMaterial = new THREE.MeshStandardMaterial({ color: 0xffa500, roughness: 0.5, metalness: 0.1 }); 
    const p1Material = new THREE.MeshStandardMaterial({ color: 0x0000dd, roughness: 0.6 }); 
    const p2Material = new THREE.MeshStandardMaterial({ color: 0x00dd00, roughness: 0.6 }); 
    const lineMaterial = new THREE.LineBasicMaterial({ color: 0xffffff }); 

    // 7. Environment Object Creation
    const tableTopGeometry = new THREE.BoxGeometry(1.525, 0.03, 2.74); 
    table = new THREE.Mesh(tableTopGeometry, tableMaterial);
    table.position.set(0, TABLE_HEIGHT - 0.03 / 2, 0); 
    scene.add(table);

    const netGeometry = new THREE.BoxGeometry(1.83, 0.1525, 0.01); 
    net = new THREE.Mesh(netGeometry, netMaterial);
    net.position.set(0, TABLE_HEIGHT + 0.1525 / 2, 0); 
    scene.add(net);

    // Create Center Line
    const centerLinePoints = [];
    centerLinePoints.push(new THREE.Vector3(0, TABLE_HEIGHT + 0.001, -TABLE_LENGTH / 2));
    centerLinePoints.push(new THREE.Vector3(0, TABLE_HEIGHT + 0.001, TABLE_LENGTH / 2));
    const centerLineGeometry = new THREE.BufferGeometry().setFromPoints(centerLinePoints);
    const centerLine = new THREE.Line(centerLineGeometry, lineMaterial);
    scene.add(centerLine);

    // Create Side Lines
    // Side Line 1
    const sideLine1Points = [];
    sideLine1Points.push(new THREE.Vector3(-TABLE_WIDTH / 2, TABLE_HEIGHT + 0.001, -TABLE_LENGTH / 2));
    sideLine1Points.push(new THREE.Vector3(-TABLE_WIDTH / 2, TABLE_HEIGHT + 0.001, TABLE_LENGTH / 2));
    const sideLine1Geometry = new THREE.BufferGeometry().setFromPoints(sideLine1Points);
    const sideLine1 = new THREE.Line(sideLine1Geometry, lineMaterial);
    scene.add(sideLine1);

    // Side Line 2
    const sideLine2Points = [];
    sideLine2Points.push(new THREE.Vector3(TABLE_WIDTH / 2, TABLE_HEIGHT + 0.001, -TABLE_LENGTH / 2));
    sideLine2Points.push(new THREE.Vector3(TABLE_WIDTH / 2, TABLE_HEIGHT + 0.001, TABLE_LENGTH / 2));
    const sideLine2Geometry = new THREE.BufferGeometry().setFromPoints(sideLine2Points);
    const sideLine2 = new THREE.Line(sideLine2Geometry, lineMaterial);
    scene.add(sideLine2);

    // Create End Lines
    // End Line 1 (nearer end)
    const endLine1Points = [];
    endLine1Points.push(new THREE.Vector3(-TABLE_WIDTH / 2, TABLE_HEIGHT + 0.001, TABLE_LENGTH / 2));
    endLine1Points.push(new THREE.Vector3(TABLE_WIDTH / 2, TABLE_HEIGHT + 0.001, TABLE_LENGTH / 2));
    const endLine1Geometry = new THREE.BufferGeometry().setFromPoints(endLine1Points);
    const endLine1 = new THREE.Line(endLine1Geometry, lineMaterial);
    scene.add(endLine1);

    // End Line 2 (farther end)
    const endLine2Points = [];
    endLine2Points.push(new THREE.Vector3(-TABLE_WIDTH / 2, TABLE_HEIGHT + 0.001, -TABLE_LENGTH / 2));
    endLine2Points.push(new THREE.Vector3(TABLE_WIDTH / 2, TABLE_HEIGHT + 0.001, -TABLE_LENGTH / 2));
    const endLine2Geometry = new THREE.BufferGeometry().setFromPoints(endLine2Points);
    const endLine2 = new THREE.Line(endLine2Geometry, lineMaterial);
    scene.add(endLine2);

    const floorGeometry = new THREE.PlaneGeometry(10, 10); 
    floor = new THREE.Mesh(floorGeometry, floorMaterial);
    floor.rotation.x = -Math.PI / 2; 
    floor.position.y = 0;            
    scene.add(floor);

    // 8. Game Object Instantiation
    gameBall = new Ball(); 
    const ballGeometry = new THREE.SphereGeometry(gameBall.radius, 16, 12); 
    const ballMesh = new THREE.Mesh(ballGeometry, ballMaterial);
    gameBall.mesh = ballMesh; 
    scene.add(ballMesh);      

    player1 = new Player(1, scene, 'human'); 
    const player1Geometry = new THREE.CapsuleGeometry(0.2, 0.8, 4, 16); 
    player1.mesh = new THREE.Mesh(player1Geometry, p1Material);
    player1.mesh.position.copy(player1.position); 
    scene.add(player1.mesh);
    player1.mesh.add(player1.racket.mesh); 

    player2 = new Player(-1, scene, 'ai'); 
    const player2Geometry = new THREE.CapsuleGeometry(0.2, 0.8, 4, 16);
    player2.mesh = new THREE.Mesh(player2Geometry, p2Material);
    player2.mesh.position.copy(player2.position);
    scene.add(player2.mesh);
    player2.mesh.add(player2.racket.mesh);

    // 9. Initial Game Setup
    resetForServe(); 

    // 10. Event Listener Setup
    window.addEventListener('resize', onWindowResize, false);   
    window.addEventListener('mousemove', onMouseMove, false); 
    window.addEventListener('click', onMouseClick, false);    

    // Remove the loading/error message if initialization was successful up to this point
    const initMessageElement = document.getElementById('initializationMessage');
    if (initMessageElement) {
        initMessageElement.style.display = 'none'; // Hide it
    }

    // 11. Start Animation Loop
    animate();
}

// --- UI Functions ---
// ... (updateScoreDisplay as before) ...
function updateScoreDisplay() {
    if (player1ScoreElement && player2ScoreElement) { 
        player1ScoreElement.textContent = score.player1;
        player2ScoreElement.textContent = score.player2;
    }
}

// --- Event Handlers ---
// ... (onWindowResize, onMouseMove, onMouseClick as before) ...
function onWindowResize() { 
    camera.aspect = window.innerWidth / window.innerHeight; 
    camera.updateProjectionMatrix(); 
    renderer.setSize(window.innerWidth, window.innerHeight); 
}
function onMouseMove(event) { 
    if (player1 && player1.controlType === 'human') { 
        const screenX = (event.clientX / window.innerWidth) * 2 - 1;
        const screenY = -(event.clientY / window.innerHeight) * 2 + 1; 
        player1.handleMouseMove(screenX, screenY); 
    }
}
function onMouseClick(event) {
    if (player1 && currentGameState === GameState.PRE_SERVE && servingPlayer === 1) {
        const racketWorldPos = new THREE.Vector3();
        player1.racket.mesh.getWorldPosition(racketWorldPos); 
        
        // Position ball consistently relative to racket for serve
        // Player1's racket default relative Z is player1.side * -RACKET_OFFSET_Z from Player.js
        // RACKET_OFFSET_Z = 0.3. player1.side = 1. So, -0.3.
        // We want ball slightly in front of this (more negative Z).
        const ballServeRelativePos = new THREE.Vector3(
            0, // Centered on racket face X
            gameBall.radius + 0.02, // Slightly above racket center Y
            (player1.side * -0.15) // Slightly in front of racket face (player1.side * -RACKET_OFFSET_Z is racket center Z)
                                   // Racket mesh itself is 0.16 wide (Z for racket), so half is 0.08
                                   // This places it near front edge.
        );
        
        // Convert relative position to world and set ball position
        // Clone racket position and add relative offset
        const ballServePosition = player1.racket.mesh.localToWorld(ballServeRelativePos.clone());
        gameBall.position.copy(ballServePosition);

        // Consistent serve velocity and spin
        let serveVelocity = new THREE.Vector3(
            (Math.random() - 0.5) * 0.5,  // Reduced X-randomness for more straight serves
            0.9 + (Math.random() * 0.2),  // Consistent upward component (0.9 to 1.1)
            -3.0 - (Math.random() * 0.3)  // Consistent forward Z speed (-3.0 to -3.3)
        );
        // Moderate and predictable topspin
        let serveSpin = new THREE.Vector2(
            (Math.random() - 0.5) * 1.0, // Minimal sidespin
            2.5 // Consistent moderate topspin
        ); 
        
        gameBall.hit(serveVelocity, serveSpin, 1); // Player 1 hits the ball
        currentGameState = GameState.SERVE_IN_MOTION; 
        console.log("Player 1 serves (adjusted mechanics)!");
    } else if (player1 && currentGameState === GameState.RALLY && gameBall.lastHitBy !== 1) {
        if (gameBall.status === 0 || (gameBall.status === 3 && gameBall.position.z > NET_POS_Z - 0.5) ) {
             player1.swing(); 
        }
    }
}

// --- Game Logic Functions ---
// ... (resetForServe, awardPointTo, checkGameRules as before, ensure they are complete) ...
function resetForServe() {
    gameBall.reset(servingPlayer); 
    currentGameState = GameState.PRE_SERVE;
    console.log(`Ready for Player ${servingPlayer} to serve.`);
    if (servingPlayer === 2 && player2) { 
        console.log("AI (Player 2) is preparing to serve...");
        setTimeout(() => {
            if (currentGameState === GameState.PRE_SERVE && servingPlayer === 2) {
                const racketWorldPos = new THREE.Vector3();
                player2.racket.mesh.getWorldPosition(racketWorldPos); 
                gameBall.position.copy(racketWorldPos);
                const forwardOffset = new THREE.Vector3(0, 0, player2.side * -0.12); 
                gameBall.position.add(forwardOffset);
                gameBall.position.y += gameBall.radius * 0.5;
                let serveVelocity = new THREE.Vector3(
                    (Math.random() - 0.5) * 1.5,
                    0.8 + (Math.random() * 0.4),
                    3.0 + (Math.random() * 0.8)  
                );
                let serveSpin = new THREE.Vector2((Math.random() - 0.5) * 3, 1.5 + Math.random() * 3);
                gameBall.hit(serveVelocity, serveSpin, 2); 
                currentGameState = GameState.SERVE_IN_MOTION; 
                console.log("Player 2 (AI) serves!");
            }
        }, 1000 + Math.random() * 500); 
    }
}
function awardPointTo(winnerID) { 
    if (winnerID === 1) {
        score.player1++;
    } else { 
        score.player2++;
    }
    updateScoreDisplay(); 
    console.log(`Point for Player ${winnerID}! Score: P1: ${score.player1} - P2: ${score.player2}`);
    currentGameState = GameState.POINT_SCORED; 
    servingPlayer = (servingPlayer === 1) ? 2 : 1; 
    setTimeout(resetForServe, 1500); 
} 
function checkGameRules() { 
    if (currentGameState === GameState.POINT_SCORED || currentGameState === GameState.PRE_SERVE) return;
    if (gameBall.status === -1) { 
        console.log("Rule Check: Ball hit net structure during play.");
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); 
        return; 
    }
    if (gameBall.status === -2) { 
        console.log("Rule Check: Ball hit floor.");
        if (gameBall.lastHitBy === 1) { 
            if (gameBall.position.z < NET_POS_Z && gameBall.bouncedOnReceiverSide) awardPointTo(1); 
            else awardPointTo(2); 
        } else if (gameBall.lastHitBy === 2) { 
            if (gameBall.position.z > NET_POS_Z && gameBall.bouncedOnServerSide) awardPointTo(2);
            else awardPointTo(1); 
        } else { 
            if (gameBall.position.z > NET_POS_Z) awardPointTo(2); 
            else awardPointTo(1); 
        }
        return; 
    }
    if (gameBall.status === -3 || gameBall.status === -4 || gameBall.status === -5) { 
        console.log(`Rule Check: Ball out of bounds (status ${gameBall.status}). Last hit by ${gameBall.lastHitBy}`);
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); 
        return; 
    }
    if (currentGameState === GameState.SERVE_IN_MOTION) {
        if (gameBall.lastHitBy === servingPlayer) {
            if (servingPlayer === 1 && gameBall.status === 1) { 
                if (gameBall.bouncedOnServerSide && gameBall.bouncedOnReceiverSide) { 
                    console.log("Serve by P1 is IN. Rally begins (P2 to hit).");
                    currentGameState = GameState.RALLY; 
                    gameBall.status = 2; 
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; 
                } else if (gameBall.bouncedOnReceiverSide && !gameBall.bouncedOnServerSide) { 
                    console.log("Serve Fault (P1): Hit opponent's side first.");
                    awardPointTo(2); 
                } 
            } else if (servingPlayer === 2 && gameBall.status === 3) { 
                if (gameBall.bouncedOnReceiverSide && gameBall.bouncedOnServerSide) { 
                    console.log("Serve by P2 is IN. Rally begins (P1 to hit).");
                    currentGameState = GameState.RALLY;
                    gameBall.status = 0; 
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false;
                } else if (gameBall.bouncedOnServerSide && !gameBall.bouncedOnReceiverSide) { 
                    console.log("Serve Fault (P2): Hit opponent's side first.");
                    awardPointTo(1); 
                }
            }
        }
    } else if (currentGameState === GameState.RALLY) {
        if (gameBall.lastHitBy === 1 && gameBall.status === 1) { 
            if (gameBall.bouncedOnReceiverSide) { 
                console.log("P1's rally shot is IN. P2 to play.");
                gameBall.status = 2; 
                gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; 
            }
        }
        else if (gameBall.lastHitBy === 2 && gameBall.status === 3) { 
            if (gameBall.bouncedOnServerSide) { 
                console.log("P2's rally shot is IN. P1 to play.");
                gameBall.status = 0; 
                gameBall.bouncedOnReceiverSide = false; gameBall.bouncedOnServerSide = false; 
            }
        }
    }
}

// --- Main Animation Loop (`animate`) ---
function animate() {
    requestAnimationFrame(animate); 
    if (player1) player1.update(gameBall); 
    if (player2) player2.update(gameBall); 
    if (gameBall) {
        gameBall.update(player1.racket.mesh, player2 ? player2.racket.mesh : null); 
    }
    checkGameRules(); 
    renderer.render(scene, camera); 
}

// --- Entry Point ---
// Wrap the main initialization in a try-catch to update the message div on critical failure.
try {
    init(); 
    console.log("Three.js CannonSmash initialized with loading/error message handling.");
} catch (error) {
    console.error("Critical error during game initialization:", error);
    const initMessageElement = document.getElementById('initializationMessage');
    if (initMessageElement) {
        initMessageElement.textContent = "A critical error occurred. Could not start the game. Please check the console for details.";
        initMessageElement.style.color = 'red';
        initMessageElement.style.display = 'block'; // Ensure it's visible
    }
}
