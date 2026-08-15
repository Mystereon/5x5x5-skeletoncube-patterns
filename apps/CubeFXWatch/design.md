# CubeFX Watch — Watch8 Classic Interface Design

## Product intent

CubeFX Watch is a native Wear OS remote for the 5×5×5 SkeletonCube. It is designed specifically for the **46 mm Galaxy Watch8 Classic** and treats the physical rotating bezel as the principal control: a turn previews an adjacent visual mode, a tap sends it to the cube. The interface is deliberately a compact remote, not a reduced phone app. It retains CubeFX’s graphite, lime, cyan, and warm-orange visual language while prioritising glanceability, tactile control, and one-handed use on a circular display.

## Screen list

| Screen | Primary content and functionality |
|---|---|
| **Live control** | Connection state, current CubeFX pattern, a five-voxel visual marker, and direct access to the primary pattern action. |
| **Pattern bezel picker** | Large centered pattern number and name. The rotating bezel moves through the CubeFX browser modes; tapping the center sends the currently highlighted mode. |
| **Engine controls** | Brightness and motion-speed rows, each adjusted by bezel rotation while focused, with a large Apply action. |
| **Quick actions** | Auto/Manual toggle, Banner entry, Next pattern, Life reseed, and pattern-aware primary/secondary actions. |
| **Banner controls** | Compact message presets, 3×5/5×5 font selector, hue, and scrolling-speed controls. |
| **Connect** | Scan results, CubeFX-5x5x5 pairing status, reconnect and disconnect actions. |

## Key user flows

The main sequence is **open CubeFX Watch → turn the bezel to browse the central pattern title → tap Select → watch sends the selected mode over BLE**. Brightness and speed use a two-stage flow: **touch a control tile → turn bezel in fixed steps → tap Apply**. Banner control is intentionally short: **tap Banner → choose a preset or voice-free compact entry → choose 3×5 or 5×5 → Send**. When Single-player Pong is active, the quick-action screen presents two large opposing touch buttons for the player’s full-height paddle: **left** and **right**.

## Layout and interaction

The primary canvas is circular. The center 60% is reserved for the selected pattern name and status; the outer edge is visually calm so it does not compete with the physical bezel. Touch targets have a minimum 48 dp effective diameter, concise two-line labels, and high-contrast state changes. Bezel input moves exactly one mode per detent-equivalent threshold and provides a light haptic tick; a center tap is required before transmitting a new pattern, preventing accidental selection while the wearer scrolls.

## Colour choices

The watch uses near-black **Graphite** `#0B0D0B` as its background, lime **Cube Green** `#C8FF20` for active selection and apply actions, cyan **BLE Blue** `#32C8FF` for connected state, orange **Nixie Amber** `#FF7A16` for live animation/impact feedback, warning red `#FF432E` for offline or manual-state cautions, and off-white `#EEF4E8` for labels. The palette matches the Android CubeFX controller while remaining readable in bright outdoor conditions.
