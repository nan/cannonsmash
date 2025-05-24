// main.js - Corrected version for stable revert (compatible with Player.js from Turn 75)
import * as THREE from 'three';     
import { Ball } from './Ball.js';     // Assumes Ball.js is from Turn 61 (neutral reset, no toss method)
import { Player } from './Player.js'; // Assumes Player.js is from Turn 75 (speed fix, no toss/serveHit methods)

// ... (Global variables, GameState enum as in Turn 69) ...
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
const TABLE_HEIGHT = 0.76; 
const NET_POS_Z = 0;   
const RACKET_OFFSET_Z = 0.3; // Player.js constant, useful for AI serve positioning

function init() {
    // ... (Setup code as in Turn 69, down to player instantiation) ...
    player1ScoreElement = document.getElementById('player1Score');
    player2ScoreElement = document.getElementById('player2Score');
    updateScoreDisplay(); 
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0xaaaaaa); 
    camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.set(0, 2.0, 3.0); camera.lookAt(0, 0.5, 0);         
    try {
        renderer = new THREE.WebGLRenderer({ antialias: true }); 
        renderer.setSize(window.innerWidth, window.innerHeight); 
        document.body.appendChild(renderer.domElement); 
    } catch (e) { /* ... error handling ... */ 
        console.error("Three.js renderer initialization failed:", e);
        const initMessageElement = document.getElementById('initializationMessage');
        if (initMessageElement) {
            initMessageElement.textContent = "Error: Could not initialize WebGL. Please use a modern browser with WebGL enabled, and ensure hardware acceleration is active.";
            initMessageElement.style.color = 'red';
        } throw e;
    }
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.7); scene.add(ambientLight);
    const directionalLight = new THREE.DirectionalLight(0xffffff, 1.0); 
    directionalLight.position.set(-4, 6, 4); directionalLight.lookAt(0,0,0); scene.add(directionalLight);
    const tableMaterial = new THREE.MeshStandardMaterial({ color: 0x006400, roughness: 0.8, metalness: 0.2 }); 
    const netMaterial = new THREE.MeshStandardMaterial({ color: 0x333333, transparent: true, opacity: 0.8, roughness: 0.9 }); 
    const floorMaterial = new THREE.MeshStandardMaterial({ color: 0x808080, roughness: 0.9 }); 
    const ballMaterial = new THREE.MeshStandardMaterial({ color: 0xffa500, roughness: 0.5, metalness: 0.1 }); 
    const p1Material = new THREE.MeshStandardMaterial({ color: 0x0000dd, roughness: 0.6 }); 
    const p2Material = new THREE.MeshStandardMaterial({ color: 0x00dd00, roughness: 0.6 }); 
    const tableTopGeometry = new THREE.BoxGeometry(1.525, 0.03, 2.74); 
    table = new THREE.Mesh(tableTopGeometry, tableMaterial);
    table.position.set(0, TABLE_HEIGHT - 0.03 / 2, 0); scene.add(table);
    const netGeometry = new THREE.BoxGeometry(1.83, 0.1525, 0.01); 
    net = new THREE.Mesh(netGeometry, netMaterial);
    net.position.set(0, TABLE_HEIGHT + 0.1525 / 2, 0); scene.add(net);
    const floorGeometry = new THREE.PlaneGeometry(10, 10); 
    floor = new THREE.Mesh(floorGeometry, floorMaterial);
    floor.rotation.x = -Math.PI / 2; floor.position.y = 0; scene.add(floor);
    gameBall = new Ball(); 
    const ballGeometry = new THREE.SphereGeometry(gameBall.radius, 16, 12); 
    const ballMesh = new THREE.Mesh(ballGeometry, ballMaterial);
    gameBall.mesh = ballMesh; scene.add(ballMesh);      
    player1 = new Player(1, scene, 'human'); 
    const player1Geometry = new THREE.CapsuleGeometry(0.2, 0.8, 4, 16); 
    player1.mesh = new THREE.Mesh(player1Geometry, p1Material);
    player1.mesh.position.copy(player1.position); scene.add(player1.mesh);
    player1.mesh.add(player1.racket.mesh); 
    player2 = new Player(-1, scene, 'ai'); 
    const player2Geometry = new THREE.CapsuleGeometry(0.2, 0.8, 4, 16);
    player2.mesh = new THREE.Mesh(player2Geometry, p2Material);
    player2.mesh.position.copy(player2.position); scene.add(player2.mesh);
    player2.mesh.add(player2.racket.mesh);
    
    resetForServe(); 

    window.addEventListener('resize', onWindowResize, false);   
    window.addEventListener('mousemove', onMouseMove, false); 
    window.addEventListener('click', onMouseClick, false);    
    const initMessageElement = document.getElementById('initializationMessage');
    if (initMessageElement) { initMessageElement.style.display = 'none'; }
    animate();
}

function updateScoreDisplay() { /* ... as in Turn 69 ... */ 
    if (player1ScoreElement && player2ScoreElement) { 
        player1ScoreElement.textContent = score.player1;
        player2ScoreElement.textContent = score.player2;
    }
}
function onWindowResize() { /* ... as in Turn 69 ... */ 
    camera.aspect = window.innerWidth / window.innerHeight; camera.updateProjectionMatrix(); 
    renderer.setSize(window.innerWidth, window.innerHeight); 
}
function onMouseMove(event) { /* ... as in Turn 69 ... */ 
    if (player1 && player1.controlType === 'human') { 
        const screenX = (event.clientX / window.innerWidth) * 2 - 1;
        const screenY = -(event.clientY / window.innerHeight) * 2 + 1; 
        player1.handleMouseMove(screenX, screenY); 
    }
}
function onMouseClick(event) { /* ... as in Turn 69 ... */ 
    if (player1 && currentGameState === GameState.PRE_SERVE && servingPlayer === 1) {
        const racketWorldPos = new THREE.Vector3();
        player1.racket.mesh.getWorldPosition(racketWorldPos); 
        const ballServeRelativePos = new THREE.Vector3(0, gameBall.radius + 0.02, (player1.side * -0.15) );
        const ballServePosition = player1.racket.mesh.localToWorld(ballServeRelativePos.clone());
        gameBall.position.copy(ballServePosition);
        let serveVelocity = new THREE.Vector3((Math.random() - 0.5) * 0.5, 0.9 + (Math.random() * 0.2), -3.0 - (Math.random() * 0.3));
        let serveSpin = new THREE.Vector2((Math.random() - 0.5) * 1.0, 2.5); 
        gameBall.hit(serveVelocity, serveSpin, 1); 
        currentGameState = GameState.SERVE_IN_MOTION; 
        console.log("Player 1 serves (restored consistent mechanics)!");
    } else if (player1 && currentGameState === GameState.RALLY && gameBall.lastHitBy !== 1) {
        if (gameBall.status === 0 || (gameBall.status === 3 && gameBall.position.z > NET_POS_Z - 0.5) ) {
             player1.swing(gameBall); 
        }
    }
}

function resetForServe() {
    gameBall.reset(servingPlayer); 
    currentGameState = GameState.PRE_SERVE;
    
    // Player.isServing flag does not exist in this Player.js version, so calls are removed.
    // Ball positioning for P1 is handled in onMouseClick.
    // Ball positioning for P2 is handled in its AI serve logic below.

    console.log(`Ready for Player ${servingPlayer} to serve. P1 Click to serve. AI will auto-serve.`);

    if (servingPlayer === 2 && player2) { 
        console.log("AI (Player 2) is preparing to serve...");
        setTimeout(() => {
            if (currentGameState === GameState.PRE_SERVE && servingPlayer === 2) {
                // AI simple serve: position ball and hit directly
                const aiServeX = (Math.random() - 0.5) * 0.5; 
                // Ball is positioned near AI player before hit
                // Using RACKET_OFFSET_Z constant for consistent positioning logic
                gameBall.position.set(aiServeX, TABLE_HEIGHT + BALL_RADIUS + 0.2, player2.position.z + player2.side * (RACKET_OFFSET_Z + 0.05) );

                let serveVelocity = new THREE.Vector3(
                    (Math.random() - 0.5) * 1.5,
                    0.8 + (Math.random() * 0.4),
                    player2.side * (-3.0 - (Math.random() * 0.5)) // Serve towards P1
                );
                let serveSpin = new THREE.Vector2((Math.random() - 0.5) * 3, 1.5 + Math.random() * 3);
                
                gameBall.hit(serveVelocity, serveSpin, 2); 
                currentGameState = GameState.SERVE_IN_MOTION; 
                console.log("Player 2 (AI) serves (direct hit mechanics)!");
            }
        }, 1000 + Math.random() * 500); 
    }
}

function awardPointTo(winnerID) { /* ... as in Turn 69 ... */ 
    if (winnerID === 1) score.player1++; else score.player2++;
    updateScoreDisplay(); 
    console.log(`Point for Player ${winnerID}! Score: P1: ${score.player1} - P2: ${score.player2}`);
    currentGameState = GameState.POINT_SCORED; 
    servingPlayer = (servingPlayer === 1) ? 2 : 1; 
    setTimeout(resetForServe, 1500); 
} 
function checkGameRules() { /* ... as in Turn 69 ... */ 
    if (currentGameState === GameState.POINT_SCORED || currentGameState === GameState.PRE_SERVE) {
        return; 
    }
    if (gameBall.status === -1) { 
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); return; }
    if (gameBall.status === -2) { 
        if (gameBall.lastHitBy === 1) { 
            if (gameBall.position.z < NET_POS_Z && gameBall.bouncedOnReceiverSide) awardPointTo(1); 
            else awardPointTo(2); 
        } else if (gameBall.lastHitBy === 2) { 
            if (gameBall.position.z > NET_POS_Z && gameBall.bouncedOnServerSide) awardPointTo(2);
            else awardPointTo(1); 
        } else { awardPointTo(servingPlayer === 1 ? 2 : 1); }
        return; 
    }
    if (gameBall.status === -3 || gameBall.status === -4 || gameBall.status === -5) { 
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); return; }

    if (currentGameState === GameState.SERVE_IN_MOTION) {
        if (gameBall.lastHitBy === servingPlayer) {
            if (servingPlayer === 1 && gameBall.status === 1) { 
                if (gameBall.bouncedOnServerSide && gameBall.bouncedOnReceiverSide) { 
                    currentGameState = GameState.RALLY; gameBall.status = 2; 
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; 
                } else if (gameBall.bouncedOnReceiverSide && !gameBall.bouncedOnServerSide) { 
                    awardPointTo(2); 
                } else if (gameBall.bouncedOnServerSide && !gameBall.bouncedOnReceiverSide && gameBall.status < 0){
                     awardPointTo(2);
                }
            } else if (servingPlayer === 2 && gameBall.status === 3) { 
                if (gameBall.bouncedOnReceiverSide && gameBall.bouncedOnServerSide) { 
                    currentGameState = GameState.RALLY; gameBall.status = 0; 
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false;
                } else if (gameBall.bouncedOnServerSide && !gameBall.bouncedOnReceiverSide) { 
                    awardPointTo(1); 
                } else if (gameBall.bouncedOnReceiverSide && !gameBall.bouncedOnServerSide && gameBall.status < 0){
                     awardPointTo(1);
                }
            }
        }
    } else if (currentGameState === GameState.RALLY) {
        if (gameBall.lastHitBy === 1 && gameBall.status === 1) { 
            if (gameBall.bouncedOnReceiverSide) { 
                gameBall.status = 2; 
                gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; }
        } else if (gameBall.lastHitBy === 2 && gameBall.status === 3) { 
            if (gameBall.bouncedOnServerSide) { 
                gameBall.status = 0; 
                gameBall.bouncedOnReceiverSide = false; gameBall.bouncedOnServerSide = false; }
        }
    }
}

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

try {
    init(); 
    console.log("Three.js CannonSmash: Reverted main.js to be compatible with simpler Player.js, player speed fix applied.");
} catch (error) { /* ... error handling ... */ 
    console.error("Critical error during game initialization:", error);
    const initMessageElement = document.getElementById('initializationMessage');
    if (initMessageElement) {
        initMessageElement.textContent = "A critical error occurred. Could not start the game. Please check the console for details.";
        initMessageElement.style.color = 'red';
        initMessageElement.style.display = 'block'; 
    }
}
