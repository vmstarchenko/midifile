#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>
#include "MidiFile.h"
#include <random>
#include <iostream>
#include "test_outs.hpp"

bool SAVE = false;

using namespace std;
using namespace smf;
void setTempo(MidiFile& midifile, int index, double& tempo) {
   static int count = 1;

   MidiEvent& mididata = midifile[0][index];

   int microseconds =
       (mididata[3] << 16)
       | (mididata[4] << 8)
       | (mididata[5] << 0);

   double newtempo = 60.0 / microseconds * 1000000.0;
   if (count <= 1) {
      tempo = newtempo;
   } else if (tempo != newtempo) {
      cout << "; WARNING: change of tempo from " << tempo
           << " to " << newtempo << " ignored" << endl;
   }
}

stringstream& convertMidiFileToText2(const MidiFile& m, stringstream& out) {
   MidiFile midifile = m;
   for (int i = 0; i < midifile.size(); i++) {
       out << "Track:" << i << endl;
       const auto& track = midifile[i];
       for (int j = 0; j < track.size(); j++) {
	  const auto& msg = track[j];
	  if (msg.size()) {
              out << int(msg[0]);
	  }
	  for (int k = 1; k < msg.size(); ++k) {
              out << ":" << int(msg[k]);
	  }
	  out << endl;
      }
   }

   return out;

}



stringstream& convertMidiFileToText(const MidiFile& m, stringstream& out) {
   MidiFile midifile = m;
   midifile.absoluteTicks();
   midifile.joinTracks();

   vector<double> ontimes(128);
   vector<int> onvelocities(128);
   int i;
   for (i=0; i<128; i++) {
      ontimes[i] = -1.0;
      onvelocities[i] = -1;
   }

   double offtime = 0.0;
   double tempo = 0;

   int key = 0;
   int vel = 0;

   for (i = 0; i < midifile.getNumEvents(0); i++) {
      int command = midifile[0][i][0] & 0xf0;
      if (command == 0x90 && midifile[0][i][2] != 0) {
         key = midifile[0][i][1];
         vel = midifile[0][i][2];
         ontimes[key] = midifile[0][i].tick;
         onvelocities[key] = vel;
      } else if (command == 0x90 || command == 0x80) {
         key = midifile[0][i][1];
         offtime = midifile[0][i].tick;
         out << "note " << ontimes[key]
              << " " << offtime
              << " " << key << " " << onvelocities[key] << endl;
         onvelocities[key] = -1;
         ontimes[key] = -1.0;
      }

      // check for tempo indication
      if (midifile[0][i][0] == 0xff &&
                 midifile[0][i][1] == 0x51) {
         setTempo(midifile, i, tempo);
      }
   }

   return out;

}



string dump(const MidiFile& midifile) {
    stringstream out;
    MidiFile m = midifile;
    // m.writeBinascWithComments(out);
    convertMidiFileToText2(midifile, out);
    return out.str();
}

string read(const string& path) {
    std::ifstream t(path);
    BOOST_CHECK(t.is_open());
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}


void check(const string& data, string filename, bool save=SAVE) {
    if (save) {
        filename = "out/" + filename;
        ofstream f;
        cerr << "Write: " << filename << endl;
        f.open(filename, ifstream::out);
        BOOST_CHECK(f.is_open());
        f << data;
        f.close();
    } else {
       	BOOST_CHECK_EQUAL(TEST_OUTS.at(filename), data);
    }
};

void check(const MidiFile& m, string filename, bool save=SAVE) {
    check(dump(m), filename, save);
}


BOOST_AUTO_TEST_SUITE( test_read_s )

BOOST_AUTO_TEST_CASE(test_read, * boost::unit_test::timeout(2)) {
    // test existed file
    MidiFile m1("files/chor001.mid");
    BOOST_CHECK(m1.status());
    check(m1, "test_read_1");

    // test missed file
    MidiFile m2;
    m2.read("files/missing_file.mid");
    BOOST_CHECK(!m2.status());

    // test other MidiFile
    MidiFile m3(m2);

    // istream
    ifstream f4;
    f4.open ("files/chor001.mid", ifstream::in);
    MidiFile m4(f4);

    {
    // test empty file
    MidiFile t;
    stringstream ss;
    t.read(ss);
    BOOST_CHECK(!t.status());
    }

    {
    // test broken file
    MidiFile t;
    stringstream ss;
    for (int i = 0; i < 10000; ++i) {
        ss << '\0';
    }
    t.read(ss);
    BOOST_CHECK(!t.status());
    }

}

BOOST_AUTO_TEST_SUITE_END()

MidiFile load(const vector<vector<int>>& messages) {
   MidiFile midifile;
   int tracks = 0;
   for (auto& msg : messages) {
       if (msg.size() > 4 && msg[4] > tracks) {
	   tracks = msg[4];
       }
   }
   if (tracks) {
       midifile.addTracks(midifile.size() - tracks + 1);
   }

   int tpq = midifile.getTPQ();
   for (auto& msg : messages) {
       // start, end, key, volume, track, channel, instrument
       int volume = 100, track = 0, channel = 0, instr = 1;
       midifile.addTimbre(track, 0, channel, instr);

       int starttick = msg[0] * tpq / 4;
       int endtick = starttick + msg[1] * tpq / 4;
       int key = msg[2];
       if (msg.size() > 3) {
	   volume = msg[3];
       }
       if (msg.size() > 4) {
	   track = msg[4];
       }
       if (msg.size() > 5) {
	   channel = msg[5];
       }
       if (msg.size() > 6) {
	   instr = msg[6];
       }

       midifile.addNoteOn(track, starttick, channel, key, volume);
       midifile.addNoteOff(track, endtick, channel, key);
   }
   midifile.sortTracks();

   return midifile;
}

const static MidiFile m1 = load({
    // start, end, key, volume, track, channel, instrument
    {0, 1, 60, 100, 0, 0, 1},
    {1, 1, 61, 80, 0, 1, 2}
});


const static MidiFile m2 = load({
    // start, end, key, volume, track, channel, instrument
    {0, 1, 60, 100, 0, 0, 1},
    {1, 1, 61, 80, 1, 1, 2}
});


BOOST_AUTO_TEST_SUITE( test_functions_s )

BOOST_AUTO_TEST_CASE(test_functions) {
    // test existed file
    BOOST_CHECK(m1.status());
    check(m1, "test_functions_m1");

    BOOST_CHECK_EQUAL(m1.size(), 1);

    /*
    MidiFile t0 = m0;
    const MidiEventList& l1 = m1[0];
    MidiEventList& l2 = m2[0];
    */

    {
    auto t = m1;
    t[0][2].resize(0);
    t.removeEmpties();
    check(t, "test_functions_removeEmpties");
    }

    {
    auto t = m2;
    t.joinTracks();
    BOOST_CHECK_EQUAL(t.getTrackCount(), 1);
    BOOST_CHECK(t.hasJoinedTracks());
    check(t, "test_functions_joinTracks");

    t.joinTracks();
    BOOST_CHECK_EQUAL(t.getTrackCount(), 1);
    BOOST_CHECK(t.hasJoinedTracks());
    check(t, "test_functions_joinTracks");
    }

    {
    auto t = m1;
    BOOST_CHECK(!t.hasJoinedTracks());
    t.joinTracks();
    BOOST_CHECK_EQUAL(t.getTrackCount(), 1);
    BOOST_CHECK(t.hasJoinedTracks());
    check(t, "test_functions_joinTracks_single_track");
    }

    {
    auto t = m2;
    t.joinTracks();
    BOOST_CHECK(t.hasJoinedTracks());
    BOOST_CHECK(!t.hasSplitTracks());
    t.splitTracksByChannel();
    BOOST_CHECK(!t.hasJoinedTracks());
    BOOST_CHECK(t.hasSplitTracks());
    check(t, "test_functions_splitTracksByChannel");
    BOOST_CHECK_EQUAL(t.getTrackCount(), 3);
    }

    {
    auto t = m1;
    BOOST_CHECK_EQUAL(t.getFileDurationInTicks(), 60);
    BOOST_CHECK_EQUAL(t.getFileDurationInQuarters(), 0.5);
    BOOST_CHECK_EQUAL(t.getFileDurationInSeconds(), 0.25);
    BOOST_CHECK_EQUAL(t.getEventCount(0), 6);
    }

    {
    auto t = m1;
    t.setTPQ(60);
    BOOST_CHECK_EQUAL(t.getFileDurationInQuarters(), 1);
    BOOST_CHECK_EQUAL(t.getFileDurationInSeconds(), 0.5);
    }



}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE( test_write_s )

BOOST_AUTO_TEST_CASE(test_write) {
   /*
   {
   auto t = m1;

   t.write("files/missed/test_write");
   t.write("files/test_write");
   check(read("files/test_write"), "test_write");
   }
   */

   {
   auto t = m1;
   t.writeHex("files/missed/test_writeHex");
   t.writeHex("files/test_writeHex");
   check(read("files/test_writeHex"), "test_writeHex");
   }

   {
   auto t = m1;
   t.writeBinasc("files/missed/test_writeBinasc");
   t.writeBinasc("files/test_writeBinasc");
   check(read("files/test_writeBinasc"), "test_writeBinasc");
   MidiFile t1("files/test_writeBinasc");
   check(dump(t), "test_readBinasc");
   }

   {
   auto t = m1;
   t.writeBinascWithComments("files/missed/test_writeBinascWithComments");
   t.writeBinascWithComments("files/test_writeBinascWithComments");
   check(read("files/test_writeBinascWithComments"), "test_writeBinascWithComments");
   }
}
BOOST_AUTO_TEST_SUITE_END()
