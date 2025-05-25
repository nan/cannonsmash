// Player.js - Applying user's ballRelativePos changes
import * as THREE from 'three';

const TABLE_LENGTH = 2.74;      
const TABLE_WIDTH = 1.525;      
const PLAYER_Z_OFFSET = 0.2;    
const RACKET_OFFSET_Z = 0.3;    
const RACKET_DEFAULT_Y = 0.2;   
const TABLE_HEIGHT = 0.76;      
const NET_POS_Z = 0;            
const BALL_RADIUS = 0.02;       
const TOSS_POWER = 2.5; 
const RACKET_HEIGHT = 0.22;     
const RACKET_WIDTH_Z = 0.16;    

export class Player {
    constructor(side = 1, scene, controlType = 'human') { 
        this.side = side; this.scene = scene; this.controlType = controlType; 
        this.position = new THREE.Vector3(0, 0.4, (TABLE_LENGTH / 2 + PLAYER_Z_OFFSET) * this.side );
        this.mesh = null; 
        this.racket = {
            mesh: null, targetPosition: new THREE.Vector3(0, RACKET_DEFAULT_Y, this.side * -RACKET_OFFSET_Z), 
            currentPosition: new THREE.Vector3(0, RACKET_DEFAULT_Y, this.side * -RACKET_OFFSET_Z),
            lerpFactor: this.controlType === 'human' ? 0.2 : 0.15 
        };
        this.createRacketMesh();
        this.mouseScreenX = 0; this.mouseScreenY = 0;  
        const xRangeMultiplier = this.controlType === 'human' ? 0.9 : 0.8;
        this.minX = -TABLE_WIDTH / 2 * xRangeMultiplier; this.maxX =  TABLE_WIDTH / 2 * xRangeMultiplier;
        this.aiHitting = false; this.isServing = false; 
    }

    createRacketMesh() {
        const racketGeometry = new THREE.BoxGeometry(0.02, RACKET_HEIGHT, RACKET_WIDTH_Z); 
        const racketMaterial = new THREE.MeshStandardMaterial({ 
            color: this.controlType === 'human' ? 0xcc0000 : 0x00cc00, roughness: 0.7, metalness: 0.3  
        });
        this.racket.mesh = new THREE.Mesh(racketGeometry, racketMaterial);
        this.racket.mesh.position.copy(this.racket.currentPosition); 
    }

    handleMouseMove(screenX, screenY) {
        // From Turn 109/85/87
        if (this.controlType !== 'human' || !this.mesh) return; 
        if (this.isServing) { 
             this.mouseScreenX = screenX; 
             this.mouseScreenY = screenY;
             const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
             this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.2);
             this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35); 
             this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35); 
             this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z;
            return; 
        }
        this.mouseScreenX = screenX; this.mouseScreenY = screenY;
        const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.2); 
        this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35); 
        this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35); 
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
    }
    
    positionBallRelativeToHand(gameBall) { 
        if (!this.mesh) return;
        
        const ballRelativePos = new THREE.Vector3(
            this.side * 0.30,  // User's change
            0.4,               // User's change
            0.0                // User's change
        );
        gameBall.position.copy(this.mesh.localToWorld(ballRelativePos.clone()));
    }
    
    serveToss(gameBall) {
        // From Turn 109/85/87
        if (!this.mesh) return; 
        console.log(`Player ${this.side === 1 ? 1 : 2} executes toss action. Ball at world: ${gameBall.position.x.toFixed(2)}`);
        gameBall.toss(TOSS_POWER, this.side === 1 ? 1 : 2);
        this.isServing = true; 
    }
    
    serveHit(gameBall, calculatedVelocity, calculatedSpin) {
        // From Turn 109/85/87
        if (!this.mesh || !this.racket.mesh) return false; 
        const expectedBallStatus = this.side === 1 ? 6 : 7; 
        if (gameBall.status !== expectedBallStatus) {
            console.log("Serve hit: Ball not in correct tossed state for this player."); return false;
        }
        const racketWorldPos = new THREE.Vector3();
        this.racket.mesh.getWorldPosition(racketWorldPos); 
        const hitZone = new THREE.Box3(
            new THREE.Vector3(-0.05, -RACKET_HEIGHT/2 - 0.05, -RACKET_WIDTH_Z/2 - 0.05), 
            new THREE.Vector3( 0.05,  RACKET_HEIGHT/2 + 0.05,  RACKET_WIDTH_Z/2 + 0.05)  
        );
        const ballPosInRacketSpace = this.racket.mesh.worldToLocal(gameBall.position.clone());
        if (!hitZone.containsPoint(ballPosInRacketSpace)) {
            console.warn("Serve hit: Auto-serve calculated but ball NOT in racket's hitting zone. Hitting with fault.", ballPosInRacketSpace);
            const faultVelocity = new THREE.Vector3(this.side * 0.1, 0.1, this.side * -0.5);
            const faultSpin = new THREE.Vector2(0,0);
            gameBall.hit(faultVelocity, faultSpin, this.side === 1 ? 1 : 2);
            this.isServing = false; return false; 
        }
        console.log(`Player ${this.side === 1 ? 1 : 2} hits the serve with calculated trajectory!`);
        gameBall.hit(calculatedVelocity, calculatedSpin, this.side === 1 ? 1 : 2);
        this.isServing = false; return true; 
    }

    swing(gameBall, calculatedServeVelocity = null, calculatedServeSpin = null) { 
        // From Turn 109/85/87
        if (this.isServing) { 
            if (calculatedServeVelocity && calculatedServeSpin) {
                return this.serveHit(gameBall, calculatedServeVelocity, calculatedServeSpin);
            } else {
                console.error("Serve hit called without calculated trajectory for player type: " + this.controlType);
                const faultVel = new THREE.Vector3(this.side*0.1, 0.5, this.side * -1);
                return this.serveHit(gameBall, faultVel, new THREE.Vector2(0,0));
            }
        }
        if (this.controlType === 'human') {
            console.log("Human Player rally swing!");
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; }, 120); 
        } else if (this.controlType === 'ai') {
            console.log("AI Player rally swing!"); this.aiHitting = true; 
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { this.aiHitting = false; this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; }, 150); 
        }
        return true; 
    }
    
    updateAI(ball) {
        // From Turn 109/85/87
        if (!ball || !this.mesh || ball.status < 0) return;
        if (this.isServing) return; 
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
        // From Turn 109/85/87
        if (this.controlType === 'ai') {
            if (!this.isServing) { 
                 this.updateAI(ball);
            }
        }
        if (this.racket.mesh) {
            if (!(this.controlType === 'human' && this.isServing)) {
                this.racket.currentPosition.lerp(this.racket.targetPosition, this.racket.lerpFactor);
                this.racket.mesh.position.copy(this.racket.currentPosition);
            }
        }
    }
}
