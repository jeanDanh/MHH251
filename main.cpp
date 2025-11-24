#include "petriNet.h"

int main() {
    // Task 1: parser
    try {
        PetriNet net = loadPNML("simple_example.pnml");
        verify(net);
        printPetriNetInfo(net);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    // end of task 1
    return 0;
}