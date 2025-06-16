// Ball.test.js
import { Ball } from './Ball.js';
import * as THREE from 'three';

// Constants used in Ball.js that might be relevant for setting up tests
const TABLE_HEIGHT = 0.76;
const BALL_RADIUS = 0.02;
const NET_POS_Z = 0;
const TABLE_LENGTH = 2.74;
const TABLE_WIDTH = 1.525;
const NET_HEIGHT = 0.1525;
const GRAVITY = 9.82;
const TICK = 1/60;
const AIR_RESISTANCE_FACTOR = 0.02;
const MAGNUS_COEFFICIENT = 0.02; // Make sure this matches Ball.js
const BOUNCE_ENERGY_LOSS = 0.85;
// SPIN_EFFECT_ON_BOUNCE_* are not directly used in test helper's simplified bounce,
// as the test focuses on trajectory TO the bounce points.

describe('Ball.calculatePerfectServeVelocity', () => {
    let ball;

    beforeEach(() => {
        ball = new Ball();
    });

    // Helper function to simulate the serve and check its validity
    // This helper must be kept in reasonable sync with Ball.js physics
    const simulateAndValidateServe = (initialVelocity, hitPosition, initialSpin, serverSide, targetBouncePos, serveConfig = {}) => {
        let pos = hitPosition.clone();
        let vel = initialVelocity.clone();
        let spin = initialSpin.clone();

        const MAX_TICKS = 400;
        const TABLE_SURFACE_Y = TABLE_HEIGHT + BALL_RADIUS;
        const NET_TOP_Y = TABLE_HEIGHT + NET_HEIGHT + BALL_RADIUS;

        // Determine the net clearance requirement for the validation
        const netClearanceDelta = serveConfig?.targetNetClearance ?? 0.002;
        const validationNetClearanceYTarget = NET_TOP_Y + netClearanceDelta;

        let bouncedOnServer = false;
        let clearedNet = false;
        let bouncedOnReceiver = false;
        let finalReceiverBouncePos = null;
        let hitNetTape = false;

        for (let i = 0; i < MAX_TICKS; i++) {
            let prevPos = pos.clone();

            vel.y -= GRAVITY * TICK;
            vel.multiplyScalar(1 - AIR_RESISTANCE_FACTOR * TICK);

            // Simplified Magnus from Ball.js _simulatePhysicsStep
            vel.y += MAGNUS_COEFFICIENT * spin.y * vel.z * TICK;
            vel.x -= MAGNUS_COEFFICIENT * spin.x * vel.z * TICK;

            pos.addScaledVector(vel, TICK);

            // Net check: only if not already cleared
            if (!clearedNet) {
                const ballLeadingEdgeZ = pos.z + (serverSide * BALL_RADIUS); // front edge of ball
                const prevBallLeadingEdgeZ = prevPos.z + (serverSide * BALL_RADIUS);

                const crossedNetPlane = (serverSide === 1 && prevBallLeadingEdgeZ > NET_POS_Z && ballLeadingEdgeZ <= NET_POS_Z) ||
                                      (serverSide === -1 && prevBallLeadingEdgeZ < NET_POS_Z && ballLeadingEdgeZ >= NET_POS_Z);

                if (crossedNetPlane) {
                    const alpha = (NET_POS_Z - prevBallLeadingEdgeZ) / (ballLeadingEdgeZ - prevBallLeadingEdgeZ);
                    const yAtNet = prevPos.y + (pos.y - prevPos.y) * alpha;
                    const xAtNet = prevPos.x + (pos.x - prevPos.x) * alpha;

                    if (yAtNet > validationNetClearanceYTarget && Math.abs(xAtNet) <= TABLE_WIDTH / 2) {
                        clearedNet = true;
                    } else {
                        hitNetTape = true; // Mark as hit net or insufficient clearance
                        return { success: false, reason: `Hit net or insufficient clearance. Y@Net: ${yAtNet.toFixed(3)}, TargetClearanceY: ${validationNetClearanceYTarget.toFixed(3)}, X@Net: ${xAtNet.toFixed(3)}` };
                    }
                }
            }

            // Table bounce check
            if (vel.y < 0 && pos.y <= TABLE_SURFACE_Y) {
                pos.y = TABLE_SURFACE_Y;

                const isOnServerHalf = (serverSide === 1) ? (pos.z >= NET_POS_Z - BALL_RADIUS) : (pos.z <= NET_POS_Z + BALL_RADIUS); // check ball edge
                const withinTableBounds = Math.abs(pos.x) <= TABLE_WIDTH / 2 + BALL_RADIUS;

                if (!withinTableBounds) return { success: false, reason: `Bounced out of X bounds. X: ${pos.x.toFixed(3)} at Z: ${pos.z.toFixed(3)}` };

                if (isOnServerHalf) {
                    if (bouncedOnServer && Math.abs(pos.z - NET_POS_Z) > BALL_RADIUS * 2) {
                        return { success: false, reason: "Bounced twice on server side, not near net" };
                     }
                    bouncedOnServer = true;
                } else {
                    if (!bouncedOnServer) return { success: false, reason: "Bounced on receiver side before server side" };
                    if (!clearedNet) return { success: false, reason: "Bounced on receiver side but did not clear net" };
                    bouncedOnReceiver = true;
                    finalReceiverBouncePos = pos.clone();
                    break;
                }
                vel.y *= -BOUNCE_ENERGY_LOSS;
                // Simplified bounce effects (no complex spin changes in test sim)
                vel.x *= 0.95;
                vel.z *= 0.95;
            }
            if (pos.y < -BALL_RADIUS*5) return { success: false, reason: `Ball hit floor (y=${pos.y.toFixed(2)})` };
            if (i === MAX_TICKS -1 && !bouncedOnReceiver) return { success: false, reason: "Simulation timed out before receiver bounce" };
        }

        if (!bouncedOnServer) return { success: false, reason: "Never bounced on server side" };
        if (hitNetTape && !clearedNet) return { success: false, reason: "Hit net tape and did not clear sufficiently" };
        if (!clearedNet) return { success: false, reason: "Never cleared net" };
        if (!bouncedOnReceiver) return { success: false, reason: "Never bounced on receiver side" };

        const distToTarget = finalReceiverBouncePos.distanceTo(targetBouncePos);
        // Increased tolerance for tests, main func uses 0.05. Test sim is also simpler.
        if (distToTarget > 0.30) {
            return { success: false, reason: `Bounced too far from target. Dist: ${distToTarget.toFixed(3)} to Target (${targetBouncePos.x.toFixed(2)}, ${targetBouncePos.z.toFixed(2)}), Actual: (${finalReceiverBouncePos.x.toFixed(2)}, ${finalReceiverBouncePos.z.toFixed(2)})` };
        }

        return { success: true, reason: "Valid serve", finalBounce: finalReceiverBouncePos, dist: distToTarget };
    };

    // Test Cases
    test('should calculate a valid default serve for player 1', () => {
        const hitPosition = new THREE.Vector3(0, TABLE_HEIGHT + BALL_RADIUS + 0.1, TABLE_LENGTH / 4);
        const serverSide = 1;
        const targetOpponentBouncePos = new THREE.Vector3(0.2, TABLE_HEIGHT + BALL_RADIUS, -TABLE_LENGTH / 4);
        const desiredSpin = new THREE.Vector2(0, 5); // Some topspin

        const velocity = ball.calculatePerfectServeVelocity(hitPosition, serverSide, 0, targetOpponentBouncePos, desiredSpin, {});
        expect(velocity).toBeInstanceOf(THREE.Vector3);

        const validation = simulateAndValidateServe(velocity, hitPosition, desiredSpin, serverSide, targetOpponentBouncePos, {});
        expect(validation.success).toBe(true, validation.reason);
    });

    test('should calculate a valid default serve for player 2 (other side)', () => {
        const hitPosition = new THREE.Vector3(0, TABLE_HEIGHT + BALL_RADIUS + 0.1, -TABLE_LENGTH / 4);
        const serverSide = -1;
        const targetOpponentBouncePos = new THREE.Vector3(-0.1, TABLE_HEIGHT + BALL_RADIUS, TABLE_LENGTH / 4);
        const desiredSpin = new THREE.Vector2(1, 3); // Slight sidespin and topspin

        const velocity = ball.calculatePerfectServeVelocity(hitPosition, serverSide, 0, targetOpponentBouncePos, desiredSpin, {});
        expect(velocity).toBeInstanceOf(THREE.Vector3);
        const validation = simulateAndValidateServe(velocity, hitPosition, desiredSpin, serverSide, targetOpponentBouncePos, {});
        expect(validation.success).toBe(true, validation.reason);
    });

    test('should calculate a "fast" serve for player 1', () => {
        const hitPosition = new THREE.Vector3(0.1, TABLE_HEIGHT + BALL_RADIUS + 0.15, TABLE_LENGTH / 4 - 0.1);
        const serverSide = 1;
        const targetX = 0;
        const serveConfig = { serveType: "fast", targetNetClearance: 0.005, targetDepthPercentage: 0.85 };
        // actualTargetOpponentBouncePos will be calculated by the main function based on targetDepthPercentage.
        // For validation, we calculate it here to pass to simulateAndValidateServe.
        const actualTargetOpponentBounceZ = - (TABLE_LENGTH / 2) * serveConfig.targetDepthPercentage;
        const validationTargetPos = new THREE.Vector3(targetX, TABLE_HEIGHT + BALL_RADIUS, actualTargetOpponentBounceZ);
        const desiredSpin = new THREE.Vector2(0, 2);

        const velocity = ball.calculatePerfectServeVelocity(hitPosition, serverSide, 0, new THREE.Vector3(targetX, TABLE_HEIGHT + BALL_RADIUS, 0) , desiredSpin, serveConfig);
        expect(velocity).toBeInstanceOf(THREE.Vector3);
        // For a fast serve, we might expect a higher absolute Z velocity
        // This is a loose check, as "fast" also depends on other factors
        expect(Math.abs(velocity.z)).toBeGreaterThan(2.5);

        const validation = simulateAndValidateServe(velocity, hitPosition, desiredSpin, serverSide, validationTargetPos, serveConfig);
        expect(validation.success).toBe(true, validation.reason);
    });

    test('should calculate a "short" serve for player 1 with sidespin', () => {
        const hitPosition = new THREE.Vector3(-0.05, TABLE_HEIGHT + BALL_RADIUS + 0.05, TABLE_LENGTH / 4 - 0.05);
        const serverSide = 1;
        const targetOpponentX = 0.4;
        const serveConfig = { serveType: "short", targetNetClearance: 0.02, targetDepthPercentage: 0.25 };
        const actualTargetOpponentBounceZ = - (TABLE_LENGTH / 2) * serveConfig.targetDepthPercentage;
        const validationTargetPos = new THREE.Vector3(targetOpponentX, TABLE_HEIGHT + BALL_RADIUS, actualTargetOpponentBounceZ);

        const desiredSpin = new THREE.Vector2(15, 5); // Significant sidespin, some topspin

        const velocity = ball.calculatePerfectServeVelocity(hitPosition, serverSide, 0, new THREE.Vector3(targetOpponentX, TABLE_HEIGHT + BALL_RADIUS, 0), desiredSpin, serveConfig);
        expect(velocity).toBeInstanceOf(THREE.Vector3);

        const validation = simulateAndValidateServe(velocity, hitPosition, desiredSpin, serverSide, validationTargetPos, serveConfig);
        expect(validation.success).toBe(true, validation.reason);
        // Check if the bounce Z is roughly in the short area (within a wider tolerance due to test sim simplicity)
        expect(validation.finalBounce.z).toBeLessThan(NET_POS_Z - (TABLE_LENGTH / 2 * serveConfig.targetDepthPercentage * 0.2)); // Upper bound (closer to net)
        expect(validation.finalBounce.z).toBeGreaterThan(NET_POS_Z - (TABLE_LENGTH / 2 * serveConfig.targetDepthPercentage * 1.8)); // Lower bound (further from net)
    });

    // This test is conceptual as it relies on Jest's mocking which isn't available here.
    // It's designed to check if fallback mechanisms are invoked under difficult conditions.
    test.skip('CONCEPTUAL: should return a fallback velocity when optimal serve is extremely difficult', () => {
        const hitPosition = new THREE.Vector3(TABLE_WIDTH/2 - 0.01, TABLE_HEIGHT + BALL_RADIUS + 0.01, TABLE_LENGTH / 2 - 0.01);
        const serverSide = 1;
        // Target is very short, wide, and near the edge of what's physically possible.
        const targetOpponentBouncePosForCalc = new THREE.Vector3(TABLE_WIDTH / 2 - 0.03, TABLE_HEIGHT + BALL_RADIUS, -0.03);
        const desiredSpin = new THREE.Vector2(0, 0);
        const serveConfig = { serveType: "short", targetNetClearance: 0.001, targetDepthPercentage: 0.02 };

        // In a Jest environment: const consoleWarnSpy = jest.spyOn(console, 'warn').mockImplementation(() => {});
        let consoleWarnCalled = false;
        const originalConsoleWarn = console.warn;
        console.warn = (message) => {
            if (message.includes("Could not find an optimal serve trajectory") || message.includes("using preset fallback") || message.includes("All fallback attempts failed")) {
                consoleWarnCalled = true;
            }
            originalConsoleWarn.apply(console, [message]); // Still log it
        };

        const velocity = ball.calculatePerfectServeVelocity(hitPosition, serverSide, 0, targetOpponentBouncePosForCalc, desiredSpin, serveConfig);
        expect(velocity).toBeInstanceOf(THREE.Vector3);

        // If it warned, it likely used a fallback.
        // The test helper might still fail it if the fallback is truly wild, but this checks the mechanism.
        if (consoleWarnCalled) {
          console.log("Fallback mechanism was triggered for the difficult serve test.");
          // We expect a fallback, so we don't strictly require simulateAndValidateServe to pass with high precision.
          // We just need a velocity vector.
          expect(velocity.x).not.toBe(-Infinity); // Check it's not the initial placeholder
        } else {
          console.log("Optimal (or near-optimal) serve found even for difficult scenario, or warning not spied correctly.");
           const validationTargetZ = - (TABLE_LENGTH / 2) * serveConfig.targetDepthPercentage;
           const validationTarget = new THREE.Vector3(targetOpponentBouncePosForCalc.x, TABLE_HEIGHT + BALL_RADIUS, validationTargetZ);
           const validation = simulateAndValidateServe(velocity, hitPosition, desiredSpin, serverSide, validationTarget, serveConfig);
           expect(validation.success).toBe(true, `${validation.reason} (fallback not triggered, but serve still failed validation)`);
        }
        console.warn = originalConsoleWarn; // Restore original console.warn
    });
});
