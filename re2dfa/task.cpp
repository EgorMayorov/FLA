#include "api.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

using namespace std;

struct Tree {
    Tree* left{};
    char elem{};
    int pos{};
    Tree* right{};
    bool nullable{};
    set<int> firstpos;
    set<int> lastpos;
};

set<int> find_state (const map<int, set<int>> &followpos, const map<char, set<int>> &letter_pos,
                     const set<int> &state, const char &ch) {
    set<int> state_for_letter;
    auto letter_iter = letter_pos.find(ch);
    for (const int &pos: letter_iter->second) {
        if (state.count(pos)) {
            auto followpos_iter = followpos.find(pos);
            if (followpos_iter == followpos.end()) continue;
            state_for_letter.insert(followpos_iter->second.begin(), followpos_iter->second.end());
        }
    }
    return state_for_letter;
}

string find_nmstate (const set<string> &all_states, const set<string> &marked) {
    string res;
    for (const string &s: all_states) {
        if (!marked.count(s)) {
            res = s;
            break;
        }
    }
    return res;
}

set<int> find_set_in_Q (const map<set<int>, string> &Q, const string &nmstate) {
    set<int> res;
    map<set<int>, string>::const_iterator iter;
    for (iter = Q.begin(); iter != Q.end(); iter++) {
        if (iter->second == nmstate) {
            res = iter->first;
            break;
        }
    }
    return res;
}

DFA re2dfa(const std::string &s) {
    if (s.empty()) {
        Alphabet ealphabet = Alphabet("a");
        DFA eres = DFA(ealphabet);
        eres.create_state("0", true);
        eres.set_initial("0");
        return eres;
    }

    const char eps = ':';
    const char concat = '$';
    const char end = '#';
//    cout << "start string:  " << s << endl;
    string s1 = "(" +s + ")" + string(1, end);
//    cout << "string with #:  " << s1 << endl;
    string str;
    char last = '\0';
    for (const char &c: s1) {
        if (last != '\0') {
            if ((isalnum(c) || c == '#') && ((isalnum(last)) || (last == '*') || (last == ')'))) str.push_back(concat);
            if ((c == '*') && ((last == '(') || (last == '|'))) str.push_back(eps);
            if ((c == '(') && ((isalnum(last)) || (last == '*') || (last == ')'))) str.push_back(concat);
            if ((c == ')') && ((last == '(') || (last == '|'))) str.push_back(eps);
            if ((c == '|') && ((last == '(') || (last == '|'))) str.push_back(eps);
        }
        str.push_back(c);
        last = c;
    }
//    cout << "new string:  " << str << endl;
    vector<char> pin;
    vector<char> stack;
    for (const char &c: str) {
        if (c == '(') stack.push_back(c);
        if ((isalnum(c)) || (c == eps)) pin.push_back(c);
        if (c == '|') {
            while (!stack.empty() && (stack.back() != '(')) {
                pin.push_back(stack.back());
                stack.pop_back();
            }
            stack.push_back(c);
        }
        if (c == concat) {
            while ((stack.back() != '|') && !stack.empty()){
                if (stack.back() == '(') break;
                pin.push_back(stack.back());
                stack.pop_back();
            }
            stack.push_back(c);
        }
        if (c == '*') {
            while (!stack.empty() && (stack.back() == '*')) {
                pin.push_back(stack.back());
                stack.pop_back();
            }
            stack.push_back(c);
        }
        if (c == ')') {
            while (stack.back() != '(') {
                pin.push_back(stack.back());
                stack.pop_back();
            }
            stack.pop_back();
        }
        if (c == end) {
            pin.push_back(c);
            while (!stack.empty()) {
                pin.push_back(stack.back());
                stack.pop_back();
            }
        }
//        cout << "my poliz:  ";
//        for (const char &c1: pin) {
//            cout << c1;
//        }
//        cout << endl;
    }
//    cout << "my final poliz:  ";
//    for (const char &c: pin) {
//        cout << c;
//    }
//    cout << endl;
    vector<Tree*> tree_stack;
    Tree* vertex;
    int pos_count = 1;
    int pos_end;
    map<int, set<int>> followpos;
    map<char, set<int>> letter_pos;
    for (const char &c: pin) {
        if ((isalnum(c)) || (c == eps) || (c == end)) {
            Tree* list = new Tree();
            list->elem = c;
            list->pos = pos_count;
            letter_pos[c].insert(pos_count);
            if (c == end) pos_end = pos_count;
            pos_count++;
            if (c == eps) {
                list->nullable = true;
            } else {
                list->nullable = false;
                list->firstpos.insert(list->pos);
                list->lastpos.insert(list->pos);
            }
            tree_stack.push_back(list);
//            cout << list->elem << "  " << list->pos << endl;
        }
        if (c == '|') {
            Tree* list = new Tree();
            list->elem = c;
            list->right = tree_stack.back();
            tree_stack.pop_back();
            list->left = tree_stack.back();
            tree_stack.pop_back();
            list->nullable = list->left->nullable || list->right->nullable;
            list->firstpos = list->left->firstpos;
            list->firstpos.insert(list->right->firstpos.begin(), list->right->firstpos.end());
            list->lastpos = list->left->lastpos;
            list->lastpos.insert(list->right->lastpos.begin(), list->right->lastpos.end());
            tree_stack.push_back(list);
//            cout << list->elem << endl;
        }
        if (c == concat) {
            Tree* list = new Tree();
            list->elem = c;
            list->right = tree_stack.back();
            tree_stack.pop_back();
            list->left = tree_stack.back();
            tree_stack.pop_back();
            list->nullable = list->left->nullable && list->right->nullable;
            list->firstpos = list->left->firstpos;
            if (list->left->nullable) list->firstpos.insert(list->right->firstpos.begin(), list->right->firstpos.end());
            list->lastpos = list->right->lastpos;
            if (list->right->nullable) list->lastpos.insert(list->left->lastpos.begin(), list->left->lastpos.end());
            for (const int &p: list->left->lastpos) {
                followpos[p].insert(list->right->firstpos.begin(), list->right->firstpos.end());
            }
            tree_stack.push_back(list);
//            cout << list->elem << endl;
        }
        if (c == '*') {
            Tree* list = new Tree();
            list->elem = c;
            list->left = tree_stack.back();
            tree_stack.pop_back();
            list->nullable = true;
            list->firstpos = list->left->firstpos;
            list->lastpos = list->left->lastpos;
            for (const int &p: list->left->lastpos) {
                followpos[p].insert(list->left->firstpos.begin(), list->left->firstpos.end());
            }
            tree_stack.push_back(list);
//            cout << list->elem << endl;
        }
    }
    vertex = tree_stack.back();
    tree_stack.pop_back();

//    cout << "вершина дерева  " << vertex->elem << endl <<"firstpos:  ";
//    for (const int &f: vertex->firstpos) {
//        cout << f << ", ";
//    }
//    cout << endl << "lastpos:  ";
//    for (const int &f: vertex->lastpos) {
//        cout << f << ", ";
//    }
//    cout << endl << "followpos:" << endl;
//    map<int, set<int>>::iterator it;
//    for (it = followpos.begin(); it != followpos.end(); it++) {
//        cout << it->first << ":  ";
//        for (const int &f: it->second) {
//            cout << f << ", ";
//        }
//        cout << endl;
//    }
//    cout << "letter_pos:" << endl;
//    map<char, set<int>>::iterator iter;
//    for (iter = letter_pos.begin(); iter != letter_pos.end(); iter++) {
//        cout << iter->first << ":  ";
//        for (const int &i: iter->second) {
//            cout << i << ", ";
//        }
//        cout << endl;
//    }

    Alphabet alphabet = Alphabet(s);
    map<set<int> , string> Q;
    DFA res = DFA(alphabet);
    int state_name = 0;
    Q[vertex->firstpos] = to_string(state_name);
    set<string> marked;
    set<string> all_states;
    all_states.insert("0");
    if (vertex->firstpos.count(pos_end)) {
        res.create_state(to_string(state_name), true);
    } else {
        res.create_state(to_string(state_name), false);
    }
    res.set_initial("0");
    state_name++;
    while (marked.size() != all_states.size()) {
        string nmstate = find_nmstate(all_states, marked);
        map<char, set<int>>::iterator letter;
        for (letter = letter_pos.begin(); letter != letter_pos.end(); letter++) {
            // find state for this letter
            set<int> letter_state = find_state(followpos, letter_pos, find_set_in_Q(Q, nmstate), letter->first);
            if (!Q.count(letter_state)) {
                // new state
                if (letter_state.count(pos_end)) {
                    res.create_state(to_string(state_name), true);
                } else {
                    res.create_state(to_string(state_name), false);
                }
                Q[letter_state] = to_string(state_name);
                all_states.insert(to_string(state_name));
                state_name++;
            }
            // add transition
            res.set_trans(nmstate, letter->first, Q[letter_state]);
        }
        marked.insert(nmstate);
    }
	return res;
}
