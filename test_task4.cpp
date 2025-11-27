#include "petriNet.h"
#include "symbolicPetriNet.h"
#include "deadlockDetector.h"
#include <iostream>
#include <cassert>

using namespace std;

void testLoadAndDetect() {
    cout << "\n[TEST 1] Loading simple_example.pnml and detecting deadlock..." << endl;
    try {
        // Lưu ý: Đảm bảo file simple_example.pnml tồn tại cùng thư mục chạy
        PetriNet net = loadPNML("simple_example.pnml");
        
        SymbolicPetriNet symNet(net);
        symNet.initialize();
        symNet.encodeInitialMarking();
        symNet.buildTransitionRelations();
        symNet.computeReachability();
        
        DeadlockDetector detector(net, symNet);
        bool hasDeadlock = detector.detectDeadlock();
        cout << "Deadlock detected? " << (hasDeadlock ? "YES" : "NO") << endl;
        detector.printResults();
        
        cout << "[TEST 1] Finished." << endl;
    } catch (const exception& e) {
        cerr << "Error in Test 1: " << e.what() << endl;
    }
}

void testManualDeadlock() {
    cout << "\n[TEST 2] Creating manual Petri Net with known deadlock..." << endl;
    /*
     * Simple Deadlock Scenario:
     * P1 (1 token) -> T1 -> P2
     * P2 has no outgoing transitions.
     * * Initial: {P1=1, P2=0}
     * Fire T1 -> {P1=0, P2=1} -> DEADLOCK
     */
    
    PetriNet net;
    
    // Places
    Place p1; p1.id = "p1"; p1.name = "P1"; p1.initialMarking = 1;
    Place p2; p2.id = "p2"; p2.name = "P2"; p2.initialMarking = 0;
    net.places.push_back(p1);
    net.places.push_back(p2);
    
    // Transitions
    Transition t1; t1.id = "t1"; t1.name = "T1";
    net.transitions.push_back(t1);
    
    // Arcs
    Arc a1; a1.id = "a1"; a1.source = "p1"; a1.target = "t1";
    Arc a2; a2.id = "a2"; a2.source = "t1"; a2.target = "p2";
    net.arcs.push_back(a1);
    net.arcs.push_back(a2);
    
    try {
        SymbolicPetriNet symNet(net);
        symNet.initialize();
        symNet.encodeInitialMarking();
        symNet.buildTransitionRelations();
        symNet.computeReachability();
        
        DeadlockDetector detector(net, symNet);
        bool hasDeadlock = detector.detectDeadlock();
        detector.printResults();
        
        if (hasDeadlock) {
            // --- ĐOẠN SỬA LỖI Ở ĐÂY ---
            // Lấy ra struct Marking, sau đó truy cập vào thuộc tính .tokens (là vector<int>)
            vector<int> deadlockMarking = detector.getDeadlockMarking().tokens;
            
            // Expecting {P1=0, P2=1}
            // Note: Order depends on how places are stored/indexed.
            // Here p1 is index 0, p2 is index 1.
            if (deadlockMarking.size() == 2 && deadlockMarking[0] == 0 && deadlockMarking[1] == 1) {
                cout << "[TEST 2] PASSED: Correct deadlock marking found." << endl;
            } else {
                cout << "[TEST 2] FAILED: Incorrect deadlock marking." << endl;
                cout << "Found: ";
                for(int t : deadlockMarking) cout << t << " ";
                cout << endl;
            }
        } else {
            cout << "[TEST 2] FAILED: Deadlock NOT detected." << endl;
        }
        
    } catch (const exception& e) {
        cerr << "Error in Test 2: " << e.what() << endl;
    }
}

int main() {
    testLoadAndDetect();
    testManualDeadlock();
    return 0;
}