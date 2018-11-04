/*
 * Block.h
 *
 *  Created on: Oct 17, 2018
 *      Author: chentuju
 */

#ifndef BLOCK_H_
#define BLOCK_H_

#include <iostream>
#include <iomanip>
#include <vector>
#include "Net.h"

class Net;

class Block {
public:
	Block();
	~Block();

	// helper methods
	void setBlockInfo(int index, int num);
	int getBlockInfo(int index);
	void setBlockCoords(double x, double y);
	double getBlockCoords(int coord);
	void addNet(Net *net);
	double getTotalNetWeights();
	double getNegTotalWeightsBlock(Block *block);
	double getTotalFixedNetWeightProducts(int coord);
	const std::vector<Net*>& getNets();

	// debugging methods
	void printBlock();

private:
	int m_id;
	int m_type;
	int m_movable;	// 0: fixed, 1: movable, 2:virtual
	float m_xCoord;
	float m_yCoord;
	std::vector<Net*> m_nets;
};

#endif /* BLOCK_H_ */
