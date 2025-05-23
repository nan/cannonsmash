// Player.js
import * as THREE from 'three';

// (Keep existing constants: TABLE_LENGTH, TABLE_WIDTH, PLAYER_Z_OFFSET, RACKET_OFFSET_Z, RACKET_HEIGHT, RACKET_WIDTH_Z, RACKET_DEFAULT_Y, TABLE_HEIGHT, NET_POS_Z, BALL_RADIUS, TOSS_POWER)
const TABLE_LENGTH = 2.74;      
const TABLE_WIDTH = 1.525;      
const PLAYER_Z_OFFSET = 0.2;    
const RACKET_OFFSET_Z = 0.3;    
const RACKET_HEIGHT = 0.22;     
const RACKET_WIDTH_Z = 0.16;    
const RACKET_DEFAULT_Y = 0.2;   
const TABLE_HEIGHT = 0.76;      
const NET_POS_Z = 0;            
const BALL_RADIUS = 0.02;       
const TOSS_POWER = 2.5;

export class Player {
    // ... (constructor, createRacketMesh, handleMouseMove as in Turn 49/51) ...
    constructor(side = 1, scene, controlType = 'human') {
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
        this.mouseScreenX = 0;  
        this.mouseScreenY = 0;  
        const xRangeMultiplier = this.controlType === 'human' ? 0.9 : 0.8;
        this.minX = -TABLE_WIDTH / 2 * xRangeMultiplier; 
        this.maxX =  TABLE_WIDTH / 2 * xRangeMultiplier; 
        this.aiHitting = false; 
        this.isServing = false; 
    }

    createRacketMesh() {
        // ... (as in Turn 49/51)
        const racketGeometry = new THREE.BoxGeometry(0.02, RACKET_HEIGHT, RACKET_WIDTH_Z); 
        const racketMaterial = new THREE.MeshStandardMaterial({ 
            color: this.controlType === 'human' ? 0xcc0000 : 0x00cc00, 
            roughness: 0.7, metalness: 0.3  
        });
        this.racket.mesh = new THREE.Mesh(racketGeometry, racketMaterial);
        this.racket.mesh.position.copy(this.racket.currentPosition); 
    }

    handleMouseMove(screenX, screenY) {
        // ... (as in Turn 49/51)
        if (this.controlType !== 'human' || !this.mesh) return; 
        if (this.isServing && this.controlType === 'human') { 
            this.mouseScreenX = screenX; this.mouseScreenY = screenY;
            this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35);
            this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35);
            this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
            this.racket.currentPosition.copy(this.racket.targetPosition); 
            this.racket.mesh.position.copy(this.racket.currentPosition);
            return;
        }
        this.mouseScreenX = screenX; this.mouseScreenY = screenY;
        const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.1); 
        this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35); 
        this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35); 
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
    }

    /**
     * Positions the ball correctly near the player's "hand" before the toss.
     * @param {Ball} gameBall - The global game ball object.
     */
    positionBallForServeStart(gameBall) {
        if (!this.mesh) return;

        // Position ball slightly to the player's forehand side (e.g., right for P1)
        // and in front, at a comfortable "holding" height before toss.
        const ballInitialRelativePos = new THREE.Vector3(
            this.side * 0.15, // Slightly to the forehand side (player's right if side=1)
            RACKET_DEFAULT_Y - 0.05,  // Start "holding" ball slightly below default racket Y
            this.side * -RACKET_OFFSET_Z + (this.side * -0.05) // Slightly in front of default racket Z, but closer to body
        );
        // Convert this relative position to world space based on player's current body position
        gameBall.position.copy(this.mesh.localToWorld(ballInitialRelativePos.clone()));
        console.log(`Player ${this.side === 1 ? 1 : 2} positioned ball for serve at ${gameBall.position.x.toFixed(2)}, ${gameBall.position.y.toFixed(2)}, ${gameBall.position.z.toFixed(2)}`);
    }
    
    /**
     * Initiates the serve toss. Assumes ball is already positioned by positionBallForServeStart.
     * Calls gameBall.toss().
     * @param {Ball} gameBall - The global game ball object.
     */
    serveToss(gameBall) {
        if (!this.mesh) return; // Should not happen if called correctly
        
        // Ball should already be positioned by positionBallForServeStart, called from main.js
        console.log(`Player ${this.side === 1 ? 1 : 2} executes toss action. Ball current pos: ${gameBall.position.x.toFixed(2)}`);
        
        gameBall.toss(TOSS_POWER, this.side === 1 ? 1 : 2);
        this.isServing = true; 
    }
    
    // ... (serveHit, swing, updateAI, update methods remain the same as in Turn 49/51) ...
    serveHit(gameBall) {
        if (!this.mesh || !this.racket.mesh) return false; 
        const expectedBallStatus = this.side === 1 ? 6 : 7; 
        if (gameBall.status !== expectedBallStatus) {
            console.log("Serve hit: Ball not in correct tossed state."); return false;
        }
        const racketWorldPos = new THREE.Vector3();
        this.racket.mesh.getWorldPosition(racketWorldPos); 
        const hitZone = new THREE.Box3(
            new THREE.Vector3(-0.03, -RACKET_HEIGHT/2 - 0.02, -RACKET_WIDTH_Z/2 - 0.02), 
            new THREE.Vector3( 0.03,  RACKET_HEIGHT/2 + 0.02,  RACKET_WIDTH_Z/2 + 0.02)  
        );
        const ballPosInRacketSpace = this.racket.mesh.worldToLocal(gameBall.position.clone());
        if (!hitZone.containsPoint(ballPosInRacketSpace)) {
            console.log("Serve hit: Ball missed racket's hitting zone.", ballPosInRacketSpace);
            this.isServing = false; return false; 
        }
        console.log(`Player ${this.side === 1 ? 1 : 2} hits the serve!`);
        let baseServeSpeed = 3.0;
        let upFactor = 0.8 + (this.racket.targetPosition.y * 0.5); 
        upFactor = THREE.MathUtils.clamp(upFactor, 0.2, 1.5);
        let sideFactor = this.racket.targetPosition.x * 1.5; 
        let serveVelocity = new THREE.Vector3(
            this.side * sideFactor, upFactor, this.side * -(baseServeSpeed + Math.random() * 0.3) 
        );
        let serveSpin = new THREE.Vector2(
            this.side * sideFactor * 2.0, 2.0 + (this.racket.targetPosition.y * 2.0) + Math.random() * 1.0 
        ); 
        serveSpin.y = THREE.MathUtils.clamp(serveSpin.y, -2, 5); 
        gameBall.hit(serveVelocity, serveSpin, this.side === 1 ? 1 : 2);
        this.isServing = false; return true; 
    }
    swing(gameBall) { 
        if (this.isServing) { return this.serveHit(gameBall); }
        if (this.controlType === 'human') {
            console.log("Human Player rally swing!");
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; }, 120); 
        } else if (this.controlType === 'ai') {
            console.log("AI Player rally swing!"); 
            this.aiHitting = true; 
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { this.aiHitting = false; this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; }, 150); 
        }
        return true; 
    }
    updateAI(ball) {
        // (Logic from Turn 41 - which is the same as Turn 45 for updateAI)
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
            if (!(this.controlType === 'human' && this.isServing)) {
                this.racket.currentPosition.lerp(this.racket.targetPosition, this.racket.lerpFactor);
                this.racket.mesh.position.copy(this.racket.currentPosition);
            }
        }
    }
}
