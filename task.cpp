#include "api.hpp"
#include <string>
#include <map>
#include <stack>
#include <iostream>
#include <queue>
#include <algorithm>

int nextpos = 1;
std::map <int, char> nextpos_to_sym;
std::map <int, std::set <int>> followpos;

int find_close_bracket(std::string &s, int i) {
    std::stack <int> st;
    st.push(i);
    int len = s.size();
    for (int j = i + 1; j < len; j++) {
        if (s[j] == '(') {
            st.push(j);
        }
        if (s[j] == ')') {
            if (st.top() == i) {
                return j;
            }
            st.pop();
        }
    }
}

struct node {
    char sym;
    int pos;
    bool nullable;
    std::set<int> firstpos;
    std::set<int> lastpos;
    node *left;
    node * right;
    node() {
        left = NULL;
        right = NULL;
        pos = 0;
        sym = 0;
    }
};

node *create_tree(std::string s) {
    std::stack <node*> st;
    std::stack <node*> st_revers;
    Alphabet alp = Alphabet(s);
    int len = s.size();
    for (int i = 0; i < len; i++) {
        if (s[i] == '*') {
            node *tmp = new node();
            tmp->sym = s[i];
            tmp->left = st.top();
            tmp->right = NULL;
            st.pop();
            st.push(tmp);
        } else if (s[i] == '|') {
            node *tmp = new node();
            tmp->sym = s[i];
            st.push(tmp);
        } else if (s[i] == '(') {
            int pos = find_close_bracket(s, i);
            st.push(create_tree(s.substr(i + 1, pos - i - 1)));
            i = pos;
        } else if (alp.has_char(s[i])){
            node *tmp = new node();
            tmp->left = NULL;
            tmp->right = NULL;
            tmp->pos = nextpos;
            tmp->sym = s[i];
            nextpos_to_sym[tmp->pos] = tmp->sym;
            nextpos++;
            st.push(tmp);
        }
    }
    node *op1 = NULL;
    node *op2 = NULL;
    while(!st.empty()) {
        st_revers.push(st.top());
        st.pop();
    }
    while(!st_revers.empty()) {
        if (op1 == NULL) {
            op1 = st_revers.top();
            st_revers.pop();
            continue;
        }
        op2 = st_revers.top();
        st_revers.pop();
        if ((op2->left == NULL || op2->right == NULL) && op2->sym == '|') {
            op2->left = op1;
            op2->right = st_revers.top();
            st_revers.pop();
            op1 = op2;
        } else {
            node *tmp = new node();
            tmp->left = op1;
            tmp->right = op2;
            tmp->sym = '.';
            op1 = tmp;
        }
    }
    return op1;
}

void fill_nullable_firstpos_lastpos(node *head) {
    if (head == NULL) {
        return;
    }
    fill_nullable_firstpos_lastpos(head->left);
    fill_nullable_firstpos_lastpos(head->right);
    char sym = head->sym;
    if (sym == '*') {
        head->nullable = true;
        head->lastpos = head->left->lastpos;
        head->firstpos = head->left->firstpos;
    } else if (sym == '|') {
        if (head->left->nullable || head->right->nullable) {
            head->nullable = true;
        } else {
            head->nullable = false;
        }
        head->firstpos.insert(head->left->firstpos.begin(), head->left->firstpos.end());
        head->lastpos.insert(head->left->lastpos.begin(), head->left->lastpos.end());
        head->firstpos.insert(head->right->firstpos.begin(), head->right->firstpos.end());
        head->lastpos.insert(head->right->lastpos.begin(), head->right->lastpos.end());
    } else if (sym == '.') {
        if (head->left->nullable && head->right->nullable) {
            head->nullable = true;
        } else {
            head->nullable = false;
        }
        head->firstpos.insert(head->left->firstpos.begin(), head->left->firstpos.end());
        head->lastpos.insert(head->right->lastpos.begin(), head->right->lastpos.end());
        if (head->left->nullable) {
            head->firstpos.insert(head->right->firstpos.begin(), head->right->firstpos.end());
        }
        if (head->right->nullable) {
            head->lastpos.insert(head->left->lastpos.begin(), head->left->lastpos.end());
        }
    } else {
        if (sym == '$') { //eps
            head->nullable = true;
        } else {
            head->nullable = false;
        }
        head->firstpos.insert(head->pos);
        head->lastpos.insert(head->pos);
    }
}

void fill_followpos(node *head) {
    if (head == NULL) {
        return;
    }
    fill_followpos(head->left);
    fill_followpos(head->right);
    std::set <int> tmp;
    if (head->sym == '.') {
        tmp = head->right->firstpos;
    }
    if (head->sym == '*') {
        tmp = head->left->firstpos;
    }
    if (tmp.empty()) {
        return;
    }
    std::set <int>::iterator it;
    for(it = head->left->lastpos.begin(); it != head->left->lastpos.end(); it++) {
        followpos[*it].insert(tmp.begin(),tmp.end());
    }
}
void treeprint(node *tree) {
    if (tree!=NULL) {
        std::set <int>::iterator it;
        for (it = tree->firstpos.begin(); it != tree->firstpos.end(); it++) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        for (it = tree->lastpos.begin(); it != tree->lastpos.end(); it++) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        std::cout << tree->pos << " ";
        std::cout << tree->sym << " ";
        std::cout << tree->nullable << std::endl;
        std::cout << "==================" << std::endl;
        treeprint(tree->left);
        treeprint(tree->right);
    }
}

void followposprint() {
    std::map <int, std::set <int>>::iterator it1;
    std::set <int>::iterator it2;
    for (it1 = followpos.begin(); it1 != followpos.end(); it1++) {
        std::cout << it1->first << ":" << std::endl;
        for (it2 = it1->second.begin(); it2 != it1->second.end(); it2++) {
            std::cout << *it2 << " ";
        }
        std::cout << std::endl;
    }
}
DFA create_dfa(DFA res, std::set <int> firstpos) {
    nextpos--;
    int counter = 0;
    std::map <std::set<int>, int> statenum;
    std::map<char, std::set <int>> statenext;
    std::queue <std::set <int>> q;
    std::map <std::pair<int, int>, std::set <char>> transition;
    std::set <int> curr;
    std::set <int> next;
    std::set <int> final;
    char sym;
    q.push(firstpos);
    while(!q.empty()) {
        curr = q.front();
        statenext.clear();
        q.pop();
        if (statenum.find(curr) == statenum.end()) {
            statenum[curr] = counter;
            counter++;
        }
        if (curr.find(nextpos) != curr.end()) {
            final.insert(statenum[curr]);
        }
        std::set<int>::iterator it;
        for (it = curr.begin(); it != curr.end(); it++) {
            sym = nextpos_to_sym[*it];
            if (followpos.find(*it) != followpos.end()) {
                statenext[sym].insert(followpos[*it].begin(),followpos[*it].end());
            }
        }
        std::map<char, std::set <int>>::iterator it2;
        for (it2 = statenext.begin(); it2 != statenext.end(); it2++) {
            sym = it2->first;
            if (sym != '$') {
                next = it2->second;
                if (statenum.find(next) == statenum.end()) {
                    statenum[next] = counter;
                    counter++;
                    q.push(next);
                }
                transition[std::make_pair(statenum[curr], statenum[next])].insert(sym);
            }
        }
    }
    std::map <std::pair<int, int>, std::set <char>>::iterator it3;
    for (it3 = transition.begin(); it3 != transition.end(); it3++)  {
        if (!res.has_state(std::to_string(it3->first.first))) {
            if (final.find(it3->first.first) == final.end()) {
                res.create_state(std::to_string(it3->first.first), false);
            } else {
                res.create_state(std::to_string(it3->first.first), true);
            }
        }
        if (!res.has_state(std::to_string(it3->first.second))) {
            if (final.find(it3->first.second) == final.end()) {
                res.create_state(std::to_string(it3->first.second), false);
            } else {
                res.create_state(std::to_string(it3->first.second), true);
            }
        }
        std::set <char>::iterator it4;
        for (it4 = it3->second.begin(); it4 != it3->second.end(); it4++) {
            res.set_trans(std::to_string(it3->first.first), *it4, std::to_string(it3->first.second));
        }
    }
    res.set_initial("0");
    return res;
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}


DFA re2dfa(const std::string &s) {
    std::string str = s;
    Alphabet alp = Alphabet(s);
    std::string new_str;

    std::vector<std::string> tmp;
    int len = str.size();

    for (int i = 0; i < len; i++) {
        if (alp.has_char(str[i]) || str[i] == '*' ||  str[i] == ')' || str[i] == '(') {
            int balance = 0;
            new_str += '(';
            while (alp.has_char(str[i]) || str[i] == '*' ||  str[i] == ')' || str[i] == '(') {
                if (str[i] == '(') {
                    new_str += '(';
                }
                if (str[i] == ')') {
                    new_str += str[i];
                    new_str += ')';
                    i++;
                    continue;
                }
                new_str += str[i];
                i++;
            }
            new_str += ')';
            i--;
        } else {
            new_str += str[i];
        }
    }

    new_str = ReplaceAll(new_str, "(|", "($|");
    new_str = ReplaceAll(new_str, "|)", "|$)");
    new_str = ReplaceAll(new_str, "||", "|$|");
    new_str = ReplaceAll(new_str, "()", "($)");
    new_str = ReplaceAll(new_str, "((*))", "*");
    new_str = ReplaceAll(new_str, "(*)", "*");
    new_str = ReplaceAll(new_str, "(*", "*(");

    new_str = "(" + new_str + ")#";

    node* head = create_tree(new_str);
    fill_nullable_firstpos_lastpos(head);
    fill_followpos(head);
    DFA res = DFA(alp);
    res = create_dfa(res, head->firstpos);
    return res;
}
