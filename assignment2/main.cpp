/*
 * main.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: chentuju
 */

#include <iostream>
#include "Placer.h"

int main(int argc, char **argv) {

    if(argc == 1) {
        std::cout << "No circuit file specify!" << std::endl;
        return 0;
    }

	Placer placer;
	placer.init(argv[1]);
	placer.run();

	return 0;
}
