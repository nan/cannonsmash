// Player.js
import * as THREE from 'three';

// Constants from Turn 41 baseline
const TABLE_LENGTH = 2.74;      
const TABLE_WIDTH = 1.525;      
const PLAYER_Z_OFFSET = 0.2;    
const RACKET_OFFSET_Z = 0.3;    
const RACKET_DEFAULT_Y = 0.2;   
const TABLE_HEIGHT = 0.76;      
const NET_POS_Z = 0;            
const BALL_RADIUS = 0.02;       

// New constant for toss power
const TOSS_POWER = 2.5; 

// New constants for racket dimensions (used in serveHit)
const RACKET_HEIGHT = 0.22;     
const RACKET_WIDTH_Z = 0.16;    

export class Player {
    constructor(side = 1, scene, controlType = 'human') { 
        this.side = side;       
        this.scene = scene;     
        this.controlType = controlType; 
        this.position = new THREE.Vector3(
            0,                                              
            0.4, // Player mesh origin Y
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
        this.isServing = false; // ADDED: Flag for serve sequence
    }

    createRacketMesh() {
        // Use RACKET_HEIGHT, RACKET_WIDTH_Z
        const racketGeometry = new THREE.BoxGeometry(0.02, RACKET_HEIGHT, RACKET_WIDTH_Z); 
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

        // ADDED: Direct racket control for aiming if serving
        if (this.isServing && this.controlType === 'human') { 
            this.mouseScreenX = screenX;
            this.mouseScreenY = screenY;
            this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35);
            this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35);
            this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
            this.racket.currentPosition.copy(this.racket.targetPosition); 
            this.racket.mesh.position.copy(this.racket.currentPosition);
            return; // Player body doesn't move during serve aim after toss
        }

        // Original rally mouse move logic
        this.mouseScreenX = screenX;
        this.mouseScreenY = screenY;
        const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.1); 
        this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35); 
        this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35); 
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
    }
    
    /**
     * ADDED: Positions the ball correctly near the player's "hand" before the toss.
     * @param {Ball} gameBall - The global game ball object.
     */
    positionBallForServeStart(gameBall) {
        if (!this.mesh) return;
        if (this.controlType === 'human') { // Center human player slightly for serve
            this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, 0, 0.1);
        }
        const ballInitialRelativePos = new THREE.Vector3(
            this.side * 0.15, 
            RACKET_DEFAULT_Y - 0.05,  // Holding height
            this.side * -RACKET_OFFSET_Z + (this.side * -0.05) 
        );
        gameBall.position.copy(this.mesh.localToWorld(ballInitialRelativePos.clone()));
        console.log(`Player ${this.side === 1 ? 1 : 2} positioned ball for serve.`);
    }
    
    /**
     * ADDED: Initiates the serve toss.
     * @param {Ball} gameBall - The global game ball object.
     */
    serveToss(gameBall) {
        if (!this.mesh) return; 
        // Ball should have been positioned by positionBallForServeStart
        console.log(`Player ${this.side === 1 ? 1 : 2} executes toss action.`);
        gameBall.toss(TOSS_POWER, this.side === 1 ? 1 : 2); // Pass player ID
        this.isServing = true; 
    }
    
    /**
     * ADDED: Executes the serve hit on a tossed ball.
     * @param {Ball} gameBall - The global game ball object.
     * @returns {boolean} True if hit was successful, false otherwise.
     */
    serveHit(gameBall) {
        if (!this.mesh || !this.racket.mesh) return false; 

        const expectedBallStatus = this.side === 1 ? 6 : 7; 
        if (gameBall.status !== expectedBallStatus) {
            console.log("Serve hit: Ball not in correct tossed state for this player.");
            return false;
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
            this.isServing = false; 
            return false; 
        }
        
        console.log(`Player ${this.side === 1 ? 1 : 2} hits the serve!`);
        let baseServeSpeed = 3.0;
        let upFactor = 0.8 + (this.racket.targetPosition.y * 0.5); 
        upFactor = THREE.MathUtils.clamp(upFactor, 0.2, 1.5);
        let sideFactor = this.racket.targetPosition.x * 1.5; 

        let serveVelocity = new THREE.Vector3(
            this.side * sideFactor, 
            upFactor,  
            this.side * -(baseServeSpeed + Math.random() * 0.3) 
        );
        let serveSpin = new THREE.Vector2(
            this.side * sideFactor * 2.0, 
            2.0 + (this.racket.targetPosition.y * 2.0) + Math.random() * 1.0 
        ); 
        serveSpin.y = THREE.MathUtils.clamp(serveSpin.y, -2, 5); 
        
        gameBall.hit(serveVelocity, serveSpin, this.side === 1 ? 1 : 2);
        this.isServing = false; 
        return true; 
    }

    // MODIFIED: swing now calls serveHit if isServing
    swing(gameBall) { 
        if (this.isServing) { 
            return this.serveHit(gameBall);
        }
        // Rally swing logic
        if (this.controlType === 'human') {
            console.log("Human Player rally swing!");
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => {
                 this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z;
            }, 120); 
        } else if (this.controlType === 'ai') {
            console.log("AI Player rally swing!"); 
            this.aiHitting = true; 
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { 
                this.aiHitting = false; 
                this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
            }, 150); 
        }
        return true; // Rally swing attempt considered successful action-wise
    }
    
    updateAI(ball) { // This is the refined AI logic from Turn 40/41
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
            // MODIFIED: Racket lerping logic considering 'isServing' for human
            if (!(this.controlType === 'human' && this.isServing)) {
                this.racket.currentPosition.lerp(this.racket.targetPosition, this.racket.lerpFactor);
                this.racket.mesh.position.copy(this.racket.currentPosition);
            }
            // If human and isServing, racket position is handled directly by handleMouseMove.
        }
    }
}
