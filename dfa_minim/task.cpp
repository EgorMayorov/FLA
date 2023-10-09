#include "api.hpp"
#include <string>
#include <set>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

string T = "T";

set<string> find_reachable_iter (const DFA &d, const set<string> &reach) {
    string symbols = d.get_alphabet().to_string();
    set<string> reachable;
    reachable = reach;
    for (const string &s: reach) {
        for (const char &c: symbols) {
            if (d.has_trans(s, c)) {
                reachable.insert(d.get_trans(s, c));
            }
        }
    }
    return reachable;
}

DFA delete_unreachable_states (DFA &d) {
    set<string> all_states = d.get_states();
    string symbols = d.get_alphabet().to_string();
    string init_state = d.get_initial_state();
    set<string> reachable;
    reachable.insert(init_state);
    unsigned long size = 0;
    while (reachable.size() != size) {
        size = reachable.size();
        reachable = find_reachable_iter(d, reachable);
    }
    for (const string &s: all_states) {
        if (!reachable.count(s)) {
            d.delete_state(s);
        }
    }
    return d;
}

DFA delete_non_generating_states (DFA &d) {
    set<string> all_states = d.get_states();
    string symbols = d.get_alphabet().to_string();
    set<string> generating_states = d.get_final_states();
    unsigned long size = 0;
    while (generating_states.size() != size) {
        size = generating_states.size();
        for (const string &s: all_states) {
            for (const char &c: symbols) {
                if (d.has_trans(s, c)) {
                    if (generating_states.count(d.get_trans(s, c))) {
                        generating_states.insert(s);
                    }
                }
            }
        }
    }
    for (const string &s: all_states) {
        if (!generating_states.count(s)) {
            d.delete_state(s);
        }
    }
    return d;
}

DFA make_dfa_full (DFA &d) {
    set<string> all_states = d.get_states();
    string symbols = d.get_alphabet().to_string();
    bool is_new_state_created = false;
    for (const string &s: all_states) {
        for (const char &c: symbols) {
            if (!d.has_trans(s, c)) {
                if (!is_new_state_created) {
                    d.create_state(T);
                    for (const char &ch: symbols) {
                        d.set_trans(T, ch, T);
                    }
                    is_new_state_created = true;
                }
                d.set_trans(s, c, T);
            }
        }
    }
    return d;
}

vector<set<string>> split (const DFA &d, const set<string> &R, const set<string> &C, const char &c) {
    vector<set<string>> res;
    set<string> R1;
    set<string> R2;
    for (const string &s: R) {
        if (C.count(d.get_trans(s, c))) {
            R1.insert(s);   // trans to C
        } else {
            R2.insert(s);   // trans not to C
        }
    }
    res.push_back(R1);
    res.push_back(R2);
    return res;
}

set<set<string>> P_cycle (const DFA &d, const set<set<string>> &P, const set<string> &itS, const char &c, set<set<string>> &S) {
    set<set<string>> Pres = P;
    set<set<string>>::iterator R;
    for (R = P.begin(); R != P.end(); R++) {
        // split
        vector<set<string>> RR = split(d, *R, itS, c);
        set<string> R2 = RR.back();
        RR.pop_back();
        set<string> R1 = RR.back();
        RR.pop_back();
        if ((!R1.empty()) && (!R2.empty())) {
            Pres.erase(*R);
            Pres.insert(R1);
            Pres.insert(R2);
            S.insert(R1);
            S.insert(R2);
        }
    }
    return Pres;
}

DFA create_dfa (const DFA &d, const set<set<string>> &P) {
    string symbols = d.get_alphabet().to_string();
    set<string> all_states = d.get_states();
    set<string> final_states = d.get_final_states();
    string initial = d.get_initial_state();
    DFA res = DFA(symbols);
    int state_name = 0;
    map<set<string>, string> Q;
    for (const set<string> &ss: P) {
        Q[ss] = to_string(state_name);
        res.create_state(to_string(state_name));
        state_name++;
    }
    for (const set<string> &ss: P) {
        bool is_final = false;
        bool is_initial = false;
        for (const string &s: ss) {
            if (final_states.count(s)) {
                is_final = true;
            }
            if (s == initial) {
                is_initial = true;
            }
        }
        if (is_final){
            res.make_final(Q[ss]);
        }
        if (is_initial) {
            res.set_initial(Q[ss]);
        }
    }
    for (const set<string> &ss: P) {
        auto cur_s = ss.begin();
        for (const char &c: symbols) {
            string real_to_state;
            if (d.has_trans(*cur_s, c)) {
                real_to_state = d.get_trans(*cur_s, c);
            }
            for (const set<string> &to_st: P) {
                if (to_st.count(real_to_state)) {
                    res.set_trans(Q[ss], c, Q[to_st]);
                }
            }
        }
    }
    return res;
}

DFA build_new_dfa (DFA &d) {
    set<string> all_states = d.get_states();
    string symbols = d.get_alphabet().to_string();
    set<string> final_states = d.get_final_states();
    set<set<string>> P;
    set<set<string>> S;
    P.insert(final_states);
    set<string> non_final_states;
    for (const string &s: all_states) {
        if (!final_states.count(s)) {
            non_final_states.insert(s);
        }
    }
    S.insert(final_states);
    if (!non_final_states.empty()) {
        P.insert(non_final_states);
        S.insert(non_final_states);
    }
    while (!S.empty()) {
        auto itS = S.begin();
        for (const char &c: symbols) {
            P = P_cycle(d, P, *itS, c, S);
        }
        S.erase(*itS);
    }
    // P filled
    // create DFA from P
    d = create_dfa(d, P);
    return d;
}

DFA dfa_minim (DFA &d) {
    DFA myd = d;
    myd = delete_unreachable_states(myd);
    myd = delete_non_generating_states(myd);
    if (myd.get_states().empty()) {
        myd.create_state("0");
        return myd;
    }
    myd = make_dfa_full(myd);
    myd = build_new_dfa(myd);
    myd = delete_non_generating_states(myd);
    return myd;
}
