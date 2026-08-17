import { Color3 } from "@babylonjs/core/Maths/math.color";
import { Vector3 } from "@babylonjs/core/Maths/math.vector";
import { LinesMesh } from "@babylonjs/core/Meshes/linesMesh";
import { Mesh } from "@babylonjs/core/Meshes/mesh";
import { MeshBuilder } from "@babylonjs/core/Meshes/meshBuilder";
import { TransformNode } from "@babylonjs/core/Meshes/transformNode";
import { VertexData } from "@babylonjs/core/Meshes/mesh.vertexData";
import { StandardMaterial } from "@babylonjs/core/Materials/standardMaterial";
import type { Scene } from "@babylonjs/core/scene";
import type { InputController } from "./InputController";
import type { Terrain } from "./Terrain";
import { WORLD_MAX, WORLD_MIN, worldToGrid } from "./constants";

export class Player {
  readonly root: TransformNode;
  readonly velocity = Vector3.Zero();
  private hull: Mesh;
  private outline: LinesMesh;
  private hullMaterial: StandardMaterial;

  constructor(scene: Scene) {
    this.root = new TransformNode("player", scene);
    this.root.position.set(0, 6.2, 1.5);

    this.hullMaterial = new StandardMaterial("player-tetra-hull", scene);
    this.hullMaterial.diffuseColor = new Color3(0.30, 1, 0.05);
    this.hullMaterial.emissiveColor = new Color3(0.25, 0.86, 0.035);
    this.hullMaterial.specularColor = Color3.Black();
    this.hullMaterial.disableLighting = true;
    this.hullMaterial.backFaceCulling = false;

    const points = [
      new Vector3(0, 0.02, -0.95),
      new Vector3(-0.66, -0.12, 0.50),
      new Vector3(0.66, -0.12, 0.50),
      new Vector3(0, 0.48, 0.42),
    ];
    this.hull = new Mesh("player-swept-tetrahedron", scene);
    const data = new VertexData();
    data.positions = points.flatMap((point) => [point.x, point.y, point.z]);
    data.indices = [0, 1, 2, 0, 3, 1, 0, 2, 3, 1, 3, 2];
    data.normals = [];
    VertexData.ComputeNormals(data.positions, data.indices, data.normals);
    data.applyToMesh(this.hull);
    this.hull.material = this.hullMaterial;
    this.hull.parent = this.root;

    this.outline = MeshBuilder.CreateLineSystem("player-vector-outline", {
      lines: [
        [points[0], points[1], points[2], points[0]],
        [points[0], points[3], points[1]],
        [points[3], points[2]],
      ],
    }, scene);
    this.outline.color = new Color3(0.05, 0.96, 1);
    this.outline.alpha = 0.96;
    this.outline.parent = this.root;
  }

  update(dt: number, input: InputController, terrain: Terrain): void {
    const boost = input.isHeld("boost") ? 1.7 : 1;
    const desired = new Vector3(
      (input.isHeld("right") ? 1 : 0) - (input.isHeld("left") ? 1 : 0),
      (input.isHeld("up") ? 1 : 0) - (input.isHeld("down") ? 1 : 0),
      (input.isHeld("back") ? 1 : 0) - (input.isHeld("forward") ? 1 : 0),
    );
    this.integrate(dt, desired, boost, terrain);
  }

  updateAutopilot(dt: number, target: Vector3, terrain: Terrain): void {
    const desired = target.subtract(this.root.position);
    if (desired.lengthSquared() > 0.01) desired.normalize();
    this.integrate(dt, desired, 1.15, terrain);
  }

  muzzlePosition(): Vector3 {
    return this.root.position.add(new Vector3(0, 0.02, -0.98));
  }

  private integrate(dt: number, desired: Vector3, boost: number, terrain: Terrain): void {
    if (desired.lengthSquared() > 1) desired.normalize();
    this.velocity.addInPlace(desired.scale(8.5 * boost * dt));
    this.velocity.scaleInPlace(Math.pow(0.045, dt));
    const maxSpeed = 4.8 * boost;
    if (this.velocity.lengthSquared() > maxSpeed * maxSpeed) this.velocity.normalize().scaleInPlace(maxSpeed);
    this.root.position.addInPlace(this.velocity.scale(dt));
    this.root.position.x = Math.max(WORLD_MIN + 0.55, Math.min(WORLD_MAX - 0.55, this.root.position.x));
    this.root.position.z = Math.max(WORLD_MIN + 0.55, Math.min(WORLD_MAX - 0.55, this.root.position.z));
    const floor = terrain.surfaceHeightAtGrid(worldToGrid(this.root.position.x), worldToGrid(this.root.position.z)) + 1.15;
    this.root.position.y = Math.max(floor, Math.min(9.25, this.root.position.y));
    this.root.rotation.z = -this.velocity.x * 0.055;
    this.root.rotation.x = this.velocity.z * 0.045;
  }

  dispose(): void {
    this.outline.dispose();
    this.hull.dispose();
    this.hullMaterial.dispose();
    this.root.dispose();
  }
}
