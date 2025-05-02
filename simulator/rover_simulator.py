#!/usr/bin/env python3

import socket
import threading
import json
import math
import random
import time
import argparse

ARENA_SIZE    = 100.0
TICK_RATE_HZ  = 10
TICK_DT       = 1.0 / TICK_RATE_HZ

RANDOM_EVENTS = [
    "⚠️  Obstacle detected, rerouting...",
    "📡  Signal strength: nominal",
    "🌡️  Core temp: 42 °C — nominal",
    "🔍  Anomaly in sector 7 — investigating",
    "✅  Waypoint reached",
    "🧊  Unexpected terrain — slowing down",
    "💨  Dust storm advisory active",
    "🔋  Power draw spike — throttling",
    "🎯  Target acquired",
    "📸  Image captured at current position",
]

class Rover:
    def __init__(self):
        self.x         = ARENA_SIZE / 2
        self.y         = ARENA_SIZE / 2
        self.heading   = random.uniform(0, 360)
        self.speed     = 2.5
        self.battery   = 100.0
        self.auto_mode = True
        self.status    = "EXPLORING"
        self.uptime    = 0

    def tick(self) -> dict:
        self.uptime += 1
        self.battery = max(0.0, self.battery - 0.008)

        if self.auto_mode and self.battery > 0:
            self.heading += random.gauss(0, 1.5)

            if random.random() < 0.03:
                self.heading += random.uniform(-60, 60)

            rad    = math.radians(self.heading)
            spd    = self.speed + random.gauss(0, 0.15)
            new_x  = self.x + spd * math.cos(rad) * TICK_DT
            new_y  = self.y + spd * math.sin(rad) * TICK_DT

            margin = 4.0
            if new_x < margin or new_x > ARENA_SIZE - margin:
                self.heading = 180.0 - self.heading
                new_x = max(margin, min(ARENA_SIZE - margin, new_x))
            if new_y < margin or new_y > ARENA_SIZE - margin:
                self.heading = -self.heading
                new_y = max(margin, min(ARENA_SIZE - margin, new_y))

            self.x = new_x
            self.y = new_y

        if self.battery < 15 and self.status == "EXPLORING":
            self.status = "LOW BATTERY"

        event = ""
        if random.random() < 0.008:
            event = random.choice(RANDOM_EVENTS)

        return {
            "timestamp":   round(time.time(), 3),
            "x":           round(self.x, 3),
            "y":           round(self.y, 3),
            "heading_deg": round(self.heading % 360, 1),
            "speed":       round(max(0.0, self.speed + random.gauss(0, 0.1)), 2),
            "battery":     round(self.battery, 1),
            "uptime_sec":  self.uptime,
            "status":      self.status,
            "error_code":  0,
            "event":       event,
        }


def handle_client(conn: socket.socket, addr, rover: Rover):
    print(f"  ✅  Client connected from {addr}")
    conn.settimeout(0.01)

    try:
        while True:
            state = rover.tick()
            payload = json.dumps(state) + "\n"
            try:
                conn.sendall(payload.encode())
            except (BrokenPipeError, ConnectionResetError):
                break

            try:
                raw = conn.recv(256).decode().strip()
                for cmd in raw.splitlines():
                    cmd = cmd.strip().upper()
                    if cmd == "START":
                        rover.auto_mode = True
                        rover.status    = "EXPLORING"
                        print("  ▶  Command: START")
                    elif cmd == "STOP":
                        rover.auto_mode = False
                        rover.status    = "STOPPED"
                        print("  ⏸  Command: STOP")
                    elif cmd == "ABORT":
                        rover.auto_mode = False
                        rover.status    = "ABORTED"
                        print("  🔴  Command: ABORT")
            except socket.timeout:
                pass

            time.sleep(TICK_DT)

    except Exception as e:
        print(f"  ⚠️  Client error: {e}")
    finally:
        conn.close()
        print(f"  ❌  Client disconnected: {addr}")


def main():
    parser = argparse.ArgumentParser(description="Rover Simulator")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9000)
    args = parser.parse_args()

    rover  = Rover()
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((args.host, args.port))
    server.listen(5)

    print(f"")
    print(f"  🛰   Rover Simulator")
    print(f"  ────────────────────────────────────")
    print(f"  Listening on  {args.host}:{args.port}")
    print(f"  Tick rate     {TICK_RATE_HZ} Hz")
    print(f"  Arena size    {int(ARENA_SIZE)}×{int(ARENA_SIZE)} m")
    print(f"  Commands      START  |  STOP  |  ABORT")
    print(f"  ────────────────────────────────────")
    print(f"  Waiting for Mission Control to connect…")
    print(f"")

    try:
        while True:
            conn, addr = server.accept()
            t = threading.Thread(target=handle_client,
                                 args=(conn, addr, rover),
                                 daemon=True)
            t.start()
    except KeyboardInterrupt:
        print("\n  Shutting down simulator.")
    finally:
        server.close()


if __name__ == "__main__":
    main()
