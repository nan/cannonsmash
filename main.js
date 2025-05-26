// main.js - Correct auto-serve hit height
import * as THREE from 'three';     
import { Ball } from './Ball.js';     
import { Player } from './Player.js'; 

let scene, camera, renderer;            
let player1ScoreElement, player2ScoreElement; // Moved up for clarity
let table, net, floor;                  
let gameBall;                           
let player1, player2;                   

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
const RACKET_OFFSET_Z = 0.3; // From Player.js
const BALL_RADIUS = 0.02;    // From Ball.js
// RACKET_DEFAULT_Y from Player.js is relative to player, not used for world Y hit calc.

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
    const p1Material = new THREE.MeshStandardMaterial({ color: 0x0000dd, roughness: 0.6, transparent: true, opacity: 0.5 }); 
    const p2Material = new THREE.MeshStandardMaterial({ color: 0x00dd00, roughness: 0.6, transparent: true, opacity: 0.5 }); 
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

function updateScoreDisplay() { 
    if (player1ScoreElement && player2ScoreElement) { 
        player1ScoreElement.textContent = score.player1;
        player2ScoreElement.textContent = score.player2;
    }
}
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
    // From Turn 176 (based on Turn 111/113)
    if (player1 && player1.controlType === 'human') {
        if (currentGameState === GameState.AWAITING_SERVE_TOSS && servingPlayer === 1) {
            player1.serveToss(gameBall); 
            currentGameState = GameState.BALL_TOSSED;
            console.log("Player 1 tossed the ball. Auto-hit will occur.");
        } 
        else if (currentGameState === GameState.RALLY && gameBall.lastHitBy !== 1) {
            if (gameBall.status === 0 || (gameBall.status === 3 && gameBall.position.z > NET_POS_Z - 0.5) ) {
                 player1.swing(gameBall); 
            }
        }
    }
}
function resetForServe() {
    // From Turn 176 (based on Turn 111/113)
    gameBall.reset(servingPlayer); 
    currentGameState = GameState.AWAITING_SERVE_TOSS;
    if (player1 && servingPlayer === 1) { player1.isServing = false; }
    if (player2 && servingPlayer === 2) { player2.isServing = false; }
    console.log(`Ready for Player ${servingPlayer} to serve. P1 Click to toss. AI will auto-serve.`);
    if (servingPlayer === 2 && player2) { 
        console.log("AI (Player 2) is preparing to serve (position & toss)...");
        setTimeout(() => {
            if (currentGameState === GameState.AWAITING_SERVE_TOSS && servingPlayer === 2) {
                player2.positionBallRelativeToHand(gameBall); 
                player2.serveToss(gameBall);                 
                currentGameState = GameState.BALL_TOSSED; 
                console.log("AI (Player 2) tossed the ball. Auto-hit will occur.");
            }
        }, 1000 + Math.random() * 500); 
    }
}
function awardPointTo(winnerID) { 
    // From Turn 176 (based on Turn 111/113)
    if (winnerID === 1) score.player1++; else score.player2++;
    updateScoreDisplay(); 
    console.log(`Point for Player ${winnerID}! Score: P1: ${score.player1} - P2: ${score.player2}`);
    currentGameState = GameState.POINT_SCORED; 
    servingPlayer = (servingPlayer === 1) ? 2 : 1; 
    setTimeout(resetForServe, 1500); 
} 
function checkGameRules() { 
    // From Turn 176 (based on Turn 111/113)
    if (currentGameState === GameState.POINT_SCORED || currentGameState === GameState.AWAITING_SERVE_TOSS) {
        return; 
    }
    if (currentGameState === GameState.BALL_TOSSED && gameBall.status === -2) {
        console.log("Rule Check: Tossed ball hit floor before being hit by server.");
        awardPointTo(servingPlayer === 1 ? 2 : 1); 
        return;
    }
    if (currentGameState === GameState.BALL_TOSSED && (gameBall.status === 6 || gameBall.status === 7)) {
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

// MODIFIED: animate() loop for corrected auto-hit height
function animate() {
    requestAnimationFrame(animate); 
    
    if (player1) player1.update(gameBall); 
    if (player2) player2.update(gameBall); 

    if (currentGameState === GameState.AWAITING_SERVE_TOSS && gameBall) {
        if (servingPlayer === 1 && player1) {
            player1.positionBallRelativeToHand(gameBall); 
        } else if (servingPlayer === 2 && player2) {
            player2.positionBallRelativeToHand(gameBall);
        }
    }

    // Auto-hit logic for tossed ball
    if (currentGameState === GameState.BALL_TOSSED && gameBall && (gameBall.status === 6 || gameBall.status === 7)) {
        const server = (gameBall.status === 6) ? player1 : player2; 
        if (server && server.isServing) {
            // Corrected optimalHitWorldY to use fixed RACKET_WORLD_Y (0.9)
            const RACKET_WORLD_Y_FOR_HIT = 0.9; // Racket's actual fixed world Y (from Player.js constructor logic)

            if (gameBall.velocity.y < 0 && // Ball is falling
                Math.abs(gameBall.position.y - RACKET_WORLD_Y_FOR_HIT) < 0.05) { // Ball is near racket's actual fixed height

                let hitPosition = gameBall.position.clone(); 
                let targetOpponentBounceZ = (server.side === 1) ? -TABLE_LENGTH / 4 : TABLE_LENGTH / 4;
                let firstBounceServerZ = (server.side === 1) ? TABLE_LENGTH / 4 / 2 : -TABLE_LENGTH / 4 / 2; // Simplified first bounce target
                const targetOpponentBouncePos = new THREE.Vector3(0, TABLE_HEIGHT + BALL_RADIUS, targetOpponentBounceZ);
                const defaultServeSpin = new THREE.Vector2(0, 2.5);

                const calculatedVelocity = gameBall.calculatePerfectServeVelocity(hitPosition, server.side, firstBounceServerZ, targetOpponentBouncePos, defaultServeSpin);
                
                if (server.swing(gameBall, calculatedVelocity, defaultServeSpin)) { 
                    currentGameState = GameState.SERVE_IN_MOTION;
                    console.log(`Player ${servingPlayer} auto-hit the serve at correct height!`); // Updated log
                } else {
                    console.log(`Player ${servingPlayer} auto-serve hit missed/failed at correct height.`);
                }
            }
        }
    }

    if (gameBall) {
        if (gameBall.status !== 8 && gameBall.status !== 9) { 
            gameBall.update(player1.racket.mesh, player2 ? player2.racket.mesh : null); 
        } else if (gameBall.mesh) { 
            gameBall.mesh.position.copy(gameBall.position);
        }
    }
    
    checkGameRules(); 
    renderer.render(scene, camera); 
}

try {
    init(); 
    console.log("Three.js CannonSmash: Corrected auto-serve hit height.");
} catch (error) { 
    console.error("Critical error during game initialization:", error);
    const initMessageElement = document.getElementById('initializationMessage');
    if (initMessageElement) {
        initMessageElement.textContent = "A critical error occurred. Could not start the game. Please check the console for details.";
        initMessageElement.style.color = 'red';
        initMessageElement.style.display = 'block'; 
    }
}
