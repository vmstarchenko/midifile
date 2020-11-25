#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>
#include "MidiFile.h"

BOOST_AUTO_TEST_CASE(test1) {
    smf::MidiFile m1("files/chor001.mid");
    BOOST_CHECK(m1.status());

    smf::MidiFile m2("files/missing_file.mid");
    BOOST_CHECK(!m2.status());

}
