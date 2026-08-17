import { Color3 } from "@babylonjs/core/Maths/math.color";
import { Vector3 } from "@babylonjs/core/Maths/math.vector";
import { Mesh } from "@babylonjs/core/Meshes/mesh";
import { MeshBuilder } from "@babylonjs/core/Meshes/meshBuilder";
import { TransformNode } from "@babylonjs/core/Meshes/transformNode";
import { StandardMaterial } from "@babylonjs/core/Materials/standardMaterial";
import type { Scene } from "@babylonjs/core/scene";
import type { Terrain } from "./Terrain";
import { gridToWorld } from "./constants";

export class Lander {
  readonly root: TransformNode;
  readonly targetX: number;
  readonly targetZ: number;
  private meshes: Mesh[] = [];
  private materials: StandardMaterial[] = [];
  private age = 0;
  private readonly targetY: number;
  private readonly descentSpeed: number;
  private readonly phase: number;

  constructor(scene: Scene, terrain: Terrain, targetX: number, targetZ: number, speed: number, phase: number) {
    this.targetX = targetX;
    this.targetZ = targetZ;
    this.targetY = terrain.surfaceHeightAtGrid(targetX, targetZ) + 0.82;
    this.descentSpeed = speed;
    this.phase = phase;
    this.root = new TransformNode(`lander-${targetX}-${targetZ}`, scene);
    this.root.position.set(gridToWorld(targetX) + Math.sin(phase) * 2.4, 9.25, -4.45 + Math.cos(phase) * 0.35);

    const red = new StandardMaterial("lander-red", scene);
    red.diffuseColor = new Color3(0.78, 0.08, 0.025);
    red.emissiveColor = new Color3(0.58, 0.035, 0.012);
    red.specularColor = Color3.Black();
    const amber = new StandardMaterial("lander-amber", scene);
    amber.diffuseColor = new Color3(1, 0.34, 0.035);
    amber.emissiveColor = new Color3(0.72, 0.15, 0.01);
    amber.specularColor = Color3.Black();
    this.materials.push(red, amber);

    const core = MeshBuilder.CreateCylinder("lander-core", { height: 0.58, diameterTop: 0.46, diameterBottom: 0.74, tessellation: 6 }, scene);
    core.material = red;
    core.parent = this.root;
    const ring = MeshBuilder.CreateTorus("lander-ring", { diameter: 1.05, thickness: 0.12, tessellation: 12 }, scene);
    ring.rotation.x = Math.PI / 2;
    ring.position.y = -0.08;
    ring.material = amber;
    ring.parent = this.root;
    const probe = MeshBuilder.CreateBox("lander-probe", { width: 0.16, height: 0.42, depth: 0.16 }, scene);
    probe.position.y = -0.42;
    probe.material = amber;
    probe.parent = this.root;
    this.meshes.push(core, ring, probe);
  }

  update(dt: number): boolean {
    this.age += dt;
    const target = new Vector3(gridToWorld(this.targetX), this.root.position.y, gridToWorld(this.targetZ));
    const steering = Math.min(1, dt * 0.72);
    this.root.position.x += (target.x - this.root.position.x) * steering;
    this.root.position.z += (target.z - this.root.position.z) * steering;
    this.root.position.y -= this.descentSpeed * dt;
    this.root.position.x += Math.sin(this.age * 2.8 + this.phase) * 0.0035;
    this.root.rotation.y += dt * 1.4;
    const warning = this.root.position.y < this.targetY + 1.6;
    const pulse = warning ? 1 + Math.sin(this.age * 15) * 0.13 : 1 + Math.sin(this.age * 4 + this.phase) * 0.035;
    this.root.scaling.setAll(pulse);
    return this.root.position.y <= this.targetY;
  }

  distanceSquaredTo(point: Vector3): number {
    return Vector3.DistanceSquared(this.root.position, point);
  }

  dispose(): void {
    this.meshes.splice(0).forEach((mesh) => mesh.dispose());
    this.materials.splice(0).forEach((material) => material.dispose());
    this.root.dispose();
  }
}
