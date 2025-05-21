// main.js - Main script for the CannonSmash Web application

// --- Module Imports ---
import * as THREE from 'three';     // Core Three.js library for 3D graphics.
import { Ball } from './Ball.js';     // Custom Ball class: handles ball physics, state, and its 3D mesh.
import { Player } from './Player.js'; // Custom Player class: handles player (human/AI) state, controls, racket, and its 3D mesh.

// --- Global Variables ---
// These variables store core components and game objects, accessible throughout the script.

// Three.js Core Components
let scene, camera, renderer;            // `scene`: container for all 3D objects. `camera`: viewpoint. `renderer`: draws the scene.

// Game Environment Meshes
let table, net, floor;                  // `THREE.Mesh` objects representing the static parts of the game environment.

// Game Logic Objects
let gameBall;                           // Instance of the `Ball` class, managing the ball's behavior.
let player1, player2;                   // Instances of the `Player` class; `player1` is human, `player2` is AI.

// UI Elements
let player1ScoreElement, player2ScoreElement; // References to HTML `<span>` elements for displaying scores.

// Game State Management
// `GameState` defines the possible states of the game, controlling behavior and rule application.
const GameState = {
    PRE_SERVE: 'PRE_SERVE',             // State: Ball is positioned, waiting for the serving player to initiate the serve hit.
    SERVE_IN_MOTION: 'SERVE_IN_MOTION', // State: Player has hit the serve, ball is in flight; specific serve rules apply.
    RALLY: 'RALLY',                     // State: Ball is in play after a valid serve; rally rules apply.
    POINT_SCORED: 'POINT_SCORED',       // State: A point has just been scored; game pauses briefly before setting up for next serve.
    GAME_OVER: 'GAME_OVER'              // State: (Future Use) Game has concluded.
};
let currentGameState = GameState.PRE_SERVE; // Variable holding the current active game state. Initialized to PRE_SERVE.
let servingPlayer = 1;                      // Variable indicating which player is currently set to serve (1 for Player 1, 2 for Player 2).
let score = { player1: 0, player2: 0 };     // Object storing the current scores for Player 1 and Player 2.

// Physical Constants (values should ideally align with those in Ball.js and Player.js if they represent the same physical entities)
// These are used for positioning and some game logic directly within main.js.
const TABLE_LENGTH = 2.74; // Length of the table (meters), primarily along Z-axis.
const TABLE_HEIGHT = 0.76; // Height of the table surface from the floor (meters).
const NET_POS_Z = 0;       // Z-coordinate of the net's center line, dividing the table.

// --- Initialization Function (`init`) ---
// This function is called once when the script loads to set up the entire game environment.
function init() {
    // 1. Scoreboard UI Setup: Get references to HTML elements for score display.
    player1ScoreElement = document.getElementById('player1Score');
    player2ScoreElement = document.getElementById('player2Score');
    updateScoreDisplay(); // Initialize the scoreboard display (e.g., to "0 | 0").

    // 2. Three.js Scene Creation: The scene is the root container for all 3D objects.
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0xaaaaaa); // Set a light grey background color for better visibility.

    // 3. Camera Setup: Defines the viewpoint into the 3D world.
    camera = new THREE.PerspectiveCamera(
        75,                                     // Field of View (FOV) in degrees.
        window.innerWidth / window.innerHeight, // Aspect ratio, based on browser window size.
        0.1,                                    // Near clipping plane (objects closer than this won't be rendered).
        1000                                    // Far clipping plane (objects further than this won't be rendered).
    );
    camera.position.set(0, 2.0, 3.0); // Position camera slightly above and behind Player 1's side for a good overview.
    camera.lookAt(0, 0.5, 0);         // Direct the camera to look towards the center of the table area.

    // 4. WebGL Renderer Setup: Handles drawing the scene to the HTML canvas.
    renderer = new THREE.WebGLRenderer({ antialias: true }); // Enable anti-aliasing for smoother object edges.
    renderer.setSize(window.innerWidth, window.innerHeight); // Set renderer size to fill the browser window.
    document.body.appendChild(renderer.domElement); // Append the renderer's canvas element to the HTML document body.

    // 5. Lighting Setup: Illuminates the scene.
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.7); // Soft white ambient light for overall, non-directional illumination.
    scene.add(ambientLight);
    const directionalLight = new THREE.DirectionalLight(0xffffff, 1.0); // Directional light to simulate a primary light source (like the sun).
    directionalLight.position.set(-4, 6, 4); // Position the light to cast from an angle.
    directionalLight.lookAt(0,0,0);          // Make the light point towards the center of the scene.
    scene.add(directionalLight);

    // 6. Material Definitions: Define appearance (color, texture, reflectivity) for game objects.
    // Standard materials are affected by light. `roughness` controls matte/glossy, `metalness` controls metallic appearance.
    const tableMaterial = new THREE.MeshStandardMaterial({ color: 0x006400, roughness: 0.8, metalness: 0.2 }); // Dark green, fairly matte table.
    const netMaterial = new THREE.MeshStandardMaterial({ color: 0x333333, transparent: true, opacity: 0.8, roughness: 0.9 }); // Dark grey, slightly transparent, matte net.
    const floorMaterial = new THREE.MeshStandardMaterial({ color: 0x808080, roughness: 0.9 }); // Grey, very matte floor.
    const ballMaterial = new THREE.MeshStandardMaterial({ color: 0xffa500, roughness: 0.5, metalness: 0.1 }); // Orange ball, slightly shiny.
    const p1Material = new THREE.MeshStandardMaterial({ color: 0x0000dd, roughness: 0.6 }); // Blue material for Player 1.
    const p2Material = new THREE.MeshStandardMaterial({ color: 0x00dd00, roughness: 0.6 }); // Green material for Player 2 (AI).

    // 7. Environment Object Creation: Create and position the table, net, and floor.
    // Table
    const tableTopGeometry = new THREE.BoxGeometry(1.525, 0.03, 2.74); // Dimensions: X (width), Y (thickness), Z (length).
    table = new THREE.Mesh(tableTopGeometry, tableMaterial);
    table.position.set(0, TABLE_HEIGHT - 0.03 / 2, 0); // Position so the top surface is at `TABLE_HEIGHT`.
    scene.add(table);

    // Net
    const netGeometry = new THREE.BoxGeometry(1.83, 0.1525, 0.01); // Dimensions: X (width), Y (height), Z (thickness).
    net = new THREE.Mesh(netGeometry, netMaterial);
    net.position.set(0, TABLE_HEIGHT + 0.1525 / 2, 0); // Position on top of the table, centered at Z=0.
    scene.add(net);

    // Floor
    const floorGeometry = new THREE.PlaneGeometry(10, 10); // A large plane for the floor.
    floor = new THREE.Mesh(floorGeometry, floorMaterial);
    floor.rotation.x = -Math.PI / 2; // Rotate the plane to be horizontal.
    floor.position.y = 0;            // Position the floor at Y=0.
    scene.add(floor);

    // 8. Game Object Instantiation: Create the ball and player instances.
    // Ball
    gameBall = new Ball(); // Create an instance of the Ball class (handles physics).
    const ballGeometry = new THREE.SphereGeometry(gameBall.radius, 16, 12); // Visual sphere for the ball.
    const ballMesh = new THREE.Mesh(ballGeometry, ballMaterial);
    gameBall.mesh = ballMesh; // Link the physics object (`gameBall`) to its Three.js mesh.
    scene.add(ballMesh);      // Add the ball's mesh to the scene.

    // Player 1 (Human)
    player1 = new Player(1, scene, 'human'); // Player on side 1 (positive Z), controlled by human.
    const player1Geometry = new THREE.CapsuleGeometry(0.2, 0.8, 4, 16); // Visual capsule for the player.
    player1.mesh = new THREE.Mesh(player1Geometry, p1Material);
    player1.mesh.position.copy(player1.position); // Set initial visual position from Player class logic.
    scene.add(player1.mesh);
    player1.mesh.add(player1.racket.mesh); // Attach the racket's mesh as a child of the player's mesh for relative movement.

    // Player 2 (AI)
    player2 = new Player(-1, scene, 'ai'); // Player on side -1 (negative Z), controlled by AI.
    const player2Geometry = new THREE.CapsuleGeometry(0.2, 0.8, 4, 16);
    player2.mesh = new THREE.Mesh(player2Geometry, p2Material);
    player2.mesh.position.copy(player2.position);
    scene.add(player2.mesh);
    player2.mesh.add(player2.racket.mesh);

    // 9. Initial Game Setup
    resetForServe(); // Prepare the game for the first serve.

    // 10. Event Listener Setup: Handle user input and browser events.
    window.addEventListener('resize', onWindowResize, false);   // Adjust scene on window resize.
    window.addEventListener('mousemove', onMouseMove, false); // Track mouse movement for player control.
    window.addEventListener('click', onMouseClick, false);    // Handle mouse clicks for serves/swings.

    // 11. Start Animation Loop: Begins the continuous rendering and game update cycle.
    animate();
}

// --- UI Functions ---
/**
 * Updates the HTML scoreboard elements (`player1ScoreElement`, `player2ScoreElement`)
 * with the current values from the `score` object.
 */
function updateScoreDisplay() {
    if (player1ScoreElement && player2ScoreElement) { // Check if elements are successfully referenced.
        player1ScoreElement.textContent = score.player1;
        player2ScoreElement.textContent = score.player2;
    }
}

// --- Event Handlers ---
/**
 * Handles the 'resize' event of the browser window.
 * Adjusts the camera's aspect ratio and the renderer's size to match the new window dimensions.
 */
function onWindowResize() { 
    camera.aspect = window.innerWidth / window.innerHeight; // Update aspect ratio.
    camera.updateProjectionMatrix(); // Apply the change to the camera's projection.
    renderer.setSize(window.innerWidth, window.innerHeight); // Resize the renderer's canvas.
}

/**
 * Handles 'mousemove' events for human player (Player 1) control.
 * Converts mouse screen coordinates to normalized values and passes them to Player 1's control handler.
 * @param {MouseEvent} event - The DOM MouseEvent object containing cursor coordinates.
 */
function onMouseMove(event) { 
    // Ensure this control is only for Player 1 and if Player 1 is human-controlled.
    if (player1 && player1.controlType === 'human') { 
        // Normalize mouse X coordinate: (clientX / windowWidth) * 2 - 1 results in a range from -1 (left) to 1 (right).
        const screenX = (event.clientX / window.innerWidth) * 2 - 1;
        // Normalize mouse Y coordinate: -(clientY / windowHeight) * 2 + 1 results in a range from -1 (bottom) to 1 (top).
        // The Y-axis is typically inverted because screen Y is 0 at top, while normalized Y is often 0 at center, +1 at top.
        const screenY = -(event.clientY / window.innerHeight) * 2 + 1; 
        player1.handleMouseMove(screenX, screenY); // Pass normalized coordinates to player object.
    }
}

/**
 * Handles 'click' events, primarily used for Player 1 to initiate a serve or swing during a rally.
 * @param {MouseEvent} event - The DOM MouseEvent object.
 */
function onMouseClick(event) {
    // Action 1: Player 1 serves the ball.
    // Conditions: Player 1 exists, game is in PRE_SERVE state, and it's Player 1's turn to serve.
    if (player1 && currentGameState === GameState.PRE_SERVE && servingPlayer === 1) {
        const racketWorldPos = new THREE.Vector3();
        player1.racket.mesh.getWorldPosition(racketWorldPos); // Get current world position of P1's racket.
        
        // Position the ball slightly in front of the racket for the serve action.
        gameBall.position.copy(racketWorldPos);
        // `player1.side` is 1. Racket's default orientation has its "front" facing towards -Z.
        const forwardOffset = new THREE.Vector3(0, 0, player1.side * -0.12); // Offset ball slightly in front.
        gameBall.position.add(forwardOffset);
        gameBall.position.y += gameBall.radius * 0.5; // Elevate ball slightly above racket's vertical center.

        // Define initial velocity and spin for the serve with some randomness.
        let serveVelocity = new THREE.Vector3(
            (Math.random() - 0.5) * 1.5,  // Sideways velocity component.
            0.8 + (Math.random() * 0.4),  // Upward velocity component.
            -3.0 - (Math.random() * 0.8)  // Forward velocity component (towards -Z for Player 1).
        );
        let serveSpin = new THREE.Vector2(
            (Math.random() - 0.5) * 3,    // Sidespin component.
            1.5 + Math.random() * 3     // Topspin/backspin component (mostly topspin).
        );
        
        gameBall.hit(serveVelocity, serveSpin, 1); // Player 1 hits the ball (updates ball's physics state).
        currentGameState = GameState.SERVE_IN_MOTION; // Transition game state to SERVE_IN_MOTION.
        console.log("Player 1 serves!");

    // Action 2: Player 1 swings during a rally.
    // Conditions: Player 1 exists, game is in RALLY state, and it's Player 1's turn to hit.
    } else if (player1 && currentGameState === GameState.RALLY && gameBall.lastHitBy !== 1) {
        // It's P1's turn if ball was last hit by P2 (gameBall.lastHitBy === 2), or if ball.status indicates P1 to play.
        // `gameBall.status`: 0 (P1 to hit), 3 (P2 hit, flying towards P1).
        // `NET_POS_Z - 0.5`: check if ball is reasonably on P1's side or approaching.
        if (gameBall.status === 0 || (gameBall.status === 3 && gameBall.position.z > NET_POS_Z - 0.5) ) {
             player1.swing(); // Player 1 performs a swing action (visual + potential collision trigger).
        }
    }
}

// --- Game Logic Functions ---
/**
 * Resets the ball and game state to prepare for a new serve by the current `servingPlayer`.
 * This function positions the ball correctly and sets the game state to `PRE_SERVE`.
 * If the AI is serving, it schedules an automatic serve action.
 */
function resetForServe() {
    gameBall.reset(servingPlayer); // Call Ball class's reset method, which positions ball and sets its status (8 or 9).
    currentGameState = GameState.PRE_SERVE; // Set the main game state.
    console.log(`Ready for Player ${servingPlayer} to serve.`);

    // If Player 2 (AI) is serving, automate its serve action after a short delay.
    if (servingPlayer === 2 && player2) { 
        console.log("AI (Player 2) is preparing to serve...");
        // Use setTimeout to simulate AI "thinking" or preparation time.
        setTimeout(() => {
            // Double-check game state as a safeguard, in case state changed rapidly (e.g., due to quick successive points).
            if (currentGameState === GameState.PRE_SERVE && servingPlayer === 2) {
                const racketWorldPos = new THREE.Vector3();
                player2.racket.mesh.getWorldPosition(racketWorldPos); // Get AI racket's position.
                
                // Position ball slightly in front of AI's racket.
                gameBall.position.copy(racketWorldPos);
                // `player2.side` is -1. Racket's default "front" faces towards +Z.
                const forwardOffset = new THREE.Vector3(0, 0, player2.side * -0.12); // Offset for P2.
                gameBall.position.add(forwardOffset);
                gameBall.position.y += gameBall.radius * 0.5;

                // Define velocity and spin for AI's serve.
                let serveVelocity = new THREE.Vector3(
                    (Math.random() - 0.5) * 1.5,
                    0.8 + (Math.random() * 0.4),
                    3.0 + (Math.random() * 0.8)  // Forward component (towards +Z for Player 2, i.e., Player 1's side).
                );
                let serveSpin = new THREE.Vector2((Math.random() - 0.5) * 3, 1.5 + Math.random() * 3);
                
                gameBall.hit(serveVelocity, serveSpin, 2); // Player 2 (AI) hits the ball.
                currentGameState = GameState.SERVE_IN_MOTION; // Update game state.
                console.log("Player 2 (AI) serves!");
            }
        }, 1000 + Math.random() * 500); // Randomized delay (1 to 1.5 seconds) for AI serve.
    }
}

/**
 * Awards a point to the specified player, updates the score, and sets up for the next serve.
 * @param {number} winnerID - The ID of the player who won the point (1 or 2).
 */
function awardPointTo(winnerID) { 
    if (winnerID === 1) {
        score.player1++;
    } else { // winnerID === 2
        score.player2++;
    }
    updateScoreDisplay(); // Update the visual scoreboard on the HTML page.
    console.log(`Point for Player ${winnerID}! Score: P1: ${score.player1} - P2: ${score.player2}`);
    currentGameState = GameState.POINT_SCORED; // Set state to indicate a point has been resolved.
    
    // Alternate the serving player for the next point.
    servingPlayer = (servingPlayer === 1) ? 2 : 1; 
    // Schedule `resetForServe` to be called after a delay, allowing players to see the score/outcome.
    setTimeout(resetForServe, 1500); // Wait 1.5 seconds before setting up the next serve.
} 

/**
 * Checks the current game rules based on the ball's status, position, and bounce history.
 * This function determines if a point should be awarded or if the game state should change (e.g., serve to rally).
 * It's called continuously from the `animate` loop.
 */
function checkGameRules() { 
    // Skip rule checking if a point has just been scored or if game is waiting for serve input.
    if (currentGameState === GameState.POINT_SCORED || currentGameState === GameState.PRE_SERVE) return;

    // Rule Check 1: Ball hit the net structure and is now "dead" (status -1).
    if (gameBall.status === -1) { 
        console.log("Rule Check: Ball hit net structure during play.");
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); // Point to the opponent of the player who last hit the ball.
        return; // Exit further rule checks for this frame.
    }

    // Rule Check 2: Ball hit the floor (status -2).
    if (gameBall.status === -2) { 
        console.log("Rule Check: Ball hit floor.");
        if (gameBall.lastHitBy === 1) { // Player 1 hit the ball last.
            // If ball landed on P2's side (negative Z) AND had a valid bounce there, P1 scores.
            // Otherwise, P2 scores (P1 hit it into own floor, or P1 hit it to P2's floor without valid bounce).
            if (gameBall.position.z < NET_POS_Z && gameBall.bouncedOnReceiverSide) awardPointTo(1); 
            else awardPointTo(2); 
        } else if (gameBall.lastHitBy === 2) { // Player 2 (AI) hit the ball last.
            // If ball landed on P1's side (positive Z) AND had a valid bounce there, P2 scores.
            if (gameBall.position.z > NET_POS_Z && gameBall.bouncedOnServerSide) awardPointTo(2);
            else awardPointTo(1); 
        } else { // Ball hit floor without being properly hit by a player (e.g., a bad serve directly to floor).
            if (gameBall.position.z > NET_POS_Z) awardPointTo(2); // Landed on P1's side -> P2 gets point.
            else awardPointTo(1); // Landed on P2's side -> P1 gets point.
        }
        return; // Exit further rule checks.
    }
    
    // Rule Check 3: Ball went out of bounds (sideways or long; statuses -3, -4, -5).
    if (gameBall.status === -3 || gameBall.status === -4 || gameBall.status === -5) { 
        console.log(`Rule Check: Ball out of bounds (status ${gameBall.status}). Last hit by ${gameBall.lastHitBy}`);
        awardPointTo(gameBall.lastHitBy === 1 ? 2 : 1); // Point to the opponent of the player who last hit the ball out.
        return; // Exit further rule checks.
    }

    // --- Serve and Rally State Progression Logic ---
    // This logic transitions the game from SERVE_IN_MOTION to RALLY if a serve is valid.
    // It also updates ball status during a rally to indicate whose turn it is.
    if (currentGameState === GameState.SERVE_IN_MOTION) {
        // `gameBall.lastHitBy` should be the current `servingPlayer`.
        // `gameBall.status` indicates flight direction (1 for P1->P2, 3 for P2->P1).
        if (gameBall.lastHitBy === servingPlayer) {
            if (servingPlayer === 1 && gameBall.status === 1) { // Player 1 is serving.
                // Valid Serve Condition: Ball bounced on server's side (P1's side) then on receiver's side (P2's side).
                if (gameBall.bouncedOnServerSide && gameBall.bouncedOnReceiverSide) { 
                    console.log("Serve by P1 is IN. Rally begins (P2 to hit).");
                    currentGameState = GameState.RALLY; // Transition to rally state.
                    gameBall.status = 2; // Set ball status to indicate it's Player 2's turn to hit.
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; // Reset bounce flags for the rally.
                } else if (gameBall.bouncedOnReceiverSide && !gameBall.bouncedOnServerSide) { // Fault: Serve hit receiver's side first.
                    console.log("Serve Fault (P1): Hit opponent's side first.");
                    awardPointTo(2); // Point to Player 2.
                } 
                // Other serve faults (e.g., net, out, double bounce on own side before crossing) are caught by negative status codes above.
            } else if (servingPlayer === 2 && gameBall.status === 3) { // Player 2 (AI) is serving.
                 // Valid Serve for P2: Bounced on P2's side ("receiver" flag for P2 logic in Ball.js) then P1's side ("server" flag for P2 logic).
                 // Note: `bouncedOnServerSide` in Ball.js means positive Z, `bouncedOnReceiverSide` means negative Z.
                 // So for P2 serving, it should be `bouncedOnReceiverSide` (P2's actual side) then `bouncedOnServerSide` (P1's actual side).
                if (gameBall.bouncedOnReceiverSide && gameBall.bouncedOnServerSide) { 
                    console.log("Serve by P2 is IN. Rally begins (P1 to hit).");
                    currentGameState = GameState.RALLY;
                    gameBall.status = 0; // Set ball status to indicate it's Player 1's turn to hit.
                    gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; // Reset bounce flags.
                } else if (gameBall.bouncedOnServerSide && !gameBall.bouncedOnReceiverSide) { // Fault: Serve hit P1's side first.
                    console.log("Serve Fault (P2): Hit opponent's side first.");
                    awardPointTo(1); // Point to Player 1.
                }
            }
        }
    } else if (currentGameState === GameState.RALLY) {
        // Logic for progressing a rally after a valid hit.
        // `gameBall.status`: 0 (P1 to hit), 1 (P1 hit, flying), 2 (P2 to hit), 3 (P2 hit, flying).
        if (gameBall.lastHitBy === 1 && gameBall.status === 1) { // Player 1 just hit the ball during a rally.
            if (gameBall.bouncedOnReceiverSide) { // Ball successfully bounced on Player 2's side.
                console.log("P1's rally shot is IN. P2 to play.");
                gameBall.status = 2; // It's now Player 2's turn to hit.
                gameBall.bouncedOnServerSide = false; gameBall.bouncedOnReceiverSide = false; // Reset bounce flags for P2's return.
            }
            // If it doesn't bounce on P2's side and goes out/floor/net, negative statuses will handle it.
        }
        else if (gameBall.lastHitBy === 2 && gameBall.status === 3) { // Player 2 (AI) just hit the ball during a rally.
            if (gameBall.bouncedOnServerSide) { // Ball successfully bounced on Player 1's side.
                console.log("P2's rally shot is IN. P1 to play.");
                gameBall.status = 0; // It's now Player 1's turn to hit.
                gameBall.bouncedOnReceiverSide = false; gameBall.bouncedOnServerSide = false; // Reset bounce flags for P1's return.
            }
        }
        // Faults like hitting net, floor, or out during rally are caught by negative statuses handled earlier.
        // A "double bounce" fault (e.g., ball status is 0 (P1 to hit) but it bounces on P1's side again before P1 hits it)
        // would typically result in the ball hitting the floor (status -2) on P1's side without P1 being lastHitBy, leading to P2 point.
    }
}

// --- Main Animation Loop (`animate`) ---
// This function is called repeatedly by `requestAnimationFrame`, creating the game loop.
// It updates game logic and re-renders the scene for each frame.
function animate() {
    requestAnimationFrame(animate); // Schedule `animate` to be called again on the next browser frame.
    
    // 1. Update Player Logic: Handle human input processing or AI decision-making.
    if (player1) player1.update(gameBall); // Player 1 updates (mouse input, racket lerp).
    if (player2) player2.update(gameBall); // Player 2 (AI) updates (AI logic, racket lerp).

    // 2. Update Ball Physics: Calculate ball's new position, handle collisions.
    if (gameBall) {
        // Pass racket meshes for collision detection.
        gameBall.update(player1.racket.mesh, player2 ? player2.racket.mesh : null); 
    }
    
    // 3. Check Game Rules: Apply table tennis rules, award points, change states.
    checkGameRules(); 
    
    // 4. Render the Scene: Draw the updated scene from the camera's perspective.
    renderer.render(scene, camera); 
}

// --- Entry Point ---
// The `init()` function is called here to set up and start the game when the script is executed.
init(); 
console.log("Three.js CannonSmash initialized. Code refined with comments for main.js.");
