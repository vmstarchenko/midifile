#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>
#include "MidiFile.h"
#include <random>
#include <iostream>
#include "test_outs.hpp"

bool SAVE = true;

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
    m.writeBinascWithComments(out);
    // convertMidiFileToText(midifile, out);
    return out.str();
}




void check(const MidiFile& m, string filename, bool save=SAVE) {
    string data = dump(m);
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


BOOST_AUTO_TEST_CASE(test_read) {
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
}

MidiFile load(const vector<vector<int>>& messages) {
   MidiFile midifile;

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


BOOST_AUTO_TEST_CASE(test_functions) {
    // test existed file
    MidiFile m1 = load({
       // start, end, key, volume, track, channel, instrument
       {0, 1, 60, 100, 0, 0, 1},
       {1, 1, 61, 80, 0, 1, 2}
    });
    BOOST_CHECK(m1.status());
    const MidiFile m2 = m1;

    check(m1, "test_functions_m1");

    BOOST_CHECK_EQUAL(m1.size(), 1);

    MidiEventList& l1 = m1[0];
    const MidiEventList& l2 = m2[0];

    m1.removeEmpties();
}

BOOST_AUTO_TEST_CASE(test_write) {
   /*
   BOOST_TEST(dump(load({
       {0, 1, 60, 100},
       {1, 1, 60, 100}
   })) == (
       "note 0 1 60 100\n"
       "note 1 1 60 100\n"
   ));
   */


   /*
   midifile.write("files/test_write_1");
   midifile.write("files/missed/test_write_2");
   midifile.writeHex("files/test_write_3");
   midifile.writeHex("files/missed/test_write_4");
   midifile.writeBinasc("files/test_write_5");
   midifile.writeBinasc("files/missed/test_write_6");
   midifile.writeBinascWithComments("files/test_write_7");
   midifile.writeBinascWithComments("files/missed/test_write_8");
   */

}
