#include "PMCEnumeratorTester.h"
#include "DataStructures.h"

/**
 * Generic test initialization.
 *
 * Declares/assumes variables 'g' and 'pmce' in the current scope.
 */
#define SETUP(_n) \
    Graph g(_n); \
    PMCEnumerator pmce(g)
#define RESET(_n) \
    g.reset(_n); \
    pmce.reset(g)

namespace tdenum {

bool PMCEnumeratorTester::sanity() {
    SETUP(0);
    RESET(1);
    RESET(10);
    return true;
}

bool PMCEnumeratorTester::trivialgraphs() {
    SETUP(0);
    ASSERT(pmce.IsPMC(NodeSet(), g));
    ASSERT_EQUAL(pmce.get(), NodeSetSet());
    RESET(1);
    NodeSet nodeset = g.getNodesVector();
    ASSERT(!pmce.IsPMC(NodeSet(), g));
    ASSERT(pmce.IsPMC(nodeset, g));
    NodeSetSet nss;
    nss.insert(nodeset);
    if (pmce.get() != nss) {
        ASSERT_PRINT("Non-equal NodeSetSets:");
        cout << "== 1 ==\n" << pmce.get().str() << "== 2 ==\n" << nss.str() << "\n";
        return false;
    }
    return true;
}

bool PMCEnumeratorTester::randomgraphs() {
    SETUP(0);
    NodeSetSet nss;
    TRACE(""); // Newline
    for (int i=2; i<FAST_GRAPH_SIZE; ++i) {
        RESET(i);
        g.randomize(0.5);
        TRACE("Random graph #" << i-1 << "/" << FAST_GRAPH_SIZE-2 << ":");
        TRACE(g.str());
        TRACE("Resulting PMCs:\n" << pmce.get().str());
    }
    return true;
}

bool PMCEnumeratorTester::smallknowngraphs() {
    // For n=2, there are two possible graphs: sparse or clique.
    // Both graphs are already triangulated, so the PMCs should
    // be {a},{b} for the sparse graph and {a,b} for the clique.
    SETUP(2);
    NodeSet a, b, ab;
    a.push_back(0);
    b.push_back(1);
    ab.push_back(0);
    ab.push_back(1);
    NodeSetSet sparsePMCs = pmce.get();

//    ASSERT_EQUAL(sparsePMCs.size(), 2);
//    ASSERT(sparsePMCs.isMember(a));
//    ASSERT(sparsePMCs.isMember(b));
    g.addEdge(0,1);
    pmce.reset(g);
    NodeSetSet cliquePMCs = pmce.get();
    ASSERT_EQUAL(cliquePMCs.size(), 1);
    ASSERT(sparsePMCs.isMember(ab));

    // For n=3, there are four different graphs (up to isomorphism),
    // differentiated by the number of edges m.
    // In any case, the graph is already triangulated (no cycles of length
    // 4 or more) so PMCs are actual maximal cliques.
    // m=0: {a},{b},{c}
    // m=1: {a,b}, {c}
    // m=2: {a,b}, {a,c}
    // m=3: {a,b,c}
    NodeSet c, ac, abc;
    c.push_back(2);
    ac.push_back(0);
    ac.push_back(2);
    abc.push_back(0);
    abc.push_back(1);
    abc.push_back(2);
    NodeSetSet pmc0, pmc1, pmc2, pmc3;
    RESET(3);
    pmc0 = pmce.get();
    ASSERT_EQUAL(pmc0.size(), 3);
    ASSERT(pmc0.isMember(a));
    ASSERT(pmc0.isMember(b));
    ASSERT(pmc0.isMember(c));
    g.addEdge(0, 1);
    pmce.reset(g);
    pmc1 = pmce.get();
    ASSERT_EQUAL(pmc1.size(), 2);
    ASSERT(pmc1.isMember(ab));
    ASSERT(pmc1.isMember(c));
    g.addEdge(0, 2);
    pmce.reset(g);
    pmc2 = pmce.get();
    ASSERT_EQUAL(pmc2.size(), 2);
    ASSERT(pmc1.isMember(ab));
    ASSERT(pmc1.isMember(ac));
    g.addEdge(1, 2);
    pmce.reset(g);
    pmc3 = pmce.get();
    ASSERT_EQUAL(pmc3.size(), 1);
    ASSERT(pmc1.isMember(abc));

    return true;
}





PMCEnumeratorTester::PMCEnumeratorTester() {

    START_TESTS();

    #define X(_func) DO_TEST(_func);
    TEST_TABLE
    #undef X

    END_TESTS();

}

}

using namespace tdenum;
int main() {
    PMCEnumeratorTester();
    return 0;
}



