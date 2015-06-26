#ifndef _PROJECT_EULER_GRAPH_H
#define _PROJECT_EULER_GRAPH_H
#include <cstdint>
#include <vector>

class Graph;

class GraphNode //: public GraphNode
{
public:
	friend class Graph;

    GraphNode(double weight = 0.0): weight(weight) {};
	~GraphNode() = default;
	//explicit GraphNode( int l ): GraphNode( l ) {};    

	GraphNode &operator=(const GraphNode &other) = delete;
	GraphNode(const GraphNode &other) = default;

    void setWeight(double w) { weight = w; }
    double getWeight() const { return weight; }

	std::vector<int>::const_iterator begin() const { return neighbors.begin(); }
	std::vector<int>::const_iterator end() const { return neighbors.end(); }

	std::vector<int>::iterator begin() { return neighbors.begin(); }
	std::vector<int>::iterator end() { return neighbors.end(); }

    int getNeighbor(int idx) const { return neighbors[idx]; }

    double getEdgeWeight(int idx) const { return edgeWeights[idx]; }
    double getEdgeWeight(const std::vector<int>::iterator &it) const { return *(edgeWeights.begin() + (it - neighbors.begin())); }

    void setEdgeWeight(int idx, double w) { edgeWeights[idx] = w; }
    void setEdgeWeight(const std::vector<int>::iterator &it, double w) { (*(edgeWeights.begin() + (it - neighbors.begin()))) = w; }

    size_t getNeighborCount() const { return neighbors.size(); }
private:
	void removeNeighbor(int to);
	void addNeighbor(int to, double weight = 0.0);

	std::vector<int> neighbors;
    std::vector<double> edgeWeights;
	double weight;
};



class Graph
{
public:
	explicit Graph(bool dir) : directed(dir) {}
	~Graph() = default;

	Graph &operator=(const Graph &other) = delete;
	Graph(const Graph &other) = delete;

	int addNode(double weight = 0.0);
	//virtual void removeNode( int toRemove );

	void addEdge(int from, int to, double weight = 0.0);
	void removeEdge(int from, int to);

	std::vector<GraphNode>::const_iterator begin() const { return nodes.begin(); }
	std::vector<GraphNode>::const_iterator end() const { return nodes.end(); }

    std::vector<GraphNode>::iterator begin() { return nodes.begin(); }
    std::vector<GraphNode>::iterator end() { return nodes.end(); }

	void resize(size_t newNodes);
	GraphNode &getNode(int idx) { return nodes[idx]; }
	const GraphNode &getNode(int idx) const { return nodes[idx]; }
	int getNodeCount() const { return nodes.size(); }

    bool isDirected() const { return directed; }
    void setDirected(bool d) { directed = d; }

    bool hasWeightedNodes() const { return weightedNodes; }
    void setWeightedNodes(bool weighted) { weightedNodes = weighted; }

    bool hasWeightedEdges() const { return weightedEdges; }
    void setWeightedEdges(bool weighted) { weightedEdges = weighted; }

    void clear();
private:
	bool directed;
    bool weightedNodes;
    bool weightedEdges;

    std::vector<GraphNode> nodes;
};


std::vector<std::pair<double, int>> findMinimalPathDijkstra(const Graph &g, int startNode, int endNode);

std::istream &operator>>(std::istream &is, Graph &g);
std::ostream &operator<<(std::ostream &os, const Graph &g);

#endif // _PROJECT_EULER_GRAPH_H
