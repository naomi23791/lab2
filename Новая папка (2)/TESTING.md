# Quick Testing Guide

## Running the Program

```bash
./main.exe
```

---

## Test Case 1: Basic Setup (5 minutes)

### Step 1: Create Companies
- Menu → Option 2 (Add a company)
- Enter: Name: "Company A", Workshops: 10, In Operation: 8, Classes: "Premium"
- Repeat with "Company B" and "Company C"

### Step 2: Create Pipes
- Menu → Option 1 (Add a pipe)
- Create 3 pipes:
  1. "Main Pipeline", Length: 100m, Diameter: 1000mm, No repair
  2. "Secondary Line", Length: 150m, Diameter: 700mm, No repair
  3. "Emergency Backup", Length: 200m, Diameter: 500mm, No repair

### Expected Result
✅ System accepts all data without errors

---

## Test Case 2: Network Creation (5 minutes)

### Step 1: Create Network
- Menu → Option 8 (Network management)
- Option 1 (Add connection)
- Source KC: 1, Destination KC: 2
- Diameter: 1000mm
- Expected: Connection created successfully

### Step 2: Add More Connections
- Create: KC 2 → KC 3 (Diameter: 700mm)
- Create: KC 1 → KC 3 (Diameter: 500mm)

### Expected Result
✅ Three connections visible in "Display connections"

---

## Test Case 3: Maximum Flow Analysis (5 minutes)

### Step 1: Analyze Flow
- Menu → Option 8 → Option 4 (Analyze flow)
- Source: 1, Sink: 3
- Example expected output:
  - Max flow: 500 m³/h (limited by bottleneck pipe)

### Step 2: Different Path
- Try Source: 1, Sink: 2
- Expected max flow: Limited by 1000mm pipe capacity (300 m³/h) or 700mm (130 m³/h) depending on path

### Expected Result
✅ Realistic flow values based on pipe capacities

---

## Test Case 4: Shortest Path Analysis (5 minutes)

### Step 1: View Shortest Path
- In flow analysis, observe "Shortest Path Analysis"
- Should show:
  - Complete path with KC names
  - Each pipe in the path with ID, name, length, capacity
  - Total distance
  - Path capacity (bottleneck)
  - Number of hops

### Example Expected Output
```
--- Shortest Path Analysis ---
Shortest path (by distance):
  [0] KC 1 (Company A)
        --[Pipe ID: 1, Name: Main Pipeline, Length: 100m, Capacity: 300 m³/h]--> 
  [1] KC 2 (Company B)
        --[Pipe ID: 2, Name: Secondary Line, Length: 150m, Capacity: 130 m³/h]--> 
  [2] KC 3 (Company C)

Path Statistics:
  Total distance: 250 m
  Path capacity (bottleneck): 130 m³/h
  Number of hops: 2
```

### Expected Result
✅ Path is logically correct, distances add up, bottleneck is accurate

---

## Test Case 5: Repair Status (5 minutes)

### Step 1: Create Pipe Under Repair
- Add a new pipe "Broken Line"
- Length: 100m, Diameter: 1000mm
- Repair status: YES

### Step 2: Verify Display
- Manage Pipes → Display all
- Verify "Broken Line" shows:
  - Capacity: 0 m³/h
  - Repair status: YES (under repair)
  - Weight: INFINITY (pipe unusable)

### Step 3: Try in Pathfinding
- Create network with this pipe: KC 2 → KC 3
- Try flow analysis: KC 1 → KC 3
- Expected: System should find path avoiding the broken pipe OR no path if it's the only option

### Expected Result
✅ Broken pipe excluded from pathfinding, capacity correctly shows 0

---

## Test Case 6: Smart Creation Assistant (5 minutes)

### Step 1: Fresh Start
- Delete all data from memory (restart program)
- Menu → Option 8 (Network management)
- Option 1 (Add connection)

### Expected Behavior
- "No companies exist" → Offer to create
- If yes → Redirects to company creation
- After creating 1 company → "Need at least 2 companies" → Offer to create another
- After 2 companies → "No pipes exist" → Offer to create
- If yes → Redirects to pipe creation

### Expected Result
✅ Smooth workflow with helpful prompts, no dead ends

---

## Test Case 7: Display Information (3 minutes)

### Step 1: View Pipe Details
- Manage Pipes → Display all
- Verify each pipe shows:
  - Name, Length, Diameter, Repair status, Capacity, Weight

### Step 2: View Network Connections
- Network Management → Display connections
- Verify shows: "KC X -> KC Y (Pipe ID: Z)"

### Step 3: View Company Details
- Manage Companies → Display all
- Verify shows all company information

### Expected Result
✅ All displays are formatted clearly and informative

---

## Test Case 8: Edge Cases (5 minutes)

### Scenario 1: No Path Exists
- Create: KC 1 → KC 2
- Analyze flow: KC 2 → KC 3
- Expected: "No path found from KC 2 to KC 3"

### Scenario 2: Same Source and Sink
- In flow analysis, enter same KC for source and sink
- Expected: Graceful handling with "0 flow" or similar message

### Scenario 3: All Pipes Under Repair
- Set all pipes to repair status
- Try to create network
- Expected: Should allow but show inability to use

### Expected Result
✅ All edge cases handled gracefully with informative messages

---

## Validation Checklist

After testing, verify:

- [ ] All data input validations work
- [ ] Maximum flow values are realistic (based on smallest capacity)
- [ ] Shortest paths are correct (by distance)
- [ ] Bottleneck capacity is minimum of path
- [ ] Repair status affects capacity (0) and pathfinding (excluded)
- [ ] Display information is clear and accurate
- [ ] Creation assistant helps users without data
- [ ] No crashes or unexplained errors
- [ ] File save/load works correctly
- [ ] Logs are written to user_actions.log

---

## Performance Notes

- Network with 5-10 companies and pipes: < 0.1 second
- Flow analysis on 10-node network: < 0.2 seconds
- No memory leaks (if using valgrind): Expected
- All C++11 features working correctly

---

## Support Information

If issues occur:

1. **Check user_actions.log** for error logs
2. **Verify data.txt** is in the same directory as main.exe
3. **Recompile** if any file changes:
   ```bash
   g++ -std=c++11 -o main.exe main.cpp Pipe.cpp KC.cpp GasNetwork.cpp Logger.cpp
   ```
4. **Run in verbose mode** to see detailed information

---

**Status**: All test cases should pass with current implementation ✅
