/*
 * Display.cpp
 *
 *  Created on: Oct 20, 2018
 *      Author: chentuju
 */

#include "Display.h"

Placer *pPlacer = NULL;
bool drawMovableBlocks = false;
bool drawRatsNest = true;
bool drawWithConstraint = true;

void openDisplay(Placer *placer) {

	pPlacer = placer;
	drawMovableBlocks = false;
	drawRatsNest = true;
	double gridSize = pPlacer->getGridSize();

	init_graphics("ECE1387 Assignment 2 - Analytical Placer (developed by Louis Chen)", WHITE);
	init_world(-0.2, -0.2, gridSize+0.2, gridSize+0.2);
	update_message("Press \"Proceed\" to start analytical placer.");
	event_loop(actOnButtonPress, NULL, NULL, drawScreen);
}

void updateDisplay() {

	if(!drawMovableBlocks) {
		drawMovableBlocks = true;
		create_button("Window", "Rat's Nest ON", actOnRatsNest);
		create_button("Rat's Nest ON", "Snap to Grid", actOnSnapToGrid);
		create_button("Snap to Grid", "Constraint ON", actOnSnapWithConstraint);
	}
	update_message("Press \"Proceed\" to spread. Press \"Rat's Nest\" to turn on/off rat's nest. Press \"Snap to Grid\" to perform fitting.");
	drawScreen();
	event_loop(actOnButtonPress, NULL, NULL, drawScreen);
}

void closeDisplay() {

	close_graphics();
}

void drawScreen() {

	set_draw_mode(DRAW_NORMAL);
	clearscreen();

	drawGrid();
	drawNets();
	drawBlocks();
}

void drawGrid() {

	double gridSize = pPlacer->getGridSize();
	setcolor(BLACK);
	drawrect(0.0, 0.0, gridSize, gridSize);
	for(unsigned int i=1; i<(unsigned int)gridSize; i++) {
		drawline(0.0, (float)i, gridSize, (float)i);
		drawline((float)i, 0.0, (float)i, gridSize);
	}
	setfontsize(10);
	for(unsigned int i=0; i<=(unsigned int)gridSize; i++) {
		for(unsigned int j=0; j<=(unsigned int)gridSize; j++) {
			std::ostringstream stm;
			stm << "(" << i << "," << j << ")";
			drawtext(i+0.2, j+0.1, stm.str().c_str(), 0.4);
		}
	}
}

void drawBlocks() {

	const std::vector<Block*> &blocks = pPlacer->getBlockList();
	for(unsigned int i=0; i<blocks.size(); i++) {

		Block *block = blocks[i];
		int blockID = block->getBlockInfo(0);
		int blockType = block->getBlockInfo(1);
		int blockMovable = block->getBlockInfo(2);
		double x = block->getBlockCoords(0);
		double y = block->getBlockCoords(1);
		std::ostringstream stm;
		stm << blockID;

		if(blockMovable == 0) {
			setcolor(DARKGREEN);
			fillrect(x-0.25, y-0.25, x+0.25, y+0.25);
			setcolor(BLACK);
			drawrect(x-0.25, y-0.25, x+0.25, y+0.25);
			setcolor(BLACK);
			setfontsize(15);
			drawtext(x, y, stm.str().c_str(), 100.0);
		}
		else if(blockMovable == 1 && drawMovableBlocks) {
			if((blockType) == 0)
				setcolor(RED);
			else
				setcolor(YELLOW);
			fillarc(x, y, 0.2, 0.0, 360.0);
			setcolor(BLACK);
			drawarc(x, y, 0.2, 0.0, 360.0);
			setcolor(BLACK);
			setfontsize(15);
			drawtext(x, y, stm.str().c_str(), 100.0);
		}
//		else if(blockMovable == 2 && drawMovableBlocks) {
//			setcolor(YELLOW);
//			fillarc(x, y, 0.1, 0.0, 360.0);
//			setcolor(BLACK);
//			drawarc(x, y, 0.1, 0.0, 360.0);
//			setcolor(BLACK);
//			setfontsize(15);
//			drawtext(x, y, stm.str().c_str(), 100.0);
//		}
	}
}

void drawNets() {

	if(drawRatsNest && drawMovableBlocks) {

		const std::vector<Block*> &blocks = pPlacer->getBlockList();
		for(unsigned int i=0; i<blocks.size(); i++) {

			Block *block = blocks[i];
			if(block->getBlockInfo(2) == 2)
				continue;
			const std::vector<Net*> &nets = block->getNets();
			for(unsigned int j=0; j<nets.size(); j++) {

				Net *net = nets[j];
				const std::vector<Block*> &adjBlocks = net->getBlocks();
				for(unsigned int k=0; k<adjBlocks.size(); k++) {

					Block *adjBlock = adjBlocks[k];
					if(adjBlock->getBlockInfo(2) == 2)
						continue;
//					if(adjBlock->getBlockInfo(2) == 2 ||
//							block->getBlockInfo(2) == 2) {
//						setcolor(MAGENTA);
//						drawline(block->getBlockCoords(0),
//								block->getBlockCoords(1),
//								adjBlock->getBlockCoords(0),
//								adjBlock->getBlockCoords(1));
//						continue;
//					}

					if(block->getBlockInfo(2) == 0 ||
							adjBlock->getBlockInfo(2) == 0)
						setcolor(BLUE);
					else
						setcolor(RED);
					drawline(block->getBlockCoords(0),
							block->getBlockCoords(1),
							adjBlock->getBlockCoords(0),
							adjBlock->getBlockCoords(1));
				}
			}
		}
	}
}

void actOnButtonPress(float x, float y) {
//   printf("User clicked a button at coordinates (%f, %f)\n", x, y);
}

void actOnMouseMove(float x, float y) {
//   printf ("Mouse move at (%f,%f)\n", x, y);
}

void actOnKeyPress(char c) {
//   printf ("Key press: %c\n", c);
}

void actOnRatsNest(void (*drawscreen_ptr) (void)) {

	if(drawRatsNest) {
		drawRatsNest = false;
		change_button_text("Rat's Nest ON", "Rat's Nest OFF");
	}
	else {
		drawRatsNest = true;
		change_button_text("Rat's Nest OFF", "Rat's Nest ON");
	}
	drawScreen();
}

void actOnSnapToGrid(void (*drawscreen_ptr) (void)) {

	pPlacer->invokeSnap();
	destroy_button("Snap to Grid");
	if(drawWithConstraint)
		destroy_button("Constraint ON");
	else
		destroy_button("Constraint OFF");
	drawScreen();
	update_message("Fitting is finished. Press \"Rat's Nest\" to turn on/off rat's nest. Press \"Proceed\" to exit the placer.");
}

void actOnSnapWithConstraint(void (*drawscreen_ptr) (void)) {

	bool snapWithConstraint = pPlacer->toggleSnapWithConstraint();
	drawWithConstraint = snapWithConstraint;
	if(drawWithConstraint)
		change_button_text("Constraint OFF", "Constraint ON");
	else
		change_button_text("Constraint ON", "Constraint OFF");
}
