#!/usr/bin/env python3
"""Generate reproducible CampusNavi map files (same format as data/campus.txt).

Usage:  python3 tools/generate_campus.py            # writes data/campus_100.txt and data/campus_300.txt

Structure of a generated campus (a grid of buildings):
  * Each building has several floors. Each floor has a hallway (chain of corridor nodes),
    rooms hanging off the hallway (classrooms/labs/offices), one staircase and one elevator.
  * Stairs/elevators connect the same spot on consecutive floors (elevator is slower).
  * Outside: a "Quad" junction grid and one "Gate" per building (ground-floor entrance).
  * Skybridges on floor 2 join neighbouring buildings (east elevator <-> west staircase).
RULE enforced here: every edge weight >= 3D straight-line distance of its endpoints
(weights are rounded UP), so the A* heuristic stays admissible and consistent.
"""
import math, random, sys
from collections import deque

FLOOR_H = 4.0   # must match FLOOR_HEIGHT_M in include/Location.h
SPACING = 70    # metres between building origins
HALL_STEP = 8   # metres between hallway nodes


def generate(rows, cols, floors_of, k, target, seed):
    rng = random.Random(seed)
    nodes, order, edges, seen = {}, [], [], set()

    def add_node(nid, bld, floor, typ, x, y):
        assert nid not in nodes, nid
        nodes[nid] = (bld, floor, typ, x, y)
        order.append(nid)

    def dist(a, b):
        _, fa, _, xa, ya = nodes[a]
        _, fb, _, xb, yb = nodes[b]
        return math.sqrt((xa - xb) ** 2 + (ya - yb) ** 2 + ((fa - fb) * FLOOR_H) ** 2)

    def add_edge(a, b, lo=1.0, hi=1.12, mult=1.0, extra=0.0):
        key = tuple(sorted((a, b)))
        if key in seen:
            return
        seen.add(key)
        d = dist(a, b)
        w = math.ceil((d * rng.uniform(lo, hi) * mult + extra) * 10) / 10
        w = max(w, math.ceil(d * 10) / 10)          # never below straight-line distance
        edges.append((a, b, w))

    letters = [chr(ord('A') + i) for i in range(rows * cols)]
    fixed = sum(floors_of[i] * (k + 2) for i in range(rows * cols)) + rows * cols + (rows + 1) * (cols + 1)
    rooms_total = target - fixed
    assert rooms_total >= 0, f"target too small; fixed nodes = {fixed}"

    # spread rooms over every (building, floor), ground floors first
    slots = sorted(((f, i) for i in range(rows * cols) for f in range(1, floors_of[i] + 1)))
    room_count = {s: rooms_total // len(slots) + (1 if n < rooms_total % len(slots) else 0)
                  for n, s in enumerate(slots)}

    # outdoor junction grid
    for i in range(rows + 1):
        for j in range(cols + 1):
            add_node(f"Quad-{i}-{j}", "Z", 1, "Other", j * SPACING, i * SPACING)

    for idx in range(rows * cols):
        b, r, c = letters[idx], idx // cols, idx % cols
        ox, oy = c * SPACING + 20, r * SPACING + 35
        add_node(f"Gate-{b}", b, 1, "Other", ox - 12, oy)
        for f in range(1, floors_of[idx] + 1):
            halls = []
            for i in range(k):
                nid = f"{b}-Hall-{f}-{i + 1}"
                add_node(nid, b, f, "Corridor", ox + i * HALL_STEP, oy)
                halls.append(nid)
            add_node(f"{b}-Stair-{f}", b, f, "Staircase", ox - 4, oy)
            add_node(f"{b}-Elev-{f}", b, f, "Elevator", ox + (k - 1) * HALL_STEP + 4, oy)
            for i in range(k - 1):
                add_edge(halls[i], halls[i + 1], 1.0, 1.05)
            add_edge(f"{b}-Stair-{f}", halls[0], 1.0, 1.05)
            add_edge(f"{b}-Elev-{f}", halls[-1], 1.0, 1.05)
            rooms = []
            for j in range(room_count[(f, idx)]):
                ci, layer = j % k, j // k
                side = 1 if layer % 2 == 0 else -1
                y = oy + side * (6 + 5 * (layer // 2))
                typ = "Lab" if j % 4 == 3 else ("Office" if j % 5 == 4 else "Classroom")
                rid = f"{b}{f}{j + 1:02d}"
                add_node(rid, b, f, typ, ox + ci * HALL_STEP, y)
                add_edge(rid, halls[ci])
                rooms.append((rid, ci, layer))
            for (ra, ca, la), (rb, cb, lb) in zip(rooms, rooms[1:]):  # a few room-to-room shortcuts
                if la == lb and cb == ca + 1 and rng.random() < 0.4:
                    add_edge(ra, rb)
        add_edge(f"Gate-{b}", f"{b}-Hall-1-1")
        for di in (0, 1):
            for dj in (0, 1):
                add_edge(f"Gate-{b}", f"Quad-{r + di}-{c + dj}", 1.0, 1.1)
        for f in range(1, floors_of[idx]):                       # floor transitions
            add_edge(f"{b}-Stair-{f}", f"{b}-Stair-{f + 1}", 1.0, 1.0, mult=1.5)
            add_edge(f"{b}-Elev-{f}", f"{b}-Elev-{f + 1}", 1.0, 1.0, extra=6.0)
    for i in range(rows + 1):
        for j in range(cols + 1):
            if j < cols: add_edge(f"Quad-{i}-{j}", f"Quad-{i}-{j + 1}", 1.0, 1.08)
            if i < rows: add_edge(f"Quad-{i}-{j}", f"Quad-{i + 1}-{j}", 1.0, 1.08)
    for idx in range(rows * cols):                               # skybridges on floor 2
        r, c = idx // cols, idx % cols
        if c + 1 < cols and floors_of[idx] >= 2 and floors_of[idx + 1] >= 2:
            add_edge(f"{letters[idx]}-Elev-2", f"{letters[idx + 1]}-Stair-2", 1.0, 1.05)

    assert len(nodes) == target, (len(nodes), target)
    return nodes, order, edges


def validate(nodes, edges):
    adj = {n: [] for n in nodes}
    for a, b, w in edges:
        adj[a].append(b); adj[b].append(a)
        _, fa, _, xa, ya = nodes[a]; _, fb, _, xb, yb = nodes[b]
        d = math.sqrt((xa - xb) ** 2 + (ya - yb) ** 2 + ((fa - fb) * FLOOR_H) ** 2)
        assert w + 1e-9 >= d, (a, b, w, d)
    start = next(iter(nodes)); seen = {start}; q = deque([start])
    while q:
        for m in adj[q.popleft()]:
            if m not in seen: seen.add(m); q.append(m)
    assert len(seen) == len(nodes), "map is not fully connected"


def write(path, title, rows, cols, floors_of, k, target, seed):
    nodes, order, edges = generate(rows, cols, floors_of, k, target, seed)
    validate(nodes, edges)
    with open(path, "w") as out:
        out.write(f"# {title}\n# GENERATED by tools/generate_campus.py (seed {seed}). {len(nodes)} locations, "
                  f"{len(edges)} connections. Coordinates in metres, floor height {FLOOR_H:g} m.\n"
                  "# Every edge weight >= straight-line 3D distance (A* heuristic stays valid).\n\n[LOCATIONS]\n"
                  "# id, building, floor, type, x, y\n")
        for nid in order:
            b, f, t, x, y = nodes[nid]
            out.write(f"{nid}, {b}, {f}, {t}, {x}, {y}\n")
        out.write("\n[EDGES]\n# idA, idB, weight\n")
        for a, b, w in edges:
            out.write(f"{a}, {b}, {w:.1f}\n")
    print(f"{path}: {len(nodes)} nodes, {len(edges)} edges")


if __name__ == "__main__":
    write("data/campus_100.txt", "CampusNavi MEDIUM campus: 4 buildings (2x2), 2-3 floors",
          2, 2, [3, 3, 2, 3], 3, 100, 100)
    write("data/campus_300.txt", "CampusNavi LARGE campus: 9 buildings (3x3), 3-4 floors",
          3, 3, [3, 3, 3, 3, 4, 3, 3, 3, 3], 4, 300, 300)
