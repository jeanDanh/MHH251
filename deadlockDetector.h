#ifndef DEADLOCK_DETECTOR_H
#define DEADLOCK_DETECTOR_H

#include "petriNet.h"
#include "symbolicPetriNet.h"
#include "cudd.h"
#include <vector>
#include <string>
#include <map>

class DeadlockDetector {
public:
    DeadlockDetector(const PetriNet& petriNet, SymbolicPetriNet& symNet);
    ~DeadlockDetector();
    bool detectDeadlock();
    void printResults();
    Marking getDeadlockMarking() const;

private:
    const PetriNet& net;                    
    SymbolicPetriNet& symbolicNet;          
    DdManager* BDD_ops;                     
    
    Marking deadlockMarking;
    bool deadlockFound;                      
    double detectionTime;     
    
    int numPlaces;
    int numTransitions;

    bool isTransitionEnabled(int transitionIdx, const vector<int>& marking);
};

#endif