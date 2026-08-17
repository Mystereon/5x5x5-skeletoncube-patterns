import { useEffect, useRef, useState } from "react";
import { Engine } from "@babylonjs/core/Engines/engine";
import { createGameScene, type GameHandle } from "@/game/scene";
import type { GameState } from "@/game/GameWorld";

const initialState: GameState = { score: 0, wave: 1, shields: 3, streak: 1, status: "INITIALISING", paused: false };

export default function GameCanvas() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const startedRef = useRef(false);
  const [state, setState] = useState(initialState);

  useEffect(() => {
    const onState = (event: Event) => setState((event as CustomEvent<GameState>).detail);
    window.addEventListener("zarch:state", onState);
    return () => window.removeEventListener("zarch:state", onState);
  }, []);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || startedRef.current) return;
    startedRef.current = true;
    const engine = new Engine(canvas, true, { preserveDrawingBuffer: true, stencil: true, adaptToDeviceRatio: true });
    let handle: GameHandle | null = null;
    let cancelled = false;
    createGameScene(engine, canvas).then((created) => {
      if (cancelled) { created.dispose(); return; }
      handle = created;
      engine.runRenderLoop(() => created.scene.render());
    });
    const onResize = () => engine.resize();
    window.addEventListener("resize", onResize);
    return () => {
      cancelled = true; window.removeEventListener("resize", onResize);
      handle?.dispose(); engine.dispose(); startedRef.current = false;
    };
  }, []);

  return (
    <main className="game-shell">
      <canvas ref={canvasRef} className="game-canvas" tabIndex={0} aria-label="Zarch Voxel Defender game viewport" />
      <header className="hud hud-top">
        <section><span className="hud-label">SCORE</span><strong>{state.score.toString().padStart(7, "0")}</strong></section>
        <section className="hud-center"><span className="eyebrow">10×10×10 // ISOMETRIC DEFENCE VOLUME</span><h1>ZARCH</h1><p>{state.status}</p></section>
        <section className="hud-right"><span className="hud-label">WAVE {state.wave.toString().padStart(2, "0")}</span><strong>{"◆".repeat(state.shields)}{"◇".repeat(3 - state.shields)}</strong></section>
      </header>
      <aside className="streak"><span>STREAK</span><strong>×{state.streak}</strong></aside>
      <footer className="hud hud-bottom">
        <div className="desktop-legend"><b>WASD / ARROWS</b> FLIGHT <b>Q / E</b> ALTITUDE <b>SPACE</b> FIRE <b>SHIFT</b> BOOST <b>R</b> RESEED</div>
        <div className="touch-controls" aria-label="Touch game controls">
          <div className="touch-dpad">
            <button data-control="forward" aria-label="Fly forward">▲</button><button data-control="left" aria-label="Fly left">◀</button>
            <button data-control="back" aria-label="Fly backward">▼</button><button data-control="right" aria-label="Fly right">▶</button>
          </div>
          <div className="touch-actions">
            <button data-control="up">UP</button><button data-control="down">DN</button>
            <button className="fire" data-control="fire">FIRE</button><button data-control="boost">BOOST</button>
          </div>
        </div>
      </footer>
      {state.paused && <div className="pause-card"><span>SIMULATION HALTED</span><strong>PAUSED</strong><small>PRESS P TO RESUME</small></div>}
      <div className="corner-mark corner-a" /><div className="corner-mark corner-b" />
    </main>
  );
}
