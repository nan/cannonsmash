// Player.js
import * as THREE from 'three';

// (Keep existing constants: TABLE_LENGTH, TABLE_WIDTH, PLAYER_Z_OFFSET, RACKET_OFFSET_Z, RACKET_DEFAULT_Y, TABLE_HEIGHT, NET_POS_Z, BALL_RADIUS, TOSS_POWER)
const TABLE_LENGTH = 2.74;      
const TABLE_WIDTH = 1.525;      
const PLAYER_Z_OFFSET = 0.2;    
const RACKET_OFFSET_Z = 0.3;    // Default Z offset of racket from player center
const RACKET_HEIGHT = 0.22;     // Visual height of the racket mesh
const RACKET_WIDTH_Z = 0.16;    // Visual depth/width of the racket mesh along Z
const RACKET_DEFAULT_Y = 0.2;   // Default Y position of racket center relative to player mesh origin
const TABLE_HEIGHT = 0.76;      
const NET_POS_Z = 0;            
const BALL_RADIUS = 0.02;       
const TOSS_POWER = 2.5;

export class Player {
    // ... (constructor, createRacketMesh, handleMouseMove, serveToss as in Turn 45) ...
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
        const racketGeometry = new THREE.BoxGeometry(0.02, RACKET_HEIGHT, RACKET_WIDTH_Z); 
        const racketMaterial = new THREE.MeshStandardMaterial({ 
            color: this.controlType === 'human' ? 0xcc0000 : 0x00cc00, 
            roughness: 0.7, metalness: 0.3  
        });
        this.racket.mesh = new THREE.Mesh(racketGeometry, racketMaterial);
        this.racket.mesh.position.copy(this.racket.currentPosition); 
    }

    handleMouseMove(screenX, screenY) {
        if (this.controlType !== 'human' || !this.mesh) return; 
        if (this.isServing && this.controlType === 'human') { 
            // Allow mouse to position racket more directly for serve hit aiming
            this.mouseScreenX = screenX;
            this.mouseScreenY = screenY;
            // Racket X relative to player's current X. Player body X is fixed during serve toss.
            this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35);
            // Racket Y relative to player's current Y (which is player's base Y + RACKET_DEFAULT_Y for racket center)
            this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35);
            // Z position of racket can be fixed for serve, or slightly adjusted by player for timing (advanced)
            this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; // Default Z for aiming
            // Update current position directly or with faster lerp for serve aiming
            this.racket.currentPosition.copy(this.racket.targetPosition); // Direct control for aiming
            this.racket.mesh.position.copy(this.racket.currentPosition);
            return;
        }

        this.mouseScreenX = screenX;
        this.mouseScreenY = screenY;
        const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.1); 
        this.racket.targetPosition.x = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, -0.35, 0.35); 
        this.racket.targetPosition.y = RACKET_DEFAULT_Y + THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, -0.15, 0.35); 
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 
    }
    
    serveToss(gameBall) {
        if (!this.mesh) return;
        console.log(`Player ${this.side === 1 ? 1 : 2} initiates toss.`);
        // Player body X should be relatively stable during toss for human. AI might adjust.
        if (this.controlType === 'human') {
            this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, 0, 0.1); // Encourage centering for serve
        }

        const ballInitialRelativePos = new THREE.Vector3(
            this.side * 0.15, 
            RACKET_DEFAULT_Y + 0.1,  
            this.side * -RACKET_OFFSET_Z + (this.side * -0.1) 
        );
        gameBall.position.copy(this.mesh.localToWorld(ballInitialRelativePos.clone()));
        gameBall.toss(TOSS_POWER, this.side === 1 ? 1 : 2);
        this.isServing = true; 
    }
    
    serveHit(gameBall) {
        if (!this.mesh || !this.racket.mesh) return false; // Return boolean for success

        const expectedBallStatus = this.side === 1 ? 6 : 7; 
        if (gameBall.status !== expectedBallStatus) {
            console.log("Serve hit: Ball not in correct tossed state.");
            return false;
        }

        const racketWorldPos = new THREE.Vector3();
        this.racket.mesh.getWorldPosition(racketWorldPos); // Racket's actual current world position

        // Define a small "hitting zone" box for the racket face
        // Racket dimensions: thickness (X=0.02), height (Y=0.22), width (Z=0.16)
        // This box is in racket's local space, centered at (0,0,0) for the racket mesh.
        const hitZone = new THREE.Box3(
            new THREE.Vector3(-0.03, -RACKET_HEIGHT/2 - 0.02, -RACKET_WIDTH_Z/2 - 0.02), // Min extent (add some margin)
            new THREE.Vector3( 0.03,  RACKET_HEIGHT/2 + 0.02,  RACKET_WIDTH_Z/2 + 0.02)  // Max extent
        );
        // Transform the ball's position into the racket's local coordinate system
        const ballPosInRacketSpace = this.racket.mesh.worldToLocal(gameBall.position.clone());

        // Check if the ball (as a point) is within the racket's local hitting zone
        if (!hitZone.containsPoint(ballPosInRacketSpace)) {
            console.log("Serve hit: Ball missed racket's hitting zone.", ballPosInRacketSpace);
            // Optional: If player swings and misses, let ball drop (will result in fault via checkGameRules)
            // No hit applied, ball continues its tossed trajectory.
            this.isServing = false; // Player attempted the hit, serve sequence over for them.
            return false; 
        }
        
        // Ball is hittable by the racket
        console.log(`Player ${this.side === 1 ? 1 : 2} hits the serve!`);
        
        // Simplified serve velocity based on racket's general orientation (aimed by mouse)
        // For a more direct hit, we want Z velocity primarily.
        // Racket targetPosition.x and .y (relative to player) can influence this.
        let baseServeSpeed = 3.0;
        let upFactor = 0.8 + (this.racket.targetPosition.y * 0.5); // Higher aim = more upward
        upFactor = THREE.MathUtils.clamp(upFactor, 0.2, 1.5);

        let sideFactor = this.racket.targetPosition.x * 1.5; // Mouse X influences side direction

        let serveVelocity = new THREE.Vector3(
            this.side * sideFactor, // Side direction based on racket aim X
            upFactor,  
            this.side * -(baseServeSpeed + Math.random() * 0.3) 
        );
        
        let serveSpin = new THREE.Vector2(
            this.side * sideFactor * 2.0, // Sidespin based on racket aim X
            2.0 + (this.racket.targetPosition.y * 2.0) + Math.random() * 1.0 // Topspin influenced by racket Y aim
        ); 
        serveSpin.y = THREE.MathUtils.clamp(serveSpin.y, -2, 5); // Clamp topspin/backspin
        
        gameBall.hit(serveVelocity, serveSpin, this.side === 1 ? 1 : 2);
        this.isServing = false; 
        return true; // Hit was successful
    }

    swing(gameBall) { 
        if (this.isServing) { 
            return this.serveHit(gameBall); // Return success/failure of serveHit
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
        return true; // Rally swing attempt always "succeeds" in terms of action trigger
    }
    
    // ... (updateAI as in Turn 41) ...
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
            // For human player during serve aim, racket position is set directly in handleMouseMove
            if (!(this.controlType === 'human' && this.isServing)) {
                this.racket.currentPosition.lerp(this.racket.targetPosition, this.racket.lerpFactor);
                this.racket.mesh.position.copy(this.racket.currentPosition);
            }
        }
    }
}
