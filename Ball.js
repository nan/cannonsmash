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
        // Intentionally unused: firstBounceServerZ, desiredSpin

        const NET_TOP_Y = TABLE_HEIGHT + NET_HEIGHT + BALL_RADIUS; // Access global
        const NET_CLEARANCE_MIN = 0.02;
        const Vy_initial_options = [2.5, 3.0, 3.5, 4.0]; // m/s
        const t_flight_options = [0.35, 0.4, 0.45, 0.5, 0.55]; // seconds
        // Access global GRAVITY, NET_POS_Z

        for (const Vy_initial of Vy_initial_options) {
            for (const t_flight of t_flight_options) {
                const deltaX = targetOpponentBouncePos.x - hitPosition.x;
                const deltaZ = targetOpponentBouncePos.z - hitPosition.z;

                const Vx_initial = deltaX / t_flight;
                const Vz_initial = deltaZ / t_flight;

                // Net Clearance Check
                if (Math.abs(Vz_initial) < 0.1) { // Avoid division by zero or near-zero Vz
                    continue; // This Vz is not suitable for reaching the net with this t_flight
                }

                const t_to_net = (NET_POS_Z - hitPosition.z) / Vz_initial;

                // If t_to_net is very small, it means the ball starts very close to or past the net.
                // If t_to_net is much larger than t_flight, it means the ball would cross the net
                // (if it were to continue on this Vz) much later than its intended flight time to the target,
                // or Vz is directed away from the net relative to hitPosition.z.
                if (t_to_net < 0.01 || t_to_net > t_flight * 1.1) {
                    continue; // Try next t_flight or Vy_initial
                }

                const y_at_net = hitPosition.y + (Vy_initial * t_to_net) - (0.5 * GRAVITY * t_to_net * t_to_net);

                if (y_at_net > NET_TOP_Y + NET_CLEARANCE_MIN) {
                    console.log("Optimal Serve Found (Simplified Target-Based): ", new THREE.Vector3(Vx_initial, Vy_initial, Vz_initial));
                    return new THREE.Vector3(Vx_initial, Vy_initial, Vz_initial);
                }
            }
        }

        // After All Loops (Fallback)
        console.warn("Could not find an optimal serve trajectory (Simplified Target-Based), using fallback serve.");
        let fallbackVx = (targetOpponentBouncePos.x - hitPosition.x) / 0.4;
        let fallbackVz = (targetOpponentBouncePos.z - hitPosition.z) / 0.4;
        if (Math.abs(fallbackVz) < 2.0) { // Ensure fallback Vz is directed towards opponent
            fallbackVz = serverSide * -3.0; // serverSide is 1 or -1, so this makes it -3.0 or 3.0
        }
        fallbackVx = THREE.MathUtils.clamp(fallbackVx, -1.5, 1.5);
        return new THREE.Vector3(fallbackVx, 1.8, fallbackVz);
    }
}
