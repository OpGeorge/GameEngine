#include "gen_appctrl.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>


int main() {

    try {
        gen::AppCtrl app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << "\n";
        return 1;
    }
    return 0;

}