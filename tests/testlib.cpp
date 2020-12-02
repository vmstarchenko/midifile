#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>
#include "MidiFile.h"
#include <random>
#include <iostream>

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


class TestOut : public stringstream {
public:
    void check(string filename, bool clear=true, bool save=true) {
	filename = "out/" + filename;
        if (save) {
            ofstream f;
	    cerr << "Write: " << filename << endl;
            f.open(filename, ifstream::out);
	    BOOST_CHECK(f.is_open());
            f << rdbuf();
	    f.close();
        } else {
            ifstream f;
            std::stringstream buffer;
            f.open(filename, ifstream::in);
	    BOOST_CHECK(f.is_open());
            buffer << f.rdbuf();
	    f.close();
            BOOST_TEST(buffer.str() == str());
        }

        if (clear) {
            str(string());
        }

    }
};


stringstream& convertMidiFileToText(MidiFile& midifile, stringstream& out) {
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

   for (i=0; i<midifile.getNumEvents(0); i++) {
      int command = midifile[0][i][0] & 0xf0;
      if (command == 0x90 && midifile[0][i][2] != 0) {
         // store note-on velocity and time
         key = midifile[0][i][1];
         vel = midifile[0][i][2];
         ontimes[key] = midifile[0][i].tick;
         onvelocities[key] = vel;
      } else if (command == 0x90 || command == 0x80) {
         // note off command write to output
         key = midifile[0][i][1];
         offtime = midifile[0][i].tick;
         out << "note\t" << ontimes[key]
              << "\t" << offtime
              << "\t" << key << "\t" << onvelocities[key] << endl;
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

TestOut& convertMidiFileToText(MidiFile& midifile, TestOut& out) {
    convertMidiFileToText(midifile, (stringstream&) out);
    return out;
}

BOOST_AUTO_TEST_CASE(test_read) {
    // test existed file
    MidiFile m1("files/chor001.mid");
    BOOST_CHECK(m1.status());
    TestOut out;
    convertMidiFileToText(m1, out).check("test_read_1");

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

BOOST_AUTO_TEST_CASE(test_functions) {
    // test existed file
    MidiFile m1("files/chor001.mid");
    const MidiFile m2("files/chor001.mid");

    BOOST_CHECK(m1.status());
    m1.size();
    MidiEventList& l1 = m1[0];
    const MidiEventList& l2 = m2[0];

    m1.removeEmpties();
    m1.markSequence();
    m1.markSequence(0);
    m1.markSequence(1000);
    m1.clearSequence();
    m1.clearSequence(0);
    m1.clearSequence(1000);
    m1.joinTracks();
    m1.splitTracks();
    m1.splitTracksByChannel();
}

MidiFile buildMidiFile(const vector<vector<int>>& messages) {
   MidiFile midifile;
   int track   = 0;
   int channel = 0;
   int instr   = 1;
   midifile.addTimbre(track, 0, channel, instr);

   int tpq     = midifile.getTPQ();
   int count   = 20;
   for (auto& msg : messages) {
      int starttick = int(msg[0] * tpq);
      cout << "TICKS" << starttick << endl;
      int endtick   = starttick + int(msg[1] * tpq);
      int key       = msg[2];
      midifile.addNoteOn(track, starttick, channel, key, msg[3]);
      midifile.addNoteOff(track, endtick,   channel, key);
   }
   midifile.sortTracks();

   return midifile;
}

BOOST_AUTO_TEST_CASE(test_write) {
   MidiFile midifile = buildMidiFile({
       {0, 1, 60, 100}
   });
   TestOut out;
   convertMidiFileToText(midifile, out).check("test_write_1");


   midifile.write("files/test_write_1");
   midifile.write("files/missed/test_write_2");
   midifile.writeHex("files/test_write_3");
   midifile.writeHex("files/missed/test_write_4");
   midifile.writeBinasc("files/test_write_5");
   midifile.writeBinasc("files/missed/test_write_6");
   midifile.writeBinascWithComments("files/test_write_7");
   midifile.writeBinascWithComments("files/missed/test_write_8");

}
