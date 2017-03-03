[[67,70,74,75],
[67,70,74,75],
[67,70,74,75],
[66,69,74,75],
[66,69,74,75],
[65,68,74,75],
[65,68,74,75],
[65,68,74,75],
[64,67,74,75],
[64,67,74,75],
[63,67,72,74],
[63,67,72,74],
[63,67,72,74],
[63,67,72,74],
[62,67,72,74],
[62,67,72,74],
[62,67,72,74],
[62,67,72,74],
[60,66,69,72,66,69,72,75,69,72,75,72,75,78,75,78,81,78,81,84]] @=> int arpChords[][];

[1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1] @=> int chordChanges[];

[[55],
[58],
[62],
[61],
[54],
[53],
[56],
[61],
[60],
[52],
[51,50,51],
[55],
[63],
[62],
[50,49,50],
[55],
[62],
[61],
[50,54,57,54,57,60,57,60,63,60,63,66,69,66,63,60,63,60,57,54]] @=> int melodySeq[][];

[8, 4, 4, 4, 12, 11, 3, 2, 4, 12, 8, 3, 3, 2, 8, 3, 3, 2, 20] @=> int melodyBeats[];


[[67,70,74],
[67,70,74],
[67,70,74],
[66,69,74],
[66,69,74],
[65,68,74],
[65,68,74],
[65,68,74],
[64,67,72],
[64,67,72],
[63,67,72],
[63,67,72],
[63,67,72],
[63,67,72],
[62,67,70],
[62,67,70],
[62,67,70],
[62,67,70],
[62,66,69]] @=> int heldChords[][];


//*********************************************** Look-at detection ***********************************************/

class myVec {
    float x;
    float y;
    float z;
}

fun myVec locToMyVec(VREntity v)
{
    myVec result;
    v.loc.x => result.x;
    v.loc.y => result.y;
    v.loc.z => result.z;
    return result;
}

fun myVec oriToMyVec(VREntity v)
{
    //This gets a lot more hairy because the ori vector is in degrees of rotation about that axis,
    //but we need an actual vector. This rotates the point (0, 0, 1) about the appropriate vectors.
    myVec result;
    //First, convert all our rotations to radians
    v.ori.x * Math.PI / 180.0 => float xRot;
    v.ori.y * Math.PI / 180.0 => float yRot;
    v.ori.z * Math.PI / 180.0 => float zRot;
    
    //The following is a baked in "rotation about x, then y, then z" matrix transformation of (0, 0, 1)
    Math.cos(xRot) * Math.sin(yRot) * Math.cos(zRot) + Math.sin(xRot) * Math.sin(zRot) => result.x;
    -Math.sin(xRot) * Math.cos(zRot) + Math.cos(xRot) * Math.sin(yRot) * Math.sin(zRot) => result.y;
    Math.cos(xRot) * Math.cos(yRot) => result.z;
    return result;
}

fun myVec vecDiff(myVec v1, myVec v2)
{
    myVec result;
    v1.x - v2.x => result.x;
    v1.y - v2.y => result.y;
    v1.z - v2.z => result.z;
    return result;
}

fun myVec vecCross(myVec u, myVec v)
{
    myVec result;
    u.y * v.z - u.z * v.y => result.x;
    u.z * v.x - u.x * v.z => result.y;
    u.x * v.y - u.y * v.x => result.z;
    return result;
}

fun float vecLength(myVec v)
{
    return Math.sqrt(Math.pow(v.x, 2) + Math.pow(v.y, 2) + Math.pow(v.z, 2));
}

//Given VREntities h and o, returns how close h is to pointing to o
fun float gazeDistance(VREntity h, VREntity o)
{
    locToMyVec(o) @=> myVec p0;
    locToMyVec(h) @=> myVec p1;
    oriToMyVec(h) @=> myVec p2;
    //HACK: To only check distance along the x-z plane, set y to 0 in all these vectors
    0 => p0.y;
    0 => p1.y;
    0 => p2.y;
    vecLength(vecCross(vecDiff(p2, p1),vecDiff(p1, p0))) => float num;
    vecLength(vecDiff(p2, p1)) => float den;
    return num / den;
}

//*********************************************** End look-at detection ***********************************************/


//*********************************************** Gametrak stuff ***********************************************/

// z axis deadzone
.032 => float DEADZONE;

// which joystick
0 => int device;
// get from command line
if( me.args() ) me.arg(0) => Std.atoi => device;

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
int isBtnDown;
int prevIsBtnDown;

// HID objects
Hid trak;
HidMsg msg;

// open joystick 0, exit on fail
if( !trak.openJoystick( device ) ) me.exit();

// print
<<< "joystick '" + trak.name() + "' ready", "" >>>;

// spork control
spork ~ gametrak();
// print
//spork ~ print();

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
                    if( msg.which != 2 && msg.which != 5 )
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
                isBtnDown => prevIsBtnDown;
                1 => isBtnDown;
                onBtnDown();
            }
            
            // joystick button up
            else if( msg.isButtonUp() )
            {
                isBtnDown => prevIsBtnDown;
                0 => isBtnDown;
            }
        }
    }
}

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
    //.2::second => now; //added delay for latency purposes (wanted sound to line up with intention), beat seems to be coming early
	0.1::second => now; //Decreased latency since we're now lining up on beats
    if ( g_beatZ == 1 ) hearFoundBeat(g_beatZ);
    0 => g_beatZ;
    0 => g_downBeat;
    detectBeat();
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
        } 
    }
}


//*********************************************** End Gametrak stuff ***********************************************/

//Toggle for whether to listen to gestures
0 => int gesturesActive;

//Event handler for button presses
fun void onBtnDown()
{
    !gesturesActive => gesturesActive;
}

//Constants for UGens in Players
400 => float LOWPASS_MIN;

//Tempo
90 => float tempo; //Defined in BPM
4 => float beatSubdivisions;
((60.0/tempo)/beatSubdivisions)::second => dur beat; //Our "beat" unit duration

//The maximum number of renderings for a player. Defined external to the player class cause some other code sometimes needs it
90 => int MAX_QTY;

//LFO for "hover" animation for player renderings. All players have to share a single set because ChucK starts choking if you create too many SinOscs.
SinOsc hover[MAX_QTY];
for(0 => int i; i < MAX_QTY; i++)
{
	hover[i] => blackhole;
	if(i == 0)
		0.5 => hover[i].freq;
	else
		Math.random2f(0.3,1.0) => hover[i].freq;
}

//*********************************************** Performer class ***********************************************/
class Player
{
    //The following variables define the color of our object
	0.0 => float default_r;
	0.0 => float default_g;
	0.0 => float default_b;
    default_r => float base_r;
    default_g => float base_g;
    default_b => float base_b;
    0.7 => float saturation;
    1.0 => float value;
    1.0 => float alpha;
    
    //Active?
    0 => int isActive;
    
    //Base size
    float baseSize;
	
	//Number of objects rendered
	1 => int numObj;
	
	
	fun int getNumObj()
	{
		return numObj;
	}
	
	//VR Entity
	VREntity models[MAX_QTY];
    
    //Ugens to put controls on
    Gain vol;
    0 => vol.gain;
    LPF lowpass;
	Chorus chorus;
	
    
    JCRev reverb;
    0.05 => reverb.mix;
	
	0 => chorus.mix;
	0.5 => chorus.modDepth;
	0.1 => chorus.modFreq;
	
	10000 => float lowpassRange;
    

    
    //Envelope for "pulsing" to the beat
    100::ms => dur PULSE_DUR;
    Envelope pulseEnv => blackhole;
    PULSE_DUR => pulseEnv.duration;
	
	7.0 => float INITIAL_RING_SPACING;
	15.0 => float RING_SPACING;
	1.5 => float POS_VARIANCE;
	
    
    fun void setup(string modelName, int posIndex, int numPlayers)
    {
		//Set up some positioning stuff
		Math.PI / (numPlayers) => float wedgeSize;
		1 => int rowPos;
		
		for(0 => int i; i < MAX_QTY; i++)
		{
			VR.makeEntity(modelName + "_" + i, "light") @=> models[i];
			models[i].eval("num 3");
			posIndex + (rowPos/(rowNum(i) + 1.0)) => float wedgeFraction;
			rowPos++;
			if(rowPos > rowNum(i))
				1 => rowPos;
			wedgeFraction * wedgeSize => float angle;
			(rowNum(i)-1) * RING_SPACING + INITIAL_RING_SPACING=> float r;
			r * Math.cos(angle) => models[i].loc.x;
			r * Math.sin(angle) => models[i].loc.z;
			if(i != 0)
			{
				models[i].loc.x + Math.random2f(-POS_VARIANCE, POS_VARIANCE) => models[i].loc.x;
				models[i].loc.z + Math.random2f(-POS_VARIANCE, POS_VARIANCE) => models[i].loc.z;
			}
			VR.root().addChild(models[i]);
		}
		updateRotation();
		updateColor();
    }
	
	fun int rowNum(int idx)
	{
		0 => int cur;
		0 => int i;
		while(cur <= idx)
		{
			(i+1) +=> cur;
			i++;
		}
		return i;
	}
    
	fun void setTexture(string tex)
	{
		for(0 => int i; i < MAX_QTY; i++)
			models[i].setString( "texture", tex);
	}
	
    fun void setBaseSize(float newSize)
    {
        newSize => baseSize;
        updateSize();
    }
	
	fun void setLowpassRange(float newRange)
	{
		newRange => lowpassRange;
		LOWPASS_MIN + lowpassRange => lowpass.freq;
	}
    
    fun void updateSize()
    {
		for(0 => int i; i < MAX_QTY; i++)
		{
			baseSize * vol.gain() * 20 * (1.0 + 0.1 * pulseEnv.value()) => float newSize;
			models[i].sca.setAll(newSize);
		}
    }
	
	fun void updateQty(int newQty)
	{
		if(newQty > MAX_QTY)
			MAX_QTY => numObj;
		else if(newQty < 1)
			1 => numObj;
		else
			newQty => numObj;
		updateColor();
		
		//Correlated audio updates
		(numObj - 1.0)/(MAX_QTY - 1.0) => float percent;
		if(percent < 0)
			0 => percent;
		//Chorus
		percent * 0.3 => chorus.mix;
		percent * 0.2 => reverb.mix;
	}
    
    fun void updateRotation()
    {
        reverb.mix() * 20 => float rotateVal;
		for(0 => int i; i < MAX_QTY; i++)
			models[i].eval("rotate " + rotateVal);
    }
    
    fun void setSoundSource(UGen source)
    {
        //Signal chain defined here (if changing, be sure to also change EnvPlayer)
        source => chorus => lowpass => reverb => vol => dac;
    }
    
    fun void adjustGain(float deltaGain)
    {
        deltaGain * 3 + vol.gain() => float newGain;
        if(newGain < 0) 
        {
            0 => newGain;
        }
        newGain => vol.gain;
        updateSize();
    }
	
	fun void adjustLowpass(float deltaFreq)
    {
        lowpass.freq() + deltaFreq => float newFreq;
		if(newFreq < LOWPASS_MIN)
			LOWPASS_MIN => newFreq;
		if(newFreq > (LOWPASS_MIN + lowpassRange))
			(LOWPASS_MIN + lowpassRange) => newFreq;
		newFreq => lowpass.freq;
        (newFreq / (lowpassRange + LOWPASS_MIN)) * 0.6 + 0.1 => saturation;
    }
    
    fun void adjustReverb(float newVerb)
    {
        newVerb => reverb.mix;
        updateRotation();
    }
	
	fun void adjustChorus(float newChorus)
	{
		newChorus => chorus.mix;
	}
    
	500::ms => dur HEIGHT_CHANGE_DUR;
	Envelope heightSmoother => blackhole;
	HEIGHT_CHANGE_DUR => heightSmoother.duration;
	0 => heightSmoother.target;
	
	false => int prevActive;
    fun void update(int areGesturesActive)
    {
        if(isActive)
        {
			for(0 => int i; i < MAX_QTY; i++)
			{
				hover[i].last() => float newY;
				if(!prevActive)
				{
					if(i == 0)
						0 => hover[i].phase;
					else
						Math.random2(0,1)/2.0 => hover[i].phase;
					0 => newY;
				}
				newY => models[i].loc.y;
			}
            if(areGesturesActive)
                setBaseColor(0, 1, 0);
            else
                setBaseColor(0, 0, 1);
        }
        else
        {
			for(0 => int i; i < MAX_QTY; i++)
			{
				if(prevActive)
				{
					models[i].loc.y => heightSmoother.value;
					0 => heightSmoother.target;
				}
				heightSmoother.value() => models[i].loc.y;
			}
            setBaseColor(1, 0.8, 0);
        }
        updateColor();
        updateSize();
		isActive => prevActive;
    }
	
	fun void setX(float newX)
	{
		for(0 => int i; i < MAX_QTY; i++)
			newX => models[i].loc.x;
	}
    
	fun void setDefaultColor(float r, float g, float b)
	{
		r => default_r;
		g => default_g;
		b => default_b;
	}
	
    fun void setBaseColor(float r, float g, float b)
    {
        r => base_r;
        g => base_g;
        b => base_b;
    }
	
	200::ms => dur COLOR_CHANGE_DUR;
	Envelope alphaSmoothers[MAX_QTY];
	for(0 => int i; i < MAX_QTY; i++)
	{
		alphaSmoothers[i] => blackhole;
		COLOR_CHANGE_DUR => alphaSmoothers[i].duration;
	}
	
    fun void updateColor()
    {
        (base_r + (1.0 - base_r) * (1.0 - saturation)) * value => float r;
        (base_g + (1.0 - base_g) * (1.0 - saturation)) * value => float g;
        (base_b + (1.0 - base_b) * (1.0 - saturation)) * value => float b;
		
		for(0 => int i; i < MAX_QTY; i++)
		{
			alpha => float myAlpha;
			if(i >= numObj)
				0 => myAlpha;
			if(alphaSmoothers[i].value() != myAlpha)
			{
				myAlpha => alphaSmoothers[i].target;
			}
			models[i].rgba.set(r, g, b, alphaSmoothers[i].value());
		}
    }
    
    fun void onSeqChange()
    {
        //Player "pulses"
        spork ~ pulse(); //TODO: differnet players need to pulse at different triggers
        
        //Pass control to sub-classes
        handleSeqChange();
    }
	
	fun void onNewBeat()
	{
		handleBeat();
	}
    
    fun void pulse()
    {
        pulseEnv.keyOn();
        PULSE_DUR => now;
        pulseEnv.keyOff();
        PULSE_DUR => now;
    }
    
	fun void handleBeat()
	{
		//Intentionally blank
	}
	
    fun void handleSeqChange()
    {
        //Intentionally blank
    }
}

class EnvPlayer extends Player
{
    ADSR env;
    env.set(50::ms, 10::ms, .5, 50::ms);
    
    fun void setSoundSource(UGen source)
    {
        //Signal chain defined here (if changing, consider changing base class as well)
        source => env => chorus => lowpass => reverb => vol => dac;
    }
    
    fun void playNote()
    {
        spork ~ runEnvelope();
    }
    
    fun void runEnvelope()
    {
        env.keyOn();
        200::ms => now;
        env.keyOff();
        50::ms => now;
    }
}

class SeqPlayer extends Player
{
    int seqs[][];
    -1 => int curSeq;
	-1 => int curBeat;
    1.0/4.0 => float beatSubdivision;
    
    fun void playNote(float freq)
    {
        //Intentionally Blank
    }
    
    fun void handleSeqChange()
    {
        curSeq++;
        if(curSeq >= seqs.size())
            0 => curSeq;
		-1 => curBeat;
        
        // spork ~ runCurSeq();
    }
    
	fun void handleBeat()
	{
		//Advance beat in seq
		curBeat++;
		if(curBeat < seqs[curSeq].size()) //Only play if we haven't reached the end of the sequence yet.
		{
			//Translate midi note for current note to freq
			seqs[curSeq][curBeat] => Std.mtof => float curFreq;
			//Play note
			if(curFreq > 0)
				playNote(curFreq);
		}
	}
}

class RhodesSeqPlayer extends SeqPlayer
{
    Rhodey rhode;
    setSoundSource(rhode);
    0.2 => vol.gain;
    0 => rhode.freq;
    
    fun void playNote(float freq)
    {
        freq => rhode.freq;
        1 => rhode.noteOn;
    }
}

class ArpgPlayer extends EnvPlayer
{
    int chords[][];
    -1 => int curChord;
	-1 => int curBeat;
    false => int arpsRunning; //So we can give an initial downbeat
    
    fun void setFreq(float freq)
    {
        //Intentionally blank
    }
    
    fun void handleSeqChange()
    {
        curChord++;
        if(curChord >= chords.size())
            0 => curChord;
		if(chordChanges[curChord])
			-1 => curBeat;
        
        //If not running, kick arpeggios off
        if(!arpsRunning)
        {
            true => arpsRunning;
            // spork ~ runArp();
        }
    }
	
	fun void handleBeat()
	{
		if(arpsRunning)
		{
			curBeat++;
			if(curBeat >= chords[curChord].size())
				0 => curBeat;
			
			chords[curChord][curBeat] => Std.mtof => float curFreq;
			if(curFreq > 0)
			{
				setFreq(curFreq);
				playNote();
			}
		}
	}
}

class SqrArpgPlayer extends ArpgPlayer
{
    SqrOsc square;
    setSoundSource(square);
    0.2 => vol.gain;
    0 => square.freq;
    
    fun void setFreq(float freq)
    {
        freq => square.freq;
    }
}

class ChordPlayer extends Player
{
	int chords[][];
    -1 => int curChord;
    
    fun void setFreqs(int freq[])
    {
        //Intentionally blank
    }
    
    fun void handleSeqChange()
    {
        curChord++;
        if(curChord >= chords.size())
            0 => curChord;
		
		setFreqs(chords[curChord]);
    }
}

class OrganPlayer extends ChordPlayer
{
	Gain hub;
	FMVoices inst1 => hub;
	FMVoices inst2 => hub;
	FMVoices inst3 => hub;
	0.25 => inst1.gain;
	0.25 => inst2.gain;
	0.25 => inst3.gain;
	1.0 => hub.gain;
	setSoundSource(hub);
	0.2 => vol.gain;
	
	fun void setFreqs(int freq[])
	{
		Std.mtof(freq[0]) => inst1.freq;
		Std.mtof(freq[1]) => inst2.freq;
		Std.mtof(freq[2]) => inst3.freq;
		
		inst1.noteOn(0.8);
		inst2.noteOn(0.8);
		inst3.noteOn(0.8);
	}
}


//*********************************************** Set up ***********************************************/

//Define head entity
VR.head() @=> VREntity head;

//Parameters
1 => float sphereRadius; //Size to pass to each player
0 => int USE_BUTTON_TOGGLE; //Set to 1 to have button toggle gestures active/inactive, 0 to have gestures active only when button is held
3.5 => float MIN_GAZE_THRESHOLD; //User's gaze must come at least this close to a player for the system to consider it being looked at

//Create and set up players array
3 => int NUM_PLAYERS;
Player @ players[NUM_PLAYERS];
new SqrArpgPlayer @=> players[0];
new RhodesSeqPlayer @=> players[1];
new OrganPlayer @=> players[2];
for(0 => int i; i < NUM_PLAYERS; i++)
{
    players[i].setup("player" + i, i, NUM_PLAYERS);
    players[i].setBaseSize(sphereRadius);
}
players[0].setTexture("naryu");
players[1].setTexture("din");
players[2].setTexture("farore");
players[0].setLowpassRange(10000);
players[1].setLowpassRange(5000);
players[2].setLowpassRange(5000);


//Player positions on stage
arpChords @=> (players[0]$ArpgPlayer).chords;
melodySeq @=> (players[1]$SeqPlayer).seqs;
heldChords @=> (players[2]$ChordPlayer).chords;


//Next-note HUD
VR.makeEntity("noteRing", "flare") @=> VREntity noteRing;
noteRing.setString("texture", "noteRing");
noteRing.rgba.set(1,1,1,1);
5 => noteRing.loc.x;
10 => noteRing.loc.y;
10 => noteRing.loc.z;
10.0 / 16.0 => float hudBeatLength; // Fit 16 beats in 10 screen distance units

VREntity hudNotes[melodyBeats.size()];
0.0 => float curXOffset;
for(0 => int i; i < hudNotes.size(); i++)
{
	VR.makeEntity("hudNote_" + i, "flare") @=> hudNotes[i];
	hudNotes[i].setString("texture", "note");
	hudNotes[i].rgba.set(1,1,1,1);
	-curXOffset => hudNotes[i].loc.x;
	melodyBeats[i] * hudBeatLength +=> curXOffset;
	noteRing.addChild(hudNotes[i]);
}

VR.makeEntity("dummyHead", "node") @=> VREntity dummyHead;
head.loc.x => dummyHead.loc.x;
head.loc.y => dummyHead.loc.y;
head.loc.z => dummyHead.loc.z;
head.ori.x => dummyHead.ori.x;
head.ori.y => dummyHead.ori.y;
head.ori.z => dummyHead.ori.z;
dummyHead.addChild(noteRing);
VR.root().addChild(dummyHead);


//Floor texture
VR.makeEntity("floor", "flare") @=> VREntity floor;
floor.setString("texture", "floorTex");
90 => floor.ori.x;
floor.sca.setAll(0);
floor.rgba.set(0,0.3,0,1);
VR.root().addChild(floor);
Envelope floorSizeEnv => blackhole;
0 => floorSizeEnv.value;
beat * 20 => floorSizeEnv.duration;



//Stars
VR.makeEntity("stars","mesh") @=> VREntity stars;
stars.eval("draw", "points");
1000 => float starRadius;
2000 => int NUM_STARS;
Envelope starsEnv => blackhole;
0 => starsEnv.value;
beat * 20 => starsEnv.duration;
for(0 => int i; i < NUM_STARS; i++)
{
    vec3 v1;
	-1 => v1.y;
	
	while(v1.y < 0) //Reject points below the ground
	{
		Math.TWO_PI * Math.randomf() => float theta;
		Math.acos(2.0 * Math.randomf() - 1.0) => float phi;
		
		starRadius * Math.sin(phi) * Math.cos(theta) => v1.x;
		starRadius * Math.sin(phi) * Math.sin(theta) => v1.y;
		starRadius * Math.cos(phi) => v1.z;
	}

    stars.eval( "vertex", v1 );
	
	stars.rgba.set(1,1,1,0);
}
VR.root().addChild( stars );

VR.allLightsOff();

fun float[] array_mtof(int midiNotes[])
{
    float result[midiNotes.size()];
    for(0 => int i; i < midiNotes.size(); i++)
    {
        if(midiNotes[i] <= 0)
            midiNotes[i] => result[i];
        else
            Std.mtof(midiNotes[i]) => result[i];
    }
    return result;  
}

//*********************************************** Beat/Seq event handlers ***********************************************/
false => int firstDownbeatFound;
fun void hearFoundBeat( int isABeat ){
    if ( isABeat == 1 ) 
    {
        advanceSeq();
    }
}

-1 => int curSeqIndex;
-1 => int curBeatIndex;
false => int isCrowdRun;
fun void advanceSeq()
{
	if(curSeqIndex == 18 && curBeatIndex < 19)
		return; //Disallow progressing the sequence during the crowd transition
	curSeqIndex++;
	-1 => curBeatIndex;
    for(0 => int i; i < NUM_PLAYERS; i++)
    {
        players[i].onSeqChange();
    }
	if(!firstDownbeatFound)
	{
		true => firstDownbeatFound;
		spork ~ beatDriver();
	}
}

fun void advanceBeat()
{
	curBeatIndex++;
	if(curSeqIndex == 19)
	{
		0 => curSeqIndex;
		!isCrowdRun => isCrowdRun;
	}
	if(curBeatIndex == 0)
	{
		//Do some stuff with the rhythm hud if this is the start of a new sequence
		if(hudNotes[curSeqIndex].loc.x < 0) //If the user triggered the beat early...
		{
			//Teleport hud notes over so that the current frontrunner is at 0
			-hudNotes[curSeqIndex].loc.x => float hudXShift;
			for(0 => int i; i < hudNotes.size(); i++)
			{
				hudNotes[i].loc.x + hudXShift => hudNotes[i].loc.x;
			}
		}
		//Bump frontrunner hud note to the back of the line
		0 => int beatSum;
		for(0 => int i; i < melodyBeats.size(); i++)
			melodyBeats[i] +=> beatSum;
		-beatSum * hudBeatLength => hudNotes[curSeqIndex].loc.x;
		
		
		//On crowd on/off, trigger envelopes for floor and stars
		if(curSeqIndex == 18)
		{
			if(!isCrowdRun)
			{
				1000 => floorSizeEnv.target;
				starsEnv.keyOn();
			}
			else
			{
				0 => floorSizeEnv.target;
				starsEnv.keyOff();
			}
		}
	}
    for(0 => int i; i < NUM_PLAYERS; i++)
    {
        players[i].onNewBeat();
		if(curSeqIndex == 18)
		{
			if(isCrowdRun)
				players[i].updateQty(players[i].getNumObj() - 5);
			else
				players[i].updateQty(players[i].getNumObj() + 5);
		}
    }
}

fun void beatDriver()
{
	while(true)
	{
		advanceBeat();
		beat => now;
	}
}

//*********************************************** Main control loop ***********************************************/
time prevFrameTime;
while(true)
{
	//Sync dummy head with head
	-head.ori.x => dummyHead.ori.x;
	head.ori.y => dummyHead.ori.y;
	head.ori.z => dummyHead.ori.z;
	
    //Determine which player is active
    -1 => int selectedPlayerIndex;
    999999999 => float minGazeDist;
    for(0 => int i; i < NUM_PLAYERS; i++)
    {
        gazeDistance(head, players[i].models[0]) => float curGazeDist;
        if(curGazeDist < minGazeDist)
        {
            curGazeDist => minGazeDist;
            i => selectedPlayerIndex;
        }
        0 => players[i].isActive;
    }
    if(minGazeDist <= MIN_GAZE_THRESHOLD)
        1 => players[selectedPlayerIndex].isActive;
    
    //Run updates for each player, pass gametrax controls to active player if appropriate
    (USE_BUTTON_TOGGLE && gesturesActive) || (!USE_BUTTON_TOGGLE && isBtnDown) => int allowGestures;
    for(0 => int i; i < NUM_PLAYERS; i++)
    {
        players[i].update(allowGestures);
        
        if(allowGestures && players[i].isActive)
        {
            players[i].adjustGain(gt.axis[2] - gt.lastAxis[2]);
            players[i].adjustLowpass(((gt.axis[1] - gt.lastAxis[1])) * 4 * players[i].lowpassRange);
        }
    }
	
	//Update notes HUD
	now - prevFrameTime => dur frameDur;
	now => prevFrameTime;
	curSeqIndex + 1 => int curHudNoteIndex;
	if(curHudNoteIndex >= hudNotes.size())
		hudNotes.size() -=> curHudNoteIndex;
	if(hudNotes[curHudNoteIndex].loc.x < 0)
	{
		(tempo * 4) / 60.0 * hudBeatLength * (frameDur / 1::second) => float dx;
		for(0 => int i; i < hudNotes.size(); i++)
		{
			if(hudNotes[i].loc.x + dx > 0)
				0 => hudNotes[i].loc.x;
			else
				hudNotes[i].loc.x + dx => hudNotes[i].loc.x;
		}
	}
	
	//Update floor
	floor.sca.setAll(floorSizeEnv.value());
	
	//Update stars
	starsEnv.value() => stars.rgba.a;
	.1 +=> stars.ori.y;
	if(stars.ori.y > 360)
		360 -=> stars.ori.y;
	
    // advance time
    VR.displaySync() => now;
}