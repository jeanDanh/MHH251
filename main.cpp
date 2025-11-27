#include "petriNet.h"
#include "symbolicPetriNet.h"
#include "deadlockDetector.h"


int main() {
    try {
        // Task 1: parser
        PetriNet net = loadPNML("simple_example.pnml");
        verify(net);
        printPetriNetInfo(net);
        
        std::cout << "\n========================================\n" << std::endl;
        
        // Task 3: Symbolic computation
        SymbolicPetriNet symNet(net);
        symNet.initialize();
        symNet.encodeInitialMarking();
        symNet.buildTransitionRelations();
        symNet.computeReachability();
        symNet.printResults();

        // Task 4: Deadlock detection
        DeadlockDetector detector(net, symNet);
        detector.detectDeadlock();
        detector.printResults();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}