/*
 * Placer.h
 *
 *  Created on: Oct 16, 2018
 *      Author: chentuju
 */

#ifndef PLACER_H_
#define PLACER_H_

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <queue>
#include "Block.h"
#include "Net.h"
#include "umfpack.h"
#include "Display.h"

class Placer {
public:
	Placer();
	~Placer();

	// main methods (invoked by main.cpp)
	void init(const char *inputFile);
	void run();

	// API methods for Display.cpp to invoke
	double getGridSize();
	const std::vector<Block*>& getBlockList();
	bool toggleSnapWithConstraint();
	void invokeSnap();

protected:
	// functional methods
	void place();
	void spread();
	void snap();

	// helper methods for place()
	void computeCliqueWeights();
	void constructMatrix();
	void solveMatrixUpdateCoord();
	void solveLinearEquations(unsigned int n, int *Ap, int *Ai,
			double *Ax, double *b, double *x);

	// helper methods for spread()
	Block *allocateVirtualBlock(int id, double x, double y);
	Net *allocateVirtualNet(int id, double spreadDepth, double length);
	void constructBlockCategory(
			std::vector<std::vector<std::vector<Block*>>> &blockCategory,
			const std::vector<Block*> &blockList,
			int x1,	int y1,	int x2,	int y2, int currDepth, int depth);

	// helper methods for snap()
	void snapFixedBlock(Block *block);
	void snapMovableBlock(Block *block);
	bool findClosestFreeBlock(int &x, int &y, int type);

	// debugging methods
	void printBlockList();
	void printNetList();
	void printHPBBWL();

private:
	// main data members
	double m_gridSize;
	std::vector<Block*> m_blockList;
	std::vector<Net*> m_netList;
	std::unordered_map<int, Net*> m_netHash;
	std::vector<Block*> m_movableBlockList;

	// data members for place()
	std::vector<int> m_matrixQp;
	std::vector<int> m_matrixQi;
	std::vector<double> m_matrixQxy;
	std::vector<double> m_matrixbx;
	std::vector<double> m_matrixby;

	// data members for spread();
	unsigned int m_spreadDepth;
	int m_nextAvailableVirtualNetId;
	int m_nextAvailableVirtualBlockId;

	// data members for snap();
	bool m_snapped;
	bool m_snapWithConstraint;
	bool **m_snapOccupied;
	bool **m_snapVisited;
};

#endif /* PLACER_H_ */
