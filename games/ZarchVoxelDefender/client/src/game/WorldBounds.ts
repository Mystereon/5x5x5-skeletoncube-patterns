import { Color3 } from "@babylonjs/core/Maths/math.color";
import { Vector3 } from "@babylonjs/core/Maths/math.vector";
import { Mesh } from "@babylonjs/core/Meshes/mesh";
import { MeshBuilder } from "@babylonjs/core/Meshes/meshBuilder";
import type { Scene } from "@babylonjs/core/scene";
import { HALF_VOLUME } from "./constants";

export class WorldBounds {
  private meshes: Mesh[] = [];

  constructor(scene: Scene) {
    const min = -HALF_VOLUME;
    const max = HALF_VOLUME;
    const corners = [
      new Vector3(min, 0, min), new Vector3(max, 0, min), new Vector3(max, 0, max), new Vector3(min, 0, max),
      new Vector3(min, 10, min), new Vector3(max, 10, min), new Vector3(max, 10, max), new Vector3(min, 10, max),
    ];
    const edges = [[0,1],[1,2],[2,3],[3,0],[4,5],[5,6],[6,7],[7,4],[0,4],[1,5],[2,6],[3,7]];
    const boundary = MeshBuilder.CreateLineSystem("volume-boundary", { lines: edges.map(([a,b]) => [corners[a], corners[b]]) }, scene);
    boundary.color = new Color3(0.58, 1, 0.04);
    boundary.alpha = 0.56;
    boundary.isPickable = false;
    this.meshes.push(boundary);

    const ticks: Vector3[][] = [];
    for (let i = 1; i < 10; i += 1) {
      const p = min + i;
      ticks.push([new Vector3(p, 0, min), new Vector3(p, 0.18, min)]);
      ticks.push([new Vector3(min, 0, p), new Vector3(min, 0.18, p)]);
      ticks.push([new Vector3(min, i, min), new Vector3(min + 0.18, i, min)]);
    }
    const tickMesh = MeshBuilder.CreateLineSystem("volume-ticks", { lines: ticks }, scene);
    tickMesh.color = new Color3(0.42, 0.72, 0.08);
    tickMesh.alpha = 0.72;
    tickMesh.isPickable = false;
    this.meshes.push(tickMesh);
  }

  dispose(): void { this.meshes.splice(0).forEach((mesh) => mesh.dispose()); }
}
