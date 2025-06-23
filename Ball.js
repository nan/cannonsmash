// Ball.js - Applying fixes to calculatePerfectServeVelocity
import * as THREE from 'three';

// (Constants as in Turn 191)
const TABLE_HEIGHT = 0.76;         
const BALL_RADIUS = 0.02;          
const GRAVITY = 9.82;              
const TICK = 1/60; 
const AIR_RESISTANCE_FACTOR = 0.02; 
const SPIN_DECAY_FACTOR = 0.03;     
const BOUNCE_ENERGY_LOSS = 0.85;    
const SPIN_EFFECT_ON_BOUNCE_Y = 0.2; 
const SPIN_EFFECT_ON_BOUNCE_Z = 0.15; 
const SPIN_EFFECT_ON_BOUNCE_X = 0.1; 
const TABLE_LENGTH = 2.74;          
const TABLE_WIDTH = 1.525;          
const NET_HEIGHT = 0.1525;          
const NET_POS_Z = 0;                
const NET_TOP_Y = TABLE_HEIGHT + NET_HEIGHT + BALL_RADIUS; // Global const
const TABLE_SURFACE_Y = TABLE_HEIGHT + BALL_RADIUS; // Global const

// Constants for racket collision physics
const RACKET_IMPACT_BASE_Y_VELOCITY = 1.8;
const RACKET_IMPACT_RANDOM_Y_VELOCITY_ADD = 0.5;
const RACKET_IMPACT_BASE_Z_VELOCITY = 2.5;
const RACKET_IMPACT_RANDOM_Z_VELOCITY_ADD = 1.0;
const RACKET_IMPACT_X_VELOCITY_FACTOR = 5.0;
const RACKET_IMPACT_SPIN_X_FACTOR = 0.5;
const RACKET_IMPACT_BASE_SPIN_Y = 2.0;
const RACKET_IMPACT_RANDOM_SPIN_Y_ADD = 3.0;

export class Ball {
    // (constructor, updatePhysics, checkRacketCollision, update, toss, reset, hit as in Turn 191)
    constructor(initialPosition = new THREE.Vector3(0, TABLE_HEIGHT + BALL_RADIUS + 0.2, 0)) {
        this.position = initialPosition.clone(); 
        this.velocity = new THREE.Vector3(0, 0, -2);   
        this.spin = new THREE.Vector2(0, 0);           
        this.radius = BALL_RADIUS;                     
        this.mesh = null;                              
        this.status = 8; 
        this.lastHitBy = 0;             
        this.bouncedOnServerSide = false;   
        this.bouncedOnReceiverSide = false; 
    }

    _handleNetCollision() {
        const isBallNearNetHeight = this.position.y > TABLE_HEIGHT && this.position.y < TABLE_HEIGHT + NET_HEIGHT + this.radius;
        const isBallNearNetZ = Math.abs(this.position.z - NET_POS_Z) < this.radius + 0.01; // Small tolerance

        if (isBallNearNetHeight && isBallNearNetZ) {
            const isMovingTowardsNetZ = (this.velocity.z > 0 && this.position.z < NET_POS_Z + this.radius) ||
                                      (this.velocity.z < 0 && this.position.z > NET_POS_Z - this.radius);   
            if (isMovingTowardsNetZ) {
                console.log("Ball hit net body");
                this.velocity.z *= -0.3; // Reverse Z velocity, reduce speed
                this.velocity.y *= 0.4;  // Reduce Y velocity
                this.spin.y *= 0.5;      // Reduce topspin/backspin
                this.position.z += Math.sign(this.velocity.z) * this.radius * 0.2; // Move ball slightly away from net
                this.status = -1; // Status for hitting the net
            }
        }
    }

    _handleTableBounce() {
        const isBallFalling = this.velocity.y < 0;
        const isBallAtTableHeight = this.position.y < TABLE_HEIGHT + this.radius;
        // Check if the ball is within the table's X and Z boundaries (including radius)
        const ballOnTableX = Math.abs(this.position.x) <= TABLE_WIDTH / 2 + this.radius;
        const ballOnTableZ = Math.abs(this.position.z) <= TABLE_LENGTH / 2 + this.radius;

        if (isBallFalling && isBallAtTableHeight && ballOnTableX && ballOnTableZ) {
            this.position.y = TABLE_HEIGHT + this.radius; // Correct ball position to be exactly on table surface

            const preBounceVelocityY = this.velocity.y; // Store Y velocity before bounce for spin calculations

            // Apply bounce physics: reverse Y velocity and apply energy loss
            this.velocity.y *= -BOUNCE_ENERGY_LOSS;     

            // Apply spin effects on bounce
            this.velocity.y += this.spin.y * SPIN_EFFECT_ON_BOUNCE_Y * Math.abs(preBounceVelocityY); 
            this.velocity.z -= this.spin.y * SPIN_EFFECT_ON_BOUNCE_Z; 
            this.velocity.x += this.spin.x * SPIN_EFFECT_ON_BOUNCE_X; 

            // Reduce spin after bounce
            this.spin.y *= 0.6;
            this.spin.x *= 0.7;

            console.log(`Ball bounced on table. X: ${this.position.x.toFixed(2)}, Z: ${this.position.z.toFixed(2)}, Side: ${this.position.z >= NET_POS_Z ? "P1_Side(Pos-Z)" : "P2_Side(Neg-Z)"}`);

            // Update bounce flags based on which side of the net the ball bounced
            if (this.position.z >= NET_POS_Z) { // Server's side (positive Z for player 1)
                this.bouncedOnServerSide = true;
            } else { // Receiver's side (negative Z for player 2)
                this.bouncedOnReceiverSide = true;
                // Check if this is likely the second bounce of a serve
                // (bounced on server, now on receiver, and was hit by a player recently)
                if (this.bouncedOnServerSide && (this.status === 1 || this.status === 3 || this.status === 6 || this.status === 7)) { // Status 1 or 3 for ball in play after hit, could be a serve's first hit. 6/7 for active serve.
                    console.log(`[Test Log] Actual second bounce Z: ${this.position.z.toFixed(4)} at Y: ${this.position.y.toFixed(4)}`);
                }
            }
        }
    }

    _handleOutOfBoundsAndFloor() {
        // Check for hitting the floor
        const hitFloor = this.position.y < this.radius && this.velocity.y < 0;
        if (hitFloor) {
            console.log("Ball hit floor");
            this.status = -2; // Status for hitting the floor
            return; // No further out-of-bounds checks needed if it hit the floor
        }

        // Check for out of table width (sideways)
        const outOfTableX = Math.abs(this.position.x) > TABLE_WIDTH / 2 + this.radius;
        if (outOfTableX) {
            console.log("Ball out of table width (sideways)");
            this.status = -3; // Status for out of table (width)
            return;
        }

        // These variables are also used in _handleTableBounce, consider passing as params if DRY principle is paramount
        // For now, re-declaring for clarity within this specific context of "long" or "wide" after potential bounce.
        const ballCurrentlyOnTableX = Math.abs(this.position.x) <= TABLE_WIDTH / 2 + this.radius;
        const ballCurrentlyAtTableSurface = this.position.y <= TABLE_HEIGHT + this.radius;


        // Check for out of table length (long) - Player 1's side (positive Z)
        const outOfTableZPositive = this.position.z > (TABLE_LENGTH / 2 + this.radius * 2) && this.velocity.z > 0;
        if (outOfTableZPositive) {
            // If it went long on P1's side, it's a fault if it hasn't bounced on P1's side yet,
            // unless it's currently on the table surface (e.g. rolling)
            if (!this.bouncedOnServerSide && !(ballCurrentlyOnTableX && ballCurrentlyAtTableSurface)) {
                 this.status = -4; // Status for out of table (length on server side)
            }
        }

        // Check for out of table length (long) - Player 2's side (negative Z)
        const outOfTableZNegative = this.position.z < -(TABLE_LENGTH / 2 + this.radius * 2) && this.velocity.z < 0;
        if (outOfTableZNegative) {
            // If it went long on P2's side, it's a fault if it hasn't bounced on P2's side yet,
            // unless it's currently on the table surface.
             if (!this.bouncedOnReceiverSide && !(ballCurrentlyOnTableX && ballCurrentlyAtTableSurface)) {
                 this.status = -5; // Status for out of table (length on receiver side)
             }
        }
    }

    updatePhysics(player1RacketMesh, player2RacketMesh) {
        // 1. Apply basic physics (gravity, air resistance, spin decay, position update)
        this.velocity.y -= GRAVITY * TICK;
        this.velocity.multiplyScalar(1 - AIR_RESISTANCE_FACTOR * TICK);
        this.spin.multiplyScalar(1 - SPIN_DECAY_FACTOR * TICK);
        this.position.addScaledVector(this.velocity, TICK);

        // 2. Handle collisions and bounces
        this._handleNetCollision();
        // Only handle table bounce if the ball hasn't hit the net
        if (this.status !== -1) {
            this._handleTableBounce();
        }

        // 3. Handle out-of-bounds and floor hits
        // Only check if ball status is still potentially in play (not already -1, -2 etc. from net/floor)
        // However, out-of-bounds can override a bounce status if it's subsequently determined to be out.
        // Let _handleOutOfBoundsAndFloor manage its own status changes.
        this._handleOutOfBoundsAndFloor();

        // 4. Check for racket collisions (remains unchanged)
        if (player1RacketMesh && this.checkRacketCollision(player1RacketMesh, 1)) {}
        if (player2RacketMesh && this.checkRacketCollision(player2RacketMesh, 2)) {}
    }
    checkRacketCollision(racketMesh, hittingPlayerID) {
        if (this.status === 6 || this.status === 7) { return false; }
        if (!racketMesh || !this.mesh || this.lastHitBy === hittingPlayerID) return false;
        const ballBox = new THREE.Box3().setFromObject(this.mesh);
        const racketBox = new THREE.Box3().setFromObject(racketMesh);
        if (ballBox.intersectsBox(racketBox)) {
            console.log(`Ball collided with racket of player ${hittingPlayerID} (rally hit)`);
            let newVelocity = new THREE.Vector3(); const racketWorldPos = new THREE.Vector3();
            racketMesh.getWorldPosition(racketWorldPos); 
            const impactOffset = this.position.clone().sub(racketWorldPos); 

            // Calculate Y velocity: base upward velocity + some randomness
            newVelocity.y = RACKET_IMPACT_BASE_Y_VELOCITY + Math.random() * RACKET_IMPACT_RANDOM_Y_VELOCITY_ADD;

            // Calculate Z velocity: base forward/backward speed + some randomness
            const baseZSpeed = RACKET_IMPACT_BASE_Z_VELOCITY + Math.random() * RACKET_IMPACT_RANDOM_Z_VELOCITY_ADD;
            newVelocity.z = (hittingPlayerID === 1) ? -baseZSpeed : baseZSpeed; // Negative Z for player 1, positive for player 2

            // Calculate X velocity: based on impact offset from racket center, player direction, and some randomness
            // (Math.random() - 0.5) adds a value between -0.5 and 0.5 for slight variation
            newVelocity.x = impactOffset.x * (hittingPlayerID === 1 ? -RACKET_IMPACT_X_VELOCITY_FACTOR : RACKET_IMPACT_X_VELOCITY_FACTOR) + (Math.random() - 0.5);

            // Calculate spin: X spin based on resulting X velocity, Y spin (topspin/backspin) with base + randomness
            let newSpin = new THREE.Vector2(
                newVelocity.x * RACKET_IMPACT_SPIN_X_FACTOR,
                RACKET_IMPACT_BASE_SPIN_Y + Math.random() * RACKET_IMPACT_RANDOM_SPIN_Y_ADD
            );
            this.hit(newVelocity, newSpin, hittingPlayerID); return true; 
        } return false; 
    }
    update(player1RacketMesh, player2RacketMesh) {
        if (this.status >= 0) { 
            this.updatePhysics(player1RacketMesh, player2RacketMesh);
        }
        if (this.mesh) {
            this.mesh.position.copy(this.position); 
        }
    }
    reset(forPlayerID = 1) { 
        this.lastHitBy = 0; 
        this.bouncedOnServerSide = false;   
        this.bouncedOnReceiverSide = false;
        this.spin.set(0,0);                 
        this.position.set(0, TABLE_HEIGHT + 0.5, 0); 
        this.velocity.set(0, 0, 0); 
        if (forPlayerID === 1) { this.status = 8; } 
        else { this.status = 9; }
    }
    toss(tossPower, servingPlayerID) {
        this.velocity.set(0, tossPower, 0); 
        this.spin.set(0, 0);                
        this.lastHitBy = 0;                 
        this.bouncedOnServerSide = false;
        this.bouncedOnReceiverSide = false;
        if (servingPlayerID === 1) { this.status = 6; } 
        else { this.status = 7; }
        console.log(`Ball tossed by Player ${servingPlayerID} with power ${tossPower}. Status: ${this.status}`);
    }
    hit(newVelocity, newSpin, hittingPlayerID) {
        this.velocity.copy(newVelocity);
        if (newSpin) { this.spin.copy(newSpin); }
        this.lastHitBy = hittingPlayerID;
        this.bouncedOnServerSide = false; 
        this.bouncedOnReceiverSide = false;
        if (hittingPlayerID === 1) { this.status = 1; } 
        else { this.status = 3; } 
        console.log(`Ball hit by Player ${hittingPlayerID}! Status: ${this.status}, Vel:(${this.velocity.x.toFixed(1)},${this.velocity.y.toFixed(1)},${this.velocity.z.toFixed(1)}), Pos:(${this.position.x.toFixed(1)},${this.position.y.toFixed(1)},${this.position.z.toFixed(1)})`);
    }

    // Private helper method to simulate a single serve trajectory
    // Parameters:
    //   initialVelocity: THREE.Vector3 - The initial velocity to simulate.
    //   hitPosition: THREE.Vector3 - The starting position of the ball.
    //   desiredSpin: THREE.Vector2 - The spin applied to the ball.
    //   serverSide: number (1 or -1) - Indicates which side the server is on.
    //   targetOpponentBouncePos: THREE.Vector3 - The desired bounce position on the opponent's side.
    //   MAX_SIMULATION_TICKS: number - Maximum physics steps for the simulation.
    //   NET_CLEARANCE_MIN: number - Minimum height the ball must clear the net by.
    //   TARGET_BOUNCE_DISTANCE_TOLERANCE: number - Allowed distance from the target bounce position.
    // Returns an object: { success: boolean, reason?: string, finalSimPosition: THREE.Vector3, clearedNet: boolean, bouncedOnServer: boolean, bouncedOnOpponent: boolean, distanceToTarget?: number, netClearanceHeight?: number, firstBouncePosition?: THREE.Vector3 }
    _simulateServeTrajectory(initialVelocity, hitPosition, desiredSpin, serverSide, targetOpponentBouncePos, MAX_SIMULATION_TICKS, NET_CLEARANCE_MIN, TARGET_BOUNCE_DISTANCE_TOLERANCE) {
        let simBallPosition = hitPosition.clone();
        let simBallVelocity = initialVelocity.clone();
        let simBallSpin = desiredSpin.clone(); // Spin is applied but assumed constant for this simplified simulation

        let hasBouncedOnServerSide = false;
        let hasClearedNet = false;
        let hasBouncedOnOpponentSide = false;

        // Initialize detailed result fields
        let result = {
            success: false,
            reason: "unknown",
            finalSimPosition: simBallPosition.clone(),
            clearedNet: false,
            bouncedOnServer: false,
            bouncedOnOpponent: false,
            distanceToTarget: Infinity,
            netClearanceHeight: -Infinity, // Negative means it didn't clear or hit net
            firstBouncePosition: null
        };

        // Simulate physics tick by tick
        for (let tick_num = 0; tick_num < MAX_SIMULATION_TICKS; tick_num++) {
            let prevSimBallPosition = simBallPosition.clone();

            // Apply basic physics (gravity and air resistance)
            simBallVelocity.y -= GRAVITY * TICK;
            simBallVelocity.multiplyScalar(1 - AIR_RESISTANCE_FACTOR * TICK); // Air resistance affects velocity
            simBallSpin.multiplyScalar(1 - SPIN_DECAY_FACTOR * TICK);
            simBallPosition.addScaledVector(simBallVelocity, TICK);

            // Check if the ball is crossing the net in this tick
            const crossedNetPlaneThisTickZ = (serverSide === 1) ?
                (prevSimBallPosition.z > NET_POS_Z && simBallPosition.z <= NET_POS_Z) :
                (prevSimBallPosition.z < NET_POS_Z && simBallPosition.z >= NET_POS_Z);

            if (crossedNetPlaneThisTickZ) {
                let yAtNet = NET_TOP_Y + 1.0; // Default high, assumes clear if no precise calculation possible
                // Interpolate to find ball's Y position exactly at the net's Z position
                if (Math.abs(simBallPosition.z - prevSimBallPosition.z) > 1e-6) { // Avoid division by zero
                    const interpolationFactor = (NET_POS_Z - prevSimBallPosition.z) / (simBallPosition.z - prevSimBallPosition.z);
                    yAtNet = prevSimBallPosition.y + (simBallPosition.y - prevSimBallPosition.y) * interpolationFactor;
                }
                result.netClearanceHeight = yAtNet - NET_TOP_Y;

                if (result.netClearanceHeight > NET_CLEARANCE_MIN) {
                    hasClearedNet = true;
                    result.clearedNet = true;
                } else {
                    // Failed to clear the net
                    result.success = false;
                    result.reason = "did not clear net";
                    result.finalSimPosition = simBallPosition.clone();
                    // result.clearedNet is already false
                    result.bouncedOnServer = hasBouncedOnServerSide;
                    result.bouncedOnOpponent = hasBouncedOnOpponentSide;
                    return result;
                }
            }

            // Check for bounce on table surface
            const isApproachingTableSurface = simBallVelocity.y < 0;
            const isAtOrBelowTableSurface = simBallPosition.y <= TABLE_SURFACE_Y;

            if (isApproachingTableSurface && isAtOrBelowTableSurface) {
                simBallPosition.y = TABLE_SURFACE_Y; // Correct position to be exactly on table surface

                const isOnServerHalf = (serverSide === 1) ? (simBallPosition.z > NET_POS_Z) : (simBallPosition.z < NET_POS_Z);
                const isWithinTableWidth = Math.abs(simBallPosition.x) <= TABLE_WIDTH / 2;

                if (!hasBouncedOnServerSide && isOnServerHalf) { // First bounce, should be on server's side
                    if (isWithinTableWidth) {
                        hasBouncedOnServerSide = true;
                        result.bouncedOnServer = true;
                        result.firstBouncePosition = simBallPosition.clone();
                        // Apply simplified bounce physics (energy loss and spin effects)
                        const preBounceVy = simBallVelocity.y;
                        simBallVelocity.y *= -BOUNCE_ENERGY_LOSS; // Vertical velocity reverses and loses energy
                        // Spin effects on bounce (simplified)
                        simBallVelocity.y += simBallSpin.y * SPIN_EFFECT_ON_BOUNCE_Y * Math.abs(preBounceVy);
                        simBallVelocity.x += simBallSpin.x * SPIN_EFFECT_ON_BOUNCE_X;
                        simBallVelocity.z -= simBallSpin.y * SPIN_EFFECT_ON_BOUNCE_Z;
                    } else {
                        // Missed the table width on the first bounce
                        result.success = false;
                        result.reason = "missed first bounce (out of width)";
                        result.finalSimPosition = simBallPosition.clone();
                        // result.bouncedOnServer is already false
                        result.bouncedOnOpponent = hasBouncedOnOpponentSide; // keep current state
                        return result;
                    }
                }
                else if (hasBouncedOnServerSide && !isOnServerHalf) { // Second bounce, should be on opponent's side
                    if (isWithinTableWidth) {
                        hasBouncedOnOpponentSide = true;
                        result.bouncedOnOpponent = true;
                        result.finalSimPosition = simBallPosition.clone(); // Update final position for distance calc

                        result.distanceToTarget = Math.sqrt(
                            Math.pow(simBallPosition.x - targetOpponentBouncePos.x, 2) +
                            Math.pow(simBallPosition.z - targetOpponentBouncePos.z, 2)
                        );

                        if (!(result.distanceToTarget < TARGET_BOUNCE_DISTANCE_TOLERANCE && hasClearedNet)) {
                            // Bounced on opponent side but missed target or hadn't cleared net properly
                            result.success = false;
                            result.reason = "missed target or net after first bounce";
                            // result.bouncedOnServer is true
                            // result.bouncedOnOpponent is true
                            return result;
                        }
                        // If conditions met, success is checked later.
                    } else {
                        // Out of table width on the opponent's side
                        result.success = false;
                        result.reason = "out of bounds X on opponent side";
                        result.finalSimPosition = simBallPosition.clone();
                        // result.bouncedOnServer is true
                        result.bouncedOnOpponent = false;
                        return result;
                    }
                } else {
                    // Invalid bounce state (e.g., bounced twice on server side, or bounced on receiver side first without server bounce)
                    result.success = false;
                    result.reason = "invalid bounce state";
                    result.finalSimPosition = simBallPosition.clone();
                    // result.bouncedOnServer and result.bouncedOnOpponent reflect current state
                    return result;
                }
            }

            // Check for hitting the floor (ball Y position is below zero)
            if (simBallPosition.y < 0) {
                result.success = false;
                result.reason = "hit floor";
                result.finalSimPosition = simBallPosition.clone();
                return result;
            }

            // Check for successful serve conditions only if all stages have been passed in order
            if (hasBouncedOnServerSide && hasClearedNet && hasBouncedOnOpponentSide) {
                 // distanceToTarget would have been calculated when hasBouncedOnOpponentSide became true
                if (result.distanceToTarget < TARGET_BOUNCE_DISTANCE_TOLERANCE) {
                    result.success = true;
                    result.reason = "successful serve";
                    result.finalSimPosition = simBallPosition.clone(); // Ensure final position is updated
                    return result;
                }
                // If it bounced on opponent side but rolled away from target,
                // the "missed target or net after first bounce" check would have caught it if it was over tolerance initially.
                // Or, it could be that it was within tolerance, but the loop continued and it rolled out.
                // For now, we rely on the check when the bounce first happens.
            }
        }
        // If loop finishes, it means MAX_SIMULATION_TICKS reached.
        result.success = false;
        result.reason = "max simulation ticks reached";
        result.finalSimPosition = simBallPosition.clone();
        // bouncedOnServer, bouncedOnOpponent, clearedNet, netClearanceHeight, firstBouncePosition, distanceToTarget will have their latest values.
        return result;
    }

    calculatePerfectServeVelocity(hitPosition, serverSide, firstBounceServerZ_UNUSED, targetOpponentBouncePos, desiredSpin) {
        console.log("Calculating serve velocity (Hill Climbing)...");

        const MAX_SIMULATION_TICKS = 300; // From original
        const NET_CLEARANCE_MIN = 0.002; // From original
        const TARGET_BOUNCE_DISTANCE_TOLERANCE = 0.05; // From original

        // Hill Climbing Parameters
        const maxIterations = 100; // Max attempts to find a better solution
        let initialStepSize = 0.5;   // Initial adjustment to velocity components
        const minStepSize = 0.01;    // Smallest step size before giving up on a path
        const stepSizeDecay = 0.95;  // Factor to reduce step size by
        const noImprovementThreshold = 5; // Iterations without improvement before reducing step size

        // Initial guess for velocity (using the old fallback as a starting point)
        let currentBestVelocity = new THREE.Vector3(
            THREE.MathUtils.clamp((targetOpponentBouncePos.x - hitPosition.x) / 0.4, -1.5, 1.5),
            1.8, // A reasonable initial Vy
            serverSide * -3.0 // Initial Vz, ensuring minimum speed and correct direction
        );
        if (Math.abs(currentBestVelocity.z) < 2.0) { // from old fallback
             currentBestVelocity.z = serverSide * -3.0;
        }


        let initialSimResult = this._simulateServeTrajectory(currentBestVelocity, hitPosition, desiredSpin, serverSide, targetOpponentBouncePos, MAX_SIMULATION_TICKS, NET_CLEARANCE_MIN, TARGET_BOUNCE_DISTANCE_TOLERANCE);
        let currentBestScore = this._evaluateServeTrajectory(initialSimResult, NET_CLEARANCE_MIN);

        let overallBestSuccessfulVelocity = null;
        if (initialSimResult.success) {
            overallBestSuccessfulVelocity = currentBestVelocity.clone();
        }

        let noImprovementStreak = 0;
        let stepSize = initialStepSize;

        for (let i = 0; i < maxIterations; i++) {
            let improvementFoundThisIteration = false;
            let candidateVelocity = null; // To hold the best neighbor of this iteration

            // Explore neighbors (6 directions: +/- stepSize for each component Vx, Vy, Vz)
            for (let j = 0; j < 3; j++) { // Iterate through components (0:x, 1:y, 2:z)
                for (let k = -1; k <= 1; k += 2) { // Iterate through directions (-1, 1)
                    let neighborVelocity = currentBestVelocity.clone();
                    if (j === 0) neighborVelocity.x += k * stepSize;
                    else if (j === 1) neighborVelocity.y += k * stepSize;
                    else neighborVelocity.z += k * stepSize;

                    // Ensure Vz maintains correct general direction for the server
                    if (serverSide * neighborVelocity.z > 0 && serverSide * currentBestVelocity.z < 0) {
                        // Allow Vz to cross zero if current is also near zero, but not flip from strong positive to negative
                        if (Math.abs(currentBestVelocity.z) > 0.5) continue;
                    }
                     // Prevent Vz from becoming too slow or going backwards if it's meant to be strong
                    if (serverSide === 1 && neighborVelocity.z > -0.5) neighborVelocity.z = -0.5; // P1 serves towards -Z
                    if (serverSide === -1 && neighborVelocity.z < 0.5) neighborVelocity.z = 0.5;   // P2 serves towards +Z


                    const simResult = this._simulateServeTrajectory(neighborVelocity, hitPosition, desiredSpin, serverSide, targetOpponentBouncePos, MAX_SIMULATION_TICKS, NET_CLEARANCE_MIN, TARGET_BOUNCE_DISTANCE_TOLERANCE);
                    const neighborScore = this._evaluateServeTrajectory(simResult, NET_CLEARANCE_MIN);

                    if (neighborScore < currentBestScore) {
                        currentBestScore = neighborScore;
                        candidateVelocity = neighborVelocity.clone(); // Store this promising neighbor
                        improvementFoundThisIteration = true;

                        if (simResult.success) {
                            if (!overallBestSuccessfulVelocity ||
                                (overallBestSuccessfulVelocity && Math.abs(neighborVelocity.z) > Math.abs(overallBestSuccessfulVelocity.z))) {
                                // Prioritize successful serves, and among them, faster ones (like original logic)
                                overallBestSuccessfulVelocity = neighborVelocity.clone();
                                console.log(`New best successful serve found: V=(${neighborVelocity.x.toFixed(2)}, ${neighborVelocity.y.toFixed(2)}, ${neighborVelocity.z.toFixed(2)}), Score=${neighborScore.toFixed(0)}, TargetDist=${simResult.distanceToTarget.toFixed(3)}`);
                            }
                        }
                    }
                }
            }

            if (improvementFoundThisIteration && candidateVelocity) {
                currentBestVelocity.copy(candidateVelocity); // Move to the best neighbor found
                noImprovementStreak = 0; // Reset streak
            } else {
                noImprovementStreak++;
                if (noImprovementStreak >= noImprovementThreshold) {
                    stepSize *= stepSizeDecay; // Reduce step size
                    noImprovementStreak = 0;   // Reset streak for new step size
                    if (stepSize < minStepSize) {
                        console.log("Hill climbing converged or stuck (min step size reached).");
                        break; // Stop if step size is too small
                    }
                }
            }
            if (i % 10 === 0) { // Log progress occasionally
                 console.log(`Iter ${i}: Best score so far: ${currentBestScore.toFixed(0)}, Step: ${stepSize.toFixed(3)}, Current V: (${currentBestVelocity.x.toFixed(2)},${currentBestVelocity.y.toFixed(2)},${currentBestVelocity.z.toFixed(2)})`);
            }
        }

        if (overallBestSuccessfulVelocity) {
            console.log("Optimal Serve Found (Hill Climbing): ", overallBestSuccessfulVelocity);
            return overallBestSuccessfulVelocity;
        }

        // Fallback if no successful trajectory is found by hill climbing
        console.warn("Could not find an optimal serve trajectory (Hill Climbing), using fallback serve.");
        let fallbackVx = (targetOpponentBouncePos.x - hitPosition.x) / 0.4;
        let fallbackVz = (targetOpponentBouncePos.z - hitPosition.z) / 0.4;
        if (Math.abs(fallbackVz) < 2.0) fallbackVz = serverSide * -3.0;
        fallbackVx = THREE.MathUtils.clamp(fallbackVx, -1.5, 1.5);
        return new THREE.Vector3(fallbackVx, 1.8, fallbackVz);
    }

    // Scoring function for serve trajectory evaluation
    _evaluateServeTrajectory(simulationResult, NET_CLEARANCE_MIN) {
        let score = 0;
        const PENALTY_BASE_FAILURE = 1000;
        const PENALTY_NO_SERVER_BOUNCE = 5000;
        const PENALTY_NO_NET_CLEARANCE = 2000;
        const PENALTY_HIT_FLOOR = 600;
        const PENALTY_OUT_OF_BOUNDS = 400;
        const PENALTY_INVALID_BOUNCE_STATE = 500;
        const PENALTY_MAX_TICKS = 200;
        const PENALTY_MISSED_TARGET_AREA = 300; // For "missed target or net after first bounce"

        // Fundamental requirement: bounced on server side
        if (!simulationResult.bouncedOnServer) {
            score += PENALTY_NO_SERVER_BOUNCE;
        }

        // Fundamental requirement: cleared net (unless already penalized by specific reason)
        if (!simulationResult.clearedNet && simulationResult.reason !== "did not clear net") {
            score += PENALTY_NO_NET_CLEARANCE;
        }

        if (simulationResult.success) {
            // Primary score component for successful serves
            score += simulationResult.distanceToTarget * 100; // Scale factor for distance

            // Net clearance preference: encourage clearing by a bit more than MIN, but not too much
            const idealNetClearance = NET_CLEARANCE_MIN + 0.03; // e.g., clear by 3cm + min
            const clearanceDeviation = Math.abs(simulationResult.netClearanceHeight - idealNetClearance);
            score += clearanceDeviation * 500; // Penalty for deviating from ideal clearance

            // Bonus for being a success (negative score indicates better)
            score -= PENALTY_BASE_FAILURE; // Counteract base failure penalty if it were to be added later
        } else {
            score += PENALTY_BASE_FAILURE;

            switch (simulationResult.reason) {
                case "did not clear net":
                    // Penalty proportional to how much it missed by, plus a base
                    score += PENALTY_NO_NET_CLEARANCE; // Already a strong penalty
                    score += Math.abs(simulationResult.netClearanceHeight) * 10000; // Heavy penalty for missing net
                    if (simulationResult.netClearanceHeight < -0.05) { // Hit well below net top
                        score += 500; // Extra penalty for very low hit
                    }
                    break;
                case "missed first bounce (out of width)":
                    score += PENALTY_OUT_OF_BOUNDS * 1.5; // Higher than generic out of bounds
                    break;
                case "out of bounds X on opponent side":
                    score += PENALTY_OUT_OF_BOUNDS;
                    // Could add penalty proportional to distance out of bounds if available
                    break;
                case "hit floor":
                    score += PENALTY_HIT_FLOOR;
                    // Add penalty based on where it hit the floor relative to the table
                    if (simulationResult.finalSimPosition) {
                        const distFromTableCenter = simulationResult.finalSimPosition.length(); // Simplified
                        score += distFromTableCenter * 10;
                    }
                    break;
                case "invalid bounce state":
                    score += PENALTY_INVALID_BOUNCE_STATE;
                    break;
                case "max simulation ticks reached":
                    score += PENALTY_MAX_TICKS;
                    // This case might still have useful info like distanceToTarget if it bounced on opponent side
                    if (simulationResult.bouncedOnOpponent && simulationResult.distanceToTarget !== Infinity) {
                        score += simulationResult.distanceToTarget * 150; // Higher factor than success due to uncertainty
                    }
                    break;
                case "missed target or net after first bounce":
                    score += PENALTY_MISSED_TARGET_AREA;
                    if (simulationResult.distanceToTarget !== Infinity) {
                        score += simulationResult.distanceToTarget * 120;
                    }
                    if (!simulationResult.clearedNet) { // Double check if net was an issue
                        score += PENALTY_NO_NET_CLEARANCE / 2; // Add half, as it's a complex failure
                    }
                    break;
                default:
                    score += 100; // Small penalty for unknown failure reason
            }
        }
        return score;
    }
}