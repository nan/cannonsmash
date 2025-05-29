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
        // firstBounceServerZ is intentionally not used to constrain the first bounce Z position.
        console.log("Calculating serve velocity (Iterative Search - Modified First Bounce)...");

        const NET_TOP_Y = TABLE_HEIGHT + NET_HEIGHT + BALL_RADIUS; 
        const SIMULATION_TICK = TICK; 
        const MAX_SIMULATION_TICKS = 300;
        // const FIRST_BOUNCE_Z_TOLERANCE = 0.10; // No longer used for Z check
        const NET_CLEARANCE_MIN = 0.002;
        const TARGET_BOUNCE_DISTANCE_TOLERANCE = 0.15;
        const TABLE_SURFACE_Y = TABLE_HEIGHT + BALL_RADIUS; 

        for (let Vy_initial_loop = -5.0; Vy_initial_loop <= 5.0; Vy_initial_loop += 0.1) {
            for (let Vz_initial_abs_loop = 1.0; Vz_initial_abs_loop <= 7.0; Vz_initial_abs_loop += 0.1) {
                for (let Vx_initial_loop = -3.0; Vx_initial_loop <= 3.0; Vx_initial_loop += 0.1) {
                    
                    const currentInitialVelocity = new THREE.Vector3(
                        Vx_initial_loop,
                        Vy_initial_loop,
                        serverSide * -Vz_initial_abs_loop
                    );

                    let simBallPosition = hitPosition.clone();
                    let simBallVelocity = currentInitialVelocity.clone();
                    let simBallSpin = desiredSpin.clone(); 

                    let hasBouncedOnServerSide = false;
                    let hasClearedNet = false;
                    // let hasBouncedOnOpponentSide = false; // Not strictly needed as success is returned directly

                    if (Vx_initial_loop === -3.0 && Vy_initial_loop === -5.0 && Vz_initial_abs_loop === 1.0) { // Log only for one of the earliest attempts
                        console.log("Debug Serve: Starting simulation with initial Vel (Vx,Vy,VzAbs):", Vx_initial_loop.toFixed(1), Vy_initial_loop.toFixed(1), Vz_initial_abs_loop.toFixed(1));
                    }

                    for (let tick_num = 0; tick_num < MAX_SIMULATION_TICKS; tick_num++) {
                        let prevSimBallPosition = simBallPosition.clone();

                        simBallVelocity.y -= GRAVITY * SIMULATION_TICK;
                        simBallPosition.addScaledVector(simBallVelocity, SIMULATION_TICK);

                        let netCrossedInThisSegment = false;
                        if (serverSide === 1) {
                            netCrossedInThisSegment = (prevSimBallPosition.z > NET_POS_Z && simBallPosition.z <= NET_POS_Z);
                        } else { 
                            netCrossedInThisSegment = (prevSimBallPosition.z < NET_POS_Z && simBallPosition.z >= NET_POS_Z);
                        }
                        
                        if (netCrossedInThisSegment) {
                            let yAtNet = NET_TOP_Y + 1.0; 
                            if (Math.abs(simBallPosition.z - prevSimBallPosition.z) > 1e-6) { 
                                const alpha = (NET_POS_Z - prevSimBallPosition.z) / (simBallPosition.z - prevSimBallPosition.z);
                                yAtNet = prevSimBallPosition.y + (simBallPosition.y - prevSimBallPosition.y) * alpha;
                            }

                            if (yAtNet > NET_TOP_Y + NET_CLEARANCE_MIN) {
                                hasClearedNet = true;
                            } else {
                                console.log("Debug Serve: Net Fail. yAtNet:", yAtNet.toFixed(3), "Vel (x,y,z):", currentInitialVelocity.x.toFixed(1), currentInitialVelocity.y.toFixed(1), currentInitialVelocity.z.toFixed(1));
                                break; 
                            }
                        }

                        if (simBallVelocity.y < 0 && simBallPosition.y <= TABLE_SURFACE_Y) {
                            simBallPosition.y = TABLE_SURFACE_Y; 

                            const isOnServerHalf = (serverSide === 1) ? (simBallPosition.z > NET_POS_Z) : (simBallPosition.z < NET_POS_Z);

                            if (!hasBouncedOnServerSide && isOnServerHalf) {
                                // MODIFIED: Only check X bounds, not Z against firstBounceServerZ
                                if (Math.abs(simBallPosition.x) <= TABLE_WIDTH / 2) { 
                                    hasBouncedOnServerSide = true;
                                    const preBounceVy = simBallVelocity.y; 
                                    simBallVelocity.y *= -BOUNCE_ENERGY_LOSS;
                                    simBallVelocity.y += simBallSpin.y * SPIN_EFFECT_ON_BOUNCE_Y * Math.abs(preBounceVy); 
                                    simBallVelocity.x += simBallSpin.x * SPIN_EFFECT_ON_BOUNCE_X;
                                    simBallVelocity.z -= simBallSpin.y * SPIN_EFFECT_ON_BOUNCE_Z;
                                } else {
                                    console.log("Debug Serve: First Bounce Out X. simX:", simBallPosition.x.toFixed(3), "Vel (x,y,z):", currentInitialVelocity.x.toFixed(1), currentInitialVelocity.y.toFixed(1), currentInitialVelocity.z.toFixed(1));
                                    break; 
                                }
                            } 
                            else if (hasBouncedOnServerSide && !isOnServerHalf) { 
                                if (Math.abs(simBallPosition.x) <= TABLE_WIDTH / 2) { 
                                    // hasBouncedOnOpponentSide = true; // Not strictly needed
                                    const targetDist = Math.sqrt(
                                        Math.pow(simBallPosition.x - targetOpponentBouncePos.x, 2) +
                                        Math.pow(simBallPosition.z - targetOpponentBouncePos.z, 2)
                                    );

                                    // console.log("Debug Serve Target: targetDist =", targetDist, "simBallPosition (x,y,z) =", simBallPosition.x.toFixed(3), simBallPosition.y.toFixed(3), simBallPosition.z.toFixed(3)); // Original log
                                    if (targetDist < TARGET_BOUNCE_DISTANCE_TOLERANCE && hasClearedNet) {
                                        console.log("Optimal Serve Found (Iterative - Modified First Bounce): ", currentInitialVelocity, ` TargetDist: ${targetDist.toFixed(3)}`);
                                        return currentInitialVelocity;
                                    } else {
                                        console.log("Debug Serve: Target Miss or Net Not Cleared. targetDist:", targetDist.toFixed(3), "hasClearedNet:", hasClearedNet, "Vel (x,y,z):", currentInitialVelocity.x.toFixed(1), currentInitialVelocity.y.toFixed(1), currentInitialVelocity.z.toFixed(1));
                                        break; 
                                    }
                                } else {
                                    console.log("Debug Serve: Opponent Bounce Out X. simX:", simBallPosition.x.toFixed(3), "Vel (x,y,z):", currentInitialVelocity.x.toFixed(1), currentInitialVelocity.y.toFixed(1), currentInitialVelocity.z.toFixed(1));
                                    break; 
                                }
                            } else {
                                console.log("Debug Serve: Invalid Bounce State. hasBouncedServer:", hasBouncedOnServerSide, "isOnServerHalf:", isOnServerHalf, "Vel (x,y,z):", currentInitialVelocity.x.toFixed(1), currentInitialVelocity.y.toFixed(1), currentInitialVelocity.z.toFixed(1));
                                break; 
                            }
                        }

                        if (simBallPosition.y < 0) { 
                            console.log("Debug Serve: Floor Hit. simY:", simBallPosition.y.toFixed(3), "Vel (x,y,z):", currentInitialVelocity.x.toFixed(1), currentInitialVelocity.y.toFixed(1), currentInitialVelocity.z.toFixed(1));
                            break;
                        }
                        if (Math.abs(simBallPosition.x) > TABLE_WIDTH / 2 + 0.2 || Math.abs(simBallPosition.z) > TABLE_LENGTH / 2 + 0.2) {
                            console.log("Debug Serve: Table Area Out of Bounds. simX:", simBallPosition.x.toFixed(3), "simZ:", simBallPosition.z.toFixed(3), "Vel (x,y,z):", currentInitialVelocity.x.toFixed(1), currentInitialVelocity.y.toFixed(1), currentInitialVelocity.z.toFixed(1));
                            break; 
                        }
                        if (tick_num === MAX_SIMULATION_TICKS - 1) {
                            console.log("Debug Serve: MAX_TICKS reached for Vel (x,y,z):", currentInitialVelocity.x.toFixed(1), currentInitialVelocity.y.toFixed(1), currentInitialVelocity.z.toFixed(1));
                        }
                    } 
                } 
            } 
        } 

        console.warn("Could not find an optimal serve trajectory (Iterative Search - Modified First Bounce), using fallback serve.");
        let fallbackVx = (targetOpponentBouncePos.x - hitPosition.x) / 0.4;
        let fallbackVz = (targetOpponentBouncePos.z - hitPosition.z) / 0.4;
        if (Math.abs(fallbackVz) < 2.0) fallbackVz = serverSide * -3.0;
        fallbackVx = THREE.MathUtils.clamp(fallbackVx, -1.5, 1.5);
        return new THREE.Vector3(fallbackVx, 1.8, fallbackVz);
    }
}
