#include "simulatedannealing.h"
#include "board.h"
#include <unordered_set>
#include <random>
#include <cmath>
#include <iostream>
#include <cstdlib>

int SudokuSA::Anneal()
{
    FillEmptyCells();
    // cout << "\n" << sol.AsString(true);
    double coolingRate = 0.995;
    double stoppingTemp = 0.01;
    double temp = 1.5;
    int currentCost = ComputeCost();
    int worst = 0;
    int moves = 0;
    int cycles = 0;

    // cout << to_string(currentCost) + "\n";

    if (currentCost == 0){
        return currentCost;
    }
    
    
    bestSol.Copy(sol);
    bestCost = currentCost;

    // cout << "\nFrom " + to_string(bestCost) + " to ";
    

    while(temp > stoppingTemp){
        for (int i = 0; i < 1; i++){
            int origCost = currentCost;
            currentSol.Copy(sol);
            
            int newCost = TryRandomSwap(origCost);
            int delta = newCost - currentCost;
            if(delta <= 0){
                moves = moves+1;
                currentCost = newCost;
                if (currentCost < bestCost){
                    bestSol.Copy(sol);
                    bestCost = currentCost;
                    if (currentCost == 0){
                        // cout << "\n" << sol.AsString(true);
                        return 0;
                    }
                }
                
            }
            else{
                acceptanceProbability = exp(-delta / temp);
                double rnd = (double) rand() / RAND_MAX;
                if (rnd < acceptanceProbability){
                    moves = moves+1;
                    currentCost = newCost;
                }
                else{
                    sol.Copy(currentSol);
                }
            }
            
            // cout << "Temperature = " + to_string(temp) + "; "+ to_string(origCost) + " to " + to_string(currentCost);
            if (currentCost > worst)
                worst = currentCost;
        }
        
        cycles = cycles+1;
        temp = temp* coolingRate;
        if (temp < stoppingTemp){
            // cout <<to_string(currentCost) + " Worst: " + to_string(worst) + " Moves: " + to_string(moves) + " Cycles: " + to_string(cycles) +".\n ";
        }
        
        
    }

    
  

    // for (int i = 0; i < 100; i++){
    //     string origCost =  to_string(ComputeCost());
    //     cout << origCost + " to " + to_string(TryRandomSwap());
        
    // }
    //cout << "\n" << sol.AsString(true);
    //cout << "\n******************\n";

    // abort();
    CleanDuplicates();
    return bestCost;
    
}

int SudokuSA::ComputeCost()
{
    int cost = 0;
    int numUnits = sol.GetNumUnits();
    int numCells = sol.CellCount();

    // --- Empty cell cost ---
    for (int i = 0; i < numCells; i++)
    {
        const ValueSet& cell = sol.GetCell(i);
        if (!cell.Fixed())
            cost += 1; // empty cell adds cost
    }

    // --- Row duplicate cost ---
    for (int row = 0; row < numUnits; row++)
    {
        std::unordered_set<int> seen;
        for (int col = 0; col < numUnits; col++)
        {
            const ValueSet& cell = sol.GetCell(sol.RowCell(row, col));
            if (cell.Fixed())
            {
                int val = cell.Index();
                if (seen.count(val))
                    cost += 1; // duplicate value in row
                else
                    seen.insert(val);
            }
        }
    }

    // --- Column duplicate cost ---
    for (int col = 0; col < numUnits; col++)
    {
        std::unordered_set<int> seen;
        for (int row = 0; row < numUnits; row++)
        {
            const ValueSet& cell = sol.GetCell(sol.ColCell(col, row));
            if (cell.Fixed())
            {
                int val = cell.Index();
                if (seen.count(val))
                    cost += 1; // duplicate value in column
                else
                    seen.insert(val);
            }
        }
    }

    return cost;
}

void SudokuSA::FillEmptyCells()
{
    int numCells = sol.CellCount();
    int numUnits = sol.GetNumUnits();      // e.g., 16 for 16x16
    int blockSize = numUnits;              // number of cells per block
    int numBlocks = numCells / blockSize;  // total number of blocks

    std::random_device rd;
    std::mt19937 gen(rd());

    // For each block
    for (int block = 0; block < numBlocks; block++)
    {
        // Track which numbers are already present in fixed cells of this block
        std::vector<bool> used(numUnits, false);
        std::vector<int> emptyCells; // store indices of non-fixed cells

        // Scan the block
        for (int pos = 0; pos < numUnits; pos++)
        {
            int cellIndex = sol.BoxCell(block, pos);
            if (!sol.IsEmpty(cellIndex))
            {
                
                int val = sol.GetCell(cellIndex).Index();
                if (val >= 0 && val < numUnits)
                    used[val] = true;
            }
            else
            {
                emptyCells.push_back(cellIndex);
            }
        }

        // Build missing numbers for this block
        std::vector<int> missing;
        missing.reserve(emptyCells.size());
        for (int n = 0; n < numUnits; n++)
        {
            if (!used[n])
                missing.push_back(n);
        }

        // Shuffle missing numbers
        std::shuffle(missing.begin(), missing.end(), gen);

        // Fill only empty (non-fixed) cells
        for (size_t i = 0; i < emptyCells.size(); i++)
        {
            int cellIndex = emptyCells[i];
            int value = missing[i];
            sol.ForceSetCell(cellIndex, ValueSet(numUnits, (int64_t)1 << value));
        }
    }
}




int SudokuSA::TryRandomSwap(int oldCost)
{
    int numUnits = sol.GetNumUnits();

    // Identify conflict cells same as before
    std::vector<int> conflictedCells;
    conflictedCells.reserve(sol.CellCount());

    std::vector<bool> rowHasDup(numUnits, false);
    std::vector<bool> colHasDup(numUnits, false);

    // Detect conflict rows
    for (int row = 0; row < numUnits; ++row)
    {
        std::unordered_set<int> seen;
        bool dup = false;
        for (int c = 0; c < numUnits; ++c)
        {
            int idx = sol.RowCell(row,c);
            if (!sol.GetCell(idx).Fixed()) continue;
            int v = sol.GetCell(idx).Index();
            if (!seen.insert(v).second) { dup = true; break; }
        }
        rowHasDup[row] = dup;
    }

    // Detect conflict columns
    for (int col = 0; col < numUnits; ++col)
    {
        std::unordered_set<int> seen;
        bool dup = false;
        for (int r = 0; r < numUnits; ++r)
        {
            int idx = sol.ColCell(col,r);
            if (!sol.GetCell(idx).Fixed()) continue;
            int v = sol.GetCell(idx).Index();
            if (!seen.insert(v).second) { dup = true; break; }
        }
        colHasDup[col] = dup;
    }

    // Collect swap candidates
    for (int idx = 0; idx < sol.CellCount(); ++idx)
    {
        if (sol.IsClue(idx)) continue;
        if (rowHasDup[sol.RowForCell(idx)] ||
            colHasDup[sol.ColForCell(idx)])
        {
            conflictedCells.push_back(idx);
        }
    }

    if (conflictedCells.empty())
        return oldCost;

    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> d1(0, conflictedCells.size()-1);

    int idx1 = conflictedCells[d1(gen)];
    int box = sol.BoxForCell(idx1);

    // Pick partner in same box
    std::vector<int> same;
    for (int pos = 0; pos < numUnits; ++pos)
    {
        int idx = sol.BoxCell(box,pos);
        if (!sol.IsClue(idx) && idx!=idx1)
            same.push_back(idx);
    }
    if (same.empty())
        return oldCost;

    std::uniform_int_distribution<> d2(0, same.size()-1);
    int idx2 = same[d2(gen)];

    // Compute local conflicts BEFORE swap
    int before =
        LocalConflicts(idx1) +
        LocalConflicts(idx2);

    // Swap
    ValueSet v1 = sol.GetCell(idx1);
    ValueSet v2 = sol.GetCell(idx2);
    sol.ForceSetCell(idx1,v2);
    sol.ForceSetCell(idx2,v1);

    // Compute local conflicts AFTER swap
    int after =
        LocalConflicts(idx1) +
        LocalConflicts(idx2);

    return oldCost + (after - before);
}

int SudokuSA::LocalConflicts(int idx)
{
    int numUnits = sol.GetNumUnits();
    int row = sol.RowForCell(idx);
    int col = sol.ColForCell(idx);
    int val = sol.GetCell(idx).Index();
    int conflicts = 0;

    // Check row duplicates
    for(int c = 0; c < numUnits; c++){
        int id = sol.RowCell(row,c);
        if(id != idx && sol.GetCell(id).Fixed()){
            if(sol.GetCell(id).Index() == val)
                conflicts++;
        }
    }

    // Check column duplicates
    for(int r = 0; r < numUnits; r++){
        int id = sol.ColCell(col,r);
        if(id != idx && sol.GetCell(id).Fixed()){
            if(sol.GetCell(id).Index() == val)
                conflicts++;
        }
    }

    return conflicts;
}


void SudokuSA::CleanDuplicates()
{
    int numUnits = sol.GetNumUnits();
    int numCells = sol.CellCount();

    // conflictCount[i] = number of duplicate conflicts the cell participates in
    std::vector<int> conflictCount(numCells, 0);

    // --- Count conflicts in rows ---
    for (int row = 0; row < numUnits; row++)
    {
        std::unordered_map<int, std::vector<int>> positions;

        for (int col = 0; col < numUnits; col++)
        {
            int idx = sol.RowCell(row, col);
            const ValueSet& cell = sol.GetCell(idx);

            if (cell.Fixed())
                positions[cell.Index()].push_back(idx);
        }

        // Any number with >1 occurrences is a duplicate
        for (auto& p : positions)
        {
            if (p.second.size() > 1)
            {
                for (int idx : p.second)
                    conflictCount[idx]++;
            }
        }
    }

    // --- Count conflicts in columns ---
    for (int col = 0; col < numUnits; col++)
    {
        std::unordered_map<int, std::vector<int>> positions;

        for (int row = 0; row < numUnits; row++)
        {
            int idx = sol.ColCell(col, row);
            const ValueSet& cell = sol.GetCell(idx);

            if (cell.Fixed())
                positions[cell.Index()].push_back(idx);
        }

        // duplicates → increment conflict counts
        for (auto& p : positions)
        {
            if (p.second.size() > 1)
            {
                for (int idx : p.second)
                    conflictCount[idx]++;
            }
        }
    }

    // --- Now remove one cell among duplicates (the worst one) ---

    // Rows first
    for (int row = 0; row < numUnits; row++)
    {
        std::unordered_map<int, std::vector<int>> positions;

        for (int col = 0; col < numUnits; col++)
        {
            int idx = sol.RowCell(row, col);
            const ValueSet& cell = sol.GetCell(idx);

            if (cell.Fixed())
                positions[cell.Index()].push_back(idx);
        }

        for (auto& p : positions)
        {
            auto& duplicates = p.second;
            if (duplicates.size() <= 1)
                continue;

            // Pick the duplicate with highest conflict count
            int worstIdx = duplicates[0];
            int worstScore = conflictCount[worstIdx];

            for (int idx : duplicates)
            {
                if (conflictCount[idx] > worstScore)
                {
                    worstIdx = idx;
                    worstScore = conflictCount[idx];
                }
            }

            // Remove the worst duplicate (only one removed)
            sol.ForceSetCell(worstIdx, ValueSet(numUnits, 0));
        }
    }

    // Columns next
    for (int col = 0; col < numUnits; col++)
    {
        std::unordered_map<int, std::vector<int>> positions;

        for (int row = 0; row < numUnits; row++)
        {
            int idx = sol.ColCell(col, row);
            const ValueSet& cell = sol.GetCell(idx);

            if (cell.Fixed())
                positions[cell.Index()].push_back(idx);
        }

        for (auto& p : positions)
        {
            auto& duplicates = p.second;
            if (duplicates.size() <= 1)
                continue;

            int worstIdx = duplicates[0];
            int worstScore = conflictCount[worstIdx];

            for (int idx : duplicates)
            {
                if (conflictCount[idx] > worstScore)
                {
                    worstIdx = idx;
                    worstScore = conflictCount[idx];
                }
            }

            sol.ForceSetCell(worstIdx, ValueSet(numUnits, 0));
        }
    }
}
