// Player.js
import * as THREE from 'three';

// --- Constants for Player positioning and behavior ---
const TABLE_LENGTH = 2.74;      // Length of the table (along Z-axis), used for positioning player relative to table.
const TABLE_WIDTH = 1.525;      // Width of the table (along X-axis), used for player's lateral movement range.
const PLAYER_Z_OFFSET = 0.2;    // Default distance the player stands behind their respective table edge.
const RACKET_OFFSET_Z = 0.3;    // How far in front of the player's body center the racket is typically held (depth).
const RACKET_DEFAULT_Y = 0.2;   // Default height of the racket relative to the player's base/origin (center of the capsule mesh).

// **** ADD THESE MISSING CONSTANTS ****
const TABLE_HEIGHT = 0.76;      // Height of the table surface from the floor (meters) - Used in AI logic
const NET_POS_Z = 0;            // Z-coordinate of the net's center line, used by AI to determine ball's side.
const BALL_RADIUS = 0.02;       // Radius of the ball - Used in AI logic for height calculation.

export class Player {
    /**
     * Constructor for a Player object.
     * @param {number} side - Which side of the table: 1 for positive Z (human default), -1 for negative Z (AI default).
     * @param {THREE.Scene} scene - The Three.js scene object (currently unused within Player class but passed).
     * @param {string} controlType - 'human' for mouse-controlled player, 'ai' for AI-controlled.
     */
    constructor(side = 1, scene, controlType = 'human') {
        this.side = side;       // Player's assigned side of the table (1 or -1).
        this.scene = scene;     // Reference to the Three.js scene (can be used for context or future extensions).
        this.controlType = controlType; // Determines if player is controlled by 'human' input or 'ai' logic.

        // Logical base position of the player (center of their capsule mesh).
        this.position = new THREE.Vector3(
            0,                                              
            // Player's base is on the floor, visual mesh origin might be center.
            // If capsule origin is at its center, its Y position should be capsuleHeight/2.
            // Let's assume player mesh (CapsuleGeometry 0.8 high) origin is at its center.
            // So, player base Y position should be 0.4 for it to sit on floor.
            0.4, // Half of capsule height to sit on floor. Original: 0.76
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
        if (this.controlType === 'human') {
            console.log("Human Player swings!");
            this.racket.targetPosition.z += this.side * -0.25; 
            setTimeout(() => {
                 this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z;
            }, 120); 
        } else if (this.controlType === 'ai') {
            console.log("AI Player swings!");
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

        const targetPlayerX = THREE.MathUtils.clamp(ball.position.x, this.minX, this.maxX); 
        this.mesh.position.x = THREE.MathUtils.lerp(this.mesh.position.x, targetPlayerX, 0.07); 

        let desiredRacketX = ball.position.x - this.mesh.position.x; 
        // Use the defined BALL_RADIUS constant here
        let desiredRacketY = RACKET_DEFAULT_Y + (ball.position.y - (TABLE_HEIGHT + BALL_RADIUS)); 
        
        desiredRacketX = THREE.MathUtils.clamp(desiredRacketX, -0.4, 0.4); 
        desiredRacketY = THREE.MathUtils.clamp(desiredRacketY, 0.0, 0.5);   
        
        this.racket.targetPosition.x = desiredRacketX;
        this.racket.targetPosition.y = desiredRacketY;
        this.racket.targetPosition.z = this.side * -RACKET_OFFSET_Z; 

        const ballComingTowardsAI = (this.side === -1 && ball.velocity.z > 0.1) || 
                                  (this.side === 1 && ball.velocity.z < -0.1);   

        // Use the defined NET_POS_Z constant here
        const ballOnAISideOfNet = (this.side === -1 && ball.position.z < (NET_POS_Z + 0.2)) || 
                                  (this.side === 1 && ball.position.z > (NET_POS_Z - 0.2));
        
        const distZToBall = Math.abs(ball.position.z - (this.mesh.position.z + this.racket.targetPosition.z));

        // Use TABLE_HEIGHT for vertical hittable window check
        if (ballComingTowardsAI && ballOnAISideOfNet && distZToBall < 0.25 && 
            ball.position.y < TABLE_HEIGHT + 0.3 && ball.position.y > TABLE_HEIGHT - 0.1) { 
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
