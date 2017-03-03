// x LFO
SinOsc x => blackhole;
// y LFO
SinOsc y => blackhole;

// set frequency
1 => x.freq;
// set frequency
1 => y.freq;
// set phase
pi/4 => x.phase;

// make a dot
VR.makeEntity("dot","dot");
// get a name 
VR.getEntity("dot") @=> VREntity dot;
// print pointer
<<< "dot:", dot >>>;
// set color
 dot.rgba.setAll(1);
// set scale
 dot.sca.setAll(1);

// infinite time loop
while( true )
{
    // control location of dot
    x.last() * 5 => dot.loc.x;
    y.last() * 5 => dot.loc.y;

    // print out
    <<< dot.loc >>>;

    // when there is a new graphics draw
    VR.displaySync() => now;
}
