/*
 * Display.h
 *
 *  Created on: Oct 20, 2018
 *      Author: chentuju
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <iostream>
#include <sstream>
#include "Placer.h"
#include "graphics.h"

class Placer;

// Function for Placer::run to invoke
void openDisplay(Placer *placer);
void updateDisplay();
void closeDisplay();

// Callback functions for event-driven window handling.
void drawScreen();
void actOnButtonPress(float x, float y);
void actOnMouseMove(float x, float y);
void actOnKeyPress(char c);

// Callback functions for custom buttons
void actOnRatsNest(void (*drawscreen_ptr) (void));
void actOnSnapToGrid(void (*drawscreen_ptr) (void));
void actOnSnapWithConstraint(void (*drawscreen_ptr) (void));

// Helper functions
void drawGrid();
void drawBlocks();
void drawNets();

#endif /* DISPLAY_H_ */
