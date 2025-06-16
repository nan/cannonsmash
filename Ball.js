// Ball.js - Applying fixes to calculatePerfectServeVelocity
import * as THREE from 'three';

// (Constants as in Turn 191)
const TABLE_HEIGHT = 0.76;         
const BALL_RADIUS = 0.02;          
const GRAVITY = 9.82;              
const TICK = 1/60; 
const AIR_RESISTANCE_FACTOR = 0.02; 
const SPIN_DECAY_FACTOR = 0.03;
const MAGNUS_COEFFICIENT = 0.02; // Estimated value for Magnus effect
const BOUNCE_ENERGY_LOSS = 0.85;    
// Adjusted spin effects on bounce (reduced by ~50% due to continuous Magnus effect)
const SPIN_EFFECT_ON_BOUNCE_Y = 0.1;
const SPIN_EFFECT_ON_BOUNCE_Z = 0.075;
const SPIN_EFFECT_ON_BOUNCE_X = 0.05;
const TABLE_LENGTH = 2.74;          
const TABLE_WIDTH = 1.525;          
const NET_HEIGHT = 0.1525;          
const NET_POS_Z = 0;                
const TARGET_BOUNCE_DISTANCE_TOLERANCE = 0.05; // Max distance from target for opponent bounce

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
    updatePhysics(player1RacketMesh, player2RacketMesh) { 
        this.velocity.y -= GRAVITY * TICK; 
        this.velocity.multiplyScalar(1 - AIR_RESISTANCE_FACTOR * TICK); 
        this.spin.multiplyScalar(1 - SPIN_DECAY_FACTOR * TICK); 
        this.position.addScaledVector(this.velocity, TICK);
        if (this.position.y > TABLE_HEIGHT && 
            this.position.y < TABLE_HEIGHT + NET_HEIGHT + this.radius &&
            Math.abs(this.position.z - NET_POS_Z) < this.radius + 0.01) { 
            const movingTowardsNetZ = (this.velocity.z > 0 && this.position.z < NET_POS_Z + this.radius) || 
                                      (this.velocity.z < 0 && this.position.z > NET_POS_Z - this.radius);   
            if (movingTowardsNetZ) {
                console.log("Ball hit net body");
                this.velocity.z *= -0.3; this.velocity.y *= 0.4; this.spin.y *= 0.5;      
                this.position.z += Math.sign(this.velocity.z) * this.radius * 0.2; this.status = -1;        
            }
        }
        const ballOnTableX = Math.abs(this.position.x) <= TABLE_WIDTH / 2 + this.radius;
        const ballOnTableZ = Math.abs(this.position.z) <= TABLE_LENGTH / 2 + this.radius;
        if (this.position.y < TABLE_HEIGHT + this.radius && this.velocity.y < 0 && ballOnTableX && ballOnTableZ) {
            this.position.y = TABLE_HEIGHT + this.radius; 
            const preBounceVelocityY = this.velocity.y; 
            this.velocity.y *= -BOUNCE_ENERGY_LOSS;     
            this.velocity.y += this.spin.y * SPIN_EFFECT_ON_BOUNCE_Y * Math.abs(preBounceVelocityY); 
            this.velocity.z -= this.spin.y * SPIN_EFFECT_ON_BOUNCE_Z; 
            this.velocity.x += this.spin.x * SPIN_EFFECT_ON_BOUNCE_X; 
            this.spin.y *= 0.6; this.spin.x *= 0.7;
            console.log(`Ball bounced on table. X: ${this.position.x.toFixed(2)}, Z: ${this.position.z.toFixed(2)}, Side: ${this.position.z >= NET_POS_Z ? "P1_Side(Pos-Z)" : "P2_Side(Neg-Z)"}`);
            if (this.position.z >= NET_POS_Z) { this.bouncedOnServerSide = true; } 
            else { this.bouncedOnReceiverSide = true; }
        }
        if (this.position.y < this.radius && this.velocity.y < 0) { 
            console.log("Ball hit floor"); this.status = -2; }
        if (Math.abs(this.position.x) > TABLE_WIDTH / 2 + this.radius) {
            console.log("Ball out of table width (sideways)"); this.status = -3; }
        if (this.position.z > (TABLE_LENGTH / 2 + this.radius * 2) && this.velocity.z > 0) { 
            if (!this.bouncedOnServerSide && !(ballOnTableX && this.position.y <= TABLE_HEIGHT + this.radius)) this.status = -4; }
        if (this.position.z < -(TABLE_LENGTH / 2 + this.radius * 2) && this.velocity.z < 0) { 
             if (!this.bouncedOnReceiverSide && !(ballOnTableX && this.position.y <= TABLE_HEIGHT + this.radius)) this.status = -5; }
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
            newVelocity.y = 1.8 + Math.random() * 0.5; 
            const baseZSpeed = 2.5 + Math.random() * 1.0; 
            newVelocity.z = (hittingPlayerID === 1) ? -baseZSpeed : baseZSpeed; 
            newVelocity.x = impactOffset.x * (hittingPlayerID === 1 ? -5.0 : 5.0) + (Math.random() - 0.5);
            let newSpin = new THREE.Vector2(newVelocity.x * 0.5, 2 + Math.random() * 3); 
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

    // Note: simBallSpin is a THREE.Vector2: simBallSpin.x is sidespin (around world Y), simBallSpin.y is top/backspin (around world X or ball's local X)
    _simulatePhysicsStep(simBallPosition, simBallVelocity, simBallSpin) {
        // 1. Gravity
        simBallVelocity.y -= GRAVITY * TICK;

        // 2. Air Resistance
        simBallVelocity.multiplyScalar(1 - AIR_RESISTANCE_FACTOR * TICK);

        // 3. Magnus Effect
        // Assumed: simBallSpin.y is topspin/backspin (rotation around an X-like axis)
        //          simBallSpin.x is sidespin (rotation around a Y-like axis)
        // The primary velocity component interacting is simBallVelocity.z (forward motion)
        // Magnus force causes acceleration perpendicular to both spin axis and velocity vector.
        // For topspin (positive spin.y) on a ball moving with negative Vz (player 1 serve):
        //   - Spin axis: +X
        //   - Velocity: -Z
        //   - Force: spin x velocity => (+X) x (-Z) = +Y (if spin.y were angular velocity around X)
        //   However, it's simpler to use the derived effects:
        //   - Topspin pulls the ball down, backspin pulls it up.
        //   - Sidespin curves the ball left/right.

        // Effect of Topspin/Backspin (simBallSpin.y) on Vertical motion (Vy)
        // If simBallSpin.y > 0 is topspin, it should accelerate ball downwards (more negative Vy or less positive Vy).
        // If simBallVelocity.z is negative (player 1 serves forward), then simBallSpin.y * simBallVelocity.z is negative for topspin.
        // So, a positive MAGNUS_COEFFICIENT here will add this negative value, pushing it down. This seems correct.
        simBallVelocity.y += MAGNUS_COEFFICIENT * simBallSpin.y * simBallVelocity.z * TICK;

        // Effect of Sidespin (simBallSpin.x) on Horizontal motion (Vx)
        // If simBallSpin.x > 0 (e.g. server applies spin making ball go from their right to their left),
        // and simBallVelocity.z is negative (player 1 serves forward),
        // this should accelerate ball towards positive X (to server's right).
        // So, simBallSpin.x * simBallVelocity.z is negative.
        // We need a negative sign in the formula to achieve positive X acceleration.
        simBallVelocity.x -= MAGNUS_COEFFICIENT * simBallSpin.x * simBallVelocity.z * TICK;

        // 4. Update position
        simBallPosition.addScaledVector(simBallVelocity, TICK);
    }

    // (Duplicate _simulatePhysicsStep removed)

    // Checks if the ball clears the net during the current tick
    // Returns true if cleared, false otherwise.
    _checkNetClearance(prevSimBallPosition, simBallPosition, serverSide, netTopY, netClearanceMin) {
        let crossedNetInThisTick = false;
        if (serverSide === 1) { // Server is on positive Z side
            crossedNetInThisTick = (prevSimBallPosition.z > NET_POS_Z && simBallPosition.z <= NET_POS_Z);
        } else { // Server is on negative Z side
            crossedNetInThisTick = (prevSimBallPosition.z < NET_POS_Z && simBallPosition.z >= NET_POS_Z);
        }

        if (crossedNetInThisTick) {
            let yAtNet = netTopY + 1.0; // Assume it clears initially
            // Interpolate Y position at the exact Z of the net
            if (Math.abs(simBallPosition.z - prevSimBallPosition.z) > 1e-6) { // Avoid division by zero
                const alpha = (NET_POS_Z - prevSimBallPosition.z) / (simBallPosition.z - prevSimBallPosition.z);
                yAtNet = prevSimBallPosition.y + (simBallPosition.y - prevSimBallPosition.y) * alpha;
            }
            return yAtNet > netTopY + netClearanceMin;
        }
        return false; // Did not cross net in this tick, or already crossed.
    }

    // Handles table bounce logic for the serve simulation.
    // Modifies simBallVelocity and returns an object:
    // { bounceValid: bool, newHasBouncedOnServerSide: bool, targetHit: bool }
    _handleServeTableBounce(simBallPosition, simBallVelocity, simBallSpin, serverSide, currentHasBouncedOnServerSide,
                             targetOpponentBouncePos, hasClearedNet) {

        simBallPosition.y = TABLE_HEIGHT + BALL_RADIUS; // Correct position to be exactly on table.

        const isOnServerHalf = (serverSide === 1) ? (simBallPosition.z > NET_POS_Z) : (simBallPosition.z < NET_POS_Z);
        let bounceDetails = {
            bounceValid: false,
            newHasBouncedOnServerSide: currentHasBouncedOnServerSide,
            targetHit: false
        };

        if (!currentHasBouncedOnServerSide && isOnServerHalf) {
            // First bounce attempt on server's side.
            if (Math.abs(simBallPosition.x) <= TABLE_WIDTH / 2) { // Check if it's within table width.
                bounceDetails.newHasBouncedOnServerSide = true;
                bounceDetails.bounceValid = true;
                // Apply bounce physics.
                const preBounceVy = simBallVelocity.y;
                simBallVelocity.y *= -BOUNCE_ENERGY_LOSS; // Reverse and reduce Y velocity.
                // Apply spin effects.
                simBallVelocity.y += simBallSpin.y * SPIN_EFFECT_ON_BOUNCE_Y * Math.abs(preBounceVy);
                simBallVelocity.x += simBallSpin.x * SPIN_EFFECT_ON_BOUNCE_X;
                simBallVelocity.z -= simBallSpin.y * SPIN_EFFECT_ON_BOUNCE_Z;
            } else {
                // Bounced out of bounds (X) on server side.
                bounceDetails.bounceValid = false;
            }
        } else if (currentHasBouncedOnServerSide && !isOnServerHalf) {
            // Second bounce attempt, should be on opponent's side.
            if (Math.abs(simBallPosition.x) <= TABLE_WIDTH / 2) { // Check if it's within table width.
                bounceDetails.bounceValid = true;
                // No change to newHasBouncedOnServerSide, it remains true.
                // Check distance to target.
                const distanceToTarget = Math.sqrt(
                    Math.pow(simBallPosition.x - targetOpponentBouncePos.x, 2) +
                    Math.pow(simBallPosition.z - targetOpponentBouncePos.z, 2)
                );

                if (distanceToTarget < TARGET_BOUNCE_DISTANCE_TOLERANCE && hasClearedNet) {
                    bounceDetails.targetHit = true; // Successful hit on opponent's side.
                } else {
                    // Missed target or didn't clear net properly (though hasClearedNet is for the trajectory to this point).
                    bounceDetails.targetHit = false;
                    // bounceValid remains true because it's a legitimate bounce on opponent's table side, just not at the target.
                }
            } else {
                // Bounced out of bounds (X) on opponent side.
                bounceDetails.bounceValid = false;
            }
        } else {
            // Invalid bounce sequence (e.g., two bounces on server side, or first bounce on opponent side).
            bounceDetails.bounceValid = false;
        }
        return bounceDetails;
    }

    /**
     * Calculates an optimal initial velocity for a serve.
     * Iterates through a range of initial velocities (Vx, Vy, Vz) and simulates the ball trajectory
     * to find one that meets serve criteria:
     * 1. Bounces once on the server's side of the table.
     * 2. Clears the net.
     * 3. Bounces on the opponent's side of the table, close to the targetOpponentBouncePos.
     *
     * @param {THREE.Vector3} hitPosition - The starting position of the ball (where the racket hits it).
     * @param {number} serverSide - 1 if serving from positive Z side, -1 if from negative Z side.
     * @param {number} firstBounceServerZ - (Currently unused) Expected Z position for the first bounce.
     * @param {THREE.Vector3} targetOpponentBouncePos - The desired X,Z coordinates for the bounce on the opponent's side.
     * @param {THREE.Vector2} desiredSpin - The spin (topspin/backspin, sidespin) to apply to the ball.
     * @returns {THREE.Vector3} The calculated initial velocity for the serve, or a fallback velocity if no optimal one is found.
     */
    calculatePerfectServeVelocity(hitPosition, serverSide, firstBounceServerZ, targetOpponentBouncePos, desiredSpin, serveTypeConfig = {}) {
        console.log("Calculating serve velocity (Multi-Stage Search)...");

        // --- Configuration for Multi-Stage Search ---
        const COARSE_STEP_VY = 0.5;
        const COARSE_STEP_VZ = 0.5;
        const COARSE_STEP_VX = 0.5;
        const FINE_STEP_VY = 0.1;
        const FINE_STEP_VZ = 0.1;
        const FINE_STEP_VX = 0.1;
        const NUM_TOP_COARSE_RESULTS = 5;

        // --- Process serveTypeConfig ---
        const config = {
            serveType: serveTypeConfig.serveType || "default",
            targetNetClearance: serveTypeConfig.targetNetClearance !== undefined ? serveTypeConfig.targetNetClearance : 0.002, // Default: 2mm
            targetDepthPercentage: serveTypeConfig.targetDepthPercentage !== undefined ? serveTypeConfig.targetDepthPercentage : 0.7, // Default: 70% depth
            // preferredSpinMagnitude: serveTypeConfig.preferredSpinMagnitude || 0, // Not directly used in search criteria yet
        };

        console.log("Serve Config Applied:", config);

        // --- Constants for simulation ---
        const NET_TOP_Y = TABLE_HEIGHT + NET_HEIGHT + BALL_RADIUS;
        const MAX_SIMULATION_TICKS = 300;
        // Use configured net clearance
        const CURRENT_NET_CLEARANCE_MIN = config.targetNetClearance;
        // const TARGET_BOUNCE_DISTANCE_TOLERANCE = 0.05; // Now a global constant
        const TABLE_SURFACE_Y = TABLE_HEIGHT + BALL_RADIUS;

        // Calculate actual target Z based on depth percentage
        // Opponent's half starts at z=0 and goes to z = -TABLE_LENGTH / 2 (for serverSide=1) or z = TABLE_LENGTH / 2 (for serverSide=-1)
        const opponentTableHalfLength = TABLE_LENGTH / 2;
        let actualTargetOpponentBounceZ = (serverSide === 1)
            ? -(config.targetDepthPercentage * opponentTableHalfLength)
            : (config.targetDepthPercentage * opponentTableHalfLength);

        // Ensure target is on opponent's side relative to net
        if (serverSide === 1 && actualTargetOpponentBounceZ > NET_POS_Z) actualTargetOpponentBounceZ = -actualTargetOpponentBounceZ;
        if (serverSide === -1 && actualTargetOpponentBounceZ < NET_POS_Z) actualTargetOpponentBounceZ = -actualTargetOpponentBounceZ;


        const actualTargetOpponentBouncePos = new THREE.Vector3(
            targetOpponentBouncePos.x, // X is still taken from the original parameter
            targetOpponentBouncePos.y, // Y is on table surface, usually not explicitly set here
            actualTargetOpponentBounceZ
        );
        console.log("Actual Target Bounce Position:", actualTargetOpponentBouncePos);


        let bestInitialVelocity = new THREE.Vector3(-Infinity, -Infinity, -Infinity);
        let bestTargetDistance = Infinity;
        let bestInitialSpeed = 0; // For "fast" serve type comparison

        let topCoarseResults = []; // To store {velocity: THREE.Vector3, distance: number}

        // --- Stage 1: Coarse Search ---
        console.log("Starting Coarse Search Stage...");
        const vyRange = { min: -5.0, max: 5.0 };
        const vzAbsRange = { min: 1.0, max: 7.0 };
        const vxRange = { min: -3.0, max: 3.0 };

        for (let initialVy = vyRange.min; initialVy <= vyRange.max; initialVy += COARSE_STEP_VY) {
            for (let initialVzAbs = vzAbsRange.min; initialVzAbs <= vzAbsRange.max; initialVzAbs += COARSE_STEP_VZ) {
                for (let initialVx = vxRange.min; initialVx <= vxRange.max; initialVx += COARSE_STEP_VX) {
                    const currentInitialVelocity = new THREE.Vector3(
                        initialVx, initialVy, serverSide * -initialVzAbs
                    );

                    let simBallPosition = hitPosition.clone();
                    let simBallVelocity = currentInitialVelocity.clone();
                    let simBallSpin = desiredSpin.clone();
                    let hasBouncedOnServerSide = false;
                    let hasClearedNet = false;

                    for (let tickCount = 0; tickCount < MAX_SIMULATION_TICKS; tickCount++) {
                        const prevSimBallPosition = simBallPosition.clone();
                        this._simulatePhysicsStep(simBallPosition, simBallVelocity, simBallSpin);

                        if (!hasClearedNet) {
                            if (this._checkNetClearance(prevSimBallPosition, simBallPosition, serverSide, NET_TOP_Y, CURRENT_NET_CLEARANCE_MIN)) {
                                hasClearedNet = true;
                            } else {
                                let crossedNetPlaneThisTick = (serverSide === 1) ? (prevSimBallPosition.z > NET_POS_Z && simBallPosition.z <= NET_POS_Z) : (prevSimBallPosition.z < NET_POS_Z && simBallPosition.z >= NET_POS_Z);
                                if (crossedNetPlaneThisTick) { break; }
                            }
                        }

                        if (simBallVelocity.y < 0 && simBallPosition.y <= TABLE_SURFACE_Y) {
                            const bounceResult = this._handleServeTableBounce(
                                simBallPosition, simBallVelocity, simBallSpin, serverSide,
                                hasBouncedOnServerSide, actualTargetOpponentBouncePos, hasClearedNet
                            );

                            if (!bounceResult.bounceValid) { break; }
                            hasBouncedOnServerSide = bounceResult.newHasBouncedOnServerSide;

                            if (bounceResult.targetHit) {
                                const distanceToTarget = Math.sqrt(
                                    Math.pow(simBallPosition.x - actualTargetOpponentBouncePos.x, 2) +
                                    Math.pow(simBallPosition.z - actualTargetOpponentBouncePos.z, 2)
                                );

                                if (topCoarseResults.length < NUM_TOP_COARSE_RESULTS || distanceToTarget < topCoarseResults[topCoarseResults.length - 1].distance) {
                                    topCoarseResults.push({ velocity: currentInitialVelocity.clone(), distance: distanceToTarget, speed: Math.abs(currentInitialVelocity.z) });
                                    topCoarseResults.sort((a, b) => a.distance - b.distance); // Primary sort by distance
                                    if (topCoarseResults.length > NUM_TOP_COARSE_RESULTS) {
                                        topCoarseResults.pop();
                                    }
                                }

                                let updateBest = false;
                                if (distanceToTarget < bestTargetDistance) {
                                    updateBest = true;
                                } else if (config.serveType === "fast" && distanceToTarget <= bestTargetDistance + (TARGET_BOUNCE_DISTANCE_TOLERANCE * 0.1) ) { // Allow slight distance trade-off for speed
                                    if (Math.abs(currentInitialVelocity.z) > bestInitialSpeed) {
                                        updateBest = true;
                                    }
                                }

                                if (updateBest) {
                                    bestTargetDistance = distanceToTarget;
                                    bestInitialVelocity = currentInitialVelocity.clone();
                                    bestInitialSpeed = Math.abs(bestInitialVelocity.z);
                                }
                            } else if (hasBouncedOnServerSide && !((serverSide === 1) ? (simBallPosition.z > NET_POS_Z) : (simBallPosition.z < NET_POS_Z))) {
                                break;
                            }
                        }
                        if (simBallPosition.y < 0) { break; }
                    }
                }
            }
        }
        console.log(`Coarse Search completed. Found ${topCoarseResults.length} promising candidates.`);

        // --- Stage 2: Fine Search ---
        if (topCoarseResults.length > 0) {
            console.log("Starting Fine Search Stage...");
            for (const coarseResult of topCoarseResults) {
                const coarseVel = coarseResult.velocity;
                // Define fine search windows around the coarse result velocity
                // Window is one coarse step wide, centered on the coarse velocity component
                const fineVyMin = coarseVel.y - COARSE_STEP_VY / 2;
                const fineVyMax = coarseVel.y + COARSE_STEP_VY / 2;
                const fineVzAbsMin = Math.abs(coarseVel.z) - COARSE_STEP_VZ / 2;
                const fineVzAbsMax = Math.abs(coarseVel.z) + COARSE_STEP_VZ / 2;
                const fineVxMin = coarseVel.x - COARSE_STEP_VX / 2;
                const fineVxMax = coarseVel.x + COARSE_STEP_VX / 2;

                for (let initialVy = fineVyMin; initialVy <= fineVyMax; initialVy += FINE_STEP_VY) {
                    for (let initialVzAbs = Math.max(0.1, fineVzAbsMin); initialVzAbs <= fineVzAbsMax; initialVzAbs += FINE_STEP_VZ) { // Ensure VzAbs is not zero or negative
                        for (let initialVx = fineVxMin; initialVx <= fineVxMax; initialVx += FINE_STEP_VX) {
                            const currentInitialVelocity = new THREE.Vector3(
                                initialVx, initialVy, serverSide * -initialVzAbs
                            );

                            let simBallPosition = hitPosition.clone();
                            let simBallVelocity = currentInitialVelocity.clone();
                            let simBallSpin = desiredSpin.clone();
                            let hasBouncedOnServerSide = false;
                            let hasClearedNet = false;

                            for (let tickCount = 0; tickCount < MAX_SIMULATION_TICKS; tickCount++) {
                                const prevSimBallPosition = simBallPosition.clone();
                                this._simulatePhysicsStep(simBallPosition, simBallVelocity, simBallSpin);

                                if (!hasClearedNet) {
                                    if (this._checkNetClearance(prevSimBallPosition, simBallPosition, serverSide, NET_TOP_Y, CURRENT_NET_CLEARANCE_MIN)) {
                                        hasClearedNet = true;
                                    } else {
                                        let crossedNetPlaneThisTick = (serverSide === 1) ? (prevSimBallPosition.z > NET_POS_Z && simBallPosition.z <= NET_POS_Z) : (prevSimBallPosition.z < NET_POS_Z && simBallPosition.z >= NET_POS_Z);
                                        if (crossedNetPlaneThisTick) { break; }
                                    }
                                }

                                if (simBallVelocity.y < 0 && simBallPosition.y <= TABLE_SURFACE_Y) {
                                    const bounceResult = this._handleServeTableBounce(
                                        simBallPosition, simBallVelocity, simBallSpin, serverSide,
                                        hasBouncedOnServerSide, actualTargetOpponentBouncePos, hasClearedNet
                                    );

                                    if (!bounceResult.bounceValid) { break; }
                                    hasBouncedOnServerSide = bounceResult.newHasBouncedOnServerSide;

                                    if (bounceResult.targetHit) {
                                        const distanceToTarget = Math.sqrt(
                                            Math.pow(simBallPosition.x - actualTargetOpponentBouncePos.x, 2) +
                                            Math.pow(simBallPosition.z - actualTargetOpponentBouncePos.z, 2)
                                        );

                                        let updateBest = false;
                                        if (distanceToTarget < bestTargetDistance) {
                                            updateBest = true;
                                        } else if (config.serveType === "fast" && distanceToTarget <= bestTargetDistance + (TARGET_BOUNCE_DISTANCE_TOLERANCE * 0.1)) { // Allow slight distance trade-off for speed
                                            if (Math.abs(currentInitialVelocity.z) > bestInitialSpeed) {
                                                updateBest = true;
                                            }
                                        }

                                        if (updateBest) {
                                            bestTargetDistance = distanceToTarget;
                                            bestInitialVelocity = currentInitialVelocity.clone();
                                            bestInitialSpeed = Math.abs(bestInitialVelocity.z);
                                            console.log(`Optimal Serve Candidate Updated (Fine Search ${config.serveType}): `, bestInitialVelocity, ` TargetDist: ${bestTargetDistance.toFixed(3)}, Speed: ${bestInitialSpeed.toFixed(2)}`);
                                        }
                                    } else if (hasBouncedOnServerSide && !((serverSide === 1) ? (simBallPosition.z > NET_POS_Z) : (simBallPosition.z < NET_POS_Z))) {
                                        break;
                                    }
                                }
                                if (simBallPosition.y < 0) { break; }
                            }
                        }
                    }
                }
            }
            console.log("Fine Search completed.");
        }


        if (bestInitialVelocity.x > -Infinity) {
            console.log("Best Optimal Serve Velocity selected:", bestInitialVelocity, ` MinDist: ${bestTargetDistance.toFixed(3)} Type: ${config.serveType}`);
            return bestInitialVelocity;
        }

        // --- New Fallback Strategy ---
        console.warn("Optimal serve not found by main search. Attempting preset safe fallback serves...");

        const safeServePresets = [
            // Vx needs to be relative to server aiming direction, Vz is absolute speed but corrected for serverSide
            { name: "CenterMid", velCfg: { vx: 0.0, vy: 2.5, vzAbs: 3.5 }, spin: new THREE.Vector2(0, 1) },
            { name: "SlightAngle", velCfg: { vx: 0.2, vy: 2.8, vzAbs: 3.0 }, spin: new THREE.Vector2(0.1, 0.5) },
            { name: "BitFaster", velCfg: { vx: 0.1, vy: 2.2, vzAbs: 4.0 }, spin: new THREE.Vector2(0, 1.5) }
        ];

        const GENEROUS_NET_CLEARANCE_FALLBACK = CURRENT_NET_CLEARANCE_MIN + 0.05; // More generous for fallback

        for (const preset of safeServePresets) {
            let currentInitialVelocity = new THREE.Vector3(
                preset.velCfg.vx * serverSide, // Make Vx relative to server side (e.g. positive vx aims to their right)
                preset.velCfg.vy,
                preset.velCfg.vzAbs * -serverSide // Vz is forward, so negative for P1, positive for P2
            );

            let simBallPosition = hitPosition.clone();
            let simBallVelocity = currentInitialVelocity.clone();
            let simBallSpin = preset.spin.clone();
            let hasBouncedOnServerSide_fb = false;
            let hasClearedNet_fb = false;
            let hasBouncedOnOpponentSide_fb = false; // Explicit flag for opponent bounce

            for (let tickCount = 0; tickCount < MAX_SIMULATION_TICKS; tickCount++) {
                const prevSimBallPosition = simBallPosition.clone();
                this._simulatePhysicsStep(simBallPosition, simBallVelocity, simBallSpin);

                if (!hasClearedNet_fb) {
                    if (this._checkNetClearance(prevSimBallPosition, simBallPosition, serverSide, NET_TOP_Y, GENEROUS_NET_CLEARANCE_FALLBACK)) {
                        hasClearedNet_fb = true;
                    } else {
                        let crossedNetPlaneThisTick = (serverSide === 1) ? (prevSimBallPosition.z > NET_POS_Z && simBallPosition.z <= NET_POS_Z) : (prevSimBallPosition.z < NET_POS_Z && simBallPosition.z >= NET_POS_Z);
                        if (crossedNetPlaneThisTick) { break; } // Hit net
                    }
                }

                if (simBallVelocity.y < 0 && simBallPosition.y <= TABLE_SURFACE_Y) {
                    simBallPosition.y = TABLE_SURFACE_Y; // Correct position to table surface
                    const isOnServerHalf = (serverSide === 1) ? (simBallPosition.z > NET_POS_Z) : (simBallPosition.z < NET_POS_Z);

                    if (!hasBouncedOnServerSide_fb && isOnServerHalf) {
                        if (Math.abs(simBallPosition.x) <= TABLE_WIDTH / 2) {
                            hasBouncedOnServerSide_fb = true;
                            // Simplified bounce: just energy loss, minimal spin effect for predictability
                            simBallVelocity.y *= -BOUNCE_ENERGY_LOSS;
                            simBallVelocity.x *= 0.9; // Dampen X
                            simBallVelocity.z *= 0.9; // Dampen Z
                        } else { break; } // Out of bounds X on server side
                    } else if (hasBouncedOnServerSide_fb && !isOnServerHalf && hasClearedNet_fb) {
                        if (Math.abs(simBallPosition.x) <= TABLE_WIDTH / 2) {
                            hasBouncedOnOpponentSide_fb = true; // Mark opponent bounce
                            // This is a successful simplified fallback serve
                            console.log(`Safe Fallback Serve successful with preset '${preset.name}':`, currentInitialVelocity);
                            return currentInitialVelocity; // Return the initial velocity of this successful preset
                        } else { break; } // Out of bounds X on opponent side
                    } else if ((hasBouncedOnServerSide_fb && isOnServerHalf) || (!hasBouncedOnServerSide_fb && !isOnServerHalf && simBallPosition.y === TABLE_SURFACE_Y)) {
                        // Bounced twice on server side, or bounced on opponent side first without clearing net or server bounce.
                        break;
                    }
                }
                if (simBallPosition.y < 0) { break; } // Hit floor
            }
        }

        // --- Ultimate Fallback (Original Basic Heuristic) ---
        console.warn("All fallback attempts (main search and presets) failed. Using original very basic heuristic fallback serve.");
        const finalFallbackTargetPos = (actualTargetOpponentBouncePos.z !== targetOpponentBouncePos.z && Math.abs(actualTargetOpponentBouncePos.z) > 0.1 ) ? actualTargetOpponentBouncePos : targetOpponentBouncePos;

        let fallbackVx = (finalFallbackTargetPos.x - hitPosition.x) / 0.5;
        let fallbackVz = (finalFallbackTargetPos.z - hitPosition.z) / 0.5;

        if (Math.abs(fallbackVz) < 2.0) fallbackVz = serverSide * -3.0;
        if (Math.abs(fallbackVz) > 7.0) fallbackVz = serverSide * -7.0;

        fallbackVx = THREE.MathUtils.clamp(fallbackVx, -2.0, 2.0);

        let fallbackVy = 2.0;
        if (Math.abs(finalFallbackTargetPos.z - hitPosition.z) > TABLE_LENGTH * 0.6) { // If target is deep
            fallbackVy = 2.5;
        }
        if (config.serveType === "short" || (finalFallbackTargetPos.z / serverSide > -TABLE_LENGTH * 0.25)) { // if target is very short
             fallbackVy = 1.5;
        }


        console.log("Basic Heuristic Fallback Applied. Target:", finalFallbackTargetPos, "Initial Vel:", new THREE.Vector3(fallbackVx, fallbackVy, fallbackVz));
        return new THREE.Vector3(fallbackVx, fallbackVy, fallbackVz);
    }
}

