// name: conducting_z_beat_only
//
// author: VR Fellowship, Madeline
// date: fall 2015

// z axis deadzone
0 => float DEADZONE;

// which joystick
0 => int device;
// get from command line
if( me.args() ) me.arg(0) => Std.atoi => device;

// HID objects
Hid trak;
HidMsg msg;

// open joystick 0, exit on fail
if( !trak.openJoystick( device ) ) me.exit();

// print
<<< "joystick '" + trak.name() + "' ready", "" >>>;

// data structure for gametrak
class GameTrak
{
    // timestamps
    time lastTime;
    time currTime;
    
    // previous axis data
    float lastAxis[6];
    // current axis data
    float axis[6];
}

// gametrack
GameTrak gt;

// spork control
spork ~ gametrak();
// print
//spork ~ print();

// ===========  BEAT DETECTION BEGINS HERE ========== //
spork ~ detectBeat();

//GLOBAL BEAT VARIABLES
0 => int g_beatZ;
0 => int g_downBeat; //added but not yet used, wanted to try to detect what the beat pattern was

//looks at Z axis to determine if beat is present
fun int detectBeat()
{
    // array for Z beat detection
    float zLog[0];
    now => time startBeat;
    
    while ( g_beatZ == 0 ) 
    {
        zLog << gt.axis[5];
        10::ms => now;
        spork ~ passZAxisThreshold(zLog);
     }   
    .2::second => now; //added delay for latency purposes (wanted sound to line up with intention), beat seems to be coming early
    if ( g_beatZ == 1 ) hearFoundBeat(g_beatZ);
    0 => g_beatZ;
    0 => g_downBeat;
    detectBeat();
}

//sin osc to hear beat
SinOsc m => ADSR env => Gain g => dac;
env.set(50::ms, 10::ms, .5, 50::ms);

fun void hearFoundBeat( int isABeat ){
    if ( isABeat == 1 ) 
    {
        <<< "its a Z beat!" >>>;
        //gt.axis[2] => g.gain; // can add for gain on z-axis of L tether
        440 => m.freq;
        env.keyOn();
        60::ms => now;
        env.keyOff();
        50::ms => now;
    }
    //else <<< "condition for beat not met yet..." >>>;
}

//checks for a change in direction of the z axis
fun void passZAxisThreshold( float array[] )
{
    10 => int checkLength;
    if( array.size() > checkLength ) {
        array.size() - checkLength => int initialLoc;
        array[initialLoc] => float smallest;
        array[initialLoc] => float largest;
        0 => int smallestLoc => int largestLoc;
        for (array.size() - checkLength => int i; i < array.size(); i++ )
        {
            if ( array[i] < smallest ) 
            {
                array[i] => smallest;
                array.size() - i => smallestLoc;
            }
            if ( array[i] > largest ) 
            {
                array[i] => largest;
                array.size() - i => largestLoc;
            }
            
        } 
        if ( ( largest - smallest > .01 ) && largestLoc < smallestLoc )
        {
            1 => g_beatZ;
        } 
        if ( ( largest - smallest > .03 ) && largestLoc < smallestLoc )
        {
            1 => g_downBeat;
            <<< "DOWNBEAT" >>>;
        } 
    }
}


//makes an avg location of where the gt tether was 
fun float movingAvg( float array[] )
{
    6 => int movAvgWindow;
    0 => float avgLoc;
    for (array.size() - movAvgWindow => int i; i < array.size(); i++ )
    {
        array[i] +=> avgLoc;
    }
    avgLoc / movAvgWindow => avgLoc;
    return avgLoc;
}



// main loop
while( true )
{
    100::ms => now;
}

// print
fun void print()
{
    // time loop
    while( true )
    {
        // values
        <<< "axes:", gt.axis[0],gt.axis[1],gt.axis[2], gt.axis[3],gt.axis[4],gt.axis[5] >>>;
        // advance time
        100::ms => now;
    }
}

// gametrack handling
fun void gametrak()
{
    while( true )
    {
        // wait on HidIn as event
        trak => now;
        
        // messages received
        while( trak.recv( msg ) )
        {
            // joystick axis motion
            if( msg.isAxisMotion() )
            {            
                // check which
                if( msg.which >= 0 && msg.which < 6 )
                {
                    // check if fresh
                    if( now > gt.currTime )
                    {
                        // time stamp
                        gt.currTime => gt.lastTime;
                        // set
                        now => gt.currTime;
                    }
                    // save last
                    gt.axis[msg.which] => gt.lastAxis[msg.which];
                    // the z axes map to [0,1], others map to [-1,1]
                    if( msg.which != 2 && msg.which != 5 && msg.which != 3)
                    { msg.axisPosition => gt.axis[msg.which]; }
                    else
                    {
                        1 - ((msg.axisPosition + 1) / 2) - DEADZONE => gt.axis[msg.which];
                        if( gt.axis[msg.which] < 0 ) 0 => gt.axis[msg.which];
                    }
                }
            }
            
            // joystick button down
            else if( msg.isButtonDown() )
            {
                <<< "button", msg.which, "down" >>>;
                //if(fp == 1) gt.axis[5] => upperHeight;
                //if(fp == 2) gt.axis[5] => lowerHeight;
            }
            
            // joystick button up
            else if( msg.isButtonUp() )
            {
                <<< "button", msg.which, "up" >>>;
            }
        }
    }
}
