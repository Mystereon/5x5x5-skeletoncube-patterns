# CubeFX Race Circuit — Scene Design

**Race Circuit** is an original low-poly, British-Grand-Prix-style fast circuit viewed through the physical 5×5×5 LED cube. It borrows the requested *flow* of a modern Silverstone-type venue—long acceleration zones, linked high-speed direction changes, broad sweepers, a braking hairpin, and a final complex—but uses a newly authored 300×300 coordinate path, original scenery placement, and no copied map or artwork.

## World and physical viewport

The virtual world is 300×300 map units. It is never stored as a 90,000-cell frame buffer. A closed integer path defines the road centreline; each requested voxel calculates its local terrain, road, scenery, cloud, and car state from stable map coordinates. The physical 5×5 ground plane samples map cells four world units apart, making the grey road **three physical LEDs wide** at the normal local view scale.

| World element | Rule | Physical cube treatment |
|---|---|---|
| Track | Closed original 300×300 integer polyline, with pit/start straight, flowing esses, sweepers, hairpin, and final sector | Three-voxel-wide dark grey road on `z = 0`; pale grey dashed/edge markings and a chequered starting grid |
| Grass and terrain | Stable hand-authored hill centres and integer falloff, never random per frame | Green grass with brown earth; terrain appears only on layers `z = 0..2` and the circuit remains level |
| Sandbags | Deterministic low clusters beside braking and corner apex areas | Dull yellow-orange pixels at trackside, never on the racing line |
| Trees | Sparse deterministic world-coordinate placement | Brown trunk/green crown, capped at three physical layers (`z = 0..2`) |
| Spectators | Fixed trackside positions around key corners | One low body pixel plus a pale-pink head pixel, exactly two LEDs high |
| Clouds | Slow deterministic world-space drift, sampled separately from scenery | Cloud ceiling only on `z = 4`; bright centres and dim neighbouring pixels create anti-aliased edges |

## Race behaviour

Four cars are each exactly one lit voxel, with distinct race colours. The camera follows whichever car has the greatest total race distance, keeping the leader near the middle of the moving world slice. The start phase shows the chequered grid, then runs exactly three laps. The leader can change through controlled pace differences; after the third lap, the winning car’s colour and `"<COLOUR> WINS"` message scroll around the perimeter without overwriting the owner’s saved banner text or hue. After the celebration, the scene resets into a new grid sequence.

The normal automatic dwell is raised to cover a complete grid, three-lap race, and winner celebration. A short **GPIO2** press restarts the race from its chequered grid; short **GPIO4** cycles a readable pace/view variant. Long-button behaviour remains the global Banner / Next-pattern model.

## Performance contract

The renderer uses fixed integer path points, squared-distance road tests, small persistent car state, and deterministic hashes. It has no world framebuffer and performs no per-voxel floating-point `sin`, `cos`, or square-root calculation. Any path-length approximation is built once at setup. This keeps the scene consistent with CubeFX’s LUT-first ESP32-S3 performance rule while retaining a coherent world seen through a very small viewport.
