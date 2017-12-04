#include "TestInterface.h"

namespace tdenum {

bool TestInterface::cleanup = false;

const TestInterface& TestInterface::start_tester() const {
    cout << endl
         << "========START '" << tester_name << "'========" << endl;
    return *this;
}
const TestInterface& TestInterface::end_tester() const {
    cout << "======== END '" << tester_name << "' ========" << endl
         << "PASSED: " << total_passed << "/" << total_tests << endl;
    return *this;
}

TestInterface::TestInterface(const string& t_name) :
              tester_name(t_name),
              tmp_dir_name(string(".tmp_") + utils__replace_substr_with_substr(t_name, " ", "_")),
              failed(false),
              done(false),
              total_passed(0),
              total_tests(0)
{
    if (!utils__delete_dir(tmp_dir_name)) {
        TRACE(TRACE_LVL__WARNING, "Couldn't delete '" << tmp_dir_name << "' before test start");
    }
    else if (!utils__mkdir(tmp_dir_name)) {
        TRACE(TRACE_LVL__ERROR, "Couldn't create '" << tmp_dir_name << "' before test start");
    }
}
TestInterface::~TestInterface() {
    if (cleanup && !utils__delete_dir(tmp_dir_name)) {
        TRACE(TRACE_LVL__WARNING, "Couldn't delete directory '" << tmp_dir_name << "'");
    }
}

TestInterface& TestInterface::start() {
    start_tester();
    go();
    done = true;
    end_tester();
    return *this;
}
bool TestInterface::test_failed() const { return failed; }
bool TestInterface::test_passed() const { return !test_failed(); }

// May be useful in macros (see DO_TEST)
TestInterface::operator bool() { return (done ? test_passed() : start().test_passed()); }
TestInterface::operator bool() const { return (done && test_passed()); }

// Set or unset the cleanup bit
void TestInterface::set_cleanup() { cleanup = true; }
void TestInterface::unset_cleanup() { cleanup = false; }

}
