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
    // Returns an object: { success: boolean, reason?: string, finalSimPosition: THREE.Vector3, clearedNet: boolean, bouncedOnServer: boolean, bouncedOnOpponent: boolean }
    _simulateServeTrajectory(initialVelocity, hitPosition, desiredSpin, serverSide, targetOpponentBouncePos, MAX_SIMULATION_TICKS, NET_CLEARANCE_MIN, TARGET_BOUNCE_DISTANCE_TOLERANCE) {
        let simBallPosition = hitPosition.clone();
        let simBallVelocity = initialVelocity.clone();
        let simBallSpin = desiredSpin.clone(); // Spin is applied but assumed constant for this simplified simulation

        let hasBouncedOnServerSide = false;
        let hasClearedNet = false;
        let hasBouncedOnOpponentSide = false;

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

                if (yAtNet > NET_TOP_Y + NET_CLEARANCE_MIN) {
                    hasClearedNet = true;
                } else {
                    // Failed to clear the net
                    return { success: false, reason: "did not clear net", finalSimPosition: simBallPosition.clone(), clearedNet: false, bouncedOnServer: hasBouncedOnServerSide, bouncedOnOpponent: hasBouncedOnOpponentSide };
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
                        // Apply simplified bounce physics (energy loss and spin effects)
                        const preBounceVy = simBallVelocity.y;
                        simBallVelocity.y *= -BOUNCE_ENERGY_LOSS; // Vertical velocity reverses and loses energy
                        // Spin effects on bounce (simplified)
                        simBallVelocity.y += simBallSpin.y * SPIN_EFFECT_ON_BOUNCE_Y * Math.abs(preBounceVy);
                        simBallVelocity.x += simBallSpin.x * SPIN_EFFECT_ON_BOUNCE_X;
                        simBallVelocity.z -= simBallSpin.y * SPIN_EFFECT_ON_BOUNCE_Z;
                    } else {
                        // Missed the table width on the first bounce
                        return { success: false, reason: "missed first bounce (out of width)", finalSimPosition: simBallPosition.clone(), clearedNet: hasClearedNet, bouncedOnServer: false, bouncedOnOpponent: hasBouncedOnOpponentSide };
                    }
                }
                else if (hasBouncedOnServerSide && !isOnServerHalf) { // Second bounce, should be on opponent's side
                    if (isWithinTableWidth) {
                        hasBouncedOnOpponentSide = true;
                        // Check if the bounce is close enough to the target position
                        const targetDist = Math.sqrt(
                            Math.pow(simBallPosition.x - targetOpponentBouncePos.x, 2) +
                            Math.pow(simBallPosition.z - targetOpponentBouncePos.z, 2)
                        );

                        if (!(targetDist < TARGET_BOUNCE_DISTANCE_TOLERANCE && hasClearedNet)) {
                            // Bounced on opponent side but missed target or hadn't cleared net properly
                            return { success: false, reason: "missed target or net after first bounce", finalSimPosition: simBallPosition.clone(), clearedNet: hasClearedNet, bouncedOnServer: true, bouncedOnOpponent: true };
                        }
                        // If conditions met, we don't return immediately; success is checked at the end of the loop or after MAX_SIMULATION_TICKS.
                    } else {
                        // Out of table width on the opponent's side
                        return { success: false, reason: "out of bounds X on opponent side", finalSimPosition: simBallPosition.clone(), clearedNet: hasClearedNet, bouncedOnServer: true, bouncedOnOpponent: false };
                    }
                } else {
                    // Invalid bounce state (e.g., bounced twice on server side, or bounced on receiver side first without server bounce)
                    return { success: false, reason: "invalid bounce state", finalSimPosition: simBallPosition.clone(), clearedNet: hasClearedNet, bouncedOnServer: hasBouncedOnServerSide, bouncedOnOpponent: hasBouncedOnOpponentSide };
                }
            }

            // Check for hitting the floor (ball Y position is below zero)
            if (simBallPosition.y < 0) {
                return { success: false, reason: "hit floor", finalSimPosition: simBallPosition.clone(), clearedNet: hasClearedNet, bouncedOnServer: hasBouncedOnServerSide, bouncedOnOpponent: hasBouncedOnOpponentSide };
            }

            // Check for successful serve conditions only if all stages have been passed in order
            if (hasBouncedOnServerSide && hasClearedNet && hasBouncedOnOpponentSide) {
                 const targetDist = Math.sqrt(
                    Math.pow(simBallPosition.x - targetOpponentBouncePos.x, 2) +
                    Math.pow(simBallPosition.z - targetOpponentBouncePos.z, 2)
                );
                // This success condition is checked *after* the bounce has been processed.
                // If the ball has bounced on server, cleared net, and bounced on opponent side near target, it's a success.
                if (targetDist < TARGET_BOUNCE_DISTANCE_TOLERANCE) {
                    return { success: true, clearedNet: true, bouncedOnServer: true, bouncedOnOpponent: true, finalSimPosition: simBallPosition.clone() };
                }
                // If it bounced on opponent side but rolled away from target, the previous check "missed target or net after first bounce" would have caught it.
            }
        }
        // If loop finishes, it means MAX_SIMULATION_TICKS reached without a conclusive success or definitive failure (like hitting floor/net).
        // This trajectory is considered unsuccessful.
        return { success: false, reason: "max simulation ticks reached", finalSimPosition: simBallPosition.clone(), clearedNet: hasClearedNet, bouncedOnServer: hasBouncedOnServerSide, bouncedOnOpponent: hasBouncedOnOpponentSide };
    }

    calculatePerfectServeVelocity(hitPosition, serverSide, firstBounceServerZ_UNUSED, targetOpponentBouncePos, desiredSpin) {
        console.log("Calculating serve velocity (Iterative Search)...");

        const MAX_SIMULATION_TICKS = 300;
        // const FIRST_BOUNCE_Z_TOLERANCE = 0.10; // This was unused, confirming removal
        const NET_CLEARANCE_MIN = 0.002;
        const TARGET_BOUNCE_DISTANCE_TOLERANCE = 0.05;

        let bestInitialVelocity = new THREE.Vector3(
            -3.0, // Default Vx
            -5.0, // Default Vy
            serverSide * -2.0 // Default Vz, ensures it moves towards opponent
        );
        let foundOptimalServe = false;

        // Iterate through a range of possible initial velocities (Vy, Vz, Vx)
        // Vy: Vertical velocity component
        // VzAbs: Absolute depth velocity component (sign determined by serverSide)
        // Vx: Sideways velocity component
        for (let initialVy = -5.0; initialVy <= 5.0; initialVy += 0.1) { // Renamed from Vy_initial_loop
            for (let initialVzAbs = 1.0; initialVzAbs <= 7.0; initialVzAbs += 0.1) { // Renamed from Vz_initial_abs_loop
                for (let initialVx = -3.0; initialVx <= 3.0; initialVx += 0.1) { // Renamed from Vx_initial_loop
                    
                    const currentInitialVelocity = new THREE.Vector3(
                        initialVx,
                        initialVy,
                        serverSide * -initialVzAbs // Adjust Vz direction based on server side
                    );

                    // Simulate the trajectory with the current initial velocity
                    const simulationResult = this._simulateServeTrajectory(
                        currentInitialVelocity,
                        hitPosition,
                        desiredSpin,
                        serverSide,
                        // firstBounceServerZ_UNUSED, // Parameter removed
                        targetOpponentBouncePos,
                        MAX_SIMULATION_TICKS,
                        // FIRST_BOUNCE_Z_TOLERANCE, // Parameter removed
                        NET_CLEARANCE_MIN,
                        TARGET_BOUNCE_DISTANCE_TOLERANCE
                    );

                    // If the simulation resulted in a successful serve
                    if (simulationResult.success) {
                        // Log found optimal serve and its characteristics
                        console.log("Optimal Serve Found (Iterative): ", currentInitialVelocity, simulationResult.finalSimPosition, 
                                    ` TargetDist: ${Math.sqrt(Math.pow(simulationResult.finalSimPosition.x - targetOpponentBouncePos.x, 2) + Math.pow(simulationResult.finalSimPosition.z - targetOpponentBouncePos.z, 2)).toFixed(3)}`);
                        console.log(`[Test Log] Simulated second bounce Z: ${simulationResult.finalSimPosition.z.toFixed(4)}`);
                        
                        // If this is the first optimal serve found, or if this serve is "stronger" (higher absolute Vz),
                        // update the bestInitialVelocity. This prioritizes faster serves if multiple solutions are found.
                        if (!foundOptimalServe || Math.abs(bestInitialVelocity.z) < Math.abs(currentInitialVelocity.z)) {
                            bestInitialVelocity.copy(currentInitialVelocity);
                            foundOptimalServe = true;
                        }
                    }
                }
            }
        }

        if (foundOptimalServe) {
            return bestInitialVelocity; // Return the best velocity found
        }

        // Fallback if no optimal trajectory is found after checking all combinations
        console.warn("Could not find an optimal serve trajectory (Iterative Search), using fallback serve.");
        let fallbackVx = (targetOpponentBouncePos.x - hitPosition.x) / 0.4; // Heuristic for Vx
        let fallbackVz = (targetOpponentBouncePos.z - hitPosition.z) / 0.4; // Heuristic for Vz
        if (Math.abs(fallbackVz) < 2.0) fallbackVz = serverSide * -3.0; // Ensure minimum speed for Vz
        fallbackVx = THREE.MathUtils.clamp(fallbackVx, -1.5, 1.5); // Clamp Vx to reasonable limits
        return new THREE.Vector3(fallbackVx, 1.8, fallbackVz); // Return a default fallback velocity
    }
}