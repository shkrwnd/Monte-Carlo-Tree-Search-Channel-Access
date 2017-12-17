#include<iostream>
#include<assert.h>
#include<random>
#include<string>
#include<algorithm>
#include<functional>
#include<fstream>
#include<limits>
#include<vector>
using namespace std;
#define LEVEL 15

#define GOAL 0
#define MAX_VALUE 300*(LEVEL-1)*LEVEL/2
#define SCALAR 1/pow(3,1/3)

/* *********************g***********  */

#define MAX_LEVELS 15
#define MAX_SIMS  600

int g_id = 0;

int randomizer(int size) {
	random_device rand_dev;
	mt19937 generator(rand_dev());
	uniform_int_distribution<int> distr(0, 999999);
	return distr(generator)%(size);
}
string makestr(vector<int> v) {
	string l;
	for (auto i = v.begin(); i < v.end(); i++) {
		l += to_string(*i);
	}
	return l;

}

class State {
public:
	vector <int> POS_MOVES = { 0,1,2,3,4,5,6,7};
	vector <int> PM = { 0,1,1,2,1,2,2,3 };
	vector <int> moves;
	int depth;
	float  value;
	int sum;
	float discount = 0.95;
	float decay = 0.01;
	
	State(float value, vector<int> moves, int depth,int sum) {
		//cout << " not init" << endl;
		this->moves = moves;
		this->depth = depth;
		this->value = value;
		this->sum = sum;
		
	}
	State(void) {
		//cout << " init" << endl;
		this->value = 0;
		this->depth = LEVEL;
	}
	State next_state(void) {
		int i = randomizer(this->POS_MOVES.size());
		int next_move = this->POS_MOVES[i];// *this->depth;   // 
		vector<int>  t = this->moves;
		t.push_back(next_move);
		int new_depth = this->depth - 1;
		State next = State(this->value + PM[i] * this->discount , t, new_depth,this->sum+ next_move);
		return next;
	}
	bool terminal(void) {
		if (this->depth <= 0) {
			return true;
		}
		return false;
	}
	float generate_reward(void) {
		float r =  float(this->value - GOAL) / MAX_VALUE;
	//cout << r << endl;
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

		for (auto i = this->moves.begin(); i < this->moves.end(); i++) {
			res += to_string(*i) + " ";

		}

		return res;
	}
	friend ostream& operator<< (ostream& os, State& n);
	
};
ostream& operator<< (ostream& os, State& n) {
	os <<"Throughput: " << n.value /(n.moves.size() * n.PM[*max_element(n.moves.begin(),n.moves.end())] )<<    " Value: " << n.value << "; " <<"Sum: " << n.sum << " moves: " << n.put_moves() << " turn: " << n.depth << endl;
	return os;
}

class Node {
public:
	int id;
	int visits;
	float reward;
	State state;
	
	vector <Node *> children;
	Node * parent;

	Node(State state) {
		this->visits = 1;
		this->id = g_id++;
		this->reward = 0;
		this->state = state;
		this->parent = NULL;
	}
	Node(State state, Node * parent) {
		this->visits = 1;
		this->reward = 0;
		this->state = state;
		this->parent = parent;
		this->id = g_id++;
	}

	void add_child(State child_state) {
		Node *temp = new Node(child_state, this);
		this->children.push_back(temp);
	}
	bool fully_expanded(void) {
		if (this->children.size() >= this->state.POS_MOVES.size()) {
			return true;
		}
		return false;
	}
	friend ostream& operator<<(ostream& os, Node &n) {
		os << "Node; children: " << n.children.size() << " " << "visits: " << n.visits << " " << "reward: " << n.reward << " id: " << n.id << endl;
		os << "State; " << n.state << endl;
		//os << "children: " << n.children.size() << " \t" << n.state.moves.size() << endl;
		return os;
	}

};


Node* BESTCHILD(Node node, int scalar) {
	float best_score = -numeric_limits<float>::max();
	vector <Node*> best_children;
	float score;
	for (int i = 0; i < node.children.size(); i++) {
		float exploit =  (*node.children[i]).reward /(*node.children[i]).visits;
		float explore = pow(2 * log(node.visits) / (*node.children[i]).visits, 1 / 2);
		score = exploit+scalar*explore;

		if (best_score == score) {
			best_children.push_back(node.children[i]);
		}
		else if (score > best_score) {
			best_children.clear();
			best_score = score;
			best_children.push_back(node.children[i]);
		}
	}
	if (best_children.size() == 0) {
		cout << "No best child found " << endl;
		system("pause");

		exit(2);
	}

	return best_children[randomizer(best_children.size())];
}
//   Another way of saying a Rollout Policy
float DEFAULTPOLICY(State state) {
	while (state.terminal() == false) {
		state = state.next_state();
	}
	return state.generate_reward();
}

Node* EXPAND(Node *node) {
	vector <State> tried_children_state;
	for (int i = 0; i < node->children.size(); i++) {
		State temp = (*node->children[i]).state;
		tried_children_state.push_back(temp);
	}
	State new_state = node->state.next_state();
	while (find(tried_children_state.begin(), tried_children_state.end(), new_state) != tried_children_state.end()) {
		new_state = node->state.next_state();
	}
	node->add_child(new_state);
	return node->children.back();
}

Node* TREEPOLICY(Node *node) {
	while (node->state.terminal() == false) {
		if (node->fully_expanded() == false) {
			return EXPAND(node);
		}
		else {
			node = BESTCHILD(*node, SCALAR);
		}
	}
	return node;
}

void BACKUP(Node *node, float reward) {
	while (node != NULL) {
		node->visits += 1;
		node->reward += reward;
		node = node->parent;
	}
}

Node* UCTSEARCH(long int budget, Node *root) {
	for (long int iter = 0; iter < budget; iter++) {
		//cout << "iter : " << iter << endl;
		if (iter == budget - 1) {
			cout << "" << endl;
		}
		Node *front = TREEPOLICY(root);
		float reward = DEFAULTPOLICY(front->state);
		BACKUP(front, reward);
	}
	return BESTCHILD(*root,0);

}


int MCTS(ofstream& f) {
	
	Node *current_node = new Node(State());
	

	try {
		for (long int l = 0; l < MAX_LEVELS; l++) {

			current_node = UCTSEARCH(MAX_SIMS, current_node);
			cout << "level " << l << endl;
			cout << "Num Children: " << current_node->children.size() << endl;
			/*for (auto i = current_node->children.begin(); i<current_node->children.end(); i++) {
				Node *temp = (*i);
				cout << *temp << endl;
			}*/
			cout << "Best Child: " << current_node->state << endl;
			State n = current_node->state;
			f << (n.value + 1000000* numeric_limits<float>::min()) / (1000000 * numeric_limits<float>::min() + n.moves.size() * n.PM[*max_element(n.moves.begin(), n.moves.end())]);
			f << endl;
			cout << "-------------------------------------" << endl;
		}
	}
	catch (const char *e) {
		cout << e << endl;
	}
	return 0;
}
void main(void) {
	assert(MAX_LEVELS <= LEVEL);
	ofstream f("log.txt", ofstream::out);
	f << MAX_LEVELS << endl;
	for (auto i = 0; i < 100; i++) {
		MCTS(f);
	}
	
	f.close();
	system("pause");
	system("python show_graph.py");
	system("pause");
}