/*
 * Placer.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: chentuju
 */

#include "Placer.h"

class Compare {
public:
	bool operator()(std::pair<double,Block*> &a, std::pair<double,Block*> &b) {
		return a.first > b.first;
	}
};

Placer::Placer() {

	m_gridSize = 0;
	m_spreadDepth = 1;
	m_nextAvailableVirtualNetId = -10;
	m_nextAvailableVirtualBlockId = -10;
	m_snapped = false;
	m_snapWithConstraint = true;
}

Placer::~Placer() {

	// free blocks
	for(unsigned int i=0; i<m_blockList.size(); i++)
		if(m_blockList[i])
			delete m_blockList[i];

	// free nets
	for(unsigned int i=0; i<m_netList.size(); i++)
		if(m_netList[i])
			delete m_netList[i];
}

void Placer::init(const char *inputFile) {

	std::ifstream file;
	file.open(inputFile, std::ifstream::in);

	// construct blocks
	char s[8];
	int num;
	while(file >> s) {

		// break if see -1 twice
		num = atoi(s);
		if(num == -1)
			break;

		// allocate a block and assign id to it
		Block *block = new Block();
		m_blockList.push_back(block);
		block->setBlockInfo(0, num);

		// assign type to block
		file >> s;
		num = atoi(s);
		block->setBlockInfo(1, num);

		file >> s;
		num = atoi(s);
		while(num != -1) {
			Net *net = NULL;
			// check if net exists
			if(m_netHash.find(num) != m_netHash.end())
				net = m_netHash[num];
			else {
				net = new Net();
				m_netList.push_back(net);
				m_netHash[num] = net;
				net->setId(num);
			}
			// add current block to net
			net->addBlock(block);
			// add current net to block
			block->addNet(net);

			// scan next number
			file >> s;
			num = atoi(s);
		}
		// hit num = -1
	}

	// update movable and x y coordinates for fixed blocks
	while(file >> s) {

		// break if see -1
		num = atoi(s);
		if(num == -1)
			break;

		// get the pointer to block[num] and update movable
		Block *block = m_blockList[num-1];
		block->setBlockInfo(2, 0);

		// update x y coordinates
		double xCoord;
		double yCoord;
		file >> s;
		xCoord = atof(s);
		file >> s;
		yCoord = atof(s);
		block->setBlockCoords(xCoord, yCoord);
	}

	m_gridSize = ceil(sqrt(m_blockList.size()));
	file.close();

	// split movable blocks from fixed blocks
	for(unsigned int i=0; i<m_blockList.size(); i++)
		if(m_blockList[i]->getBlockInfo(2) == 1)
			m_movableBlockList.push_back(m_blockList[i]);
}

void Placer::run() {

	openDisplay(this);
	place();
//	printBlockList();
//	printNetList();
	printHPBBWL();
	while(1) {
		updateDisplay();
		// snap() is called in Display.cpp with provided API
		if(m_snapped)
			break;
		spread();
		place();
//		printBlockList();
//		printNetList();
		printHPBBWL();
	}
//	snap();
//	printHPBBWL();
	closeDisplay();
}

double Placer::getGridSize() {

	return m_gridSize;
}

const std::vector<Block*>& Placer::getBlockList() {

	return m_blockList;
}

bool Placer::toggleSnapWithConstraint() {

	m_snapWithConstraint = m_snapWithConstraint ? false : true;
	return m_snapWithConstraint;
}

void Placer::invokeSnap() {

	m_snapped = true;
	snap();
	printHPBBWL();
}

void Placer::place() {

	computeCliqueWeights();
	constructMatrix();
	solveMatrixUpdateCoord();
}

void Placer::spread() {

	m_spreadDepth *= 2;
	double delta = m_gridSize / (double)m_spreadDepth;
	double offset = delta/2.0;

	std::vector<std::vector<std::vector<Block*>>> blockCategory;
	for(unsigned int i=0; i<m_spreadDepth; i++) {
		blockCategory.push_back(std::vector<std::vector<Block*>>());
		for(unsigned int j=0; j<m_spreadDepth; j++)
			blockCategory[i].push_back(std::vector<Block*>());
	}
	constructBlockCategory(blockCategory, m_movableBlockList,
			0, 0, m_spreadDepth-1, m_spreadDepth-1, 1, m_spreadDepth);

	// create a virtual block for every m_spreadDepth x m_spreadDepth
	for(unsigned int i=0; i<m_spreadDepth; i++) {
		for(unsigned int j=0; j<m_spreadDepth; j++) {

			// set virtual block id to the next available id number
			Block *virtualBlock = allocateVirtualBlock(
					m_nextAvailableVirtualBlockId--,
					i*delta+offset, j*delta+offset);
			m_blockList.push_back(virtualBlock);

			// loop every movable block that belongs to current virtual block
			for(unsigned int k=0; k<blockCategory[i][j].size(); k++) {

				// compute movable block index associated to virtual block
				Block *block = blockCategory[i][j][k];
				double x = block->getBlockCoords(0);
				double y = block->getBlockCoords(1);

				// obtain associated virtual block coordinates
				double xVirtual = virtualBlock->getBlockCoords(0);
				double yVirtual = virtualBlock->getBlockCoords(1);

				// allocate a net and initialize it
				double length = fabs(x-xVirtual) + fabs(y-yVirtual); // Manhattan distance
				Net *virtualNet = allocateVirtualNet(
						m_nextAvailableVirtualNetId,
						(double)m_spreadDepth,
						length);

				// link block, virtualBlock, virtualNet altogether
				virtualNet->addBlock(block);
				virtualNet->addBlock(virtualBlock);
				block->addNet(virtualNet);
				virtualBlock->addNet(virtualNet);

				// push virtualNet into m_netList/m_netHash
				m_netList.push_back(virtualNet);
				m_netHash[m_nextAvailableVirtualNetId--] = virtualNet;
			}
		}
	}
}

void Placer::snap() {

	unsigned int gridSize = (int) m_gridSize;

	// dynamically allocate m_snapOccupied and m_snapVisited
	m_snapOccupied = new bool*[gridSize];
	m_snapVisited = new bool*[gridSize];
	for(unsigned int i=0; i<gridSize; i++) {
		m_snapOccupied[i] = new bool[gridSize];
		m_snapVisited[i] = new bool[gridSize];
	}

	// initialize m_snapOccupied
	for(unsigned int i=0; i<gridSize; i++)
		for(unsigned int j=0; j<gridSize; j++)
			m_snapOccupied[i][j] = false;

	// snap fixed blocks
	for(unsigned int i=0; i<m_blockList.size(); i++) {
		Block *block = m_blockList[i];
		int blockMovable = block->getBlockInfo(2);
		// break while having traversed all blocks
		if(blockMovable == 2)
			break;
		// skip if current block is not fixed
		if(blockMovable == 1)
			continue;
		snapFixedBlock(block);
	}

	// snap special blocks if m_snapWithConstraint = true
	if(m_snapWithConstraint)
		for(unsigned int i=0; i<m_movableBlockList.size(); i++) {
			Block *block = m_movableBlockList[i];
			if(block->getBlockInfo(1) != 1)
				continue;
			snapMovableBlock(block);
		}

	// snap general blocks
	for(unsigned int i=0; i<m_movableBlockList.size(); i++) {
		Block *block = m_movableBlockList[i];
		if(m_snapWithConstraint)
			if(block->getBlockInfo(1) != 0)
				continue;
		snapMovableBlock(block);
	}

	// free m_snapOccupied and m_snapVisited
	for(unsigned int i=0; i<gridSize; i++) {
		delete[] m_snapOccupied[i];
		delete[] m_snapVisited[i];
	}
	delete[] m_snapOccupied;
	delete[] m_snapVisited;
}

void Placer::computeCliqueWeights() {

	for(unsigned int i=0; i<m_netList.size(); i++)
		m_netList[i]->computeWeight();
}

void Placer::constructMatrix() {

	m_matrixQp.clear();
	m_matrixQi.clear();
	m_matrixQxy.clear();
	m_matrixbx.clear();
	m_matrixby.clear();

	for(unsigned int i=0; i<m_movableBlockList.size(); i++) {

		bool firstFound = false;
		for(unsigned int j=0; j<m_movableBlockList.size(); j++) {

			double weight;
			Block *iBlock = m_movableBlockList[i];
			Block *jBlock = m_movableBlockList[j];

			if(i == j) // diagonal: weight = sum of all weights on cell i
				weight = iBlock->getTotalNetWeights();
			else // non-diagonal: weight = - sum of all weights connecting cell i, j
				weight = iBlock->getNegTotalWeightsBlock(jBlock);

//			std::cout << " " << std::setw(10) << weight;

			// update Q if weight is non-zero
			if(weight != 0.0) {
				if(!firstFound) {
					m_matrixQp.push_back((int)m_matrixQi.size());
					firstFound = true;
				}
				m_matrixQi.push_back(j);
				m_matrixQxy.push_back(weight);
			}
		}
//		std::cout << std::endl;
	}
	m_matrixQp.push_back((int)m_matrixQi.size());

	for(unsigned int i=0; i<m_movableBlockList.size(); i++) {
		Block *block = m_movableBlockList[i];
		m_matrixbx.push_back(block->getTotalFixedNetWeightProducts(0));
		m_matrixby.push_back(block->getTotalFixedNetWeightProducts(1));
//		std::cout << m_matrixbx[i] << "\t" << m_matrixby[i] << std::endl;
	}
//	std::cout << std::endl;
}

void Placer::solveMatrixUpdateCoord() {

	unsigned int n;
	int *Qp;
	int *Qi;
	double *Qxy;
	double *bx;
	double *by;
	double *x;
	double *y;

	n = m_movableBlockList.size();
	Qp = new int[m_matrixQp.size()];
	Qi = new int[m_matrixQi.size()];
	Qxy = new double[m_matrixQxy.size()];
	bx = new double[m_matrixbx.size()];
	by = new double[m_matrixby.size()];
	x = new double[n];
	y = new double[n];

	for(unsigned int i=0; i<m_matrixQp.size(); i++)
		Qp[i] = m_matrixQp[i];
	for(unsigned int i=0; i<m_matrixQi.size(); i++) {
		Qi[i] = m_matrixQi[i];
		Qxy[i] = m_matrixQxy[i];
	}
	for(unsigned int i=0; i<m_matrixbx.size(); i++) {
		bx[i] = m_matrixbx[i];
		by[i] = m_matrixby[i];
	}

	solveLinearEquations(n, Qp, Qi, Qxy, bx, x);
	solveLinearEquations(n, Qp, Qi, Qxy, by, y);

	for(unsigned int i=0; i<n; i++)
		m_movableBlockList[i]->setBlockCoords(x[i], y[i]);

	delete[] Qp;
	delete[] Qi;
	delete[] Qxy;
	delete[] bx;
	delete[] by;
	delete[] x;
	delete[] y;
}

void Placer::solveLinearEquations(unsigned int n, int *Qp, int *Qi,
		double *Qxy, double *b, double *xy) {

	void *Symbolic;
	void *Numeric;
	umfpack_di_symbolic(n, n, Qp, Qi, Qxy, &Symbolic, NULL, NULL);
	umfpack_di_numeric(Qp, Qi, Qxy, Symbolic, &Numeric, NULL, NULL);
	umfpack_di_solve(UMFPACK_A, Qp, Qi, Qxy, xy, b, Numeric, NULL, NULL);
	umfpack_di_free_symbolic(&Symbolic);
	umfpack_di_free_numeric(&Numeric);
}

Block *Placer::allocateVirtualBlock(int id, double x, double y) {

	Block *virtualBlock = new Block();
	virtualBlock->setBlockInfo(0, id);
	virtualBlock->setBlockInfo(1,0);
	virtualBlock->setBlockInfo(2,2);
	virtualBlock->setBlockCoords(x, y);
	return virtualBlock;
}

Net *Placer::allocateVirtualNet(int id, double spreadDepth, double length) {

	Net *virtualNet = new Net();
	virtualNet->setId(id);
	virtualNet->setVirtualNetWeight(spreadDepth, length);
	return virtualNet;
}

void Placer::constructBlockCategory(
		std::vector<std::vector<std::vector<Block*>>> &blockCategory,
		const std::vector<Block*> &blockList,
		int x1,
		int y1,
		int x2,
		int y2,
		int currDepth,
		int depth) {

	if(currDepth == depth) {
		for(unsigned int i=0; i<blockList.size(); i++)
			blockCategory[x1][y1].push_back(blockList[i]);
		return;
	}
	else {
		std::vector<Block*> topLeft;
		std::vector<Block*> topRight;
		std::vector<Block*> bottomLeft;
		std::vector<Block*> bottomRight;
		std::priority_queue<std::pair<double,Block*>,
							std::vector<std::pair<double,Block*>>,
							Compare> pq;
		for(unsigned int i=0; i<blockList.size(); i++) {
			Block *block = blockList[i];
			double dist = sqrt(pow(block->getBlockCoords(0), 2)+
					pow(block->getBlockCoords(1), 1));
			pq.push(std::make_pair(dist, block));
		}

		int size = pq.size();
		int step = size/4;
		for(int i=0; i<size; i++) {
			Block *block = pq.top().second;
			pq.pop();
			if(i<step)
				topLeft.push_back(block);
			else if(i<step*2)
				topRight.push_back(block);
			else if(i<step*3)
				bottomLeft.push_back(block);
			else
				bottomRight.push_back(block);
		}
		int offset = (x2 - x1) / 2;
		constructBlockCategory(blockCategory, topLeft, x1, y1, x1+offset, y1+offset, currDepth*2, depth);
		constructBlockCategory(blockCategory, topRight, x1+offset+1, y1, x2, y1+offset, currDepth*2, depth);
		constructBlockCategory(blockCategory, bottomLeft, x1, y1+offset+1, x1+offset, y2, currDepth*2, depth);
		constructBlockCategory(blockCategory, bottomRight, x1+offset+1, y1+offset+1, x2, y2, currDepth*2, depth);
	}
}

void Placer::snapFixedBlock(Block *block) {

	if(block->getBlockInfo(2) != 0) {
		std::cout << "[ERROR Placer::snapFixedBlock] Trying to snap a non-fixed block"
				<< std::endl;
		return;
	}

	float xCoord = block->getBlockCoords(0);
	float yCoord = block->getBlockCoords(1);
	int xIndex = (int) floor(xCoord);
	int yIndex = (int) floor(yCoord);

	if(xIndex < 0 || xIndex >= (int)m_gridSize) {
		std::cout << "[ERROR Placer::snapFixedBlock] xIndex = "
				<< xIndex << " is out of bound"
				<< std::endl;
		return;
	}
	if(yIndex < 0 || yIndex >= (int)m_gridSize) {
		std::cout << "[ERROR Placer::snapFixedBlock] yIndex = "
				<< yIndex << " is out of bound"
				<< std::endl;
		return;
	}

	if(!m_snapOccupied[xIndex][yIndex])
		m_snapOccupied[xIndex][yIndex] = true;
	else
		std::cout << "[ERROR Placer::snapFixedBlock] Two fixed blocks assigned to same pre-fixed locations"
		<< std::endl;
}

void Placer::snapMovableBlock(Block *block) {

	if(block->getBlockInfo(2) == 0 ) {
		std::cout << "[ERROR Placer::snapMovableBlock] Trying to snap a non-movable block"
				<< std::endl;
		return;
	}

	float xCoord = block->getBlockCoords(0);
	float yCoord = block->getBlockCoords(1);
	int xIndex = (int) floor(xCoord);
	int yIndex = (int) floor(yCoord);

	if(xIndex < 0 || xIndex >= (int)m_gridSize) {
		std::cout << "[ERROR Placer::snapMovableBlock] xIndex = "
				<< xIndex << " is out of bound"
				<< std::endl;
		return;
	}
	if(yIndex < 0 || yIndex >= (int)m_gridSize) {
		std::cout << "[ERROR Placer::snapMovableBlock] yIndex = "
				<< yIndex << " is out of bound"
				<< std::endl;
		return;
	}

	// find closest available special block
	if(findClosestFreeBlock(xIndex, yIndex, block->getBlockInfo(1))) {
		m_snapOccupied[xIndex][yIndex] = true;
		block->setBlockCoords(xIndex+0.5, yIndex+0.5);
	}
	else
		std::cout << "[ERROR Placer::snapMovableBlock] No available spot to snap block"
		<< block->getBlockInfo(0) << std::endl;
}

bool Placer::findClosestFreeBlock(int &x, int &y, int type) {

	// reset m_snapVisited
	int gridSize = (int) m_gridSize;
	for(int i=0; i<gridSize; i++)
		for(int j=0; j<gridSize; j++)
			m_snapVisited[i][j] = false;

	// overwrite type = 0 if m_snapWithConstraint = false
	if(!m_snapWithConstraint)
		type = 0;

	// find closest free block by breadth-first search
	bool found = false;
	std::queue<std::pair<int,int>> q;
	q.push(std::make_pair(x,y));

	while(!q.empty()) {

		std::pair<int,int> p = q.front();
		int px = p.first;
		int py = p.second;
		q.pop();

		if(m_snapVisited[px][py])
			continue;

		if(!m_snapOccupied[px][py]) {
			if(type == 0 ||	(type == 1 && px % 5 == 1)) {
				x = px;
				y = py;
				found = true;
				break;
			}
		}

		m_snapVisited[px][py] = true;

		if(px - 1 >= 0 && !m_snapVisited[px-1][py])
			q.push(std::make_pair(px-1, py));
		if(px + 1 < gridSize && !m_snapVisited[px+1][py])
			q.push(std::make_pair(px+1, py));
		if(py - 1 >= 0 && !m_snapVisited[px][py-1])
			q.push(std::make_pair(px, py-1));
		if(py + 1 < gridSize && !m_snapVisited[px][py+1])
			q.push(std::make_pair(px, py+1));
	}
	return found;
}

void Placer::printBlockList() {

	std::cout << "[DEBUG Placer::printBlockList] Printing Block List Information"
			<< std::endl;
	for(unsigned int i=0; i<m_blockList.size(); i++) {
		std::cout << "\t";
		m_blockList[i]->printBlock();
		std::cout << std::endl;
	}
}

void Placer::printNetList() {

	std::cout << "[DEBUG Placer::printNetList] Printing Net List Information"
			<< std::endl;
	for(unsigned int i=0; i<m_netList.size(); i++) {
		std::cout << "\t";
		m_netList[i]->printNet();
		std::cout << std::endl;
	}
}

void Placer::printHPBBWL() {

	double sumWL = 0.0;
	for(unsigned int i=0; i<m_netList.size(); i++) {

		Net *net = m_netList[i];
		if(net->getId() < 0)
			break;
		const std::vector<Block*> &blocks = net->getBlocks();

		bool set = false;
		double xMin = 0.0;
		double xMax = 0.0;
		double yMin = 0.0;
		double yMax = 0.0;
		for(unsigned int j=0; j<blocks.size(); j++) {
			Block *block = blocks[j];
			if(!set) {
				xMin = block->getBlockCoords(0);
				xMax = block->getBlockCoords(0);
				yMin = block->getBlockCoords(1);
				yMax = block->getBlockCoords(1);
				set = true;
			}
			else {
				xMin = std::min(xMin, block->getBlockCoords(0));
				xMax = std::max(xMax, block->getBlockCoords(0));
				yMin = std::min(yMin, block->getBlockCoords(1));
				yMax = std::max(yMax, block->getBlockCoords(1));
			}
		}
		sumWL += ((xMax-xMin) + (yMax-yMin));
	}
	std::cout << "[RESULT Placer::printHPBBWL] Half-Perimeter Bounding Box Wire Length: "
			<< sumWL << std::endl;
}
