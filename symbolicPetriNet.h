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
    bool contains(const vector<int>& marking);
    void printResults();
    DdManager* getBDDManager() const { return BDD_ops; }
    long long getBDDMemory() const { return Cudd_ReadMemoryInUse(BDD_ops);}
private:
    PetriNet net; //the petri net
    DdManager* BDD_ops; //pointer to DdManager utilities to get useful BDD operations
    map<string, int> placeToCurrentVar;//std::map uses a red black tree to sort 
    map<string, int> placeToNextVar;
    DdNode* initialState;
    DdNode* reachableStates;
    vector<DdNode*> transitionRelations; //many many addresses of transition relations
    int numPlaces;
    int numTransitions;
private:
    DdNode* getTransitionRelation(int transIdx);
    DdNode* imageComputation(DdNode* states);
};
#endif