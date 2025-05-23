// main.js 
// ... (imports and other global variables as before) ...
import * as THREE from 'three';     
import { Ball } from './Ball.js';     
import { Player } from './Player.js'; 

let scene, camera, renderer;            
let table, net, floor;                  
let gameBall;                           
let player1, player2;                   
let player1ScoreElement, player2ScoreElement;

// Updated Game State Management
const GameState = {
    AWAITING_SERVE_TOSS: 'AWAITING_SERVE_TOSS', // Ready for the server to initiate the toss
    BALL_TOSSED: 'BALL_TOSSED',             // Ball has been tossed, awaiting server to hit it
    SERVE_IN_MOTION: 'SERVE_IN_MOTION',     // Serve has been hit, ball is in flight, serve rules apply
    RALLY: 'RALLY',                         // Ball is in play after a valid serve/return
    POINT_SCORED: 'POINT_SCORED',           // A point has just been scored
    GAME_OVER: 'GAME_OVER'                  // (Future Use)
};
let currentGameState = GameState.AWAITING_SERVE_TOSS; // Initial game state
let servingPlayer = 1;                          
let score = { player1: 0, player2: 0 };     
const TABLE_LENGTH = 2.74; 
const TABLE_HEIGHT = 0.76; 
const NET_POS_Z = 0;   

// --- Initialization Function (`init`) ---
function init() {
    // ... (Scoreboard UI, Scene, Camera, Renderer, Lighting, Materials, Environment Objects as before) ...
    player1ScoreElement = document.getElementById('player1Score');
    player2ScoreElement = document.getElementById('player2Score');
    updateScoreDisplay(); 
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0xaaaaaa); 
    camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.set(0, 2.0, 3.0); 
    camera.lookAt(0, 0.5, 0);         
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
        throw e; 
    }
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.7); 
    scene.add(ambientLight);
    const directionalLight = new THREE.DirectionalLight(0xffffff, 1.0); 
    directionalLight.position.set(-4, 6, 4); 
    directionalLight.lookAt(0,0,0);          
    scene.add(directionalLight);
    const tableMaterial = new THREE.MeshStandardMaterial({ color: 0x006400, roughness: 0.8, metalness: 0.2 }); 
    const netMaterial = new THREE.MeshStandardMaterial({ color: 0x333333, transparent: true, opacity: 0.8, roughness: 0.9 }); 
    const floorMaterial = new THREE.MeshStandardMaterial({ color: 0x808080, roughness: 0.9 }); 
    const ballMaterial = new THREE.MeshStandardMaterial({ color: 0xffa500, roughness: 0.5, metalness: 0.1 }); 
    const p1Material = new THREE.MeshStandardMaterial({ color: 0x0000dd, roughness: 0.6 }); 
    const p2Material = new THREE.MeshStandardMaterial({ color: 0x00dd00, roughness: 0.6 }); 
    const tableTopGeometry = new THREE.BoxGeometry(1.525, 0.03, 2.74); 
    table = new THREE.Mesh(tableTopGeometry, tableMaterial);
    table.position.set(0, TABLE_HEIGHT - 0.03 / 2, 0); 
    scene.add(table);
    const netGeometry = new THREE.BoxGeometry(1.83, 0.1525, 0.01); 
    net = new THREE.Mesh(netGeometry, netMaterial);
    net.position.set(0, TABLE_HEIGHT + 0.1525 / 2, 0); 
    scene.add(net);
    const floorGeometry = new THREE.PlaneGeometry(10, 10); 
    floor = new THREE.Mesh(floorGeometry, floorMaterial);
    floor.rotation.x = -Math.PI / 2; 
    floor.position.y = 0;            
    scene.add(floor);

    // Game Object Instantiation
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

    resetForServe(); // Initial setup for the first serve

    window.addEventListener('resize', onWindowResize, false);   
    window.addEventListener('mousemove', onMouseMove, false); 
    window.addEventListener('click', onMouseClick, false);    

    const initMessageElement = document.getElementById('initializationMessage');
    if (initMessageElement) {
        initMessageElement.style.display = 'none';
    }
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
// ... (onWindowResize, onMouseMove as before) ...
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

/**
 * Handles mouse click events for player actions (serve toss, serve hit, rally swing).
 */
function onMouseClick(event) {
    if (player1 && player1.controlType === 'human') {
        if (currentGameState === GameState.AWAITING_SERVE_TOSS && servingPlayer === 1) {
            player1.serveToss(gameBall); // Player calls its own toss method
            currentGameState = GameState.BALL_TOSSED;
            console.log("Player 1 tossed the ball. Click again to hit.");
        } else if (currentGameState === GameState.BALL_TOSSED && servingPlayer === 1 && player1.isServing) {
            player1.swing(gameBall); // This will call serveHit internally due to isServing flag
            // Ball status (1 for P1 hit) and player1.isServing will be updated by Player/Ball methods.
            // Transition to SERVE_IN_MOTION happens if hit was successful (ball status becomes 1)
            if (gameBall.status === 1) { // Check if hit was registered
                 currentGameState = GameState.SERVE_IN_MOTION;
                 console.log("Player 1 hit the serve!");
            } else {
                // Optional: Handle missed toss hit (ball drops) - rules will eventually award point.
                console.log("Player 1 attempted serve hit on tossed ball.");
            }
        } else if (currentGameState === GameState.RALLY && gameBall.lastHitBy !== 1) {
            // Standard rally swing
            if (gameBall.status === 0 || (gameBall.status === 3 && gameBall.position.z > NET_POS_Z - 0.5) ) {
                 player1.swing(gameBall); 
            }
        }
    }
}

// --- Game Logic Functions ---
/**
 * Resets the ball and game state for a new serve.
 */
function resetForServe() {
    gameBall.reset(servingPlayer); // Ball positioned, status 8 (P1) or 9 (P2)
    currentGameState = GameState.AWAITING_SERVE_TOSS;
    if (player1 && servingPlayer === 1) player1.isServing = false; // Reset serving state for player
    if (player2 && servingPlayer === 2) player2.isServing = false;

    console.log(`Ready for Player ${servingPlayer} to serve. Click to toss.`);

    if (servingPlayer === 2 && player2) { 
        console.log("AI (Player 2) is preparing to serve (toss)...");
        setTimeout(() => {
            if (currentGameState === GameState.AWAITING_SERVE_TOSS && servingPlayer === 2) {
                player2.serveToss(gameBall); // AI calls its toss method
                currentGameState = GameState.BALL_TOSSED;
                console.log("AI (Player 2) tossed the ball. Will hit shortly.");
                
                // Schedule AI's serve hit after a delay (simulating toss peak and hit)
                setTimeout(() => {
                    if (currentGameState === GameState.BALL_TOSSED && servingPlayer === 2 && player2.isServing) {
                        player2.swing(gameBall); // This will call serveHit
                        if (gameBall.status === 3) { // Check if hit was registered
                            currentGameState = GameState.SERVE_IN_MOTION;
                            console.log("Player 2 (AI) hit the serve!");
                        }
                    }
                }, 600 + Math.random() * 200); // Delay for AI to "hit" the tossed ball (0.6-0.8s)
            }
        }, 1000 + Math.random() * 500); 
    }
}

// ... (awardPointTo and checkGameRules as in Turn 41, with minor console log adjustments if needed) ...
function awardPointTo(winnerID) { 
    // (No changes from Turn 41, ensure it's complete)
    if (winnerID === 1) score.player1++; else score.player2++;
    updateScoreDisplay(); 
    console.log(`Point for Player ${winnerID}! Score: P1: ${score.player1} - P2: ${score.player2}`);
    currentGameState = GameState.POINT_SCORED; 
    servingPlayer = (servingPlayer === 1) ? 2 : 1; 
    setTimeout(resetForServe, 1500); 
} 

function checkGameRules() { 
    if (currentGameState === GameState.POINT_SCORED || currentGameState === GameState.AWAITING_SERVE_TOSS || currentGameState === GameState.BALL_TOSSED) {
        // If ball is tossed (status 6 or 7) and hits floor before racket hit, it's a fault.
        if (currentGameState === GameState.BALL_TOSSED && gameBall.status === -2) {
            console.log("Rule Check: Tossed ball hit floor before being hit.");
            awardPointTo(servingPlayer === 1 ? 2 : 1); // Point to opponent
        }
        return;
    }

    // Rule Check 1: Ball hit the net structure and is now "dead" (status -1).
    if (gameBall.status === -1) { 
        console.log("Rule Check: Ball hit net structure during play.");
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); 
        return; 
    }
    // Rule Check 2: Ball hit the floor (status -2).
    if (gameBall.status === -2) { 
        console.log("Rule Check: Ball hit floor.");
        if (gameBall.lastHitBy === 1) { 
            if (gameBall.position.z < NET_POS_Z && gameBall.bouncedOnReceiverSide) awardPointTo(1); 
            else awardPointTo(2); 
        } else if (gameBall.lastHitBy === 2) { 
            if (gameBall.position.z > NET_POS_Z && gameBall.bouncedOnServerSide) awardPointTo(2);
            else awardPointTo(1); 
        } else { // Ball hit floor without a valid last hit (e.g., during a faulty toss that wasn't hit)
             awardPointTo(servingPlayer === 1 ? 2 : 1); // Point to opponent of current server
        }
        return; 
    }
    // Rule Check 3: Ball out of bounds (sideways or long; statuses -3, -4, -5).
    if (gameBall.status === -3 || gameBall.status === -4 || gameBall.status === -5) { 
        console.log(`Rule Check: Ball out of bounds (status ${gameBall.status}). Last hit by ${gameBall.lastHitBy}`);
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); 
        return; 
    }

    // --- Serve and Rally State Progression Logic ---
    if (currentGameState === GameState.SERVE_IN_MOTION) {
        // (Logic from Turn 41, ensure gameBall.status is 1 or 3 after hit)
        if (gameBall.lastHitBy === servingPlayer) {
            // Player 1 serving (ball status should be 1 after hit)
            if (servingPlayer === 1 && gameBall.status === 1) { 
                if (gameBall.bouncedOnServerSide && gameBall.bouncedOnReceiverSide) { 
                    console.log("Serve by P1 is IN. Rally begins (P2 to hit).");
                    currentGameState = GameState.RALLY; 
                    gameBall.status = 2; // P2's turn
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; 
                } else if (gameBall.bouncedOnReceiverSide && !gameBall.bouncedOnServerSide) { 
                    console.log("Serve Fault (P1): Hit opponent's side first.");
                    awardPointTo(2); 
                } 
                // Check for serve bounce on own side then out/net (fault)
                else if (gameBall.bouncedOnServerSide && !gameBall.bouncedOnReceiverSide && gameBall.status < 0){
                     console.log("Serve Fault (P1): Bounced on own side then out/net.");
                     awardPointTo(2);
                }
            } 
            // Player 2 (AI) serving (ball status should be 3 after hit)
            else if (servingPlayer === 2 && gameBall.status === 3) { 
                if (gameBall.bouncedOnReceiverSide && gameBall.bouncedOnServerSide) { // P2's side is receiver, P1's side is server for flag check
                    console.log("Serve by P2 is IN. Rally begins (P1 to hit).");
                    currentGameState = GameState.RALLY;
                    gameBall.status = 0; // P1's turn
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false;
                } else if (gameBall.bouncedOnServerSide && !gameBall.bouncedOnReceiverSide) { 
                    console.log("Serve Fault (P2): Hit opponent's side first.");
                    awardPointTo(1); 
                }
                 // Check for serve bounce on own side then out/net (fault)
                else if (gameBall.bouncedOnReceiverSide && !gameBall.bouncedOnServerSide && gameBall.status < 0){
                     console.log("Serve Fault (P2): Bounced on own side then out/net.");
                     awardPointTo(1);
                }
            }
        }
    } else if (currentGameState === GameState.RALLY) {
        // (Logic from Turn 41)
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
try {
    init(); 
    console.log("Three.js CannonSmash: Serve sequence updated.");
} catch (error) {
    // ... (error handling as before) ...
    console.error("Critical error during game initialization:", error);
    const initMessageElement = document.getElementById('initializationMessage');
    if (initMessageElement) {
        initMessageElement.textContent = "A critical error occurred. Could not start the game. Please check the console for details.";
        initMessageElement.style.color = 'red';
        initMessageElement.style.display = 'block'; 
    }
}
