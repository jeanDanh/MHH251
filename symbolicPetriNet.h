#ifndef SYMBOLIC_PETRI_NET_H
#define SYMBOLIC_PETRI_NET_H

#include "petriNet.h"
#include "cudd.h"
#include <map>
#include <set>

class SymbolicPetriNet {
public:
    SymbolicPetriNet(const PetriNet& petriNet);
    ~SymbolicPetriNet();
    void initialize();
    void encodeInitialMarking();
    void buildTransitionRelations();
    void computeReachability();
    void printResults();
private:
    PetriNet net;
    DdManager* mgr;
    map<string, int> placeToCurrentVar;
    map<string, int> placeToNextVar;
    DdNode* initialState;
    DdNode* reachableStates;
    vector<DdNode*> transitionRelations;
    int numPlaces;
    int numTransitions;
private:
    DdNode* getTransitionRelation(int transIdx);
    DdNode* imageComputation(DdNode* states);
};
#endif