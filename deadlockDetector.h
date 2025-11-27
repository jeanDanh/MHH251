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
    vector<int> getDeadlockMarking() const;

private:
    const PetriNet& net;                    
    SymbolicPetriNet& symbolicNet;          
    DdManager* BDD_ops;                     
    
    vector<int> deadlockMarking;            
    bool deadlockFound;                      
    double detectionTime;                    

    bool isTransitionEnabled(int transitionIdx, const vector<int>& marking);
};

#endif