// Player.js
import * as THREE from 'three';

// (Keep existing constants: TABLE_LENGTH, TABLE_WIDTH, PLAYER_Z_OFFSET, RACKET_OFFSET_Z, RACKET_DEFAULT_Y, TABLE_HEIGHT, NET_POS_Z, BALL_RADIUS)
const TABLE_LENGTH = 2.74;      
const TABLE_WIDTH = 1.525;      
const PLAYER_Z_OFFSET = 0.2;    
const RACKET_OFFSET_Z = 0.3;    
const RACKET_DEFAULT_Y = 0.2;   
const TABLE_HEIGHT = 0.76;      
const NET_POS_Z = 0;            
const BALL_RADIUS = 0.02;       

export class Player {
    // ... (constructor and other methods like createRacketMesh, handleMouseMove, swing remain the same as in Turn 37) ...
    constructor(side = 1, scene, controlType = 'human') {
        this.side = side;       
        this.scene = scene;     
        this.controlType = controlType; 
        this.position = new THREE.Vector3(
            0,                                              
            0.4, 
            (TABLE_LENGTH / 2 + PLAYER_Z_OFFSET) * this.side 
        );
        this.mesh = null; 
        this.racket = {
            mesh: null,         
            targetPosition: new THREE.Vector3(0, RACKET_DEFAULT_Y, this.side * -RACKET_OFFSET_Z), 
            currentPosition: new THREE.Vector3(0, RACKET_DEFAULT_Y, this.side * -RACKET_OFFSET_Z),
            lerpFactor: this.controlType === 'human' ? 0.2 : 0.15 // AI racket can be slightly more responsive
        };
        this.createRacketMesh(); 
        this.mouseScreenX = 0;  
        this.mouseScreenY = 0;  
        const xRangeMultiplier = this.controlType === 'human' ? 0.9 : 0.8;
        this.minX = -TABLE_WIDTH / 2 * xRangeMultiplier; 
        this.maxX =  TABLE_WIDTH / 2 * xRangeMultiplier; 
        this.aiHitting = false; 
    }

    createRacketMesh() {
        const racketGeometry = new THREE.BoxGeometry(0.02, 0.22, 0.16); 
        const racketMaterial = new THREE.MeshStandardMaterial({ 
            color: this.controlType === 'human' ? 0xcc0000 : 0x00cc00, 
            roughness: 0.7, 
            metalness: 0.3  
        });
        this.racket.mesh = new THREE.Mesh(racketGeometry, racketMaterial);
        this.racket.mesh.position.copy(this.racket.currentPosition); 
    }

    handleMouseMove(screenX, screenY) {
        if (this.controlType !== 'human' || !this.mesh) return; 
        this.mouseScreenX = screenX;
        this.mouseScreenY = screenY;
        const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.1); 
        this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35); 
        this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35); 
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
    }
    
    swing() {
        // (Swing logic as refined in Turn 37 - visual cue and aiHitting flag)
        if (this.controlType === 'human') {
            console.log("Human Player swings!");
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => {
                 this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z;
            }, 120); 
        } else if (this.controlType === 'ai') {
            console.log("AI Player swings for return!"); // Changed log slightly
            this.aiHitting = true; 
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { 
                this.aiHitting = false; 
                this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
            }, 150); 
        }
    }

    updateAI(ball) {
        if (!ball || !this.mesh || ball.status < 0) return;

        // --- AI Player Body Movement: Track ball's X, slightly predictive ---
        // Predict ball's X position when it crosses the net or reaches AI's hitting zone
        let predictedBallX = ball.position.x;
        if ((this.side === -1 && ball.velocity.z > 0) || (this.side === 1 && ball.velocity.z < 0)) { // If ball is coming towards AI
            const timeToNet = Math.abs((NET_POS_Z - ball.position.z) / ball.velocity.z);
            // Predict further to AI's hitting zone (approx RACKET_OFFSET_Z from player)
            const timeToHitZone = Math.abs(((this.position.z + this.side * -RACKET_OFFSET_Z) - ball.position.z) / ball.velocity.z);
            const predictionTime = Math.min(timeToNet, timeToHitZone, 0.5); // Cap prediction time
            if (predictionTime > 0 && isFinite(ball.velocity.z) && ball.velocity.z !== 0) { // Ensure velocity.z is not zero and finite
                 predictedBallX = ball.position.x + ball.velocity.x * predictionTime;
            }
        }
        const targetPlayerX = THREE.MathUtils.clamp(predictedBallX, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.08); // Slightly faster AI body reaction

        // --- AI Racket Positioning Logic ---
        let desiredRacketX = ball.position.x - this.mesh.position.x;
        let desiredRacketY = RACKET_DEFAULT_Y + (ball.position.y - (TABLE_HEIGHT + BALL_RADIUS));
        
        desiredRacketX = THREE.MathUtils.clamp(desiredRacketX, -0.45, 0.45); // Slightly wider racket range for AI
        desiredRacketY = THREE.MathUtils.clamp(desiredRacketY, 0.05, 0.55);  // Adjusted Y range

        this.racket.targetPosition.x = desiredRacketX;
        this.racket.targetPosition.y = desiredRacketY;
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z;

        // --- AI Swing Decision Logic ---
        const ballIsComingTowardsAI = (this.side === -1 && ball.velocity.z > 0.2) || // P2 is AI, ball has decent +Z velocity
                                    (this.side === 1 && ball.velocity.z < -0.2);   // P1 is AI, ball has decent -Z velocity
        
        // Ball on AI's side of net (or very close to crossing towards AI)
        const ballNearAISideOfNet = (this.side === -1 && ball.position.z < (NET_POS_Z + 0.3)) || 
                                    (this.side === 1 && ball.position.z > (NET_POS_Z - 0.3));
        
        // Z-distance from racket's default resting plane to the ball.
        const distZToBall = Math.abs(ball.position.z - (this.mesh.position.z + this.racket.targetPosition.z));
        const idealHitZ = 0.15; // Ideal Z distance to initiate swing

        // Ball status check:
        // If AI is P2 (side -1), it should hit when ball.status is 1 (coming from P1)
        // If AI is P1 (side  1), it should hit when ball.status is 3 (coming from P2)
        const isAIsTurnToHit = (this.side === -1 && ball.status === 1) || (this.side === 1 && ball.status === 3);

        if (isAIsTurnToHit && ballIsComingTowardsAI && ballNearAISideOfNet && 
            distZToBall < idealHitZ && // Ball is close enough in Z
            ball.position.y < TABLE_HEIGHT + 0.4 && ball.position.y > TABLE_HEIGHT - 0.15) { // Hittable Y window
            if (!this.aiHitting) {
                this.swing();
            }
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
