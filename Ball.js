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
            console.log(`Ball bounced on table. Z: ${this.position.z.toFixed(2)}, Side: ${this.position.z >= NET_POS_Z ? "P1_Side(Pos-Z)" : "P2_Side(Neg-Z)"}`);
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
        console.log(`Ball hit by Player ${hittingPlayerID}! Status: ${this.status}, Vel:(${this.velocity.x.toFixed(1)},${this.velocity.y.toFixed(1)},${this.velocity.z.toFixed(1)})`);
    }

    calculatePerfectServeVelocity(hitPosition, serverSide, firstBounceServerZ, targetOpponentBouncePos, desiredSpin) {
        console.log("Calculating serve velocity (Iterative Refinement - Fix Attempt)...");
        
        const targetY = TABLE_HEIGHT + BALL_RADIUS;
        const NET_TOP_Y = TABLE_HEIGHT + NET_HEIGHT + BALL_RADIUS;

        // Corrected X Interpolation for firstBouncePosServerCourt
        const dZ_total_path = targetOpponentBouncePos.z - hitPosition.z;
        const dZ_segment1_abs = Math.abs(firstBounceServerZ - hitPosition.z);
        const dZ_total_path_abs = Math.abs(dZ_total_path);
        
        let firstBounceX;
        if (dZ_total_path_abs > 0.01) { // Avoid division by zero if total path Z is negligible
            firstBounceX = hitPosition.x + (targetOpponentBouncePos.x - hitPosition.x) * (dZ_segment1_abs / dZ_total_path_abs);
        } else {
            firstBounceX = hitPosition.x; 
        }

        const firstBouncePosServerCourt = new THREE.Vector3(
            firstBounceX,
            targetY,
            firstBounceServerZ
        );

        const deltaX1 = firstBouncePosServerCourt.x - hitPosition.x;
        const deltaY1 = firstBouncePosServerCourt.y - hitPosition.y;
        const deltaZ1 = firstBouncePosServerCourt.z - hitPosition.z;

        let bestVy1 = null;
        let minDiff = Infinity;
        let optimalT1 = -1;

        // Iterate on t1_guess with a wider range and finer step
        for (let t1_guess = 0.08; t1_guess <= 0.50; t1_guess += 0.0025) { 
            if (t1_guess < 0.02) continue; // Should not be needed with loop start > 0.02

            const Vy1_current = (deltaY1 / t1_guess) + (0.5 * GRAVITY * t1_guess);
            const Vx1_current = deltaX1 / t1_guess;
            const Vz1_current = deltaZ1 / t1_guess;

            // Heuristics to prune unlikely trajectories early
            if (Vy1_current < 0 && deltaY1 > 0.05) continue; // Avoid launching downwards if target is up
            // if (Vy1_current < 0.2 && deltaY1 > 0.01 && Math.abs(deltaZ1) > 0.3) continue; // Ensure some upward for typical toss hit

            // Simulate first bounce
            let Vy_afterBounce1 = -Vy1_current * BOUNCE_ENERGY_LOSS;
            Vy_afterBounce1 += desiredSpin.y * SPIN_EFFECT_ON_BOUNCE_Y * Math.abs(Vy1_current);
            // Apply energy loss to horizontal components as well, then spin effect
            let Vx_afterBounce1 = Vx1_current * BOUNCE_ENERGY_LOSS + (desiredSpin.x * SPIN_EFFECT_ON_BOUNCE_X);
            let Vz_afterBounce1 = Vz1_current * BOUNCE_ENERGY_LOSS - (desiredSpin.y * SPIN_EFFECT_ON_BOUNCE_Z);

            // Trajectory for second segment
            const deltaX2 = targetOpponentBouncePos.x - firstBouncePosServerCourt.x;
            const deltaY2 = targetOpponentBouncePos.y - firstBouncePosServerCourt.y; // Should be 0
            const deltaZ2 = targetOpponentBouncePos.z - firstBouncePosServerCourt.z;

            let t2_estimated = 0.2; 
            if (Math.abs(Vz_afterBounce1) > 0.1) { // Prefer Z for time estimation if possible
                t2_estimated = deltaZ2 / Vz_afterBounce1;
            } else if (Math.abs(Vx_afterBounce1) > 0.1) { 
                t2_estimated = deltaX2 / Vx_afterBounce1;
            }
            t2_estimated = Math.max(0.05, Math.min(t2_estimated, 0.6)); // Clamp t2 to avoid extreme values

            if (t2_estimated <= 0.049) continue; // If t2 is still too small, skip

            const Vy2_required_at_bounce1 = (deltaY2 / t2_estimated) + (0.5 * GRAVITY * t2_estimated);

            // Robust Net Clearance Check
            let ballY_at_net = -Infinity;
            let time_to_net_from_bounce1 = -1;
            // Check if the ball is moving towards the net plane from the first bounce to the second target
            const movingTowardsNetAfterBounce = 
                (Vz_afterBounce1 > 0 && firstBouncePosServerCourt.z < NET_POS_Z && targetOpponentBouncePos.z > NET_POS_Z) ||
                (Vz_afterBounce1 < 0 && firstBouncePosServerCourt.z > NET_POS_Z && targetOpponentBouncePos.z < NET_POS_Z);

            if (movingTowardsNetAfterBounce && Math.abs(Vz_afterBounce1) > 0.01) {
                time_to_net_from_bounce1 = (NET_POS_Z - firstBouncePosServerCourt.z) / Vz_afterBounce1;
                // Ensure the time to net is within the segment duration t2_estimated
                if (time_to_net_from_bounce1 > 0.001 && time_to_net_from_bounce1 < t2_estimated) {
                    ballY_at_net = firstBouncePosServerCourt.y + Vy_afterBounce1 * time_to_net_from_bounce1 - 0.5 * GRAVITY * time_to_net_from_bounce1 * time_to_net_from_bounce1;
                } else {
                    // Ball path does not cross net Z plane within this segment time, but target is on other side.
                    // This implies it must have cleared (or will clear if path continued).
                    ballY_at_net = NET_TOP_Y + 0.1; 
                }
            } else if ( (serverSide * targetOpponentBouncePos.z < serverSide * NET_POS_Z) && // Target is beyond net
                        (serverSide * firstBouncePosServerCourt.z < serverSide * NET_POS_Z) && // First bounce is also "beyond" net (for its side)
                        (Math.sign(Vz_afterBounce1) === Math.sign(targetOpponentBouncePos.z - firstBouncePosServerCourt.z)) // Moving towards target
                      ) {
                // This case means the first bounce is already on the opponent's side of the net (relative to server's perspective of "over the net")
                // OR the path from first bounce to second bounce doesn't cross net plane because both are on receiver's side.
                // If the target is on the other side of the net from where the server is standing, assume it clears.
                ballY_at_net = NET_TOP_Y + 0.1; 
            }


            const diff = Math.abs(Vy_afterBounce1 - Vy2_required_at_bounce1);

            if (ballY_at_net > NET_TOP_Y + 0.002) { // Slightly reduced net clearance margin
                if (diff < minDiff) {
                    minDiff = diff;
                    bestVy1 = Vy1_current; 
                    optimalT1 = t1_guess; 
                }
            }
        } 

        if (bestVy1 !== null && optimalT1 > 0) {
            // Use optimalT1 directly as recalculating from bestVy1 with quadratic can be tricky
            const finalVx1 = deltaX1 / optimalT1;
            const finalVz1 = deltaZ1 / optimalT1;
            console.log(`Optimal Serve Found: t1=${optimalT1.toFixed(3)}, Vx1=${finalVx1.toFixed(2)}, Vy1=${bestVy1.toFixed(2)}, Vz1=${finalVz1.toFixed(2)}, MinDiff: ${minDiff.toFixed(4)}`);
            return new THREE.Vector3(finalVx1, bestVy1, finalVz1);
        }

        console.warn("Could not find an optimal serve trajectory satisfying net clearance and Vy match, using fallback serve.");
        let fallbackVx = (targetOpponentBouncePos.x - hitPosition.x) / 0.4; 
        let fallbackVz = (targetOpponentBouncePos.z - hitPosition.z) / 0.4;
        if (Math.abs(fallbackVz) < 2.0) fallbackVz = serverSide * -3.0; 
        fallbackVx = THREE.MathUtils.clamp(fallbackVx, -1.5, 1.5);
        return new THREE.Vector3(fallbackVx, 1.8, fallbackVz); 
    }
}
