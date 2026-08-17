# Zarch: Voxel Defender

A KISS-style browser-game companion to CubeFX pattern 56. The game presents a playable **10×10×10 isometric defence volume** with a filled low-poly terrain mesh, phosphor wireframe edges, a single swept-tetrahedron craft, descending landers, auto-aim laser fire, impact bursts, shields, score streaks, and wave progression.

## Run locally

```bash
pnpm install
pnpm dev
```

Open `http://localhost:5173`. Add `?demo` to run the deterministic autopilot showcase used for validation.

| Control | Desktop | Touch |
|---|---|---|
| Flight | WASD or arrow keys | Direction pad |
| Altitude | Q / E | UP / DN |
| Fire | Space | FIRE |
| Boost | Shift | BOOST |
| Pause | P | — |
| Restart / reseed | R | — |

## Design rules

The browser version intentionally differs from the physical 5×5×5 CubeFX effect. It expands the world to a 10×10×10 visual volume but retains the same core language: low-poly terrain, a sharply readable flight craft, rising laser fire, descending threats, and brief orange/yellow impact bursts. Terrain geometry uses a cached 10×10 height map and rebuilds only after a lander breach, not every frame.
