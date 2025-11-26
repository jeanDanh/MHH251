#include "symbolicPetriNet.h"
#include <iostream>

SymbolicPetriNet::SymbolicPetriNet(const PetriNet& petriNet){
    this->net = petriNet;
    this->BDD_ops = nullptr;
    this->initialState = nullptr;
    this->reachableStates = nullptr;
    this->numPlaces = petriNet.places.size(); 
    this->numTransitions = petriNet.transitions.size();
}

SymbolicPetriNet::~SymbolicPetriNet() {
    if (BDD_ops) {
        Cudd_Quit(BDD_ops);
    }
}

void SymbolicPetriNet::initialize() {
    int numVars = 2 * numPlaces;
    BDD_ops = Cudd_Init(numVars, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    
    if (!BDD_ops) {
        throw std::runtime_error("Failed to initialize CUDD");
    }
    
    std::cout << "\n[Task 3] Initialized BDD with " << numVars << " variables" << std::endl;
    
    // Map places to variables
    for (int i = 0; i < numPlaces; i++) {
        placeToCurrentVar[net.places[i].id] = i;
        placeToNextVar[net.places[i].id] = i + numPlaces;
    }
}

void SymbolicPetriNet::encodeInitialMarking() {
    initialState = Cudd_ReadOne(BDD_ops);
    Cudd_Ref(initialState);
    
    std::cout << "[Task 3] Encoded initial marking" << std::endl;
}

void SymbolicPetriNet::buildTransitionRelations() {
    std::cout << "[Task 3] Building transition relations..." << std::endl;
    for(int t=0;t<numTransitions;t++){
        DdNode * relation = getTransitionRelation(t);
        transitionRelations.push_back(relation);
        Cudd_Ref(relation);
    }
    cout<< "[Task 3] Built " <<transitionRelations.size()<<" transition relations"<<endl;
}

DdNode* SymbolicPetriNet::getTransitionRelation(int transIdx) {
    /*
     * What is a Transition Relation?
     * 
     * A transition relation is a BDD that represents how a transition
     * changes the state of the Petri net.
     * 
     * For a transition t with:
     *   - Input places (where tokens are consumed)
     *   - Output places (where tokens are produced)
     * 
     * The relation encodes:
     *   1. Precondition: Input places must have tokens (in current state)
     *   2. Effect: How the marking changes (current → next state)
     * 
     * Example: p1 ---> t1 ---> p2
     *   Current state: p1=1, p2=0
     *   Next state:    p1=0, p2=1
     */
    
    std::string transId = net.transitions[transIdx].id;
    std::set<std::string> inputPlaces; 
    std::set<std::string> outputPlaces; 
    
    for (const auto& arc : net.arcs) {
        if (arc.target == transId) {
            inputPlaces.insert(arc.source);
        }
        if (arc.source == transId) {
            outputPlaces.insert(arc.target);
        }
    }
    
    DdNode* relation = Cudd_ReadOne(BDD_ops);
    Cudd_Ref(relation);
    
    for (const auto& placeId : inputPlaces) {
        int currentVar = placeToCurrentVar[placeId];
        DdNode* var = Cudd_bddIthVar(BDD_ops, currentVar);
        DdNode* temp = Cudd_bddAnd(BDD_ops, relation, var);
        Cudd_Ref(temp);
        Cudd_RecursiveDeref(BDD_ops, relation);
        relation = temp;
    }
    
    for (int i = 0; i < numPlaces; i++) {
        std::string placeId = net.places[i].id;
        int currentVar = placeToCurrentVar[placeId];
        int nextVar = placeToNextVar[placeId];
        
        bool isInput = inputPlaces.count(placeId) > 0;
        bool isOutput = outputPlaces.count(placeId) > 0;
        
        DdNode* currentVarNode = Cudd_bddIthVar(BDD_ops, currentVar);
        DdNode* nextVarNode = Cudd_bddIthVar(BDD_ops, nextVar);
        DdNode* effect;
        
        if (isInput && isOutput) {
            effect = Cudd_bddXnor(BDD_ops, currentVarNode, nextVarNode);
        } else if (isInput && !isOutput) {
            effect = Cudd_Not(nextVarNode);
        } else if (!isInput && isOutput) {
            effect = nextVarNode;
        } else {
            effect = Cudd_bddXnor(BDD_ops, currentVarNode, nextVarNode);
        }
        
        DdNode* temp = Cudd_bddAnd(BDD_ops, relation, effect);
        Cudd_Ref(temp);
        Cudd_RecursiveDeref(BDD_ops, relation);
        relation = temp;
    }
    return relation;
}


void SymbolicPetriNet::computeReachability() {
    std::cout << "[Task 3] Computing reachability..." << std::endl;
    
    reachableStates = initialState;
    Cudd_Ref(reachableStates);
    
    int iteration = 0;
    
    while (true) {
        iteration++;
        
        DdNode* newStates = imageComputation(reachableStates);
        Cudd_Ref(newStates);
        
        DdNode* novel = Cudd_bddAnd(BDD_ops, newStates, Cudd_Not(reachableStates));
        Cudd_Ref(novel);
        
        bool foundNew = (novel != Cudd_ReadLogicZero(BDD_ops));
        
        Cudd_RecursiveDeref(BDD_ops, novel);
        
        if (!foundNew) {
            Cudd_RecursiveDeref(BDD_ops, newStates);
            std::cout << "[Task 3] Fixed point reached at iteration " 
                      << iteration << std::endl;
            break;
        }
        
        DdNode* temp = Cudd_bddOr(BDD_ops, reachableStates, newStates);
        Cudd_Ref(temp);
        Cudd_RecursiveDeref(BDD_ops, reachableStates);
        Cudd_RecursiveDeref(BDD_ops, newStates);
        reachableStates = temp;
        
        if (iteration > 1000) {
            std::cout << "[Task 3] Warning: Stopped at iteration limit" << std::endl;
            break;
        }
    }
}

DdNode* SymbolicPetriNet::imageComputation(DdNode* states) {
    // TODO: Implement
    DdNode * result = Cudd_ReadLogicZero(BDD_ops);
    Cudd_Ref(result);

    for (const auto& transRel : transitionRelations) {
        DdNode* temp = Cudd_bddAnd(BDD_ops, states, transRel);
        Cudd_Ref(temp);
        
        for (int i = 0; i < numPlaces; i++) {
            int currentVar = placeToCurrentVar[net.places[i].id];
            DdNode* cube = Cudd_bddIthVar(BDD_ops, currentVar);
            DdNode* temp2 = Cudd_bddExistAbstract(BDD_ops, temp, cube);
            Cudd_Ref(temp2);
            Cudd_RecursiveDeref(BDD_ops, temp);
            temp = temp2;
        }
        
        int* permutation = new int[2 * numPlaces];
        for (int i = 0; i < 2 * numPlaces; i++) {
            permutation[i] = i;  // Identity by default
        }

        for (int i = 0; i < numPlaces; i++) {
            std::string placeId = net.places[i].id;
            int currentVar = placeToCurrentVar[placeId];
            int nextVar = placeToNextVar[placeId];
            permutation[nextVar] = currentVar;
        }
        
        DdNode* renamed = Cudd_bddPermute(BDD_ops, temp, permutation);
        Cudd_Ref(renamed);
        delete[] permutation;
        Cudd_RecursiveDeref(BDD_ops, temp);
        
        DdNode* newResult = Cudd_bddOr(BDD_ops, result, renamed);
        Cudd_Ref(newResult);
        Cudd_RecursiveDeref(BDD_ops, result);
        Cudd_RecursiveDeref(BDD_ops, renamed);
        result = newResult;
    }
    
    return result;
}

void SymbolicPetriNet::printResults() {
    std::cout << "\n========== TASK 3: SYMBOLIC REACHABILITY ==========" << std::endl;
    std::cout << "Number of places: " << numPlaces << std::endl;
    std::cout << "Number of transitions: " << numTransitions << std::endl;
    
    // Count reachable states
    double stateCount = Cudd_CountMinterm(BDD_ops, reachableStates, numPlaces);
    std::cout << "Number of reachable states: " << stateCount << std::endl;
    
    std::cout << "===================================================" << std::endl;
}