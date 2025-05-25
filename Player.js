// Player.js - Corrected for ball-follow feature
import * as THREE from 'three';

// Constants (as in Turn 75)
const TABLE_LENGTH = 2.74;      
const TABLE_WIDTH = 1.525;      
const PLAYER_Z_OFFSET = 0.2;    
const RACKET_OFFSET_Z = 0.3;    
const RACKET_DEFAULT_Y = 0.2;   
const TABLE_HEIGHT = 0.76;      
const NET_POS_Z = 0;            
const BALL_RADIUS = 0.02;       
// Note: TOSS_POWER, RACKET_HEIGHT, RACKET_WIDTH_Z are NOT in this version of Player.js
// as this is the baseline for the *simple serve* before re-adding toss mechanics.

export class Player {
    constructor(side = 1, scene, controlType = 'human') { 
        // (Constructor as in Turn 75)
        this.side = side;       
        this.scene = scene;     
        this.controlType = controlType; 
        this.position = new THREE.Vector3(
            0, 0.4, (TABLE_LENGTH / 2 + PLAYER_Z_OFFSET) * this.side 
        );
        this.mesh = null; 
        this.racket = {
            mesh: null,         
            targetPosition: new THREE.Vector3(0, RACKET_DEFAULT_Y, this.side * -RACKET_OFFSET_Z), 
            currentPosition: new THREE.Vector3(0, RACKET_DEFAULT_Y, this.side * -RACKET_OFFSET_Z),
            lerpFactor: this.controlType === 'human' ? 0.2 : 0.15 
        };
        this.createRacketMesh();
        this.mouseScreenX = 0; this.mouseScreenY = 0;  
        const xRangeMultiplier = this.controlType === 'human' ? 0.9 : 0.8;
        this.minX = -TABLE_WIDTH / 2 * xRangeMultiplier;
        this.maxX =  TABLE_WIDTH / 2 * xRangeMultiplier;
        this.aiHitting = false; 
    }

    createRacketMesh() {
        // (As in Turn 75 - uses local RACKET_HEIGHT/WIDTH_Z if they were defined there, 
        // but for this baseline, it was simpler BoxGeometry)
        // For this baseline (Turn 75), racket was:
        const racketGeometry = new THREE.BoxGeometry(0.02, 0.22, 0.16); 
        const racketMaterial = new THREE.MeshStandardMaterial({ 
            color: this.controlType === 'human' ? 0xcc0000 : 0x00cc00, 
            roughness: 0.7, metalness: 0.3  
        });
        this.racket.mesh = new THREE.Mesh(racketGeometry, racketMaterial);
        this.racket.mesh.position.copy(this.racket.currentPosition); 
    }

    handleMouseMove(screenX, screenY) {
        // (As in Turn 75 - lerp factor for this.mesh.position.x is 0.2)
        if (this.controlType !== 'human' || !this.mesh) return; 
        this.mouseScreenX = screenX; this.mouseScreenY = screenY;
        const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.2); // Lerp factor is 0.2
        this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35); 
        this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35); 
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
    }
    
    /**
     * ADDED/MODIFIED for ball-follow context.
     * Calculates and sets the ball's world position relative to the player's current position.
     * This method is intended to be called repeatedly while the player is "holding" the ball pre-serve.
     * @param {Ball} gameBall - The global game ball object.
     */
    positionBallRelativeToHand(gameBall) { // Renamed for clarity for this feature
        if (!this.mesh) return;
        // REMOVED: if (this.controlType === 'human') { this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, 0, 0.1); }
        
        const ballRelativePos = new THREE.Vector3(
            this.side * 0.20,  // To Player's "right" (e.g. P1 side=1, so +0.20)
            0.1,               // Relative Y to player mesh origin (player mesh Y is 0.4, so ball world Y is ~0.5)
            this.side * -0.20  // Slightly in front of player's center
        );
        gameBall.position.copy(this.mesh.localToWorld(ballRelativePos.clone()));
        // Optional: console.log for debugging, but remove for final
        // console.log(`Player ${this.side === 1 ? 1 : 2} continuously positions ball for serve.`);
    }
    
    swing(gameBall) { 
        // (Simple swing logic from Turn 75)
        if (this.controlType === 'human') {
            console.log("Human Player swings (simple swing)!");
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; }, 120); 
        } else if (this.controlType === 'ai') {
            console.log("AI Player swings for return (simple swing)!");
            this.aiHitting = true; 
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { this.aiHitting = false; this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; }, 150); 
        }
    }
    
    updateAI(ball) { 
        // (As in Turn 75)
        if (!ball || !this.mesh || ball.status < 0) return;
        let predictedBallX = ball.position.x;
        if ((this.side === -1 && ball.velocity.z > 0) || (this.side === 1 && ball.velocity.z < 0)) {
            const timeToNet = (ball.velocity.z !== 0) ? Math.abs((NET_POS_Z - ball.position.z) / ball.velocity.z) : 0.1;
            const timeToHitZone = (ball.velocity.z !==0) ? Math.abs(((this.position.z + this.side * -RACKET_OFFSET_Z) - ball.position.z) / ball.velocity.z) : 0.1;
            const predictionTime = Math.min(timeToNet, timeToHitZone, 0.5); 
            if (predictionTime > 0 && isFinite(ball.velocity.z) && ball.velocity.z !== 0) {
                 predictedBallX = ball.position.x + ball.velocity.x * predictionTime;
            }
        }
        const targetPlayerX = THREE.MathUtils.clamp(predictedBallX, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.08); 
        let desiredRacketX = ball.position.x - this.mesh.position.x;
        let desiredRacketY = RACKET_DEFAULT_Y + (ball.position.y - (TABLE_HEIGHT + BALL_RADIUS));
        desiredRacketX = THREE.MathUtils.clamp(desiredRacketX, -0.45, 0.45); 
        desiredRacketY = THREE.MathUtils.clamp(desiredRacketY, 0.05, 0.55);  
        this.racket.targetPosition.x = desiredRacketX;
        this.racket.targetPosition.y = desiredRacketY;
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z;
        const ballIsComingTowardsAI = (this.side === -1 && ball.velocity.z > 0.2) || 
                                    (this.side === 1 && ball.velocity.z < -0.2);   
        const ballNearAISideOfNet = (this.side === -1 && ball.position.z < (NET_POS_Z + 0.3)) || 
                                  (this.side === 1 && ball.position.z > (NET_POS_Z - 0.3));
        const distZToBall = Math.abs(ball.position.z - (this.mesh.position.z + this.racket.targetPosition.z));
        const idealHitZ = 0.15; 
        const isAIsTurnToHit = (this.side === -1 && ball.status === 1) || (this.side === 1 && ball.status === 3);
        if (isAIsTurnToHit && ballIsComingTowardsAI && ballNearAISideOfNet && 
            distZToBall < idealHitZ && 
            ball.position.y < TABLE_HEIGHT + 0.4 && ball.position.y > TABLE_HEIGHT - 0.15) { 
            if (!this.aiHitting) { this.swing(ball); }
        }
    }

    update(ball) { 
        if (this.controlType === 'ai') {
            this.updateAI(ball); 
        }
        if (this.racket.mesh) {
            this.racket.currentPosition.lerp(this.racket.targetPosition, this.racket.lerpFactor);
            this.racket.mesh.position.copy(this.racket.currentPosition);
        }
    }
}
