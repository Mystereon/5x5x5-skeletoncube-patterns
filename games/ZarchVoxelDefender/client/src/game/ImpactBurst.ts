import { Vector3 } from "@babylonjs/core/Maths/math.vector";
import { Mesh } from "@babylonjs/core/Meshes/mesh";
import { MeshBuilder } from "@babylonjs/core/Meshes/meshBuilder";
import type { Material } from "@babylonjs/core/Materials/material";
import type { Scene } from "@babylonjs/core/scene";

interface BurstVoxel { mesh: Mesh; velocity: Vector3; }

const directions = [
  new Vector3(1, 0.7, 0), new Vector3(-1, 0.8, 0), new Vector3(0, 1.1, 0.4),
  new Vector3(0, 0.55, -1), new Vector3(0.75, 0.45, 0.75), new Vector3(-0.7, 0.5, -0.72),
  new Vector3(0.28, 1.35, -0.3), new Vector3(-0.3, 1.2, 0.28),
];

export class ImpactBurst {
  private bits: BurstVoxel[] = [];
  private age = 0;

  constructor(scene: Scene, origin: Vector3, materials: Material[]) {
    directions.forEach((direction, index) => {
      const mesh = MeshBuilder.CreateBox(`burst-${index}`, { size: index < 4 ? 0.19 : 0.13 }, scene);
      mesh.position.copyFrom(origin);
      mesh.material = materials[index % materials.length];
      mesh.isPickable = false;
      this.bits.push({ mesh, velocity: direction.normalizeToNew().scale(2.4 + (index % 3) * 0.55) });
    });
  }

  update(dt: number): boolean {
    this.age += dt;
    const scale = Math.max(0.04, 1 - this.age / 0.68);
    this.bits.forEach((bit) => {
      bit.velocity.y -= 2.9 * dt;
      bit.mesh.position.addInPlace(bit.velocity.scale(dt));
      bit.mesh.scaling.setAll(scale);
      bit.mesh.rotation.x += dt * 6;
      bit.mesh.rotation.y += dt * 8;
    });
    return this.age >= 0.68;
  }

  dispose(): void { this.bits.splice(0).forEach(({ mesh }) => mesh.dispose()); }
}
