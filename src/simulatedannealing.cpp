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
//    cout << "\n" << sol.AsString(true);
    double coolingRate = 0.999;
    double stoppingTemp = 0.0000000001;
    double temp = 1;
    int currentCost = ComputeCost();
    int worst = 0;
    int moves = 0;
    int cycles = 0;
    
    
    bestSol.Copy(sol);
    bestCost = currentCost;

    cout << "\nFrom " + to_string(bestCost) + " to ";
    

    while(temp > stoppingTemp){
        cycles = cycles+1;
        int origCost = currentCost;
        currentSol.Copy(sol);
        
        int newCost = TryRandomSwap();
        int delta = newCost - currentCost;
        if(delta <= 0){
            moves = moves+1;
            currentCost = newCost;
            if (currentCost < bestCost){
                bestSol.Copy(sol);
                bestCost = currentCost;
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
        temp = temp* coolingRate;
        // cout << "Temperature = " + to_string(temp) + "; "+ to_string(origCost) + " to " + to_string(currentCost);
        if (currentCost > worst)
            worst = currentCost;
        if (temp < stoppingTemp){
            cout <<to_string(currentCost) + " Worst: " + to_string(worst) + " Moves: " + to_string(moves) + " Cycles: " + to_string(cycles) +".\n ";
        }
        
        
    }

    
  

    // for (int i = 0; i < 100; i++){
    //     string origCost =  to_string(ComputeCost());
    //     cout << origCost + " to " + to_string(TryRandomSwap());
        
    // }
    //cout << "\n" << sol.AsString(true);
    //cout << "\n******************\n";

    // abort();
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
    int numUnits = sol.GetNumUnits();   // e.g., 25
    int numCells = sol.CellCount();     // e.g., 625

    // Step 1: Count how many times each number already appears in fixed cells
    std::vector<int> count(numUnits, 0);
    for (int i = 0; i < numCells; i++)
    {
        const ValueSet &cell = sol.GetCell(i);
        if (cell.Fixed())
        {
            int valIndex = cell.Index(); // 0-based index
            if (valIndex >= 0 && valIndex < numUnits)
                count[valIndex]++;
        }
    }

    // Step 2: Create a pool of values needed to reach exactly numUnits occurrences for each number
    std::vector<int> pool;
    pool.reserve(numCells);

    for (int n = 0; n < numUnits; n++)
    {
        int remaining = numUnits - count[n];
        if (remaining < 0)
            remaining = 0; // just in case of overfilled numbers
        for (int k = 0; k < remaining; k++)
            pool.push_back(n);
    }

    // Step 3: Shuffle the pool
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(pool.begin(), pool.end(), gen);

    // Step 4: Fill empty cells from the pool
    int poolIndex = 0;
    for (int i = 0; i < numCells; i++)
    {
        const ValueSet &cell = sol.GetCell(i);
        if (!cell.Fixed())
        {
            if (poolIndex < (int)pool.size())
            {
                int valIndex = pool[poolIndex++];
                // Force set directly to ensure each number appears numUnits times overall
                sol.ForceSetCell(i, ValueSet(numUnits, (int64_t)1 << valIndex));
            }
            else
            {
                // Safety fallback (shouldn’t happen)
                int valIndex = i % numUnits;
                sol.ForceSetCell(i, ValueSet(numUnits, (int64_t)1 << valIndex));
            }
        }
    }
}


int SudokuSA::TryRandomSwap()
{
    int numCells = sol.CellCount();
    int numUnits = sol.GetNumUnits();
    int oldCost = ComputeCost();

    // cout << "\n" << sol.AsString(true)
    //      << "\n\n*****************************\n";

    // Get list of non-fixed cells (for now all cells)
    std::vector<int> nonFixedIndices;
    for (int i = 0; i < numCells; i++)
    {
        if (true) // Replace with !sol.IsFixed(i) if applicable
            nonFixedIndices.push_back(i);
    }

    // Random engine
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, nonFixedIndices.size() - 1);

    // Pick two random indices to swap
    int idx1 = nonFixedIndices[dist(gen)];
    int idx2 = nonFixedIndices[dist(gen)];
    while (idx1 == idx2)
        idx2 = nonFixedIndices[dist(gen)];

    // Compute their (row, column)
    int size = static_cast<int>(sqrt(numCells)); // e.g. 25 for 25×25 Sudoku
    int row1 = idx1 / size + 1;
    int col1 = idx1 % size + 1;
    int row2 = idx2 / size + 1;
    int col2 = idx2 % size + 1;

    // Get their current values
    ValueSet cell1 = sol.GetCell(idx1);
    ValueSet cell2 = sol.GetCell(idx2);

    // cout << "\nSwapping:"
    //      << " Cell(" << row1 << "," << col1 << ") = " << cell1.Index() + 1
    //      << "  <->  "
    //      << "Cell(" << row2 << "," << col2 << ") = " << cell2.Index() + 1
    //      << "\n";

    // --- Perform swap ---
    sol.ForceSetCell(idx1, cell2);
    sol.ForceSetCell(idx2, cell1);

    // Compute new cost
    int newCost = ComputeCost();

    // cout << "\n" << sol.AsString(true)
    //      << "\n\n*****************************\n";

    // --- Return new cost (no revert logic yet) ---
    return newCost;
}
