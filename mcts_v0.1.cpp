#include<iostream>
#include<random>
#include<algorithm>
#include<string>
#include<vector>
#include <functional>
using namespace std;
/* ******************************   */
#define LEVEL 9
#define GOAL 0
#define MAX_VALUE 5*(LEVEL-1)*LEVEL/2
#define SCALAR 1/pow(2,1/2)

/* *********************g***********  */

#define MAX_LEVELS 1
#define MAX_SIMS  5

int g_id = 0;
int randomizer(int size) {
	random_device rand_dev;
	mt19937 generator(rand_dev());
	uniform_real_distribution<float> distr(0, 1);
	return distr(generator)*(size);
}
string makestr(vector<int> v) {
	//vector<int> ::iterator i;
	string l;
	for (auto i = v.begin(); i < v.end(); i++) {
		l += to_string(*i);
	}
	return l;

}

class State {
public:
	vector <int>  MOVES = {1,-1 };
	vector <int> moves;
	int turn;
	//vector<int> moves;
	int value;
	State(int value, vector <int> moves, int turn) {
		this->value = value;
		this->turn = turn;
		this->moves = moves;
	}
	State(void) {
		this->value = 0;
		this->turn = LEVEL;

	}
	State next_state(void) {
		int i = randomizer(this->MOVES.size());
		int next_move = this->MOVES[i] * this->turn;
		vector <int> t = this->moves;
		t.push_back(next_move);
		int new_turn = this->turn-1;
		//cout << "new_turn "<< new_turn <<  endl;
		State next = State(this->value + next_move, t, new_turn);
		//cout << next.moves.size()<<" "<< "********" <<next  <<endl;
		return next;
	}
	bool terminal(void) {
		if (this->turn <= 0) {
			//cout << "terminating state " << endl;
			return true;
		}
		return false;
	}
	float reward(void) {
		float r = (1 - abs(this->value - GOAL) / MAX_VALUE);
		return r;
	}
	bool operator==(State s) {
		string str1 = makestr(this->moves);
		string str2 = makestr(s.moves);
		hash<string> hash_str;
		if (hash_str(str1) == hash_str(str2)) {
			return true;
		}
		return false;
	}
	string put_moves(void) {
		string res;
		vector<int> ::iterator i;
		for (i = this->moves.begin(); i < this->moves.end(); i++) {
			res += to_string(*i) + " ";
			//cout << *i<<"\t";
		}

		return res;
	}
	friend ostream& operator<< (ostream& os, State& n);
};
ostream& operator<< (ostream& os, State& n) {
	os << "Value: " << n.value << "; " << "moves: " << n.put_moves() << " turn: " << n.turn << endl;
	return os;
}
class Node {
public:
	int id;
	int visits;
	float reward;
	State state;
	vector<Node> children;
	vector <Node> parent;
	Node(State state) {
		this->visits = 1;
		this->id = g_id++;
		this->reward = 0;
		this->state = state;
		//this->parent = parent;
	}
	Node(State state, Node parent) {

		this->visits = 1;
		this->id = g_id++;
		this->reward = 0;
		this->state = state;
		this->parent.push_back(parent);
	}
	void add_child(State child_state) {
		Node temp = Node(child_state,*this);
		cout << "pshing to node" << *this ;
		this->children.push_back(temp);

	}

	void update(float reward) {
		this->reward += reward;
		this->visits += 1;
	}

	bool fully_expanded(void) {
		if (this->children.size() == this->state.MOVES.size()) {
			//cout << "fully expanded" << endl;
			return true;
		}

		return false;
	}

	friend ostream& operator<<(ostream& os, Node &n) {
		os << "Node; children: " << n.children.size() << " Parent: " << n.parent.size() << " " << "visits: " << n.visits << " " << "reward: " << n.reward << " id: " << n.id << endl;
		os << "State; " << n.state << endl;
		os << "children: " << n.children.size() << " \t" << n.state.moves.size() << endl;
		return os;
	}


};



Node BESTCHILD(Node node, int scalar) {
	cout << "inside bestchild function" << endl;
	//cout << node << endl;
	float bestscore = 0;
	//cout << "bestchild func" << endl;
	cout << node.children.size() << endl;
	vector <Node> bestchildren;
	for (int i = 0; i < node.children.size(); i++) {
		float exploit = node.children[i].reward / node.children[i].visits;
		float explore = pow(2 * log(node.visits) / node.children[i].visits, 1 / 2);
		float score = exploit + scalar*explore;
		//cout << "score" << score << "\t" << bestscore << endl;

		if (score == bestscore) {
			bestchildren.push_back(node.children[i]);
		}
		if (score > bestscore) {
			cout << score << "\t" << bestscore << endl;
			//cout << "clearing node " << endl;
			/*if (bestchildren.size() > 0) {
				bestchildren.clear();
			}*/
			bestchildren.clear();
			bestchildren.push_back(node.children[i]);
			bestscore = score;
		}


	}
	if (bestchildren.size() == 0) {
		cout << "No best children found" << endl;
		//return Node(State());
		cout << "exiting" << endl;
		
	}

	//cout << bestchildren.size() << endl;
	//exit(1);

	return   bestchildren[randomizer(bestchildren.size())];
}
float DEFAULTPOLICY(State state) {
	//cout << "DEFAULT" << endl;
	while (state.terminal() == false) {
		state = state.next_state();
	}
	return state.reward();
}

Node EXPAND(Node &node) {
	//exit(5);
	//cout << "EXPPAND" << endl;
	vector <State> tried_children;
	//cout <<  "in expand /; " << node.children.size() << endl;
	for (int i = 0; i < node.children.size(); i++) {
		State temp = node.children[i].state;
		//cout << " STATE : " << temp << endl;
		tried_children.push_back(temp);
	}
	//exit(1);
	State new_state = node.state.next_state();
	while (find(tried_children.begin(), tried_children.end(), new_state) != tried_children.end()) {
		new_state = node.state.next_state();
		//cout << "finding"<< endl;

	}
	node.add_child(new_state);
	//cout << "child  added " << node << endl;
	return  node.children.back();

}

Node TREEPOLICY(Node &node) {
	//cout << "TREEPLOLICY" << endl;
	while (node.state.terminal() == false) {
		//cout << "children: " << node.children.size() << " state: " << node.state.MOVES.size() << endl;
		if (node.fully_expanded() == false) {
			//cout << "expand" << endl;
			return EXPAND(node);
		}

		else {
			//cout << "best child" << endl;
			node = BESTCHILD(node, SCALAR);
			cout << "";
		}
	}
	
	return node;
}
void BACKUP(Node &node, float reward) {
	//cout << "BACKUP" << endl;
	while (node.parent.size() != 0) {
		node.visits += 1;
		node.reward += reward;
		node = node.parent[0];
	}
	return;
}
Node UCTSEARCH(long int budget, Node &root) {
	
	//cout << "UCT " << endl;
	for (long int iter = 0; iter < budget; iter++) {
		cout << "****************************: " << iter << endl;
		//cout << "iter : " << iter << endl;
		if (iter % 100 == 0) {
			//cout << "simulation: " << iter << endl; 
		}
		//cout << " in UCTSEARCH " << root;
		Node front = TREEPOLICY(root);
		cout << "After front " << endl;
		float reward = DEFAULTPOLICY(front.state);
		BACKUP(front, reward);

	}

	return BESTCHILD(root, 0);
}

void test1(void) {
	for (int i = 0; i < 10; i++) {
		int y = randomizer(3);
		cout << y << endl;
	}
}

//Node node_root = Node(State());
int main(void) {
	/*test1();
	system("pause");
	exit(0);*/
	Node current_node =Node(State());
	
	vector <Node> list;
	try {
		for (long int l = 0; l < MAX_LEVELS; l++) {
			
			cout <<">>>>>>>>>>>>>>>>>    " << l << "    <<<<<<<<<<<<<<<<<<<" << endl;
			current_node = UCTSEARCH(MAX_SIMS / (l + 1), current_node);
			cout << "level " << l << endl;
			cout << "Num Children: " << current_node.children.size() << endl;
			for (auto i = current_node.children.begin(); i<current_node.children.end(); i++) {
				Node temp = *i;
				cout << temp << endl;
			}
			cout << "Best Child: " << current_node.state << endl;
			cout << "-------------------------------------" << endl;
		}
	}
	catch (const char *e) {
		cout << e << endl;
	}


	system("pause");

	return 0;
}