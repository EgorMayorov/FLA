#include "api.hpp"
#include <string>
#include <map>
#include <iostream>
#include <set>

using std::string;
using std::map;
using std::pair;
using std::make_pair;
using std::set;
using std::cout;
using std::endl;

class OwnDFA {
private:
    string const START = "START";
    string const FINAL = "FINAL";
    string const EPS;
    string init_State;
    map<pair<string, string>, string> rules;
public:
    string find_string (const string &s1, const string &s2) {
        map<pair<string, string>, string>::iterator it;
        string res;
        for (it = rules.begin(); it != rules.end(); it++) {
            if ((it->first.first == s1) && (it->first.second == s2)) {
                res = it->second;
                return res;
            }
        }
        return res;
    }

    string dfa2re (DFA &d) {
        auto final_states = d.get_final_states();
        init_State = d.get_initial_state();
        auto alphabet = d.get_alphabet();
        auto states = d.get_states();
        for (const string &curr_state: states) {
            for (const char symbol: alphabet) {
                if (d.has_trans(curr_state, symbol)) {
//                    cout << curr_state << symbol << endl;
                    if (!rules[make_pair(curr_state, d.get_trans(curr_state, symbol))].empty()) {
                        rules[make_pair(curr_state, d.get_trans(curr_state, symbol))] += "|";
                        rules[make_pair(curr_state, d.get_trans(curr_state, symbol))] += string(1, symbol);
                    } else {
                        rules[make_pair(curr_state, d.get_trans(curr_state, symbol))] = string(1, symbol);
                    }
                }
            }
            if (d.is_final(curr_state)) {
                rules[make_pair(curr_state, FINAL)] = EPS;
            }
        }

//        cout << "dictionary" << endl;
//        map<pair<string, string>, string>::iterator prd;
//        for (prd = rules.begin(); prd != rules.end(); prd++) {
//            cout << prd->first.first << "  " << prd->second << "  " << prd->first.second << endl;
//        }
//        cout << "dictionary done" << endl;

        set<string> deleted;
        for (const string &curr_state: states) {
            if (curr_state != init_State) {
//                cout << "deleted states:  ";
//                for (const string &s: deleted) cout << s << "  ";
//                cout << endl;
                set<string> in_states;
                set<string> out_states;
                map<pair<string, string>, string>::iterator it;
                for (it = rules.begin(); it != rules.end(); it++) {
                    if ((it->first.second == curr_state) && (!deleted.count(it->first.first))) {
                        in_states.insert(it->first.first);
                    }
                    if ((it->first.first == curr_state) && (!deleted.count(it->first.second))) {
                        out_states.insert(it->first.second);
                    }
                }

//                cout << "current state:  " << curr_state << endl;
//                cout << "in_states:" << endl;
//                for (const string &s: in_states) cout << s << "  ";
//                cout << endl << "out_states:" << endl;
//                for (const string &s: out_states) cout << s << "  ";
//                cout << endl;

                for (const string &st1: in_states) {
                    for (const string &st2: out_states) {
//                        cout << "from " << st1 << " to " << st2 << endl;
                        string outer_str = find_string(st1, st2);
//                        cout << "outer_str:  " << outer_str << endl;
                        string str1 = find_string(st1, curr_state);
//                        cout << "str1:  " << str1 << endl;
                        string str2 = find_string(curr_state, st2);
//                        cout << "str2:  " << str2 << endl;
                        string loop_str = find_string(curr_state, curr_state);
//                        cout << "loop_str:  " << loop_str << endl;
                        string final_string;
                        if (!str1.empty()) {
                            final_string += "(";
                            final_string += str1;
                            final_string += ")";
                        }
                        if (!loop_str.empty()) {
                            final_string += "(";
                            final_string += loop_str;
                            final_string += ")*";
                        }
                        if (!str2.empty()) {
                            final_string += "(";
                            final_string += str2;
                            if ((d.is_final(curr_state)) && (st2 == FINAL)) {
                                final_string += "|";
                            }
                            final_string += ")";
                        }
                        if (!outer_str.empty()) {
                            if (outer_str != final_string) {
                                final_string += "|";
                                final_string += outer_str;
                            }
                        }
//                        cout << "final_string:  " << final_string << endl;
                        rules[make_pair(st1, st2)] = final_string;
                    }
                }
//                cout << "end delete state " << curr_state << endl;
                deleted.insert(curr_state);
            }
        }
        string S = rules[make_pair(init_State, FINAL)];
        string R = rules[make_pair(init_State, init_State)];
//        cout << "init loop:  " << R << endl;
//        cout << "end dfa:  " << init_State << "  " << S << "  " << FINAL << endl;
        string result;
        if (!R.empty()) {
            result += "(";
            result += R;
            result += ")*";
        }
        if (!S.empty()) {
            result += "(";
        }
        if (d.is_final(init_State)) {
            result += "|";
        }
        result += S;
        if (!S.empty()) {
            result += ")";
        }
        cout << result << endl;
        return result;
    }
};

std::string dfa2re(DFA &d) {
  return OwnDFA().dfa2re(d);
}
