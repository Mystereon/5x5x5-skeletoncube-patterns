# CubeFX Next-Wave Pattern Roadmap

**Prepared for Dad (MysterEon) & Manus.** This roadmap groups the requested ideas into small, readable 5×5×5 scenes. Each scene remains `millis()`-driven, uses the rear ring as an acrylic-facing second visual plane, and avoids full virtual colour framebuffers where a procedural sample is sufficient.

| Batch | Working scene names | Core visual idea | Rear-ring role |
|---|---|---|---|
| A — Geometric energy | Reactor Core, Core Meltdown, Tumbling Tetrahedron, Oscillating Wave Field, Intersecting Planes, Rainbow Spiral, Plasma Entity Containment | Clear high-contrast forms and progressive containment/failure states | Complementary halo, containment cage, alarm pulse, or travelling energy marker |
| B — Instruments | Targeting System, Phosphor Radar, Ghost Detector, Alert, Gyroscope, Artificial Horizon, Spirit Box | Legible monitoring tools with sweep, lock, noise, level, or orientation state | Scan marker, bearing marker, alert flash, or signal noise |
| C — Environments | Earth, Green Clouds, Red Clouds, Forest Fly, Cornucopia, Theatre Chase, Voxel Blocks | Compact scenery with changing travel, palette, and depth cues | Atmosphere, sky, stage footlight, or landscape counter-colour |
| D — Creatures | Flying Gulls, Dog, Ocean Hunter | Original silhouette scenes: flock, wagging pet, and open-water predator pass | Sky, grass, or water illusion supplied through acrylic |
| E — Virtual viewport | Face Scan Window, Prenatal Scan Window, Battle Station Plans, Siege Walker | Procedural virtual volumes sampled through the physical cube window | Medical/projector scan, blueprint glow, or battlefield alert |
| F — Playful cinema | Yellow Suburb Family, Missile Defence Command | Original family-silhouette and command-screen scenes; no third-party character art | Warm domestic glow, warning pulse, or command accent |

## Naming policy

Several requests reference famous films, games, shows, or characters. The public library will preserve the *kind* of scene—such as an ocean-hunter pass, a battle-station blueprint, a four-legged siege walker, a yellow-suburb family silhouette, or a block-world explorer—but will use original names, geometry, and artwork. This keeps the project cleanly shareable while preserving the visual fun.

## Viewport principle

The **Face Scan Window** and **Prenatal Scan Window** use large procedural volumes. A future 10×10×5 cube can change only its physical viewport dimensions; it does not need to allocate a virtual 20×20×5 colour buffer. The sampling function answers only the voxels currently on display.

## Implementation standard

New scenes must obey the existing CubeFX rules: no blocking delays, no per-voxel floating-point trigonometry where a setup-time LUT or a compact reusable table works, manual-selection compatibility, scene-aware rear-ring treatment, and Android/BLE selector support.
