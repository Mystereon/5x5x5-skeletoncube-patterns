# Production Memory

## Fixed Decisions

The browser game is separate from the 5×5×5 firmware pattern. It uses a **10×10×10 logical voxel volume**, a fixed isometric orthographic camera, procedural Babylon.js meshes, and direct player controls. The firmware remains the authoritative small-display interpretation and does not stream browser frames over BLE.

## Visual Rules

The complete cube must remain on screen. Terrain is limited to the lower four layers. The upper volume stays sparse. Entity materials use emissive colour so gameplay remains readable without expensive post-processing. The outer cube is a thin, translucent line structure rather than a solid cage.

## Technical Rules

React owns only the full-screen canvas lifecycle. Gameplay is implemented as plain TypeScript classes under `client/src/game/`. Babylon is initialised once, disposed cleanly, and resized with the window. The game must include deterministic `?demo` autopilot for screenshot verification.

## Current Risks

The largest risks are isometric depth readability, bounded craft controls, procedural terrain mesh count, and fair projectile collision. These must be validated before optional polish.

## Scaffold Verification

The first live WebGL2 run rendered the complete 10×10×10 boundary, coordinate ticks, deterministic terrain, lime/cyan player craft, brutalist HUD, and desktop control legend without browser-console errors. The orthographic view clearly communicates the cube volume. The terrain currently dominates the lower half as intended, but combat entities and deterministic demo activity are still required before final visual verification. React StrictMode deliberately initialised the development scene twice during its mount probe, and the cleanup path prevented simultaneous active engines.

## Combat Verification

The deterministic `?demo` run now shows bright tiered terrain, the lime/cyan craft in open air, a red-orange descending lander, the complete cube frame, and live threat text. Within the same capture cycle, the DOM state advanced to `LANDER NEUTRALISED`, score `100`, and streak `×2`, proving the projectile-hit-score path is active even when the exact laser frame falls between screenshots. The terrain texture is retained as a subtle base underlay, while the voxel columns use emissive solid colour for reliable readability.

## Corrected Visual Direction

The user rejected stacked voxel terrain and the original winged craft. The current direction is KISS: a continuous low-poly terrain heightfield with filled dark-green triangular facets and bright phosphor wireframe edges, plus one self-lit swept tetrahedron player craft with a minimal cyan vector outline. The latest demo frame confirms the desired filled-wireframe terrain and simplified craft silhouette are both present in the full 10×10×10 cube.

## Mobile Capture

A 390×844 headless capture confirms that the mobile HUD and large touch D-pad/actions reflow into the expected lower-screen positions. The one-shot headless Chromium capture did not include WebGL pixels from the live canvas, so it is not suitable as a visual-render proof; the interactive browser WebGL capture remains the evidence for scene rendering. Mobile controls are still bound through the same semantic `data-control` input path as keyboard controls.

After increasing the narrow-screen orthographic extent, a delayed 390×844 WebGL capture shows the full isometric cube boundary, filled-wireframe terrain, swept tetrahedron craft, HUD, and touch controls together. The game is therefore framed for phone portrait without clipping the cube's top, base, or horizontal corners.
