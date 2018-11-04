/*
 * Block.cpp
 *
 *  Created on: Oct 17, 2018
 *      Author: chentuju
 */

#include "Block.h"

Block::Block() {

	m_id = -1;
	m_type = -1;
	m_movable = 1;
	m_xCoord = -1.0;
	m_yCoord = -1.0;
}

Block::~Block() {

	for(unsigned int i=0; i<m_nets.size(); i++)
		m_nets[i] = NULL;
}

void Block::setBlockInfo(int index, int num) {

	switch(index) {
	case 0: m_id = num;			break;
	case 1: m_type = num;		break;
	case 2: m_movable = num;	break;
	default:
		std::cout << "[ERROR Block::setBlockInfo] Invalid index: "
		<< index << std::endl;
		break;
	}
}

int Block::getBlockInfo(int index) {

	switch(index) {
	case 0: return m_id;
	case 1: return m_type;
	case 2: return m_movable;
	default:
		std::cout << "[ERROR Block::getBlockInfo] Invalid index: "
		<< index << std::endl;
		return -1;
	}
}

void Block::setBlockCoords(double x, double y) {

	m_xCoord = x;
	m_yCoord = y;
}

double Block::getBlockCoords(int coord) {

	return (coord == 0) ? m_xCoord : m_yCoord;
}

void Block::addNet(Net *net) {

	m_nets.push_back(net);
}

double Block::getTotalNetWeights() {

	if(m_movable == 0) {
		std::cout << "[ERROR Block::getTotalNetWeights] Trying to get total weights for fixed block "
		<< m_id << std::endl;
		return 0.0;
	}
	if(m_movable == 2) {
		std::cout << "[ERROR Block::getTotalNetWeights] Trying to get total weights for virtual block "
		<< m_id << std::endl;
		return 0.0;
	}

	double weight = 0.0;
	for(unsigned int i=0; i<m_nets.size(); i++)
		weight +=(m_nets[i]->getWeight() * (m_nets[i]->getBlocks().size()-1));
	return weight;
}

double Block::getNegTotalWeightsBlock(Block *block) {

	if(m_movable == 0) {
		std::cout << "[ERROR Block::getNegTotalWeightsBlock] Trying to get total weights for fixed block "
		<< m_id << std::endl;
		return 0.0;
	}
	if(m_movable == 2) {
		std::cout << "[ERROR Block::getNegTotalWeightsBlock] Trying to get total weights for virtual block "
		<< m_id << std::endl;
		return 0.0;
	}

	double weight = 0.0;
	for(unsigned int i=0; i<m_nets.size(); i++)
		if(m_nets[i]->blockExists(block))
			weight -= m_nets[i]->getWeight();
	return weight;
}

double Block::getTotalFixedNetWeightProducts(int index) {

	if(m_movable == 0) {
		std::cout << "[ERROR Block::getTotalFixedNetWeightProducts] Trying to get total weights for fixed block "
		<< m_id << std::endl;
		return 0.0;
	}
	if(m_movable == 2) {
		std::cout << "[ERROR Block::getTotalFixedNetWeightProducts] Trying to get total weights for virtual block "
		<< m_id << std::endl;
		return 0.0;
	}

	double total = 0.0;
	for(unsigned int i=0; i<m_nets.size(); i++) {
		Net *net = m_nets[i];
		const std::vector<Block*> &blocks = net->getBlocks();
		for(unsigned int j=0; j<blocks.size(); j++) {
			Block *block = blocks[j];
			// add to total if current block is fixed or virtual
			if(block->getBlockInfo(2) == 0 ||
					block->getBlockInfo(2) == 2) {
				total += (net->getWeight() * block->getBlockCoords(index));
			}
		}
	}
	return total;
}

const std::vector<Net*>& Block::getNets() {

	return m_nets;
}

void Block::printBlock() {

	std::cout << "(Block ID = " << std::setw(3) << m_id
			<< "; type = " << std::setw(1) << m_type
			<< "; movable = " << std::setw(1) << m_movable
			<< "; xCoord = " << std::setw(10) << m_xCoord
			<< "; yCoord = " << std::setw(10) << m_yCoord
			<< "; # nets = " << std::setw(2) << m_nets.size()
			<< "; nets =";
	for(unsigned int i=0; i<m_nets.size(); i++)
		std::cout << " " << m_nets[i]->getId();
	std::cout << ")";
}
