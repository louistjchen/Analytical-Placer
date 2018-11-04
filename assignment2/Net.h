/*
 * Net.h
 *
 *  Created on: Oct 17, 2018
 *      Author: chentuju
 */

#ifndef NET_H_
#define NET_H_

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <vector>
#include "Block.h"

class Block;

class Net {
public:
	Net();
	~Net();

	// helper methods
	void setId(int id);
	int getId();
	void computeWeight();
	void setVirtualNetWeight(double spreadDepth, double length);
	double getWeight();
	void addBlock(Block *block);
	bool blockExists(Block *block);
	const std::vector<Block*>& getBlocks();

	// debugging methods
	void printNet();

private:
	int m_id;
	double m_weight;
	std::vector<Block*> m_blocks;
};

#endif /* NET_H_ */
