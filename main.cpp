#include "gen_appctrl.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>


int main() {

	gen::AppCtrl mainApp{};


	try {
		mainApp.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;

	}

	return EXIT_SUCCESS;

}