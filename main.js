// main.js - Adjust P1 initial serve ball position
import * as THREE from 'three';     
import { Ball } from './Ball.js';     
import { Player } from './Player.js'; 

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
const RACKET_OFFSET_Z = 0.3; // From Player.js, used for AI serve ball pos & P1 racket align

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
    if (player1 && currentGameState === GameState.PRE_SERVE && servingPlayer === 1) {
        // Position ball relative to Player 1's body (mesh), to their right.
        const ballServeRelativePos = new THREE.Vector3(
            player1.side * 0.20,  // To Player 1's right (player1.side is 1)
            0.1,                  // Relative Y to player mesh origin (player mesh Y is 0.4, so ball world Y is 0.5)
            player1.side * -0.20  // Slightly in front of player's center
        );
        const ballServePosition = player1.mesh.localToWorld(ballServeRelativePos.clone());
        gameBall.position.copy(ballServePosition);
        
        // Make racket target this ball position initially for the visual
        // Racket position is relative to player mesh.
        const racketTargetLocal = player1.mesh.worldToLocal(ballServePosition.clone()); // Convert ball's world pos to player's local
        racketTargetLocal.z += player1.side * -RACKET_OFFSET_Z * 0.5; // Racket slightly behind ball's center
        racketTargetLocal.y -= 0.05; // Racket slightly below ball's center to "cup" it
        player1.racket.targetPosition.copy(racketTargetLocal);
        player1.racket.currentPosition.copy(racketTargetLocal); // Snap racket to this position for serve setup
        if(player1.racket.mesh) player1.racket.mesh.position.copy(player1.racket.currentPosition);


        // Consistent serve velocity and spin (logic from Turn 79 is fine)
        let serveVelocity = new THREE.Vector3(
            (Math.random() - 0.5) * 0.5,  
            0.9 + (Math.random() * 0.2),  
            -3.0 - (Math.random() * 0.3)  
        );
        let serveSpin = new THREE.Vector2(
            (Math.random() - 0.5) * 1.0, 
            2.5 
        ); 
        
        gameBall.hit(serveVelocity, serveSpin, 1); 
        currentGameState = GameState.SERVE_IN_MOTION; 
        console.log("Player 1 serves (ball positioned to player's right)!");

    } else if (player1 && currentGameState === GameState.RALLY && gameBall.lastHitBy !== 1) {
        if (gameBall.status === 0 || (gameBall.status === 3 && gameBall.position.z > NET_POS_Z - 0.5) ) {
             player1.swing(gameBall); 
        }
    }
}

function resetForServe() {
    gameBall.reset(servingPlayer); 
    currentGameState = GameState.PRE_SERVE;
    
    // Note: Ball positioning for P1 is now done on the click in PRE_SERVE state.
    // Ball positioning for P2 is done in its AI serve logic.
    if (player1 && servingPlayer === 1) { /* player1.isServing = false; */ } // isServing not in this Player.js
    if (player2 && servingPlayer === 2) { /* player2.isServing = false; */ }

    console.log(`Ready for Player ${servingPlayer} to serve. P1 Click to serve. AI will auto-serve.`);

    if (servingPlayer === 2 && player2) { 
        console.log("AI (Player 2) is preparing to serve...");
        setTimeout(() => {
            if (currentGameState === GameState.PRE_SERVE && servingPlayer === 2) {
                // Position ball relative to AI player body, slightly to its forehand (AI is side -1, so its right is -X)
                const ballServeRelativePosAI = new THREE.Vector3(
                    player2.side * 0.20, // To AI's "right" (AI's perspective, side is -1, so -0.20 is to its right)
                    0.1,                 // Relative Y to player mesh origin
                    player2.side * -0.20 // Slightly in front
                );
                const ballServePositionAI = player2.mesh.localToWorld(ballServeRelativePosAI.clone());
                gameBall.position.copy(ballServePositionAI);

                // Snap AI racket to near the ball as well
                const racketTargetLocalAI = player2.mesh.worldToLocal(ballServePositionAI.clone());
                racketTargetLocalAI.z += player2.side * -RACKET_OFFSET_Z * 0.5; 
                racketTargetLocalAI.y -= 0.05; 
                player2.racket.targetPosition.copy(racketTargetLocalAI);
                player2.racket.currentPosition.copy(racketTargetLocalAI);
                if(player2.racket.mesh) player2.racket.mesh.position.copy(player2.racket.currentPosition);


                let serveVelocity = new THREE.Vector3(
                    (Math.random() - 0.5) * 1.5,
                    0.8 + (Math.random() * 0.4),
                    player2.side * (-3.0 - (Math.random() * 0.5)) 
                );
                let serveSpin = new THREE.Vector2((Math.random() - 0.5) * 3, 1.5 + Math.random() * 3);
                
                gameBall.hit(serveVelocity, serveSpin, 2); 
                currentGameState = GameState.SERVE_IN_MOTION; 
                console.log("Player 2 (AI) serves (ball positioned to AI's right)!");
            }
        }, 1000 + Math.random() * 500); 
    }
}

function awardPointTo(winnerID) { 
    if (winnerID === 1) score.player1++; else score.player2++;
    updateScoreDisplay(); 
    console.log(`Point for Player ${winnerID}! Score: P1: ${score.player1} - P2: ${score.player2}`);
    currentGameState = GameState.POINT_SCORED; 
    servingPlayer = (servingPlayer === 1) ? 2 : 1; 
    setTimeout(resetForServe, 1500); 
} 
function checkGameRules() { 
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
    console.log("Three.js CannonSmash: Adjusted P1 initial serve ball position.");
} catch (error) { 
    console.error("Critical error during game initialization:", error);
    const initMessageElement = document.getElementById('initializationMessage');
    if (initMessageElement) {
        initMessageElement.textContent = "A critical error occurred. Could not start the game. Please check the console for details.";
        initMessageElement.style.color = 'red';
        initMessageElement.style.display = 'block'; 
    }
}
