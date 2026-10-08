# Deviations from the original PRD (and why)

Save your original PRD as `docs/PRD.md`. These are the places the implementation differs:

1. **Example distance.** The PRD shows `A101 -> A102 -> A-Stair-1 -> B201 -> B202 = 34m`, but its own
   edges sum to 30m. The PRD's numbers were illustrative; the real sample map gives **36m**
   (route in `data/campus.txt`). The program computes it; nothing is hand-written.
2. **Heuristic across floors.** Plain (x, y) distance ignores floors. We use 3D distance with
   `FLOOR_HEIGHT_M = 4`. A\* is only provably optimal if every edge weight >= straight-line distance
   of its endpoints, so the loader checks this and prints a warning if a map breaks the rule.
3. **Data file** has `[LOCATIONS]` and `[EDGES]` sections, because the PRD's edges-only format
   cannot carry building/floor/type/coordinates.
4. **Explicit floor-transition edges** (`A-Stair-1 <-> A-Stair-2`) are used, one of the two options the PRD allows.
5. **Extras:** `PathFinder` abstract base class, case-insensitive location lookup, and 2000-run
   averaged timing for the comparison (a single run is too short to measure).
6. **Tests** use a tiny built-in harness instead of GoogleTest, so there is nothing to install.
