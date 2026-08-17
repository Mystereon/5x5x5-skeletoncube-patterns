export type InputAction = "left" | "right" | "forward" | "back" | "up" | "down" | "fire" | "boost" | "pause" | "restart";

const keyMap: Record<string, InputAction> = {
  KeyA: "left", ArrowLeft: "left", KeyD: "right", ArrowRight: "right",
  KeyW: "forward", ArrowUp: "forward", KeyS: "back", ArrowDown: "back",
  KeyE: "up", KeyQ: "down", Space: "fire", ShiftLeft: "boost",
  ShiftRight: "boost", KeyP: "pause", KeyR: "restart",
};

export class InputController {
  private held = new Set<InputAction>();
  private pressed = new Set<InputAction>();
  private cleanups: Array<() => void> = [];

  constructor() {
    const onKeyDown = (event: KeyboardEvent) => {
      const action = keyMap[event.code];
      if (!action) return;
      if (!this.held.has(action)) this.pressed.add(action);
      this.held.add(action);
      if (event.code.startsWith("Arrow") || event.code === "Space") event.preventDefault();
    };
    const onKeyUp = (event: KeyboardEvent) => {
      const action = keyMap[event.code];
      if (action) this.held.delete(action);
    };
    const onBlur = () => this.held.clear();
    window.addEventListener("keydown", onKeyDown);
    window.addEventListener("keyup", onKeyUp);
    window.addEventListener("blur", onBlur);
    this.cleanups.push(
      () => window.removeEventListener("keydown", onKeyDown),
      () => window.removeEventListener("keyup", onKeyUp),
      () => window.removeEventListener("blur", onBlur),
    );

    document.querySelectorAll<HTMLElement>("[data-control]").forEach((element) => {
      const action = element.dataset.control as InputAction;
      const down = (event: PointerEvent) => {
        event.preventDefault();
        if (!this.held.has(action)) this.pressed.add(action);
        this.held.add(action);
        element.setPointerCapture?.(event.pointerId);
      };
      const up = (event: PointerEvent) => {
        event.preventDefault();
        this.held.delete(action);
      };
      element.addEventListener("pointerdown", down);
      element.addEventListener("pointerup", up);
      element.addEventListener("pointercancel", up);
      this.cleanups.push(
        () => element.removeEventListener("pointerdown", down),
        () => element.removeEventListener("pointerup", up),
        () => element.removeEventListener("pointercancel", up),
      );
    });
  }

  isHeld(action: InputAction): boolean { return this.held.has(action); }
  consume(action: InputAction): boolean {
    const value = this.pressed.has(action);
    this.pressed.delete(action);
    return value;
  }
  dispose(): void {
    this.cleanups.splice(0).forEach((cleanup) => cleanup());
    this.held.clear();
    this.pressed.clear();
  }
}
