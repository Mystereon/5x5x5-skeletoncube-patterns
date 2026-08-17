# Game Plan: Zarch — Voxel Defender

## Product Definition

The browser version is a separate, fully playable interpretation of the CubeFX pattern. Its entire playfield is a visible **10×10×10 voxel cube** rendered with an elevated orthographic isometric camera. The player flies a compact lime-and-cyan craft above generated terrain, intercepts descending landers, protects the surface, survives increasingly dense waves, and earns score multipliers for accurate consecutive shots.

The physical LED-cube effect remains a compact 5×5×5 self-playing scene. The browser game is allowed more space, direct input, richer game state, and a complete HUD, but it retains the same visual identity: black negative space, phosphor green terrain, cyan weapon fire, red-orange enemies, and short amber impact bursts.

## Risk Tasks

### 1. Isometric 10×10×10 Voxel Readability

- **Why isolated:** A complete cube boundary, stacked terrain, aircraft, enemies, projectiles, and particles can visually occlude one another when viewed orthographically.
- **Approach:** Use a fixed elevated orthographic camera with a carefully chosen azimuth, translucent outer cube edges, thin coordinate ticks, restrained terrain height, emissive entity materials, and no fog. Keep the upper half of the cube mostly empty so flight paths remain readable.
- **Verify:** Every cube corner is visible, the 10-voxel extent is obvious, terrain never hides the player, and projectiles can be tracked from launch to impact in both desktop and mobile screenshots.

### 2. Procedural Voxel Terrain

- **Why isolated:** Random terrain can produce blocked sight lines, flat uninteresting maps, disconnected towers, or excessive mesh count.
- **Approach:** Generate a deterministic 10×10 height field from a seeded low-frequency function, clamp heights to 0–3, smooth adjacent cells, and render occupied cells with thin instances sharing one cube mesh and one material family.
- **Verify:** The same seed produces the same landscape, every column stays within the lower four layers, terrain has hills and valleys without isolated spikes, and reseeding produces a visibly different but playable map.

### 3. Craft Motion Inside a Bounded Volume

- **Why isolated:** Free 3-D flight can feel slippery and can allow the player to leave the readable isometric volume.
- **Approach:** Use responsive acceleration with strong damping, quantised altitude bands, explicit bounds, and a slight visual lean based on horizontal velocity. Keyboard, pointer, and touch controls all feed the same input state.
- **Verify:** Pressing each direction moves screen-intuitively, releasing input settles the craft quickly, altitude never intersects terrain or exits the top of the cube, and mobile touch controls match desktop movement.

### 4. Enemy Descent and Collision Timing

- **Why isolated:** Landers must threaten terrain without moving so quickly that the player cannot react, and projectile hits must remain fair in isometric space.
- **Approach:** Spawn landers in the upper rear region, move them between voxel targets with smooth interpolation, reserve an attack column, and use simple sphere/box distance checks rather than a physics plugin. Telegraph the final descent with a brighter red pulse.
- **Verify:** Landers visibly descend toward terrain, a laser crossing an enemy destroys it once, missed landers damage a surface cell, no collision registers through distant depth, and each wave eventually completes.

### 5. Deterministic Demo Mode

- **Why isolated:** Visual verification needs genuine gameplay without manual input.
- **Approach:** `?demo` activates a seeded autopilot that moves the craft, selects targets, fires, and advances waves predictably.
- **Verify:** Repeated demo captures show the same camera, terrain seed, player route, enemies, score progression, and at least one visible impact burst.

## Main Build

The finished game includes a full-screen Babylon.js canvas, a 10×10×10 isometric cube boundary, deterministic terrain generation, player flight, lander waves, laser fire, impact particles, terrain damage, score and multiplier tracking, shields, pause/restart controls, keyboard input, touch controls, and deterministic demo mode.

| Area | Required result |
|---|---|
| Viewport | The entire 10×10×10 cube remains visible at all times in a stable isometric orthographic view. |
| Player | Lime/cyan hover craft with responsive movement, altitude control, boost, and laser fire. |
| Enemies | Red-orange landers that enter from high rear positions and descend toward terrain targets. |
| Combat | White-cyan laser bolts, orange voxel impact bursts, score, streak multiplier, and terrain damage. |
| Progression | Increasing wave count, enemy speed, and simultaneous lander pressure. |
| Controls | WASD or arrows, Space to fire, Shift to boost, R to reseed/restart; large touch controls on compact screens. |
| HUD | Score, wave, shields, streak, pause/restart, compact control legend, and accessible status messaging. |
| Art | Brutalist black interface with phosphor lime, cyan, warm cream, and red-orange accents matching the generated visual target. |

### Assets needed

The game uses procedural Babylon.js meshes for terrain voxels, craft, landers, lasers, particles, and cube bounds. The generated reference image defines palette, composition, object scale, and HUD hierarchy. A small generated surface texture may be added only if it improves terrain readability without obscuring the voxel form.

### Verification

- Movement direction matches keyboard and touch input.
- The craft remains inside the 10×10×10 volume and above terrain.
- The entire cube boundary and all three visible isometric faces remain legible.
- Lasers originate at the craft, travel predictably, and collide only with nearby landers.
- Score, wave, shield, and multiplier values update from real gameplay events.
- UI remains readable with no overlap at 1280×720 and 390×844.
- No missing textures, fallback materials, or browser console errors.
- `?demo` produces a deterministic active-play screenshot with a visible craft, enemies, projectile or impact, terrain, full cube boundary, and HUD.
- The final result matches the generated reference in palette, camera angle, entity scale, density, and negative-space discipline.

