// Ball.js
import * as THREE from 'three';

// --- Constants ---

// Game Environment and Ball Physical Properties
const TABLE_HEIGHT = 0.76;          // Height of the table surface from the floor (meters)
const BALL_RADIUS = 0.02;           // Radius of the table tennis ball (meters)
const GRAVITY = 9.82;               // Standard gravitational acceleration (m/s^2)
const TICK = 1/60;                  // Duration of a single physics update step (seconds, assuming 60 FPS)

// Physics Simulation Constants (Tunable for realism and gameplay)
const AIR_RESISTANCE_FACTOR = 0.02; // Damping factor for air resistance (unitless, applied per tick)
const SPIN_DECAY_FACTOR = 0.03;     // Damping factor for spin decay over time in air (unitless, applied per tick)
const BOUNCE_ENERGY_LOSS = 0.85;    // Coefficient of restitution for table bounce (e.g., 0.85 means 85% of vertical speed is retained)
const SPIN_EFFECT_ON_BOUNCE_Y = 0.2; // Scalar determining how much Y-spin (top/back) affects vertical bounce velocity component
const SPIN_EFFECT_ON_BOUNCE_Z = 0.15; // Scalar determining how much Y-spin (top/back) affects forward/backward bounce velocity component
const SPIN_EFFECT_ON_BOUNCE_X = 0.1; // Scalar determining how much X-spin (side) affects sideways bounce velocity component

// Table and Net Dimensions (meters)
const TABLE_LENGTH = 2.74;          // Length of the table (along Z-axis)
const TABLE_WIDTH = 1.525;          // Width of the table (along X-axis)
const NET_HEIGHT = 0.1525;          // Height of the net from the table surface
const NET_POS_Z = 0;                // Z-coordinate of the net's center line

export class Ball {
    constructor(initialPosition = new THREE.Vector3(0, TABLE_HEIGHT + BALL_RADIUS + 0.2, 0)) {
        this.position = initialPosition.clone(); // Current 3D position of the ball (Vector3)
        this.velocity = new THREE.Vector3(0, 0, -2);   // Current 3D velocity of the ball (Vector3, m/s)
        this.spin = new THREE.Vector2(0, 0);           // Current spin vector: .x for sidespin, .y for top/backspin (radians/s)
        this.radius = BALL_RADIUS;                     // Ball's radius
        this.mesh = null;                              // Reference to the Three.js Mesh object for visual updates

        /**
         * Ball status codes, indicating the current state of the ball in the game:
         * Values >= 0 mean the ball is either ready to be put in play or is currently in play.
         *   8: Ready for Player 1 to serve. Player 1 is typically on the positive Z side.
         *   9: Ready for Player 2 to serve. Player 2 is typically on the negative Z side.
         *   0: Ball is in play, flying towards Player 1's side (Player 1's turn to hit). Originally P2 hit it.
         *   1: Ball is in play, flying towards Player 2's side (Player 2's turn to hit). Originally P1 hit it.
         *   (Note: Statuses were simplified. A more complex system might distinguish serves from rally hits more explicitly here.)
         * 
         * Values < 0 mean the ball is out of play, usually resulting in a point or a fault.
         *  -1: Ball hit the net structure during play and is considered "dead".
         *  -2: Ball hit the floor.
         *  -3: Ball went out of bounds sideways (beyond table width).
         *  -4: Ball went out of bounds long on Player 1's side (positive Z end).
         *  -5: Ball went out of bounds long on Player 2's side (negative Z end).
         */
        this.status = 8; 
        this.lastHitBy = 0;             // ID of the player who last hit the ball (1 for Player1, 2 for Player2, 0 for none)
        this.bouncedOnServerSide = false;   // Flag: True if the ball bounced on the side of the net corresponding to the current server during the current flight.
        this.bouncedOnReceiverSide = false; // Flag: True if the ball bounced on the side of the net corresponding to the current receiver.
    }

    /**
     * Updates the ball's physics state for one time step (TICK).
     * This includes applying forces, updating position, and handling collisions.
     * @param {THREE.Mesh} player1RacketMesh - The mesh of Player 1's racket for collision detection.
     * @param {THREE.Mesh} player2RacketMesh - The mesh of Player 2's racket for collision detection.
     */
    updatePhysics(player1RacketMesh, player2RacketMesh) {
        // 1. Apply Environmental Forces
        this.velocity.y -= GRAVITY * TICK; // Apply gravity to vertical velocity
        this.velocity.multiplyScalar(1 - AIR_RESISTANCE_FACTOR * TICK); // Apply air resistance (acts as damping)
        this.spin.multiplyScalar(1 - SPIN_DECAY_FACTOR * TICK); // Spin magnitude decays over time

        // TODO: Implement Magnus Force for more realistic spin-induced curve in flight.
        // This would involve a force perpendicular to both velocity and spin axis.
        // F_magnus = C * (spin x velocity), where C is a coefficient.

        // 2. Update Position
        // Simple Euler integration: new_position = old_position + velocity * time_step
        this.position.addScaledVector(this.velocity, TICK);

        // 3. Collision Detection and Response

        // --- Net Collision ---
        // Check if the ball's current position intersects the volume of the net.
        if (this.position.y > TABLE_HEIGHT && // Must be above table surface
            this.position.y < TABLE_HEIGHT + NET_HEIGHT + this.radius && // Within vertical bounds of net
            Math.abs(this.position.z - NET_POS_Z) < this.radius + 0.01) { // Close to net's Z-plane (0.01 is net thickness guess)
            
            // Ensure ball is moving towards the net before registering a hit to prevent spurious collisions.
            const movingTowardsNetZ = (this.velocity.z > 0 && this.position.z < NET_POS_Z + this.radius) || // Moving in +Z dir towards net
                                      (this.velocity.z < 0 && this.position.z > NET_POS_Z - this.radius);   // Moving in -Z dir towards net
            if (movingTowardsNetZ) {
                console.log("Ball hit net body");
                this.velocity.z *= -0.3; // Significantly dampens and slightly reverses Z velocity
                this.velocity.y *= 0.4;  // Reduces Y velocity
                this.spin.y *= 0.5;      // Reduces topspin/backspin
                this.position.z += Math.sign(this.velocity.z) * this.radius * 0.2; // Push slightly out of net to prevent re-collision
                this.status = -1;        // Set status to 'netted' for game rule processing
            }
        }

        // --- Table Collision ---
        // Check if ball is at table height and moving downwards.
        if (this.position.y < TABLE_HEIGHT + this.radius && this.velocity.y < 0) {
            this.position.y = TABLE_HEIGHT + this.radius; // Correct position to sit exactly on table surface
            
            const preBounceVelocityY = this.velocity.y; // Store Y velocity before bounce for spin calculations
            this.velocity.y *= -BOUNCE_ENERGY_LOSS;     // Reflect vertical velocity and apply energy loss

            // Apply spin effects on bounce:
            // Topspin (spin.y > 0) generally makes the ball bounce lower and shoot forward more.
            // Backspin (spin.y < 0) generally makes the ball bounce higher and slow down or even come back.
            // Note: The exact effect depends on the coefficients and incoming velocity.
            this.velocity.y += this.spin.y * SPIN_EFFECT_ON_BOUNCE_Y * Math.abs(preBounceVelocityY); 
            this.velocity.z -= this.spin.y * SPIN_EFFECT_ON_BOUNCE_Z; // Topspin increases forward Z, backspin decreases/reverses
            this.velocity.x += this.spin.x * SPIN_EFFECT_ON_BOUNCE_X; // Sidespin affects X direction of bounce

            // Reduce spin due to friction with the table surface
            this.spin.y *= 0.6; 
            this.spin.x *= 0.7;

            console.log(`Ball bounced on table. Z: ${this.position.z.toFixed(2)}, Side: ${this.position.z >= NET_POS_Z ? "P1_Side(Pos-Z)" : "P2_Side(Neg-Z)"}`);

            // Update bounce flags based on which side of the net the bounce occurred.
            // Player 1 (human) is assumed to be on the positive Z side, Player 2 (AI) on the negative Z side.
            if (this.position.z >= NET_POS_Z) { // Bounced on the positive Z side of the net
                this.bouncedOnServerSide = true; 
            } else { // Bounced on the negative Z side of the net
                this.bouncedOnReceiverSide = true;
            }
        }

        // --- Floor Collision ---
        // Ball is out of play if it hits the floor.
        if (this.position.y < this.radius && this.velocity.y < 0) { // Below table height and moving down
            console.log("Ball hit floor");
            this.status = -2; // Set status to 'floored'
        }
        
        // --- Out of Bounds: Sideways ---
        // Ball is out if it goes beyond the table width.
        if (Math.abs(this.position.x) > TABLE_WIDTH / 2 + this.radius) {
            console.log("Ball out of table width (sideways)");
            this.status = -3; // Set status to 'out_sideways'
        }
        
        // --- Out of Bounds: Long ---
        // Ball is out if it goes beyond the table length on either side.
        // This check also considers if the ball was moving in the direction of going out.
        // More detailed 'out long' rules (e.g. after valid bounce) are handled by game logic using these statuses.
        if (this.position.z > (TABLE_LENGTH / 2 + this.radius * 2) && this.velocity.z > 0) { // Gone past P1's endline (positive Z)
            console.log("Ball potentially out long on P1 side (+Z)");
            // Only set status if it hasn't bounced on this side, otherwise it might be a valid shot going out after bounce.
            if (!this.bouncedOnServerSide) this.status = -4; // 'out_long_p1_side'
        }
        if (this.position.z < -(TABLE_LENGTH / 2 + this.radius * 2) && this.velocity.z < 0) { // Gone past P2's endline (negative Z)
             console.log("Ball potentially out long on P2 side (-Z)");
             if (!this.bouncedOnReceiverSide) this.status = -5; // 'out_long_p2_side'
        }

        // --- Racket Collision ---
        // Check for collisions with player rackets. The `checkRacketCollision` method handles hit registration.
        if (player1RacketMesh && this.checkRacketCollision(player1RacketMesh, 1)) { /* Collision handled in method */ }
        if (player2RacketMesh && this.checkRacketCollision(player2RacketMesh, 2)) { /* Collision handled in method */ }
    }
    
    /**
     * Checks for and handles collision with a player's racket.
     * @param {THREE.Mesh} racketMesh - The mesh of the racket to check against.
     * @param {number} hittingPlayerID - The ID of the player whose racket is being checked (1 or 2).
     * @returns {boolean} True if a collision occurred and was handled, false otherwise.
     */
    checkRacketCollision(racketMesh, hittingPlayerID) {
        // Prevent a player from hitting the ball multiple times in rapid succession (double hit).
        if (!racketMesh || !this.mesh || this.lastHitBy === hittingPlayerID) return false;

        // Use Axis-Aligned Bounding Boxes (AABB) for simple collision detection.
        const ballBox = new THREE.Box3().setFromObject(this.mesh);
        const racketBox = new THREE.Box3().setFromObject(racketMesh);

        if (ballBox.intersectsBox(racketBox)) {
            console.log(`Ball collided with racket of player ${hittingPlayerID}`);
            
            let newVelocity = new THREE.Vector3(); // To store the ball's velocity after being hit
            const racketWorldPos = new THREE.Vector3();
            racketMesh.getWorldPosition(racketWorldPos); // Get racket's world position for calculations
            
            // Calculate impact offset: vector from racket center to ball center at point of impact.
            // This can be used to influence hit direction or spin.
            const impactOffset = this.position.clone().sub(racketWorldPos); 

            // Simplified hit physics model:
            newVelocity.y = 1.8 + Math.random() * 0.5; // Base upward velocity to clear the net
            
            const baseZSpeed = 2.5 + Math.random() * 1.0; // Base forward speed of the hit
            // Determine Z direction based on which player hit it. P1 hits towards -Z, P2 towards +Z.
            newVelocity.z = (hittingPlayerID === 1) ? -baseZSpeed : baseZSpeed; 

            // Influence X (sideways) velocity by where on the racket the ball hit and some randomness.
            // A more positive impactOffset.x means ball hit right side of racket.
            newVelocity.x = impactOffset.x * (hittingPlayerID === 1 ? -5.0 : 5.0) + (Math.random() - 0.5);
            
            // Apply some default spin (can be influenced by racket angle/player input in a more advanced model).
            // Here, sidespin is related to X velocity, and a default topspin is applied.
            let newSpin = new THREE.Vector2(newVelocity.x * 0.5, 2 + Math.random() * 3); 

            this.hit(newVelocity, newSpin, hittingPlayerID); // Call the hit method to apply changes
            return true; // Collision occurred and was processed
        }
        return false; // No collision
    }

    /**
     * Main update method called from the game loop.
     * Updates physics if ball is in play and syncs the visual mesh position.
     * @param {THREE.Mesh} player1RacketMesh - Player 1's racket mesh.
     * @param {THREE.Mesh} player2RacketMesh - Player 2's racket mesh.
     */
    update(player1RacketMesh, player2RacketMesh) {
        if (this.status >= 0) { // Only update physics if ball is "in play" (status >= 0)
            this.updatePhysics(player1RacketMesh, player2RacketMesh);
        }
        if (this.mesh) {
            this.mesh.position.copy(this.position); // Update the visual mesh's position to match physics position
        }
    }

    /**
     * Resets the ball's state for a new serve.
     * @param {number} forPlayerID - The ID of the player who will be serving (1 or 2).
     */
    reset(forPlayerID = 1) { 
        this.lastHitBy = 0; // No one has hit the ball yet for this point
        this.bouncedOnServerSide = false;   // Reset bounce flags
        this.bouncedOnReceiverSide = false;
        this.spin.set(0,0);                 // Reset spin
        
        // Initial velocity for serve (simulates a small toss or ready state)
        this.velocity.set(0, 0.5 + Math.random()*0.3, 0); 

        if (forPlayerID === 1) { // Player 1 is serving
            // Position ball on Player 1's side, ready for serve.
            this.position.set(0, TABLE_HEIGHT + BALL_RADIUS + 0.25, TABLE_LENGTH / 4); 
            this.status = 8; // Set status to 'ready_to_serve_p1'
        } else { // Player 2 is serving
            // Position ball on Player 2's side, ready for serve.
            this.position.set(0, TABLE_HEIGHT + BALL_RADIUS + 0.25, -TABLE_LENGTH / 4);
            this.status = 9; // Set status to 'ready_to_serve_p2'
        }
        console.log(`Ball reset. Ready for Player ${forPlayerID} to serve.`);
    }

    /**
     * Registers a hit on the ball by a player.
     * Updates ball's velocity, spin, and game state flags.
     * @param {THREE.Vector3} newVelocity - The new velocity of the ball after the hit.
     * @param {THREE.Vector2} newSpin - The new spin of the ball after the hit.
     * @param {number} hittingPlayerID - The ID of the player who hit the ball.
     */
    hit(newVelocity, newSpin, hittingPlayerID) {
        this.velocity.copy(newVelocity);
        if (newSpin) {
            this.spin.copy(newSpin);
        }
        this.lastHitBy = hittingPlayerID;
        this.bouncedOnServerSide = false; // Reset bounce flags for the new trajectory
        this.bouncedOnReceiverSide = false;
        
        // Update ball status based on who hit it, indicating its new flight direction.
        if (hittingPlayerID === 1) {
            this.status = 1; // Ball is now flying from Player 1 towards Player 2's side.
        } else { // hittingPlayerID === 2
            this.status = 3; // Ball is now flying from Player 2 towards Player 1's side.
        }
        console.log(`Ball hit by Player ${hittingPlayerID}! Status: ${this.status}, Vel:(${this.velocity.x.toFixed(1)},${this.velocity.y.toFixed(1)},${this.velocity.z.toFixed(1)})`);
    }
}
