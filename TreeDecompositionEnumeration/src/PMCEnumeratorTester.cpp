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
