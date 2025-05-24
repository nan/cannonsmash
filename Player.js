// Player.js - Reverted state (Turn 55/71) + Lerp factor change
import * as THREE from 'three';

const TABLE_LENGTH = 2.74;      
const TABLE_WIDTH = 1.525;      
const PLAYER_Z_OFFSET = 0.2;    
const RACKET_OFFSET_Z = 0.3;    
const RACKET_DEFAULT_Y = 0.2;   
const TABLE_HEIGHT = 0.76;      
const NET_POS_Z = 0;            
const BALL_RADIUS = 0.02;       

export class Player {
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
            lerpFactor: this.controlType === 'human' ? 0.2 : 0.15 
        };
        this.createRacketMesh();

        this.mouseScreenX = 0;  
        this.mouseScreenY = 0;  

        const xRangeMultiplier = this.controlType === 'human' ? 0.9 : 0.8;
        this.minX = -TABLE_WIDTH / 2 * xRangeMultiplier;
        this.maxX =  TABLE_WIDTH / 2 * xRangeMultiplier;
        
        this.aiHitting = false; 
        // 'isServing', 'serveToss', 'serveHit' are NOT in this version
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

        // Rally mouse move logic (no 'isServing' check in this version)
        this.mouseScreenX = screenX;
        this.mouseScreenY = screenY;
        const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
        // THIS IS THE LINE TO CHANGE:
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.2); // Changed 0.1 to 0.2
        
        this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35); 
        this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35); 
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
    }
    
    swing(gameBall) { 
        if (this.controlType === 'human') {
            console.log("Human Player swings (simple swing)!");
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => {
                 this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z;
            }, 120); 
        } else if (this.controlType === 'ai') {
            console.log("AI Player swings for return (simple swing)!");
            this.aiHitting = true; 
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { 
                this.aiHitting = false; 
                this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
            }, 150); 
        }
        // No return value in this version of swing
    }
    
    updateAI(ball) { 
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
            if (!this.aiHitting) {
                this.swing(ball); 
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
