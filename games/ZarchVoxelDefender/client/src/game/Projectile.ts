import { Vector3 } from "@babylonjs/core/Maths/math.vector";
import { Mesh } from "@babylonjs/core/Meshes/mesh";
import { MeshBuilder } from "@babylonjs/core/Meshes/meshBuilder";
import type { Material } from "@babylonjs/core/Materials/material";
import type { Scene } from "@babylonjs/core/scene";

export class Projectile {
  readonly mesh: Mesh;
  private readonly velocity: Vector3;
  private age = 0;

  constructor(scene: Scene, origin: Vector3, direction: Vector3, material: Material) {
    this.mesh = MeshBuilder.CreateBox("laser", { width: 0.09, height: 0.09, depth: 0.74 }, scene);
    this.mesh.position.copyFrom(origin);
    this.mesh.material = material;
    this.mesh.isPickable = false;
    const normal = direction.normalizeToNew();
    this.velocity = normal.scale(12.5);
    this.mesh.lookAt(origin.add(normal));
  }

  update(dt: number): boolean {
    this.age += dt;
    this.mesh.position.addInPlace(this.velocity.scale(dt));
    const p = this.mesh.position;
    return this.age > 1.45 || Math.abs(p.x) > 6 || p.y < -0.5 || p.y > 11 || Math.abs(p.z) > 6;
  }

  dispose(): void { this.mesh.dispose(); }
}
