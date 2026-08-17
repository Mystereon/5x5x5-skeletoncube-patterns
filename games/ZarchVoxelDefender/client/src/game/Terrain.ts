import { Color3 } from "@babylonjs/core/Maths/math.color";
import { Vector3 } from "@babylonjs/core/Maths/math.vector";
import { Mesh } from "@babylonjs/core/Meshes/mesh";
import { LinesMesh } from "@babylonjs/core/Meshes/linesMesh";
import { MeshBuilder } from "@babylonjs/core/Meshes/meshBuilder";
import { VertexData } from "@babylonjs/core/Meshes/mesh.vertexData";
import { StandardMaterial } from "@babylonjs/core/Materials/standardMaterial";
import { Texture } from "@babylonjs/core/Materials/Textures/texture";
import type { Scene } from "@babylonjs/core/scene";
import { VOXELS, gridToWorld } from "./constants";
import { mulberry32 } from "./random";

const HEIGHT_SCALE = 0.64;
const BASE_HEIGHT = 0.08;

export class Terrain {
  readonly heights = new Uint8Array(VOXELS * VOXELS);
  private surface: Mesh | null = null;
  private wireframe: LinesMesh | null = null;
  private underlay: Mesh;
  private readonly fillMaterial: StandardMaterial;
  private readonly underlayMaterial: StandardMaterial;
  private readonly wireColor = new Color3(0.58, 1, 0.05);
  private readonly detailTexture: Texture;
  private seed = 0x5a17c0de;

  constructor(private readonly scene: Scene) {
    this.detailTexture = new Texture("/assets/zarch-terrain-texture.png", scene, true, false, Texture.TRILINEAR_SAMPLINGMODE);
    this.detailTexture.uScale = 3.8;
    this.detailTexture.vScale = 3.8;
    this.detailTexture.level = 0.22;
    this.underlayMaterial = new StandardMaterial("terrain-detail-underlay", scene);
    this.underlayMaterial.diffuseTexture = this.detailTexture;
    this.underlayMaterial.diffuseColor = new Color3(0.08, 0.3, 0.025);
    this.underlayMaterial.emissiveColor = new Color3(0.01, 0.1, 0.008);
    this.underlayMaterial.specularColor = Color3.Black();
    this.underlayMaterial.alpha = 0.62;
    this.underlay = MeshBuilder.CreateGround("terrain-detail-underlay", { width: 9.85, height: 9.85 }, scene);
    this.underlay.position.y = -0.035;
    this.underlay.material = this.underlayMaterial;
    this.underlay.isPickable = false;
    this.fillMaterial = new StandardMaterial("low-poly-terrain-fill", scene);
    this.fillMaterial.diffuseColor = new Color3(0.035, 0.24, 0.018);
    this.fillMaterial.emissiveColor = new Color3(0.035, 0.33, 0.022);
    this.fillMaterial.specularColor = Color3.Black();
    this.fillMaterial.backFaceCulling = false;
    this.fillMaterial.disableLighting = true;
    this.reseed(this.seed);
  }

  reseed(seed = (this.seed + 0x9e3779b9) >>> 0): void {
    this.seed = seed >>> 0;
    const random = mulberry32(this.seed);
    const raw = new Float32Array(VOXELS * VOXELS);
    for (let z = 0; z < VOXELS; z += 1) {
      for (let x = 0; x < VOXELS; x += 1) {
        const ridge = Math.sin((x + 1.7) * 0.54) * 0.7 + Math.cos((z - 1.2) * 0.48) * 0.65;
        raw[z * VOXELS + x] = 1.35 + ridge + (random() - 0.5) * 1.45;
      }
    }
    for (let z = 0; z < VOXELS; z += 1) {
      for (let x = 0; x < VOXELS; x += 1) {
        let sum = 0;
        let count = 0;
        for (let dz = -1; dz <= 1; dz += 1) {
          for (let dx = -1; dx <= 1; dx += 1) {
            const nx = x + dx;
            const nz = z + dz;
            if (nx >= 0 && nx < VOXELS && nz >= 0 && nz < VOXELS) {
              sum += raw[nz * VOXELS + nx];
              count += 1;
            }
          }
        }
        this.heights[z * VOXELS + x] = Math.max(0, Math.min(3, Math.round(sum / count)));
      }
    }
    this.rebuildMesh();
  }

  heightAtGrid(x: number, z: number): number {
    const cx = Math.max(0, Math.min(VOXELS - 1, Math.floor(x)));
    const cz = Math.max(0, Math.min(VOXELS - 1, Math.floor(z)));
    return this.heights[cz * VOXELS + cx];
  }

  surfaceHeightAtGrid(x: number, z: number): number {
    return BASE_HEIGHT + this.heightAtGrid(x, z) * HEIGHT_SCALE;
  }

  damageAtGrid(x: number, z: number): void {
    const cx = Math.max(0, Math.min(VOXELS - 1, Math.floor(x)));
    const cz = Math.max(0, Math.min(VOXELS - 1, Math.floor(z)));
    const index = cz * VOXELS + cx;
    if (this.heights[index] > 0) this.heights[index] -= 1;
    this.rebuildMesh();
  }

  private rebuildMesh(): void {
    this.surface?.dispose();
    this.wireframe?.dispose();

    const positions: number[] = [];
    const normals: number[] = [];
    const uvs: number[] = [];
    const indices: number[] = [];
    for (let z = 0; z < VOXELS; z += 1) {
      for (let x = 0; x < VOXELS; x += 1) {
        positions.push(gridToWorld(x), this.surfaceHeightAtGrid(x, z), gridToWorld(z));
        uvs.push(x / (VOXELS - 1), z / (VOXELS - 1));
      }
    }
    for (let z = 0; z < VOXELS - 1; z += 1) {
      for (let x = 0; x < VOXELS - 1; x += 1) {
        const a = z * VOXELS + x;
        const b = a + 1;
        const c = a + VOXELS;
        const d = c + 1;
        if ((x + z) % 2 === 0) indices.push(a, c, d, a, d, b);
        else indices.push(a, c, b, b, c, d);
      }
    }
    VertexData.ComputeNormals(positions, indices, normals);
    this.surface = new Mesh("low-poly-terrain", this.scene);
    const data = new VertexData();
    data.positions = positions;
    data.indices = indices;
    data.normals = normals;
    data.uvs = uvs;
    data.applyToMesh(this.surface);
    this.surface.material = this.fillMaterial;
    this.surface.isPickable = false;
    this.surface.renderingGroupId = 1;

    const lines: Vector3[][] = [];
    const point = (x: number, z: number) => ({ x: gridToWorld(x), y: this.surfaceHeightAtGrid(x, z), z: gridToWorld(z) });
    const vector = (x: number, z: number) => {
      const p = point(x, z);
      return new Vector3(p.x, p.y + 0.018, p.z);
    };
    for (let z = 0; z < VOXELS; z += 1) {
      const row = [];
      for (let x = 0; x < VOXELS; x += 1) row.push(vector(x, z));
      lines.push(row);
    }
    for (let x = 0; x < VOXELS; x += 1) {
      const column = [];
      for (let z = 0; z < VOXELS; z += 1) column.push(vector(x, z));
      lines.push(column);
    }
    for (let z = 0; z < VOXELS - 1; z += 1) {
      for (let x = 0; x < VOXELS - 1; x += 1) {
        lines.push((x + z) % 2 === 0 ? [vector(x, z), vector(x + 1, z + 1)] : [vector(x + 1, z), vector(x, z + 1)]);
      }
    }
    this.wireframe = MeshBuilder.CreateLineSystem("terrain-wireframe", { lines }, this.scene);
    this.wireframe.color = this.wireColor;
    this.wireframe.alpha = 0.78;
    this.wireframe.isPickable = false;
    this.wireframe.renderingGroupId = 2;
  }

  dispose(): void {
    this.surface?.dispose();
    this.wireframe?.dispose();
    this.underlay.dispose();
    this.fillMaterial.dispose();
    this.underlayMaterial.dispose();
    this.detailTexture.dispose();
  }
}
