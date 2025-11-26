#include "petriNet.h"

int findPlace(const vector<Place>& places, const string& id) {
    for (size_t i = 0; i < places.size(); ++i) {
        if (places[i].id == id) return i;
    }
    return -1;
}

int findTransition(const vector<Transition>& transitions, const string& id) {
    for (size_t i = 0; i < transitions.size(); ++i) {
        if (transitions[i].id == id) return i;
    }
    return -1;
}

/*
Chức năng: load file PNML
Đầu vào: const string& filename
Đầu ra: PetriNet
(Ctrl+click để xem chi tiết)
*/
PetriNet loadPNML(const string& filename) {
    PetriNet net;                                                                //PetriNet sẽ được trả về
    XMLDocument doc;                                                             //XMLDocument offer hàng ngàn tính năng đọc XML và thao tác trên cấu trúc XML đã parsed
    if (doc.LoadFile(filename.c_str()) != XML_SUCCESS)                           //Hàm LoadFile() làm 3 việc: đọc file, parse file, check lỗi
        throw runtime_error("Cannot open PNML file or XML format error!");       //quăng lỗi (nếu ko đọc được) và chương trình dừng lại.

    XMLElement* pnml = doc.FirstChildElement("pnml");                            //địa chỉ dẫn đến <pnml> tag trong simple_example>pnml
    if (!pnml) throw runtime_error("Invalid PNML: missing <pnml>");              //quăng lỗi ko thấy <pnml>

    XMLElement* netTag = pnml->FirstChildElement("net");                         //địa chỉ <net> tag trong simple_example.pnml
    if (!netTag) throw runtime_error("Invalid PNML: missing <net>");             //quăng lỗi ko thấy <net>

    XMLElement* rootNode = netTag->FirstChildElement("page");                    //địa chỉ <page> tag 
    if (!rootNode) rootNode = netTag;                                            //quăng lỗi ko thấy <page>

    // ----- Đọc Places -----
    
    for (XMLElement* p = rootNode->FirstChildElement("place"); p; p = p->NextSiblingElement("place")) {
        //vòng lặp p từ <place> đầu tiên đến <place> cuối cùng. 
        //Giải thích: p đi từ first child, lặp đi lặp lại duyệt qua các sibling rồi kết thúc khi p == nullptr.
        
        Place place;
        const char* idAttr = p->Attribute("id");                                 //
        if (!idAttr) continue;
        place.id = idAttr;

        // name
        if (auto nameTag = p->FirstChildElement("name")) {
            if (auto textTag = nameTag->FirstChildElement("text"))
                place.name = textTag->GetText();
        }

        // initial marking
        if (auto markTag = p->FirstChildElement("initialMarking")) {
            if (auto textTag = markTag->FirstChildElement("text")) {
                try {
                    place.initialMarking = stoi(textTag->GetText());
                } catch (...) { place.initialMarking = 0; }
            }
        } else {
            place.initialMarking = 0;
        }

        net.places.push_back(place);
    }

    // ----- Đọc Transitions -----
    for (XMLElement* t = rootNode->FirstChildElement("transition"); t; t = t->NextSiblingElement("transition")) {
        Transition trans;
        const char* idAttr = t->Attribute("id");
        if (!idAttr) continue;
        trans.id = idAttr;

        if (auto nameTag = t->FirstChildElement("name")) {
            if (auto textTag = nameTag->FirstChildElement("text"))
                trans.name = textTag->GetText();
        }

        net.transitions.push_back(trans);
    }

    // ----- Đọc Arcs -----
    for (XMLElement* a = rootNode->FirstChildElement("arc"); a; a = a->NextSiblingElement("arc")) {
        Arc arc;
        const char* idAttr = a->Attribute("id");
        const char* srcAttr = a->Attribute("source");
        const char* tgtAttr = a->Attribute("target");

        if (idAttr) arc.id = idAttr;
        if (srcAttr) arc.source = srcAttr;
        if (tgtAttr) arc.target = tgtAttr;

        if (!arc.source.empty() && !arc.target.empty()) {
            net.arcs.push_back(arc);
        }
    }

    return net;
}

void verify(const PetriNet& net) {
    // Kiểm tra trùng id place
    for (size_t i = 0; i < net.places.size(); i++) {
        for (size_t j = i + 1; j < net.places.size(); j++) {
            if (net.places[i].id == net.places[j].id)
                throw runtime_error("Duplicate place ID detected: " + net.places[i].id);
        }
    }

    // Kiểm tra arc tham chiếu hợp lệ
    for (const auto& arc : net.arcs) {
        bool sourceExists = false;
        bool targetExists = false;

        // Check Source
        if (findPlace(net.places, arc.source) != -1) sourceExists = true;
        else if (findTransition(net.transitions, arc.source) != -1) sourceExists = true;

        if (!sourceExists) throw runtime_error("Arc " + arc.id + " source not found: " + arc.source);

        // Check Target
        if (findPlace(net.places, arc.target) != -1) targetExists = true;
        else if (findTransition(net.transitions, arc.target) != -1) targetExists = true;

        if (!targetExists) throw runtime_error("Arc " + arc.id + " target not found: " + arc.target);
    }
}

void printPetriNetInfo(const PetriNet& net) {
    cout << "\n================ PETRI NET INFO ================" << endl;

    // 1. In danh sách Places
    cout << "--- "<<net.places.size() << " places ---" << endl;
    if (net.places.empty()) cout << "  (No places found)" << endl;
    for (const auto& p : net.places) {
        cout << "  ID: " << p.id 
             << " | Tokens: " << p.initialMarking 
             << " | Name: " << (p.name.empty() ? "(no name)" : p.name) << endl;
    }

    // 2. In danh sách Transitions
    cout << "\n--- " << net.transitions.size() << " transitions ---" << endl;
    if (net.transitions.empty()) cout << "  (No transitions found)" << endl;
    for (const auto& t : net.transitions) {
        cout << "  ID: " << t.id 
             << " | Name: " << (t.name.empty() ? "(no name)" : t.name) << endl;
    }

    // 3. In danh sách Arcs
    cout << "\n--- " << net.arcs.size() << " arcs ---" << endl;
    if (net.arcs.empty()) cout << "  (No arcs found)" << endl;
    for (const auto& a : net.arcs) {
        cout << "  ID: " << a.id 
             << " | " << a.source << " -> " << a.target << endl;
    }
    cout << "================================================" << endl;
}