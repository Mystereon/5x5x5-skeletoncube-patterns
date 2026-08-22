# CubeFX User Pattern Editor — Design Contract

## Scope

CubeFXWeb will expose **five user-pattern slots**. Each slot contains a **16-step sequence**. A step shows the whole physical cube as five tappable 5×5 matrix panels: `z = 4` at the top through `z = 0` at the bottom. Tapping a cell changes that voxel’s on/off state for the selected step. One colour and one hold duration belong to each step, keeping the editor readable and the stored representation compact.

The editor is a CubeFX-owned authoring system. It does not modify the pure FastLED source submodule and it does not claim that a user sequence is a FastLED pattern.

## Packed step format

| Field | Bytes | Purpose |
|---|---:|---|
| `mask[16]` | 16 | A packed 125-bit cube mask. The final three bits are always cleared. |
| `red`, `green`, `blue` | 3 | One direct RGB colour selector result for every enabled voxel in this step. |
| `holdTicks` | 1 | Step length in 20 ms ticks, constrained to 2–250 for a visible 40 ms–5 s range. |
| **Step total** | **20** | Fixed, binary-safe layout. |

Each user pattern consumes `16 × 20 = 320` bytes. Five slots consume **1,600 bytes**. The firmware keeps these bytes in RAM for playback and persists them as five separate NVS blobs (`usrpat0` through `usrpat4`) so no single preference entry grows beyond 320 bytes.

## Browser-to-cube protocol

The embedded page deliberately uses small GET commands rather than a JSON parser or a large HTTP body. It sends each edited step as a bounded request:

```text
/api/control?userStep=1&slot=0&step=0&mask=00000000000000000000000000000000&r=255&g=0&b=80&hold=12
```

`mask` is exactly 32 hexadecimal characters. The firmware rejects malformed lengths or characters, sanitises the unused final three bits, and constrains colour and duration values. The browser uploads all sixteen staged steps for the active slot, then sends `userSave=0`, which writes exactly one 320-byte NVS blob. `userLoad=0` returns the current slot as a compact JSON object. `userPlay=0` selects that slot’s dedicated user-pattern mode and disables automatic cycling.

## Playback and controls

Five explicit CubeFX modes—**User Pattern 01** through **User Pattern 05**—play slots 0–4. Their renderer fills the 125-voxel matrix black, applies the selected step’s packed mask using that step’s RGB colour, then moves to the next step by `holdTicks × 20 ms`. The 12-pixel rear ring receives a restrained complementary hue derived from the step colour.

Short GPIO2 restarts the active user pattern at step 1. Short GPIO4 advances exactly one sequence step for inspection. Global long-press banner and next-pattern behaviour remains unchanged.

## Browser layout

The new `05 USER SEQUENCER` sidebar tab contains five slot buttons, sixteen step chips, colour and hold controls, five 5×5 `z`-layer panels, and clear/load/send/play actions. Exactly one step is edited at a time. A local browser model holds all 16 staged steps; cube upload is deliberate and visibly acknowledged rather than performed on every cell tap.

> A single step colour is intentional. It makes the first authoring tool immediate to understand, keeps each saved sequence at 320 bytes, and still allows multicolour animation by changing colour from step to step. A future advanced editor can add sparse per-voxel colours without breaking this wire format.
