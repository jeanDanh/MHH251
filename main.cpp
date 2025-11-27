#include "petriNet.h"
#include "symbolicPetriNet.h"

int main() {
    try {
        // Task 1: Parser
        PetriNet net = loadPNML("simple_example.pnml");
        verify(net);
        printPetriNetInfo(net);

        
        // Task 2: BFS to enumerate all reachable markings from init
        vector<Marking> R = BFS(net);
        cout << "\nReachable markings:\n";
        for (int i = 0; i < (int)R.size(); i++) {
            cout << i << ": ";
            printMarking(R[i]);
            cout << "\n";
        }
        // Task 3: Symbolic computation
        SymbolicPetriNet symNet(net);
        symNet.initialize();
        symNet.encodeInitialMarking();
        symNet.buildTransitionRelations();
        symNet.computeReachability();
        symNet.printResults();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}