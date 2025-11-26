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
};

struct PetriNet {
    vector<Place> places; //mảng động chứa các Place
    vector<Transition> transitions; //mảng động chứa Transition
    vector<Arc> arcs;//mảng động chứa các đường nối (Place -> Transition) + (Transition -> Place) 
};

//các hàm có thể dùng, implemented ở petriNet.cpp
int findPlace(const vector<Place>& places, const string& id);
int findTransition(const vector<Transition>& transitions, const string& id);
PetriNet loadPNML(const string& filename);
void verify(const PetriNet& net);
void printPetriNetInfo(const PetriNet& net);

#endif // PETRINET_H