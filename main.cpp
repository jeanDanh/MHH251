#include "petriNet.h"
#include "symbolicPetriNet.h"
#include "deadlockDetector.h"

#include <chrono>
#include <iomanip>

long long estimateExplicitMemory(const vector<Marking>& visited, int numPlaces) {
    long long size = sizeof(visited); 
    long long markingSize = sizeof(Marking) + (numPlaces * sizeof(int));
    size += visited.size() * markingSize;
    return size;
}

int main() {
    try {
        // Task 1: Parser
        PetriNet net = loadPNML("simple_example.pnml");
        verify(net);
        printPetriNetInfo(net);

        
        // Task 2: BFS to enumerate all reachable markings from init
        vector<Marking> R = BFS(net);
        auto start1 = std::chrono::high_resolution_clock::now();
        cout << "\nReachable markings:\n";
        for (int i = 0; i < (int)R.size(); i++) {
            cout << i << ": ";
            printMarking(R[i]);
            cout << "\n";
        }

        auto end1 = std::chrono::high_resolution_clock::now();
        auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
        long long mem1 = estimateExplicitMemory(R, net.places.size());
        // Task 3: Symbolic computation
        SymbolicPetriNet symNet(net);
        auto start2 = std::chrono::high_resolution_clock::now();

        symNet.initialize();
        symNet.encodeInitialMarking();
        symNet.buildTransitionRelations();
        symNet.computeReachability();
        symNet.printResults();

        auto end2 = std::chrono::high_resolution_clock::now();
        auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
        long long mem2 = symNet.getBDDMemory();
        //Compare Performance
        std::cout << "==============PERFORMANCE COMPARISION==============" << std::endl;
        std::cout << left << setw(15) << "Method" << setw(25) << "Time(microseconds)" << setw(15) << "Memory(bytes)" << std::endl; 
        std::cout << left << setw(15) << "Explicit" << setw(25) << duration1.count() << setw(15) << mem1 << std::endl; 
        std::cout << left << setw(15) << "Symbolic" << setw(25) << duration2.count() << setw(15) << mem2 << std::endl;


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