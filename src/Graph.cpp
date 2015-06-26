#include "Graph.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <set>
#include <vector>

/////////////////////////////////////////////////////
//  Node of Graph
/////////////////////////////////////////////////////


void GraphNode::removeNeighbor(int to)
{
    auto neighborIt = std::find(neighbors.begin(), neighbors.end(), to);
    auto edgeWeightIt = edgeWeights.begin() + (neighborIt - neighbors.end());
    std::swap(*neighborIt, *neighbors.rbegin());
    std::swap(*edgeWeightIt, *edgeWeights.rbegin());
    neighbors.erase(neighbors.end() - 1, neighbors.end());
    edgeWeights.erase(edgeWeights.end() - 1, edgeWeights.end());
}

void GraphNode::addNeighbor(int to, double weight)
{
    auto it = std::lower_bound(neighbors.begin(), neighbors.end(), to);
    if (it != neighbors.end() && *it == to)
        return;
    edgeWeights.insert(edgeWeights.begin() + (it - neighbors.begin()), weight);
    neighbors.insert(it, to);
}

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//   Graph
/////////////////////////////////////////////////////

void Graph::resize(size_t newNodes)
{
    if (newNodes < nodes.size())
        throw("Graph::resize - cannot decrease size");

    if (newNodes == nodes.size())
        return;

    nodes.resize(newNodes);
}


int Graph::addNode(double weight)
{
    nodes.emplace_back(weight);
    return nodes.size() - 1;
}

void Graph::addEdge(int from, int to, double weight)
{
    nodes[from].addNeighbor(to, weight);    
    if (!directed)
    {
        nodes[to].addNeighbor(from, weight);

    }
}


void Graph::removeEdge(int from, int to)
{
    nodes[from].removeNeighbor(to);
    if (!directed)
    {
        nodes[to].removeNeighbor(from);
    }
}

void Graph::clear()
{
    nodes.clear();
}


/////////////////////////////////////////////////////
//  Graph algorithms
/////////////////////////////////////////////////////


std::vector<std::pair<double, int>> findMinimalPathDijkstra(const Graph &g, int startNode, int endNode)
{
    std::vector<std::pair<double, int>> shortestRoute(g.getNodeCount(), std::make_pair(std::numeric_limits<double>::max(), -1));
    shortestRoute[startNode] = std::make_pair(g.getNode(startNode).getWeight(), 0);

    std::set<std::pair<double, int>> q;

    q.insert(std::make_pair(g.getNode(startNode).getWeight(), startNode));

    int u = startNode;
    while (!q.empty())
    {
        u = begin(q)->second;

        if (endNode != -1 && u == endNode)
            return shortestRoute;

        for (auto &neighbor : g.getNode(u))
        {
            std::vector<std::pair<int64_t, int>> removedElements;
            double newValue = shortestRoute[u].first + g.getNode(neighbor).getWeight();
            if (newValue < shortestRoute[neighbor].first)
            {
                q.erase(std::make_pair(shortestRoute[neighbor].first, neighbor));
                q.insert(std::make_pair(newValue, neighbor));
                shortestRoute[neighbor].first = newValue;
                shortestRoute[neighbor].second = u;
            }
        }
        q.erase(begin(q));
    }
    return shortestRoute;
}

/////////////////////////////////////////////////////
//  Input / Output
/////////////////////////////////////////////////////
//
// Format:
// NumberOfNodes NumberOfEdges 
// DirectedFlag WeightedNodesFlag WeightedEdgesFlag
// If WeightedNodesFlag == 1, the node weights follow, NumberOfNodes numbers
//    else this section is omitted
// NumberOfEdges edges follow:
// From To [Weight]
// Wight is omitted if NumberOfEdges == 0

// Flags are 0 or 1
// All weights are floating point numbers
// Everything is whitespace separated
// Indexes are 1 based.

// e.g
// 3 2 
// 1 1 0
// 5.4
// 0.6
// 1
// 2 3
// 3 1

//  2  ----> 3 ----> 1
// 0.6       1      5.4

std::istream &operator>>(std::istream &is, Graph &g)
{
    g.clear();

    int n, m;
    int directedFlag, weightedNodesFlag, weightedEdgesFlag;

    is >> n >> m;
    is >> directedFlag >> weightedNodesFlag >> weightedEdgesFlag;

    g.resize(n);
    g.setDirected(directedFlag != 0);
    g.setWeightedNodes(weightedNodesFlag != 0);
    g.setWeightedEdges(weightedEdgesFlag != 0);

    if (g.hasWeightedNodes())
    {
        for (auto &node : g)
        {
            double w;
            is >> w;
            node.setWeight(w);
        }
    }

    for (int i = 0; i < m; ++i)
    {
        int from, to;
        is >> from >> to;
        double weight = 0.0;
        if (g.hasWeightedEdges())
        {
            is >> weight;
        }
        g.addEdge(from - 1, to - 1, weight);
    }

    return is;
}

std::ostream &operator<<(std::ostream &os, const Graph &g)
{
    os << g.getNodeCount() << " ";

    int edgesCount = 0;

    for (auto &node : g)
    {
        edgesCount += node.getNeighborCount();
    }

    if (g.isDirected())
    {
        os << edgesCount << "\n";
    }
    else
    {
        os << edgesCount / 2 << "\n";
    }

    os << g.isDirected() << " " << g.hasWeightedNodes() << " " << g.hasWeightedEdges() << "\n";

    if (g.hasWeightedNodes())
    {
        for (auto &node : g)
        {
            os << node.getWeight() << "\n";
        }
    }

    for (int nodeIdx = 0; nodeIdx < g.getNodeCount(); ++nodeIdx)
    {
        auto &node = g.getNode(nodeIdx);
        for (size_t i = 0; i < node.getNeighborCount(); ++i)
        {
            if (g.isDirected() || nodeIdx < node.getNeighbor(i))
            {
                os << nodeIdx + 1 << " " << node.getNeighbor(i) + 1;
            }
            if (g.hasWeightedEdges())
            {
                os << " " << node.getEdgeWeight(i);
            }
            os << "\n";
        }
    }

    os << std::flush;

    return os;
}