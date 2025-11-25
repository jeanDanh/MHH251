#ifndef PETRINET_H
#define PETRINET_H
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "tinyxml2.h" //include thư viện ngoài parse pnml
using namespace tinyxml2; //namespace
using namespace std; //namespace

//Cấu trúc Place  
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
    vector<Place> places;
    vector<Transition> transitions;
    vector<Arc> arcs;
};

int findPlace(const vector<Place>& places, const string& id);
int findTransition(const vector<Transition>& transitions, const string& id);
PetriNet loadPNML(const string& filename);
void verify(const PetriNet& net);
void printPetriNetInfo(const PetriNet& net);

#endif // PETRINET_H