// main.js 
// ... (imports and other global variables, GameState as before - from Turn 47/49) ...
import * as THREE from 'three';     
import { Ball } from './Ball.js';     
import { Player } from './Player.js'; 

let scene, camera, renderer;            
let table, net, floor;                  
let gameBall;                           
let player1, player2;                   
let player1ScoreElement, player2ScoreElement;
const GameState = {
    AWAITING_SERVE_TOSS: 'AWAITING_SERVE_TOSS', 
    BALL_TOSSED: 'BALL_TOSSED',             
    SERVE_IN_MOTION: 'SERVE_IN_MOTION',     
    RALLY: 'RALLY',                         
    POINT_SCORED: 'POINT_SCORED',           
    GAME_OVER: 'GAME_OVER'                  
};
let currentGameState = GameState.AWAITING_SERVE_TOSS; 
let servingPlayer = 1;                          
let score = { player1: 0, player2: 0 };     
const TABLE_LENGTH = 2.74; 
const TABLE_HEIGHT = 0.76; 
const NET_POS_Z = 0;   

// --- Initialization Function (`init`) ---
// ... (init() function as in Turn 47/49 - no changes here) ...
function init() {
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
    } catch (e) {
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


// --- UI Functions ---
// ... (updateScoreDisplay as in Turn 47/49) ...
function updateScoreDisplay() {
    if (player1ScoreElement && player2ScoreElement) { 
        player1ScoreElement.textContent = score.player1;
        player2ScoreElement.textContent = score.player2;
    }
}

// --- Event Handlers ---
// ... (onWindowResize, onMouseMove as in Turn 47/49) ...
function onWindowResize() { 
    camera.aspect = window.innerWidth / window.innerHeight; camera.updateProjectionMatrix(); 
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
    if (player1 && player1.controlType === 'human') {
        if (currentGameState === GameState.AWAITING_SERVE_TOSS && servingPlayer === 1) {
            player1.serveToss(gameBall); 
            currentGameState = GameState.BALL_TOSSED;
            console.log("Player 1 tossed the ball. Click again to hit.");
        } else if (currentGameState === GameState.BALL_TOSSED && servingPlayer === 1 && player1.isServing) {
            if (player1.swing(gameBall)) { // swing now returns true if serveHit was successful
                 currentGameState = GameState.SERVE_IN_MOTION;
                 console.log("Player 1 hit the serve!");
            } else {
                console.log("Player 1 missed or failed to hit the tossed ball properly.");
                // Game rule for dropped toss will eventually award point to P2 if ball status becomes < 0
            }
        } else if (currentGameState === GameState.RALLY && gameBall.lastHitBy !== 1) {
            if (gameBall.status === 0 || (gameBall.status === 3 && gameBall.position.z > NET_POS_Z - 0.5) ) {
                 player1.swing(gameBall); 
            }
        }
    }
}

// --- Game Logic Functions ---
/**
 * Resets the ball and game state for a new serve.
 * Now also calls player-specific ball positioning.
 */
function resetForServe() {
    gameBall.reset(servingPlayer); // Ball status 8 (P1) or 9 (P2), neutral position
    currentGameState = GameState.AWAITING_SERVE_TOSS;
    
    if (player1 && servingPlayer === 1) {
        player1.isServing = false; // Ensure flag is reset before positioning
        player1.positionBallForServeStart(gameBall); // Position ball near P1's hand
    }
    if (player2 && servingPlayer === 2) {
        player2.isServing = false; // Ensure flag is reset
        // AI will position ball then toss then hit in its sequence
    }

    console.log(`Ready for Player ${servingPlayer} to serve. Click to toss (P1) or AI will serve.`);

    if (servingPlayer === 2 && player2) { 
        console.log("AI (Player 2) is preparing to serve (position & toss)...");
        setTimeout(() => {
            if (currentGameState === GameState.AWAITING_SERVE_TOSS && servingPlayer === 2) {
                player2.positionBallForServeStart(gameBall); // AI positions the ball
                player2.serveToss(gameBall);                 // AI calls its toss method
                currentGameState = GameState.BALL_TOSSED;
                console.log("AI (Player 2) tossed the ball. Will hit shortly.");
                
                setTimeout(() => {
                    if (currentGameState === GameState.BALL_TOSSED && servingPlayer === 2 && player2.isServing) {
                        if (player2.swing(gameBall)) { // swing calls serveHit
                            currentGameState = GameState.SERVE_IN_MOTION;
                            console.log("Player 2 (AI) hit the serve!");
                        } else {
                             console.log("AI (Player 2) missed or failed to hit the tossed ball.");
                             // Game rules will handle the dropped ball.
                        }
                    }
                }, 600 + Math.random() * 200); 
            }
        }, 1000 + Math.random() * 500); 
    }
}

// ... (awardPointTo as in Turn 47/49) ...
function awardPointTo(winnerID) { 
    if (winnerID === 1) score.player1++; else score.player2++;
    updateScoreDisplay(); 
    console.log(`Point for Player ${winnerID}! Score: P1: ${score.player1} - P2: ${score.player2}`);
    currentGameState = GameState.POINT_SCORED; 
    servingPlayer = (servingPlayer === 1) ? 2 : 1; 
    setTimeout(resetForServe, 1500); 
} 
function checkGameRules() { 
    // (Logic from Turn 47/49, with slight adjustment for BALL_TOSSED state)
    if (currentGameState === GameState.POINT_SCORED || currentGameState === GameState.AWAITING_SERVE_TOSS || 
        (currentGameState === GameState.BALL_TOSSED && gameBall.status >=6) // Allow tossed ball (status 6 or 7) to fly
       ) {
        // Specifically check if a tossed ball (status 6 or 7) hits the floor (status -2)
        if (currentGameState === GameState.BALL_TOSSED && gameBall.status === -2) {
            console.log("Rule Check: Tossed ball hit floor before being hit.");
            awardPointTo(servingPlayer === 1 ? 2 : 1); // Point to opponent of the player who was serving
        }
        return; // Skip other rule checks if not in active play or if just handling tossed ball drop
    }
    // (The rest of checkGameRules logic for net, floor, out, serve validation, rally progression as in Turn 47/49)
    if (gameBall.status === -1) { 
        console.log("Rule Check: Ball hit net structure during play.");
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); return; }
    if (gameBall.status === -2) { 
        console.log("Rule Check: Ball hit floor.");
        if (gameBall.lastHitBy === 1) { 
            if (gameBall.position.z < NET_POS_Z && gameBall.bouncedOnReceiverSide) awardPointTo(1); 
            else awardPointTo(2); 
        } else if (gameBall.lastHitBy === 2) { 
            if (gameBall.position.z > NET_POS_Z && gameBall.bouncedOnServerSide) awardPointTo(2);
            else awardPointTo(1); 
        } else { awardPointTo(servingPlayer === 1 ? 2 : 1); } // If no last hit, fault by server
        return; 
    }
    if (gameBall.status === -3 || gameBall.status === -4 || gameBall.status === -5) { 
        console.log(`Rule Check: Ball out of bounds (status ${gameBall.status}). Last hit by ${gameBall.lastHitBy}`);
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); return; }

    if (currentGameState === GameState.SERVE_IN_MOTION) {
        if (gameBall.lastHitBy === servingPlayer) {
            if (servingPlayer === 1 && gameBall.status === 1) { 
                if (gameBall.bouncedOnServerSide && gameBall.bouncedOnReceiverSide) { 
                    console.log("Serve by P1 is IN. Rally begins (P2 to hit).");
                    currentGameState = GameState.RALLY; gameBall.status = 2; 
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; 
                } else if (gameBall.bouncedOnReceiverSide && !gameBall.bouncedOnServerSide) { 
                    console.log("Serve Fault (P1): Hit opponent's side first."); awardPointTo(2); 
                } else if (gameBall.bouncedOnServerSide && !gameBall.bouncedOnReceiverSide && gameBall.status < 0){ // Hit own side then out/net
                     console.log("Serve Fault (P1): Bounced on own side then out/net."); awardPointTo(2);
                }
            } else if (servingPlayer === 2 && gameBall.status === 3) { 
                if (gameBall.bouncedOnReceiverSide && gameBall.bouncedOnServerSide) { 
                    console.log("Serve by P2 is IN. Rally begins (P1 to hit).");
                    currentGameState = GameState.RALLY; gameBall.status = 0; 
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false;
                } else if (gameBall.bouncedOnServerSide && !gameBall.bouncedOnReceiverSide) { 
                    console.log("Serve Fault (P2): Hit opponent's side first."); awardPointTo(1); 
                } else if (gameBall.bouncedOnReceiverSide && !gameBall.bouncedOnServerSide && gameBall.status < 0){ // Hit own side then out/net
                     console.log("Serve Fault (P2): Bounced on own side then out/net."); awardPointTo(1);
                }
            }
        }
    } else if (currentGameState === GameState.RALLY) {
        if (gameBall.lastHitBy === 1 && gameBall.status === 1) { 
            if (gameBall.bouncedOnReceiverSide) { 
                console.log("P1's rally shot is IN. P2 to play."); gameBall.status = 2; 
                gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; }
        } else if (gameBall.lastHitBy === 2 && gameBall.status === 3) { 
            if (gameBall.bouncedOnServerSide) { 
                console.log("P2's rally shot is IN. P1 to play."); gameBall.status = 0; 
                gameBall.bouncedOnReceiverSide = false; gameBall.bouncedOnServerSide = false; }
        }
    }
}

// --- Main Animation Loop (`animate`) ---
// ... (animate as before) ...
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
// ... (try/catch for init as before) ...
try {
    init(); 
    console.log("Three.js CannonSmash: Initial ball positioning updated.");
} catch (error) {
    console.error("Critical error during game initialization:", error);
    const initMessageElement = document.getElementById('initializationMessage');
    if (initMessageElement) {
        initMessageElement.textContent = "A critical error occurred. Could not start the game. Please check the console for details.";
        initMessageElement.style.color = 'red';
        initMessageElement.style.display = 'block'; 
    }
}
