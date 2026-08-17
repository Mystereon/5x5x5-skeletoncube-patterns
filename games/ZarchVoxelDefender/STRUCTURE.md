# Zarch — Voxel Defender

## Runtime

- Babylon.js 9.x
- React 19 + TypeScript + Vite
- Browser URL: `http://127.0.0.1:5173`
- Dimension: 3-D game inside a fixed 10×10×10 logical voxel volume

## App Entry

- `index.html` loads `client/src/main.tsx`.
- `client/src/main.tsx` mounts React once.
- `client/src/App.tsx` renders `GameCanvas` as the only route content.
- `client/src/components/GameCanvas.tsx` owns the Babylon engine lifecycle, resize handling, canvas, HUD shell, and touch-control elements.

## Game Entry

- `client/src/game/scene.ts` exports `createGameScene(engine, canvas)` and returns a disposable `GameHandle`.
- `client/src/game/GameWorld.ts` owns active rules, score, waves, terrain, player, enemies, projectiles, particles, and state publication.
- `client/src/game/Terrain.ts` owns deterministic 10×10 height generation and voxel meshes.
- `client/src/game/WorldBounds.ts` draws the complete isometric cube boundary and coordinate ticks.
- `client/src/game/InputController.ts` converts keyboard, pointer, and touch input into semantic actions.
- `client/src/game/random.ts` supplies deterministic seeded generation and demo behaviour.

## Ownership

Gameplay classes own their Babylon nodes. Babylon remains the renderer and scene graph, not the gameplay architecture. React never calculates collisions, scores, waves, or movement; it only displays state events emitted by `GameWorld`.

## Assets

The game uses procedural meshes for all gameplay objects. The generated visual target at `/manus-storage/zarch-voxel-defender-reference_c1abc04e.png` defines palette, composition, cube framing, object scale, and HUD hierarchy.

## Verification

- `pnpm check`
- `pnpm build`
- `pnpm dev`
- deterministic `/?demo` run for visual capture
- desktop and mobile screenshots showing the complete cube, terrain, craft, enemies, projectile or impact, and HUD
