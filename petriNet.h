#ifndef PETRINET_H
#define PETRINET_H
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "tinyxml2.h" //thư viện ngoài, dùng để parse file pnml
using namespace tinyxml2; //namespace 
using namespace std; //namespace

//các cấu trúc cấu thành  
struct Place {
    string id;
    string name;
    int initialMarking = 0; 
};
struct Transition {
    string id;
    string name;
};
struct Arc {
    string id;
    string source;
    string target;
    int weight = 1;
};

//PetriNet, 3 attrs: places, transitions, arcs
struct PetriNet {
    vector<Place> places; //mảng động chứa các Place, 1 attr PetriNet
    vector<Transition> transitions; //mảng động chứa Transition, 1 attr PetriNet
    vector<Arc> arcs;//mảng động chứa các đường nối (Place -> Transition) + (Transition -> Place), 1 attr PetriNet
};

struct Marking {
    vector<int> tokens;
    bool operator==(const Marking& other) const {
        return tokens == other.tokens;
    }
};

//các hàm có thể dùng, implemented ở petriNet.cpp
int findPlace(const vector<Place>& places, const string& id);
int findTransition(const vector<Transition>& transitions, const string& id);
PetriNet loadPNML(const string& filename);
void verify(const PetriNet& net);
void printPetriNetInfo(const PetriNet& net);

//===================================== Xây bảng in/out arcs,implemented ở petriNet.cpp =================================================
void buildTables(const PetriNet& net, vector<vector<pair<int,int>>>& inArcs, vector<vector<pair<int,int>>>& outArcs);
bool isEnabled(const Marking& M, int t, const vector<vector<pair<int,int>>>& inArcs);
Marking fire(const Marking& M, int t, const vector<vector<pair<int,int>>>& inArcs, const vector<vector<pair<int,int>>>& outArcs);
bool visitedHas(const vector<Marking>& visited, const Marking& M);
vector<Marking> BFS(const PetriNet& net);
void printMarking(const Marking& M);
#endif // PETRINET_H