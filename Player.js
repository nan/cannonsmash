// Player.js - Z-axis movement and fixed racket pose
import * as THREE from 'three';

// (Constants as in Turn 171)
const TABLE_LENGTH = 2.74;      
const TABLE_WIDTH = 1.525;      
const PLAYER_Z_OFFSET = 0.2; 
const RACKET_OFFSET_Z = 0.3; // This might become effectively 0 for racket relative Z
const RACKET_DEFAULT_Y = 0.2; // This will change for fixed world Y
const TABLE_HEIGHT = 0.76;      
const NET_POS_Z = 0;            
const BALL_RADIUS = 0.02;       
const TOSS_POWER = 2.5; 
const RACKET_HEIGHT = 0.22;     
const RACKET_WIDTH_Z = 0.16;    
const PLAYER_Z_RANGE_MIN_FROM_NET = TABLE_LENGTH / 2 + 0.05; 
const PLAYER_Z_RANGE_MAX_FROM_NET = TABLE_LENGTH / 2 + PLAYER_Z_OFFSET + 0.5;

// New fixed racket Y position in world coordinates
const RACKET_WORLD_Y = 0.9;

export class Player {
    constructor(side = 1, scene, controlType = 'human') { 
        this.side = side; this.scene = scene; this.controlType = controlType; 
        let initialX = (controlType === 'human') ? this.side * 0.15 : 0;
        this.position = new THREE.Vector3(initialX, 0.4, this.side * ((PLAYER_Z_RANGE_MIN_FROM_NET + PLAYER_Z_RANGE_MAX_FROM_NET) / 2));
        this.mesh = null; 
        
        // Racket's new fixed relative position
        const relativeRacketX = this.side * 0.30; // To player's forehand side
        // Racket world Y is RACKET_WORLD_Y (0.9). Player mesh world Y is this.position.y (0.4).
        // So, racket relative Y to player mesh origin is RACKET_WORLD_Y - this.position.y
        const relativeRacketY = RACKET_WORLD_Y - this.position.y; // Should be 0.5 if player Y is 0.4
        const relativeRacketZ = 0.0; // Racket Z is aligned with player's Z (no forward offset)

        this.racket = {
            mesh: null,         
            targetPosition: new THREE.Vector3(relativeRacketX, relativeRacketY, relativeRacketZ), 
            currentPosition: new THREE.Vector3(relativeRacketX, relativeRacketY, relativeRacketZ),
            lerpFactor: this.controlType === 'human' ? 0.2 : 0.1 // Racket lerp can be faster as it's fixed to body
        };
        this.createRacketMesh();
        this.mouseScreenX = 0; this.mouseScreenY = 0;  
        const xRangeMultiplier = this.controlType === 'human' ? 0.9 : 0.8;
        this.minX = -TABLE_WIDTH / 2 * xRangeMultiplier;
        this.maxX =  TABLE_WIDTH / 2 * xRangeMultiplier;
        if (this.side === 1) { // Player 1 on positive Z side
            this.minZ = PLAYER_Z_RANGE_MIN_FROM_NET;
            this.maxZ = PLAYER_Z_RANGE_MAX_FROM_NET;
        } else { // Player 2 on negative Z side
            this.minZ = -PLAYER_Z_RANGE_MAX_FROM_NET;
            this.maxZ = -PLAYER_Z_RANGE_MIN_FROM_NET;
        }
        this.aiHitting = false; this.isServing = false; 
    }

    createRacketMesh() {
        // As in Turn 147/101
        const racketGeometry = new THREE.BoxGeometry(0.02, RACKET_HEIGHT, RACKET_WIDTH_Z); 
        const racketMaterial = new THREE.MeshStandardMaterial({ 
            color: this.controlType === 'human' ? 0xcc0000 : 0x00cc00, roughness: 0.7, metalness: 0.3  
        });
        this.racket.mesh = new THREE.Mesh(racketGeometry, racketMaterial);
        this.racket.mesh.position.copy(this.racket.currentPosition); 
    }

    handleMouseMove(screenX, screenY) {
        if (this.controlType !== 'human' || !this.mesh) return; 

        this.mouseScreenX = screenX;
        this.mouseScreenY = screenY;

        // X-axis movement for player body (remains controlled by screenX)
        const targetPlayerX = THREE.MathUtils.mapLinear(this.mouseScreenX, -1, 1, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.2);

        // Z-axis movement for player body (now controlled by screenY)
        // Map screenY (-1 to 1, inverted so +1 is "forward" towards net) to player's Z range.
        let targetPlayerZ;
        if (this.side === 1) { // Player 1, positive Z side
            targetPlayerZ = THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, this.maxZ, this.minZ); // mouse Y up -> closer to net
        } else { // Player 2, negative Z side
            targetPlayerZ = THREE.MathUtils.mapLinear(this.mouseScreenY, -1, 1, this.minZ, this.maxZ); // mouse Y up -> closer to net
        }
        this.mesh.position.z = THREE.MathUtils.lerp(this.mesh.position.z, targetPlayerZ, 0.15); // Z lerp factor

        // Racket's targetPosition is fixed relative to the player body and set in the constructor.
        // It does not change based on mouse movement anymore.
        // The 'isServing' block that modified racket aiming is removed.
    }
    
    positionBallRelativeToHand(gameBall) { 
        // As in Turn 147/101 (which is Turn 97 for this specific method's content)
        if (!this.racket.mesh) return; 
        const ballRelativePos = new THREE.Vector3(0, RACKET_HEIGHT / 2 + BALL_RADIUS + 0.01, 0.03);
        gameBall.position.copy(this.racket.mesh.localToWorld(ballRelativePos.clone()));
    }
    
    serveToss(gameBall) {
        // As in Turn 147/101
        if (!this.mesh) return; 
        console.log(`Player ${this.side === 1 ? 1 : 2} executes toss action. Ball at world: ${gameBall.position.x.toFixed(2)}`);
        gameBall.toss(TOSS_POWER, this.side === 1 ? 1 : 2);
        this.isServing = true; 
    }
    
    serveHit(gameBall, calculatedVelocity, calculatedSpin) {
        // As in Turn 147/101
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
        // As in Turn 147/101
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
            this.racket.targetPosition.z += this.side * -0.25; // Simple visual swing
            setTimeout(() => { this.racket.targetPosition.z = this.racket.currentPosition.z; }, 120); // Return to fixed Z
        } else if (this.controlType === 'ai') {
            console.log("AI Player rally swing!"); this.aiHitting = true; 
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => { this.aiHitting = false; this.racket.targetPosition.z = this.racket.currentPosition.z; }, 150); 
        }
        return true; 
    }
    
    updateAI(ball) {
        // As in Turn 147/101
        if (!ball || !this.mesh || ball.status < 0) return;
        if (this.isServing) return; 
        // AI Player Body Z Movement (Simplified: try to stay in middle of its Z range)
        let targetPlayerZ;
        if (this.side === 1) { targetPlayerZ = (this.minZ + this.maxZ) / 2; }
        else { targetPlayerZ = (this.minZ + this.maxZ) / 2; }
        this.mesh.position.z = THREE.MathUtils.lerp(this.mesh.position.z, targetPlayerZ, 0.03);

        // AI Player Body X Movement (tracks ball predictively)
        let predictedBallX = ball.position.x;
        if ((this.side === -1 && ball.velocity.z > 0) || (this.side === 1 && ball.velocity.z < 0)) {
            const timeToNet = (ball.velocity.z !== 0) ? Math.abs((NET_POS_Z - ball.position.z) / ball.velocity.z) : 0.1;
            const timeToHitZone = (ball.velocity.z !==0) ? Math.abs(((this.position.z + this.racket.targetPosition.z) - ball.position.z) / ball.velocity.z) : 0.1; // Racket Z is 0 relative
            const predictionTime = Math.min(timeToNet, timeToHitZone, 0.5); 
            if (predictionTime > 0 && isFinite(ball.velocity.z) && ball.velocity.z !== 0) {
                 predictedBallX = ball.position.x + ball.velocity.x * predictionTime;
            }
        }
        const targetPlayerX = THREE.MathUtils.clamp(predictedBallX, this.minX, this.maxX);
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.08); 
        
        // Racket is fixed relative to body, so no specific racket positioning logic needed here for AI rally.
        // AI Swing Decision Logic (as in Turn 147/101)
        const ballIsComingTowardsAI = (this.side === -1 && ball.velocity.z > 0.2) || (this.side === 1 && ball.velocity.z < -0.2);   
        const ballNearAISideOfNet = (this.side === -1 && ball.position.z < (NET_POS_Z + 0.3)) || (this.side === 1 && ball.position.z > (NET_POS_Z - 0.3));
        // distZToBall now considers racket's fixed Z (0 relative to player)
        const distZToBall = Math.abs(ball.position.z - this.mesh.position.z); 
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
            if (!this.isServing) { 
                 this.updateAI(ball);
            }
        }
        // Racket always lerps to its fixed targetPosition relative to the player.
        // targetPosition itself doesn't change after constructor for fixed racket pose.
        if (this.racket.mesh) {
            this.racket.currentPosition.lerp(this.racket.targetPosition, this.racket.lerpFactor);
            this.racket.mesh.position.copy(this.racket.currentPosition);
        }
    }
}
