#include "deadlockDetector.h"
#include <iostream>
#include <chrono>
#include <numeric>
#include <algorithm>

#include "ortools/linear_solver/linear_solver.h"

using namespace std;
using namespace std::chrono;
using namespace operations_research;

DeadlockDetector::DeadlockDetector(const PetriNet& petriNet, SymbolicPetriNet& symNet)
    : net(petriNet), symbolicNet(symNet), deadlockFound(false), detectionTime(0.0) {
    BDD_ops = symbolicNet.getBDDManager();
    deadlockMarking.resize(net.places.size(), 0);
}

DeadlockDetector::~DeadlockDetector() {}

bool DeadlockDetector::isTransitionEnabled(int transitionIdx, const vector<int>& marking) {
    string transId = net.transitions[transitionIdx].id;
    for (const auto& arc : net.arcs) {
        if (arc.target == transId) { // Arc P -> T
            int pIdx = -1;
            for(size_t i=0; i<net.places.size(); ++i) {
                if(net.places[i].id == arc.source) { pIdx = i; break; }
            }
            if (pIdx != -1 && marking[pIdx] < 1) return false;
        }
    }
    return true;
}

/*
 * Counter-Example Guided Abstraction Refinement (CEGAR):
 * 1. ILP Solver tìm một marking M thỏa mãn: "Không transition nào fire được" (Structural Deadlock).
 * 2. Check BDD: M có reachable không?
 * - Có: Deadlock thật -> Return True.
 * - Không: M là spurious (giả). Thêm constraint vào ILP để loại bỏ M. Quay lại B1.
 */
bool DeadlockDetector::detectDeadlock() {
    cout << "\n[Task 4] Bat dau tim deadlock (ILP + BDD)..." << endl;
    auto startTime = high_resolution_clock::now();

    // 1. Khởi tạo Solver
    std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
    if (!solver) {
        solver.reset(MPSolver::CreateSolver("CBC_MIXED_INTEGER_PROGRAMMING"));
    }
    if (!solver) {
        cerr << "ERROR: Khong khoi tao duoc ILP Solver (OR-Tools)!" << endl;
        return false;
    }

    int numPlaces = net.places.size();
    
    // 2. Tạo biến: x[i] là số token tại place i (Binary 0/1 vì là 1-safe Petri net)
    vector<MPVariable*> x(numPlaces);
    for (int i = 0; i < numPlaces; ++i) {
        x[i] = solver->MakeIntVar(0, 1, "x_" + net.places[i].id);
    }

    // 3. Tất cả transition đều KHÔNG THỂ FIRE (Disabled)
    // Một transition T bị disabled <=> Tồn tại ít nhất 1 input place P không có token.
    // Trong ILP với 1-safe net: Tổng token của input places <= (Số input places) - 1
    // Sum(x_p) <= |Inputs| - 1
    
    int constraintCount = 0;
    for (size_t t = 0; t < net.transitions.size(); ++t) {
        string transId = net.transitions[t].id;
        vector<int> inputIndices;

        // Tìm tất cả input places của transition t
        for (const auto& arc : net.arcs) {
            if (arc.target == transId) {
                for (int i = 0; i < numPlaces; ++i) {
                    if (net.places[i].id == arc.source) {
                        inputIndices.push_back(i);
                        break;
                    }
                }
            }
        }

        // Nếu transition không có input nào -> Luôn enabled -> Không bao giờ có deadlock
        if (inputIndices.empty()) {
            cout << "  Transition " << transId << " khong co input -> Luôn enabled -> NO DEADLOCK." << endl;
            return false; 
        }

        // Thêm ràng buộc: Sum(x_p) <= K - 1
        MPConstraint* ct = solver->MakeRowConstraint(-MPSolver::infinity(), inputIndices.size() - 1.0);
        for (int idx : inputIndices) {
            ct->SetCoefficient(x[idx], 1.0);
        }
        constraintCount++;
    }

    // 4. Vòng lặp CEGAR
    int iteration = 0;
    while (true) {
        iteration++;
        // Giải ILP
        const MPSolver::ResultStatus resultStatus = solver->Solve();

        // Nếu ILP vô nghiệm -> Không tồn tại trạng thái chết nào -> Không deadlock
        if (resultStatus != MPSolver::OPTIMAL && resultStatus != MPSolver::FEASIBLE) {
            cout << "[Iter " << iteration << "] ILP vo nghiem -> Done." << endl;
            break;
        }

        // Lấy marking ứng viên từ ILP
        vector<int> candidate(numPlaces);
        int onesCount = 0;
        for (int i = 0; i < numPlaces; ++i) {
            candidate[i] = (int)(x[i]->solution_value() + 0.5); 
            if (candidate[i] == 1) onesCount++;
        }

        // Kiểm tra marking này có Reachable không?
        if (symbolicNet.contains(candidate)) {
            deadlockFound = true;
            deadlockMarking = candidate;
            
            auto endTime = high_resolution_clock::now();
            detectionTime = duration_cast<milliseconds>(endTime - startTime).count();
            return true;
        } 
        else {
            // Marking này là Dead (cấu trúc) nhưng KHÔNG Reachable (Spurious).
            // Thêm "Cut Constraint" để loại bỏ nghiệm này khỏi không gian tìm kiếm ILP.
            // Canonical Cut: Sum(x_i | val=1) - Sum(x_j | val=0) <= (So luong so 1) - 1
            
            MPConstraint* cut = solver->MakeRowConstraint(-MPSolver::infinity(), onesCount - 1.0);
            for (int i = 0; i < numPlaces; ++i) {
                if (candidate[i] == 1) {
                    cut->SetCoefficient(x[i], 1.0);
                } else {
                    cut->SetCoefficient(x[i], -1.0);
                }
            }
        }
    }

    auto endTime = high_resolution_clock::now();
    detectionTime = duration_cast<milliseconds>(endTime - startTime).count();
    return false;
}

void DeadlockDetector::printResults() {
    cout << "\n========== TASK 4: DEADLOCK DETECTION ==========" << endl;
    cout << "Time: " << detectionTime << " ms" << endl;
    
    if (deadlockFound) {
        cout << "*** DEADLOCK DETECTED ***" << endl;
        cout << "Deadlock Marking: [ ";
        for (size_t i = 0; i < net.places.size(); i++) {
            if (deadlockMarking[i] == 1)
                cout << net.places[i].id << " ";
        }
        cout << "]" << endl;
    } else {
        cout << "*** NO DEADLOCK FOUND ***" << endl;
    }
    cout << "===================================================" << endl;
}

vector<int> DeadlockDetector::getDeadlockMarking() const {
    return deadlockMarking;
}