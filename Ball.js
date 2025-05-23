// Ball.js
import * as THREE from 'three';

// --- Constants ---
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
    constructor(initialPosition = new THREE.Vector3(0, TABLE_HEIGHT + BALL_RADIUS + 0.2, 0)) {
        this.position = initialPosition.clone(); 
        this.velocity = new THREE.Vector3(0, 0, -2);   
        this.spin = new THREE.Vector2(0, 0);           
        this.radius = BALL_RADIUS;                     
        this.mesh = null;                              
        this.status = 8; // Default to P1 ready to serve
        this.lastHitBy = 0;             
        this.bouncedOnServerSide = false;   
        this.bouncedOnReceiverSide = false; 
    }

    updatePhysics(player1RacketMesh, player2RacketMesh) {
        // (Full commented logic from Turn 33 output)
        this.velocity.y -= GRAVITY * TICK; 
        this.velocity.multiplyScalar(1 - AIR_RESISTANCE_FACTOR * TICK); 
        this.spin.multiplyScalar(1 - SPIN_DECAY_FACTOR * TICK); 
        this.position.addScaledVector(this.velocity, TICK);
        // Net collision
        if (this.position.y > TABLE_HEIGHT && 
            this.position.y < TABLE_HEIGHT + NET_HEIGHT + this.radius &&
            Math.abs(this.position.z - NET_POS_Z) < this.radius + 0.01) { 
            const movingTowardsNetZ = (this.velocity.z > 0 && this.position.z < NET_POS_Z + this.radius) || 
                                      (this.velocity.z < 0 && this.position.z > NET_POS_Z - this.radius);   
            if (movingTowardsNetZ) {
                console.log("Ball hit net body");
                this.velocity.z *= -0.3; 
                this.velocity.y *= 0.4;  
                this.spin.y *= 0.5;      
                this.position.z += Math.sign(this.velocity.z) * this.radius * 0.2; 
                this.status = -1;        
            }
        }
        // Table collision
        if (this.position.y < TABLE_HEIGHT + this.radius && this.velocity.y < 0) {
            this.position.y = TABLE_HEIGHT + this.radius; 
            const preBounceVelocityY = this.velocity.y; 
            this.velocity.y *= -BOUNCE_ENERGY_LOSS;     
            this.velocity.y += this.spin.y * SPIN_EFFECT_ON_BOUNCE_Y * Math.abs(preBounceVelocityY); 
            this.velocity.z -= this.spin.y * SPIN_EFFECT_ON_BOUNCE_Z; 
            this.velocity.x += this.spin.x * SPIN_EFFECT_ON_BOUNCE_X; 
            this.spin.y *= 0.6; 
            this.spin.x *= 0.7;
            console.log(`Ball bounced on table. Z: ${this.position.z.toFixed(2)}, Side: ${this.position.z >= NET_POS_Z ? "P1_Side(Pos-Z)" : "P2_Side(Neg-Z)"}`);
            if (this.position.z >= NET_POS_Z) { 
                this.bouncedOnServerSide = true; 
            } else { 
                this.bouncedOnReceiverSide = true;
            }
        }
        // Floor collision
        if (this.position.y < this.radius && this.velocity.y < 0) { 
            console.log("Ball hit floor"); this.status = -2; 
        }
        // Out of bounds: Sideways
        if (Math.abs(this.position.x) > TABLE_WIDTH / 2 + this.radius) {
            console.log("Ball out of table width (sideways)"); this.status = -3; 
        }
        // Out of bounds: Long
        if (this.position.z > (TABLE_LENGTH / 2 + this.radius * 2) && this.velocity.z > 0) { 
            console.log("Ball potentially out long on P1 side (+Z)");
            if (!this.bouncedOnServerSide) this.status = -4; 
        }
        if (this.position.z < -(TABLE_LENGTH / 2 + this.radius * 2) && this.velocity.z < 0) { 
             console.log("Ball potentially out long on P2 side (-Z)");
             if (!this.bouncedOnReceiverSide) this.status = -5; 
        }
        // Racket Collision
        if (player1RacketMesh && this.checkRacketCollision(player1RacketMesh, 1)) { /* Collision handled in method */ }
        if (player2RacketMesh && this.checkRacketCollision(player2RacketMesh, 2)) { /* Collision handled in method */ }
    }
    
    checkRacketCollision(racketMesh, hittingPlayerID) {
        // (Full commented logic from Turn 33 output)
        if (!racketMesh || !this.mesh || this.lastHitBy === hittingPlayerID) return false;
        const ballBox = new THREE.Box3().setFromObject(this.mesh);
        const racketBox = new THREE.Box3().setFromObject(racketMesh);
        if (ballBox.intersectsBox(racketBox)) {
            console.log(`Ball collided with racket of player ${hittingPlayerID}`);
            let newVelocity = new THREE.Vector3(); 
            const racketWorldPos = new THREE.Vector3();
            racketMesh.getWorldPosition(racketWorldPos); 
            const impactOffset = this.position.clone().sub(racketWorldPos); 
            newVelocity.y = 1.8 + Math.random() * 0.5; 
            const baseZSpeed = 2.5 + Math.random() * 1.0; 
            newVelocity.z = (hittingPlayerID === 1) ? -baseZSpeed : baseZSpeed; 
            newVelocity.x = impactOffset.x * (hittingPlayerID === 1 ? -5.0 : 5.0) + (Math.random() - 0.5);
            let newSpin = new THREE.Vector2(newVelocity.x * 0.5, 2 + Math.random() * 3); 
            this.hit(newVelocity, newSpin, hittingPlayerID); 
            return true; 
        }
        return false; 
    }

    update(player1RacketMesh, player2RacketMesh) {
        // (Full commented logic from Turn 33 output)
        if (this.status >= 0) { 
            this.updatePhysics(player1RacketMesh, player2RacketMesh);
        }
        if (this.mesh) {
            this.mesh.position.copy(this.position); 
        }
    }

    // THIS IS THE MODIFIED reset() METHOD FOR THIS STEP
    reset(forPlayerID = 1) { 
        this.lastHitBy = 0; 
        this.bouncedOnServerSide = false;   
        this.bouncedOnReceiverSide = false;
        this.spin.set(0,0);                 
        
        // Set a neutral initial position
        this.position.set(0, TABLE_HEIGHT + 0.5, 0); // Centered above net
        
        // Initial velocity should be zero.
        this.velocity.set(0, 0, 0); 

        if (forPlayerID === 1) { 
            this.status = 8; // Ready_for_P1_serve
        } else { 
            this.status = 9; // Ready_for_P2_serve
        }
        // Console log removed from here.
    }

    hit(newVelocity, newSpin, hittingPlayerID) {
        // (Full commented logic from Turn 33 output, ensure status is set to 1 or 3 directly)
        this.velocity.copy(newVelocity);
        if (newSpin) { this.spin.copy(newSpin); }
        this.lastHitBy = hittingPlayerID;
        this.bouncedOnServerSide = false; 
        this.bouncedOnReceiverSide = false;
        // This 'hit' is for rally or the *actual hit of a serve*.
        // The 'toss' method will handle status 6 & 7.
        if (hittingPlayerID === 1) { this.status = 1; } // Flying from P1
        else { this.status = 3; } // Flying from P2
        console.log(`Ball hit by Player ${hittingPlayerID}! Status: ${this.status}, Vel:(${this.velocity.x.toFixed(1)},${this.velocity.y.toFixed(1)},${this.velocity.z.toFixed(1)})`);
    }

    // NOTE: The toss() method is INTENTIONALLY NOT present in this version of Ball.js
    // It will be added in the next step of the main plan.
}
