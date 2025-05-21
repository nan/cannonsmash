// Player.js
import * as THREE from 'three';

// --- Constants for Player positioning and behavior ---
const TABLE_LENGTH = 2.74;      // Length of the table (along Z-axis), used for positioning player relative to table.
const TABLE_WIDTH = 1.525;      // Width of the table (along X-axis), used for player's lateral movement range.
const PLAYER_Z_OFFSET = 0.2;    // Default distance the player stands behind their respective table edge.
const RACKET_OFFSET_Z = 0.3;    // How far in front of the player's body center the racket is typically held (depth).
const RACKET_DEFAULT_Y = 0.2;   // Default height of the racket relative to the player's base/origin (center of the capsule mesh).
const NET_POS_Z = 0;            // Z-coordinate of the net's center line, used by AI to determine ball's side.

export class Player {
    /**
     * Constructor for a Player object.
     * @param {number} side - Which side of the table the player is on: 
     *                        1 for positive Z side (typically human player), 
     *                       -1 for negative Z side (typically AI player).
     * @param {THREE.Scene} scene - The Three.js scene object. Currently passed but not directly used for adding/removing objects by Player itself.
     * @param {string} controlType - Specifies the control mechanism for the player:
     *                             'human' for mouse-controlled player, 
     *                             'ai' for AI-controlled player.
     */
    constructor(side = 1, scene, controlType = 'human') {
        this.side = side;       // Player's assigned side of the table (1 or -1).
        this.scene = scene;     // Reference to the Three.js scene (can be used for context or future extensions).
        this.controlType = controlType; // Determines if player is controlled by 'human' input or 'ai' logic.

        // Logical base position of the player (center of their capsule mesh).
        // This position is calculated based on table dimensions and player's side.
        this.position = new THREE.Vector3(
            0,                                              // Initially centered along the table's width (X-axis).
            0.76,                                           // Base height of the player (e.g., average hip height if mesh origin is at feet).
                                                            // Assuming player capsule origin is at its center, this sets it on the floor.
                                                            // If capsule origin is at bottom, it should be 0.
                                                            // Given current main.js, capsule is 0.8 high, so 0.76 is likely for a centered origin.
            (TABLE_LENGTH / 2 + PLAYER_Z_OFFSET) * this.side // Positioned behind their respective table half along Z-axis.
        );
        this.mesh = null; // Reference to the Three.js Mesh for the player's visual body (e.g., a capsule).

        // Racket related properties.
        this.racket = {
            mesh: null,         // Three.js Mesh object representing the racket.
            // Target position for the racket, relative to the player's body/mesh origin. This is where the racket aims to be.
            targetPosition: new THREE.Vector3(0, RACKET_DEFAULT_Y, this.side * -RACKET_OFFSET_Z), 
            // Current actual position of the racket, relative to player's body/mesh origin. Used for smooth visual interpolation (lerping).
            currentPosition: new THREE.Vector3(0, RACKET_DEFAULT_Y, this.side * -RACKET_OFFSET_Z),
            // Interpolation factor (lerpFactor) for smoothing racket movement. Smaller values mean smoother/slower follow.
            // AI racket can be slightly more responsive (higher lerpFactor) or smoother (lower) based on desired behavior.
            lerpFactor: this.controlType === 'human' ? 0.2 : 0.15 
        };
        this.createRacketMesh(); // Create and initialize the racket's visual mesh.

        // Mouse screen coordinates (normalized from -1 to 1) for human player control.
        this.mouseScreenX = 0;  // Stores normalized mouse X position.
        this.mouseScreenY = 0;  // Stores normalized mouse Y position.

        // Player's lateral movement range (along X-axis, relative to table center).
        // AI might have a slightly reduced range for simpler behavior modeling or to simulate limitations.
        const xRangeMultiplier = this.controlType === 'human' ? 0.9 : 0.8;
        this.minX = -TABLE_WIDTH / 2 * xRangeMultiplier; // Minimum X position.
        this.maxX =  TABLE_WIDTH / 2 * xRangeMultiplier; // Maximum X position.
        
        // --- AI-specific properties ---
        // These properties are primarily used when `controlType` is 'ai'.
        this.aiHitting = false; // Boolean flag: true if the AI is currently executing a "hit" action/animation.
        // this.aiReactionTime = 0; // Potential property for future AI: time delay before AI reacts to ball. (Not currently implemented).
        // this.aiTargetBallX = 0; // Potential property for AI: predicted X-coordinate of the ball at intercept. (Not directly used by current simple AI).
        // this.aiTargetRacketY = RACKET_DEFAULT_Y; // AI's desired racket Y position, dynamically updated in `updateAI`.
    }

    /**
     * Creates the Three.js mesh for the player's racket.
     * Defines geometry, material (color-coded by control type), and initial relative position.
     */
    createRacketMesh() {
        // Racket geometry: a thin box. Dimensions: thickness, height, width.
        const racketGeometry = new THREE.BoxGeometry(0.02, 0.22, 0.16); // Made racket face slightly larger
        // Racket material: Standard material with color based on player type.
        const racketMaterial = new THREE.MeshStandardMaterial({ 
            color: this.controlType === 'human' ? 0xcc0000 : 0x00cc00, // Human player: Red, AI player: Green.
            roughness: 0.7, // Controls shininess; higher is more matte.
            metalness: 0.3  // Controls metallic appearance; lower is more dielectric.
        });
        this.racket.mesh = new THREE.Mesh(racketGeometry, racketMaterial);
        // Set the initial actual position of the racket mesh based on its defined `currentPosition` (relative to player).
        this.racket.mesh.position.copy(this.racket.currentPosition); 
    }

    /**
     * Handles mouse movement input for human-controlled players.
     * Updates the player's body X position and the racket's target X and Y position based on normalized mouse screen coordinates.
     * @param {number} screenX - Normalized mouse X position (from -1 at left edge to 1 at right edge).
     * @param {number} screenY - Normalized mouse Y position (from -1 at bottom edge to 1 at top edge; often inverted in input).
     */
    handleMouseMove(screenX, screenY) {
        if (this.controlType !== 'human' || !this.mesh) return; // Functionality is only for human players with a valid mesh.

        this.mouseScreenX = screenX;
        this.mouseScreenY = screenY;

        // Update player's body X position: Map mouse X to player's allowed movement range and smoothly interpolate.
        const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
        // Lerp for smoother player body movement, preventing instant jumps.
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.1); 

        // Update racket's target X and Y position (relative to player's body) based on mouse.
        // Map mouse X to a horizontal range for the racket.
        this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35); 
        // Map mouse Y to a vertical range for the racket, added to its default height.
        this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35); 
        // Racket's Z position (depth) is fixed relative to player during normal movement; it changes during a swing.
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
    }
    
    /**
     * Initiates a swing action for the player.
     * For human players, this is primarily a visual cue (racket moves forward briefly).
     * For AI players, it sets the `aiHitting` flag, which can be used by collision logic or AI decision making.
     */
    swing() {
        if (this.controlType === 'human') {
            console.log("Human Player swings!");
            // Simple visual feedback for swing: racket moves forward (towards net) and then returns.
            this.racket.targetPosition.z += this.side * -0.25; // Move racket forward (player.side * -1 is towards net)
            setTimeout(() => {
                 // Return racket to its default Z offset after a short delay.
                 this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z;
            }, 120); // Duration of the forward part of the visual swing animation.
        } else if (this.controlType === 'ai') {
            console.log("AI Player swings!");
            this.aiHitting = true; // Set flag indicating AI is attempting a hit action.
            // AI racket can also have a visual swing animation.
            this.racket.targetPosition.z += this.side * -0.25; // Move racket forward.
            setTimeout(() => { 
                this.aiHitting = false; // Reset the hitting flag after animation duration.
                this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; // Return racket to default Z.
            }, 150); // Duration of AI swing animation.
        }
    }
    
    /**
     * Updates the AI player's state, including body position and racket positioning, based on the ball's current state.
     * This method contains the core AI logic for reacting to the ball.
     * @param {Ball} ball - The game ball object, containing position, velocity, and status.
     */
    updateAI(ball) {
        // AI only acts if the ball object is valid, AI has a mesh, and ball is currently in play (status >= 0).
        if (!ball || !this.mesh || ball.status < 0) return; 

        // 1. AI Player Body Movement: AI tries to position its body (X-axis) in line with the ball's X position.
        const targetPlayerX = THREE.MathUtils.clamp(ball.position.x, this.minX, this.maxX); // Clamp to AI's movement range.
        // Smoothly interpolate player's X position towards the target X. AI body movement is intentionally a bit slow.
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.07); 

        // 2. AI Racket Positioning Logic: AI attempts to position its racket to intercept the ball.
        // Calculate desired racket X position relative to the player's body.
        let desiredRacketX = ball.position.x - this.mesh.position.x; 
        // Calculate desired racket Y position relative to player's body, aiming to match ball's height relative to table.
        let desiredRacketY = RACKET_DEFAULT_Y + (ball.position.y - (TABLE_HEIGHT + ball.radius)); 
        
        // Clamp the calculated relative racket positions to reasonable limits.
        desiredRacketX = THREE.MathUtils.clamp(desiredRacketX, -0.4, 0.4); // Max sideways reach of racket.
        desiredRacketY = THREE.MathUtils.clamp(desiredRacketY, 0.0, 0.5);   // Min/max vertical reach of racket.
        
        this.racket.targetPosition.x = desiredRacketX;
        this.racket.targetPosition.y = desiredRacketY;
        // AI keeps racket at a default Z depth relative to its body during tracking. Actual hit occurs due to ball's Z movement.
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 

        // 3. AI Swing Decision Logic: Determine if the AI should attempt a swing.
        // Conditions for AI to initiate a swing:
        //    a. Ball is moving towards the AI.
        //    b. Ball is on (or very near) the AI's side of the net.
        //    c. Ball is within a certain Z-distance (longitudinal range) making it "hittable".
        //    d. Ball is at a "reasonable" height (not too high or too low).

        // Check if ball is moving towards AI based on AI's side and ball's Z velocity.
        const ballComingTowardsAI = (this.side === -1 && ball.velocity.z > 0.1) || // P2 (AI, -Z side) and ball moving in +Z direction.
                                  (this.side === 1 && ball.velocity.z < -0.1);   // P1 (AI, +Z side) and ball moving in -Z direction. (If P1 was AI)

        // Check if ball is on AI's side of the net (or very close to crossing).
        // A small buffer (0.2m) is added to allow AI to react slightly before ball fully crosses.
        const ballOnAISideOfNet = (this.side === -1 && ball.position.z < (NET_POS_Z + 0.2)) || 
                                  (this.side === 1 && ball.position.z > (NET_POS_Z - 0.2));
        
        // Calculate Z-distance from the racket's default resting plane to the ball.
        const distZToBall = Math.abs(ball.position.z - (this.mesh.position.z + this.racket.targetPosition.z));

        // Swing condition: If all criteria are met and AI is not already in a "hitting" state.
        if (ballComingTowardsAI && ballOnAISideOfNet && distZToBall < 0.25 && // Z-distance threshold for hit.
            ball.position.y < TABLE_HEIGHT + 0.3 && ball.position.y > TABLE_HEIGHT - 0.1) { // Vertical hittable window.
            if (!this.aiHitting) { // Only swing if not already in a swing cooldown/action.
                this.swing();
            }
        }
    }

    /**
     * Main update method for the player, called in the game's animation loop.
     * If the player is AI-controlled, it calls the AI update logic.
     * It also handles the smooth interpolation of the racket's visual position.
     * @param {Ball} ball - The game ball object, passed to `updateAI` if player is AI.
     */
    update(ball) { 
        if (this.controlType === 'ai') {
            this.updateAI(ball); // AI updates its logic based on ball state.
        }

        // Smoothly interpolate racket's current visual position towards its target position for all player types.
        // This creates a smoother animation for racket movement rather than instant jumps.
        if (this.racket.mesh) {
            this.racket.currentPosition.lerp(this.racket.targetPosition, this.racket.lerpFactor);
            this.racket.mesh.position.copy(this.racket.currentPosition);
        }
    }
}
