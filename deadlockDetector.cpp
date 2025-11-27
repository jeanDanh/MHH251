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
    deadlockFound = false;
    numPlaces = net.places.size();
    numTransitions = net.transitions.size();
    
    deadlockMarking.tokens.assign(numPlaces, 0);
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
    // đo runtime
    auto start = high_resolution_clock::now();
    std::cout << "[Task 4] Bat dau tim deadlock (ILP + BDD)..." << std::endl;

    std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
    if (!solver) {
        std::cout << "[Task 4] Warning: SCIP not available, switching to CBC..." << std::endl;
        solver.reset(MPSolver::CreateSolver("CBC_MIXED_INTEGER_PROGRAMMING"));
    }

    if (!solver) {
        std::cerr << "[Task 4] Error: Neither SCIP nor CBC solver available." << std::endl;
        return false;
    }

    // tạo VARIABLES cho PETRI
    // Mỗi Place p tương ứng với một biến x_p trong bài toán ILP.
    // Vì mạng là 1-safe (theo đề bài), số token tại mỗi place chỉ có thể là 0 hoặc 1.
    // Do đó, ta tạo biến nhị phân (Integer Var từ 0 đến 1).
    std::vector<MPVariable*> vars;
    for (int i = 0; i < numPlaces; ++i) {
        // Biến x_0, x_1, ... tương ứng với token tại Place 0, Place 1...
        vars.push_back(solver->MakeIntVar(0.0, 1.0, "x_" + std::to_string(i)));
    }

    // Một trạng thái là "Dead" nếu KHÔNG CÓ transition nào kích hoạt được (disabled).
    // Transition t bị disable khi: Tổng số token ở các Input Place < Tổng trọng số cung đầu vào.
    // Công thức: Sum(weight * x_p) <= TotalInputWeight - 1
    
    for (const auto& t : net.transitions) {
        double totalInputWeight = 0;
        std::vector<std::pair<int, double>> inputPlaces;

        // Duyệt qua các cung (arc) để tìm Input Places của transition t
        for (const auto& arc : net.arcs) {
            if (arc.target == t.id) { // Nếu cung đi vào t
                int pIdx = findPlace(net.places, arc.source);
                if (pIdx != -1) {
                    double w = (double)arc.weight;
                    inputPlaces.push_back({pIdx, w});
                    totalInputWeight += w;
                }
            }
        }

        // Nếu transition không có đầu vào -> Luôn enabled -> Mạng không bao giờ deadlock.
        if (inputPlaces.empty()) {
            continue; 
        }

        // Tạo constraint cho transition t:
        // Ví dụ: t cần p1 + p2 >= 2 để chạy. Muốn t disable thì p1 + p2 <= 1.
        // MakeRowConstraint tạo biểu thức: -vocung <= ... <= (totalWeight - 1)
        MPConstraint* ct = solver->MakeRowConstraint(-MPSolver::infinity(), totalInputWeight - 1.0);
        
        // Thêm hệ số cho các biến vào constraint
        for (const auto& input : inputPlaces) {
            ct->SetCoefficient(vars[input.first], input.second);
        }
    }

    // CEGAR (TÌM KIẾM - KIỂM TRA - LOẠI BỎ)
    bool foundRealDeadlock = false;

    while (true) {
        // giải ILP để tìm một "Candidate Deadlock" (Trạng thái chết tiềm năng)
        MPSolver::ResultStatus resultStatus = solver->Solve();

        // Nếu Solver không tìm ra nghiệm -> Không còn trạng thái chết nào -> Hệ thống an toàn.
        if (resultStatus != MPSolver::OPTIMAL && resultStatus != MPSolver::FEASIBLE) {
            std::cout << "[Task 4] Khong tim thay (hoac khong con) trang thai Dead." << std::endl;
            break;
        }

        // lấy nghiệm TỪ SOLVER
        std::vector<int> candidate(numPlaces);
        int activeTokens = 0; // Đếm số lượng place có token (giá trị = 1)
        for (int i = 0; i < numPlaces; ++i) {
            // Lấy giá trị biến x_i 
            int val = (int)(vars[i]->solution_value() + 0.5);
            candidate[i] = val;
            if (val > 0) activeTokens++;
        }

        // check REACHABILITY = BDD (Task 3)
        // check marking 'candidate' này có nằm trong tập reachableStates?"
        if (symbolicNet.contains(candidate)) {
            // === case 1: TÌM THẤY DEADLOCK THẬT ===
            deadlockFound = true;
            deadlockMarking.tokens = candidate;
            foundRealDeadlock = true;
            std::cout << "[Task 4] DA TIM THAY DEADLOCK (Reachable)." << std::endl;
            break; // Thoát vòng lặp ngay lập tức
        } else {
            // === case 2: DEADLOCK GIẢ (SPURIOUS) ===
            std::cout << "[Task 4] Phat hien Spurious Deadlock (Unreachable). Them rang buoc loai bo..." << std::endl;
            // Mục tiêu: Bắt buộc Solver lần sau KHÔNG ĐƯỢC trả về đúng bộ (x_0...x_n) này nữa.
            // Công thức Canonical Cut cho biến nhị phân:
            // (Tổng các biến đang bằng 1) - (Tổng các biến đang bằng 0) <= (Số lượng biến bằng 1) - 1
            // Ý nghĩa: Ít nhất 1 biến phải đảo giá trị (0->1 hoặc 1->0).
            
            MPConstraint* cut = solver->MakeRowConstraint(-MPSolver::infinity(), activeTokens - 1.0);
            for (int i = 0; i < numPlaces; ++i) {
                if (candidate[i] > 0) {
                    // Nếu biến này đang là 1 trong candidate, hệ số là +1
                    cut->SetCoefficient(vars[i], 1.0);
                } else {
                    // Nếu biến này đang là 0 trong candidate, hệ số là -1
                    cut->SetCoefficient(vars[i], -1.0);
                }
            }
            // Solver tìm nghiệm khác tránh nghiệm vừa loại bỏ.
        }
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    detectionTime = duration.count();

    return foundRealDeadlock;
}

void DeadlockDetector::printResults() {
    std::cout << "========== TASK 4: DEADLOCK DETECTION ==========" << std::endl;
    if (deadlockFound) {
        std::cout << "*** DEADLOCK DETECTED ***" << std::endl;
        std::cout << "Deadlock Marking: [ ";
        for (size_t i = 0; i < net.places.size(); i++) {
            if (deadlockMarking.tokens[i] > 0) {
                // In ra tên place nếu có token
                std::cout << net.places[i].id << "(" << deadlockMarking.tokens[i] << ") ";
            }
        }
        std::cout << "]" << std::endl;
    } else {
        std::cout << "No deadlock found." << std::endl;
    }
    std::cout << "===================================================" << std::endl;
}

Marking DeadlockDetector::getDeadlockMarking() const { 
    return deadlockMarking; 
}