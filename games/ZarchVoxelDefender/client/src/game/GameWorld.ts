import { StandardMaterial } from "@babylonjs/core/Materials/standardMaterial";
import { Color3 } from "@babylonjs/core/Maths/math.color";
import { Vector3 } from "@babylonjs/core/Maths/math.vector";
import type { Scene } from "@babylonjs/core/scene";
import { AudioManager } from "./AudioManager";
import { ImpactBurst } from "./ImpactBurst";
import type { InputController } from "./InputController";
import { Lander } from "./Lander";
import { Player } from "./Player";
import { Projectile } from "./Projectile";
import { Terrain } from "./Terrain";
import { WorldBounds } from "./WorldBounds";
import { mulberry32 } from "./random";

export interface GameState {
  score: number;
  wave: number;
  shields: number;
  streak: number;
  status: string;
  paused: boolean;
}

export class GameWorld {
  readonly terrain: Terrain;
  readonly player: Player;
  readonly bounds: WorldBounds;
  readonly state: GameState = { score: 0, wave: 1, shields: 3, streak: 1, status: "DEFEND THE SURFACE", paused: false };

  private enemies: Lander[] = [];
  private projectiles: Projectile[] = [];
  private bursts: ImpactBurst[] = [];
  private readonly laserMaterial: StandardMaterial;
  private readonly burstMaterials: StandardMaterial[];
  private readonly audio = new AudioManager();
  private random = mulberry32(0x7a2c4f11);
  private spawnRemaining = 0;
  private spawnTimer = 0;
  private waveDelay = 0;
  private fireCooldown = 0;
  private gameOverTimer = 0;
  private demoClock = 0;
  private lastPublishedAt = 0;
  private readonly demoMode = new URLSearchParams(window.location.search).has("demo");

  constructor(private readonly scene: Scene, private readonly input: InputController) {
    this.terrain = new Terrain(scene);
    this.bounds = new WorldBounds(scene);
    this.player = new Player(scene);

    this.laserMaterial = new StandardMaterial("laser-material", scene);
    this.laserMaterial.diffuseColor = new Color3(0.72, 1, 1);
    this.laserMaterial.emissiveColor = new Color3(0.22, 0.92, 1);
    this.laserMaterial.specularColor = Color3.Black();

    const orange = new StandardMaterial("burst-orange", scene);
    orange.diffuseColor = new Color3(1, 0.25, 0.025);
    orange.emissiveColor = new Color3(0.85, 0.11, 0.005);
    orange.specularColor = Color3.Black();
    const yellow = new StandardMaterial("burst-yellow", scene);
    yellow.diffuseColor = new Color3(1, 0.75, 0.08);
    yellow.emissiveColor = new Color3(0.88, 0.42, 0.015);
    yellow.specularColor = Color3.Black();
    this.burstMaterials = [orange, yellow];

    this.beginWave();
    this.publishState();
  }

  update(dt: number): void {
    if (this.input.consume("pause")) {
      this.state.paused = !this.state.paused;
      this.state.status = this.state.paused ? "SIMULATION HALTED" : "DEFENCE RESUMED";
    }
    if (this.input.consume("restart")) this.resetGame(true);

    if (!this.state.paused) {
      this.demoClock += dt;
      this.fireCooldown -= dt;

      if (this.gameOverTimer > 0) {
        this.gameOverTimer -= dt;
        this.updateBursts(dt);
        if (this.gameOverTimer <= 0) this.resetGame(true);
      } else {
        if (this.demoMode) {
          const target = new Vector3(
            Math.sin(this.demoClock * 0.63) * 3.35,
            6.2 + Math.sin(this.demoClock * 1.17) * 1.15,
            Math.cos(this.demoClock * 0.49) * 3.1,
          );
          this.player.updateAutopilot(dt, target, this.terrain);
        } else {
          this.player.update(dt, this.input, this.terrain);
        }

        const wantsFire = this.input.isHeld("fire") || (
          this.demoMode && this.enemies.length > 0 && Math.sin(this.demoClock * 3.7) > 0.72
        );
        if (wantsFire && this.fireCooldown <= 0) this.fire();

        this.updateSpawning(dt);
        this.updateEnemies(dt);
        this.updateProjectiles(dt);
        this.updateBursts(dt);
        this.updateWave(dt);
      }
    }

    const now = performance.now();
    if (now - this.lastPublishedAt > 100) this.publishState();
  }

  private beginWave(): void {
    this.spawnRemaining = Math.min(3 + this.state.wave, 9);
    this.spawnTimer = 0.55;
    this.waveDelay = 0;
    this.state.status = `${this.demoMode ? "AUTOPILOT // " : ""}WAVE ${this.state.wave.toString().padStart(2, "0")} INBOUND`;
  }

  private updateSpawning(dt: number): void {
    if (this.spawnRemaining <= 0) return;
    this.spawnTimer -= dt;
    if (this.spawnTimer > 0) return;

    const x = Math.floor(this.random() * 10);
    const z = 1 + Math.floor(this.random() * 8);
    const speed = 0.42 + this.state.wave * 0.045;
    this.enemies.push(new Lander(this.scene, this.terrain, x, z, speed, this.random() * Math.PI * 2));
    this.spawnRemaining -= 1;
    this.spawnTimer = Math.max(0.5, 1.32 - this.state.wave * 0.055);
    this.state.status = `${this.enemies.length + this.spawnRemaining} THREATS ACTIVE`;
  }

  private updateEnemies(dt: number): void {
    for (let index = this.enemies.length - 1; index >= 0; index -= 1) {
      const enemy = this.enemies[index];
      if (!enemy.update(dt)) continue;

      this.bursts.push(new ImpactBurst(this.scene, enemy.root.position.clone(), this.burstMaterials));
      this.terrain.damageAtGrid(enemy.targetX, enemy.targetZ);
      enemy.dispose();
      this.enemies.splice(index, 1);
      this.state.shields = Math.max(0, this.state.shields - 1);
      this.state.streak = 1;
      this.state.status = this.state.shields > 0 ? "SURFACE BREACH" : "DEFENCE GRID COLLAPSED";
      this.audio.damage();
      if (this.state.shields === 0) this.gameOverTimer = 2.4;
    }
  }

  private updateProjectiles(dt: number): void {
    for (let shotIndex = this.projectiles.length - 1; shotIndex >= 0; shotIndex -= 1) {
      const shot = this.projectiles[shotIndex];
      let consumed = false;

      for (let enemyIndex = this.enemies.length - 1; enemyIndex >= 0; enemyIndex -= 1) {
        const enemy = this.enemies[enemyIndex];
        if (enemy.distanceSquaredTo(shot.mesh.position) > 0.42) continue;

        this.bursts.push(new ImpactBurst(this.scene, enemy.root.position.clone(), this.burstMaterials));
        enemy.dispose();
        this.enemies.splice(enemyIndex, 1);
        shot.dispose();
        this.projectiles.splice(shotIndex, 1);
        this.state.score += 100 * this.state.streak;
        this.state.streak = Math.min(9, this.state.streak + 1);
        this.state.status = "LANDER NEUTRALISED";
        this.audio.hit();
        consumed = true;
        break;
      }

      if (!consumed && shot.update(dt)) {
        shot.dispose();
        this.projectiles.splice(shotIndex, 1);
        if (!this.demoMode) this.state.streak = 1;
      }
    }
  }

  private updateBursts(dt: number): void {
    for (let index = this.bursts.length - 1; index >= 0; index -= 1) {
      if (!this.bursts[index].update(dt)) continue;
      this.bursts[index].dispose();
      this.bursts.splice(index, 1);
    }
  }

  private updateWave(dt: number): void {
    if (this.spawnRemaining > 0 || this.enemies.length > 0 || this.gameOverTimer > 0) return;
    this.waveDelay += dt;
    this.state.status = "VOLUME CLEAR";
    if (this.waveDelay < 1.45) return;
    this.state.wave += 1;
    this.beginWave();
  }

  private fire(): void {
    const origin = this.player.muzzlePosition();
    let direction = new Vector3(0, 0, -1);
    let nearest = Number.POSITIVE_INFINITY;

    this.enemies.forEach((enemy) => {
      const distance = enemy.distanceSquaredTo(origin);
      if (distance < nearest) {
        nearest = distance;
        direction = enemy.root.position.subtract(origin);
      }
    });

    this.projectiles.push(new Projectile(this.scene, origin, direction, this.laserMaterial));
    this.fireCooldown = this.demoMode ? 0.58 : 0.22;
    this.audio.fire();
  }

  private resetGame(reseed: boolean): void {
    this.enemies.splice(0).forEach((enemy) => enemy.dispose());
    this.projectiles.splice(0).forEach((projectile) => projectile.dispose());
    this.bursts.splice(0).forEach((burst) => burst.dispose());
    if (reseed) this.terrain.reseed(this.demoMode ? 0x5a17c0de : undefined);
    this.player.root.position.set(0, 6.2, 1.5);
    this.player.velocity.setAll(0);
    this.state.score = 0;
    this.state.wave = 1;
    this.state.shields = 3;
    this.state.streak = 1;
    this.gameOverTimer = 0;
    this.demoClock = 0;
    this.random = mulberry32(this.demoMode ? 0x7a2c4f11 : (Date.now() >>> 0));
    this.beginWave();
  }

  private publishState(): void {
    this.lastPublishedAt = performance.now();
    window.dispatchEvent(new CustomEvent<GameState>("zarch:state", { detail: { ...this.state } }));
  }

  dispose(): void {
    this.enemies.splice(0).forEach((enemy) => enemy.dispose());
    this.projectiles.splice(0).forEach((projectile) => projectile.dispose());
    this.bursts.splice(0).forEach((burst) => burst.dispose());
    this.player.dispose();
    this.terrain.dispose();
    this.bounds.dispose();
    this.laserMaterial.dispose();
    this.burstMaterials.forEach((material) => material.dispose());
    this.audio.dispose();
  }
}
