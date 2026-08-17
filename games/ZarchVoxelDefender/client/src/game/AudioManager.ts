export class AudioManager {
  private context: AudioContext | null = null;
  private cleanups: Array<() => void> = [];

  constructor() {
    const unlock = () => {
      const WindowAudioContext = window.AudioContext ?? (window as typeof window & { webkitAudioContext?: typeof AudioContext }).webkitAudioContext;
      if (!WindowAudioContext) return;
      this.context ??= new WindowAudioContext();
      void this.context.resume();
    };
    window.addEventListener("keydown", unlock, { passive: true });
    window.addEventListener("pointerdown", unlock, { passive: true });
    this.cleanups.push(
      () => window.removeEventListener("keydown", unlock),
      () => window.removeEventListener("pointerdown", unlock),
    );
  }

  fire(): void { this.tone(620, 240, 0.075, "sawtooth", 0.035); }
  hit(): void { this.tone(180, 68, 0.16, "square", 0.055); }
  damage(): void { this.tone(92, 34, 0.28, "sawtooth", 0.07); }

  private tone(start: number, end: number, duration: number, type: OscillatorType, volume: number): void {
    const context = this.context;
    if (!context || context.state !== "running") return;
    const oscillator = context.createOscillator();
    const gain = context.createGain();
    oscillator.type = type;
    oscillator.frequency.setValueAtTime(start, context.currentTime);
    oscillator.frequency.exponentialRampToValueAtTime(Math.max(1, end), context.currentTime + duration);
    gain.gain.setValueAtTime(volume, context.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.0001, context.currentTime + duration);
    oscillator.connect(gain).connect(context.destination);
    oscillator.start();
    oscillator.stop(context.currentTime + duration);
  }

  dispose(): void {
    this.cleanups.splice(0).forEach((cleanup) => cleanup());
    void this.context?.close();
    this.context = null;
  }
}
