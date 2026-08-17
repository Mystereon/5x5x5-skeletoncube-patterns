import { ArcRotateCamera } from "@babylonjs/core/Cameras/arcRotateCamera";
import { Engine } from "@babylonjs/core/Engines/engine";
import { HemisphericLight } from "@babylonjs/core/Lights/hemisphericLight";
import { DirectionalLight } from "@babylonjs/core/Lights/directionalLight";
import { Color3, Color4 } from "@babylonjs/core/Maths/math.color";
import { Vector3 } from "@babylonjs/core/Maths/math.vector";
import { Scene } from "@babylonjs/core/scene";
import { GameWorld } from "./GameWorld";
import { InputController } from "./InputController";

export interface GameHandle { scene: Scene; dispose(): void; }

export async function createGameScene(engine: Engine, canvas: HTMLCanvasElement): Promise<GameHandle> {
  const scene = new Scene(engine);
  scene.clearColor = new Color4(0.012, 0.018, 0.014, 1);
  scene.ambientColor = new Color3(0.08, 0.12, 0.08);
  const camera = new ArcRotateCamera("isometric-camera", -Math.PI / 4, Math.PI / 3.2, 24, new Vector3(0, 4.8, 0), scene);
  camera.mode = ArcRotateCamera.ORTHOGRAPHIC_CAMERA;
  camera.attachControl(canvas, false);
  camera.inputs.clear();
  camera.minZ = 0.1; camera.maxZ = 80;
  const configureCamera = () => {
    const aspect = Math.max(0.65, engine.getRenderWidth() / Math.max(1, engine.getRenderHeight()));
    const vertical = aspect < 0.85 ? 13.2 : 8.2;
    camera.orthoTop = vertical; camera.orthoBottom = -vertical;
    camera.orthoLeft = -vertical * aspect; camera.orthoRight = vertical * aspect;
  };
  configureCamera();
  const resizeObserver = engine.onResizeObservable.add(configureCamera);

  const hemi = new HemisphericLight("hemi", new Vector3(0.1, 1, -0.2), scene);
  hemi.intensity = 0.62; hemi.diffuse = new Color3(0.58, 0.75, 0.48); hemi.groundColor = new Color3(0.02, 0.06, 0.03);
  const key = new DirectionalLight("key", new Vector3(-0.55, -1, 0.4), scene);
  key.position.set(8, 14, -10); key.intensity = 1.25; key.diffuse = new Color3(0.74, 0.95, 0.58);

  const input = new InputController();
  const world = new GameWorld(scene, input);
  let lastFrame = performance.now();
  const beforeRender = scene.onBeforeRenderObservable.add(() => {
    const now = performance.now();
    const dt = Math.min(0.04, (now - lastFrame) / 1000);
    lastFrame = now;
    world.update(dt);
  });
  return { scene, dispose() {
    scene.onBeforeRenderObservable.remove(beforeRender);
    engine.onResizeObservable.remove(resizeObserver);
    input.dispose(); world.dispose(); scene.dispose();
  } };
}
