#pragma once
#include "board.h"

class SudokuSA
{
	Board sol;	// current working solution
    Board bestSol;
    Board currentSol;
    int bestCost;
    double acceptanceProbability;

public:	
	SudokuSA(Board sol): sol(sol) {}
    int Anneal();
    int ComputeCost();
    void FillEmptyCells();
    Board GetSolution(){return sol;}
private:
    int TryRandomSwap();
    
}; 
