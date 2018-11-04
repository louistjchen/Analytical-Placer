/*
 * Net.cpp
 *
 *  Created on: Oct 17, 2018
 *      Author: chentuju
 */

#include "Net.h"

Net::Net() {

	m_id = -1;
	m_weight = 1.0;
}

Net::~Net() {

	for(unsigned int i=0; i<m_blocks.size(); i++)
		m_blocks[i] = NULL;
}

void Net::setId(int id) {

	m_id = id;
}

int Net::getId() {

	return m_id;
}

void Net::computeWeight() {

	if(m_id >= 0)
		m_weight = 2.0/m_blocks.size();
}

void Net::setVirtualNetWeight(double spreadDepth, double length) {

	if(length == 0.0)
		m_weight = 0.0;
	else if(length > 0.0)
		m_weight = pow(length, spreadDepth) / length;
	else
		std::cout << "[ERROR Net::setVirtualNetWeight] Input length is less than zero."
		<< std::endl;
}

double Net::getWeight() {

	return m_weight;
}

void Net::addBlock(Block *block) {

	m_blocks.push_back(block);
}

bool Net::blockExists(Block *block) {

	bool exists = false;
	for(unsigned int i=0; i<m_blocks.size(); i++)
		if(m_blocks[i] == block) {
			exists = true;
			break;
		}
	return exists;
}

const std::vector<Block*>& Net::getBlocks() {

	return m_blocks;
}

void Net::printNet() {

	std::cout << "(Net ID = " << std::setw(3) << m_id
			<< "; weight = " << std::setw(8) << m_weight
			<< "; # blocks = " << std::setw(4) << m_blocks.size()
			<< "; blocks =";
	for(unsigned int i=0; i<m_blocks.size(); i++)
		std::cout << " " << m_blocks[i]->getBlockInfo(0);
	std::cout << ")";
}
