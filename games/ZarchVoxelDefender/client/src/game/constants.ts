export const VOXELS = 10;
export const HALF_VOLUME = VOXELS / 2;
export const WORLD_MIN = -HALF_VOLUME;
export const WORLD_MAX = HALF_VOLUME;

export function gridToWorld(cell: number): number {
  return cell - HALF_VOLUME + 0.5;
}

export function worldToGrid(value: number): number {
  return Math.max(0, Math.min(VOXELS - 1, Math.floor(value + HALF_VOLUME)));
}
