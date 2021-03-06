//------------------------------------------------------------------------------
// name: ofck-entities.cpp
// desc: graphics objects
//
// @author  Ge Wang
// @author  Tim O'Brien
// @author  Kitty Shi
// @author  Madeline Huberth
//
// @date    Winter 2015
//------------------------------------------------------------------------------
#include "ofck-entities.h"
#include <algorithm>
#include <string>
#include <sstream>
using namespace std;




//------------------------------------------------------------------------------
// name: lowerCase()
// desc: lower case a string
//------------------------------------------------------------------------------
string lowerCase( const string & s )
{
    // copy
    string str = s;
    // to lower each letter via iteration
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    // return
    return str;
}




//------------------------------------------------------------------------------
// name: trimStr()
// desc: trim whitespace from beginning and end of string
//------------------------------------------------------------------------------
string trimStr( const string & str )
{
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last-first+1));
}




//------------------------------------------------------------------------------
// name: makeEntity()
// desc: instantiate entity of certain type
//------------------------------------------------------------------------------
VREntity * VREntityFactory::makeEntity( const std::string & type )
{
    // lower case it
    string s = lowerCase( type );
    // check/remove prefix
    if( s.length() > 2 && s[0] == 'v' && s[1] == 'r' )
    {
        // remove prefix
        s = s.substr(2);
    }

    // the pointer to return
    VREntity * e = NULL;

    // check type
    if( type == "flare" ) { e = new VRFlare(); }
    else if( type == "node" || type == "entity" ) { e = new VREntity(); }
    else if( type == "mesh" ) { e = new VRMeshEntity(); }
    else if( type == "text" ) { e = new VRTextEntity(); }
    else if( type == "lines" ) { e = new VRLinesEntity(); }
    else if( type == "light" ) { e = new VRLightEntity(); }
    else if( type == "trail" ) { e = new VRTrailEntity(); }
    else if( type == "circle" ) { }
    else if( type == "ugen" ) { }
    else if( type == "dot" ) { e = new VRDotEntity(); }
    else if( type == "blowstring" ) { e = new VRBlowStringEntity(); }
	else if( type == "ring" ) { e = new RingArranger(); }

    // log
    if( !e )
    {
        // warning
        cerr << "[VREntityFactory]: unknown type: '" << type << "'..." << endl;
    }

    // done
    return e;
}




//------------------------------------------------------------------------------
// name: VRMeshEntity()
// desc: constructor
//------------------------------------------------------------------------------
VRMeshEntity::VRMeshEntity()
{
    // set draw
    eval( "draw lines" );
    // default
    m_fill = true;
    // default
    m_texture = NULL;
    // line width
    m_lineWidth = 1;
}




//------------------------------------------------------------------------------
// name: render()
// desc: render
//------------------------------------------------------------------------------
void VRMeshEntity::render()
{
    // check if draw texture
    bool drawTexture = (m_texture != NULL);
    // bind texture
    if( drawTexture ) m_texture->getTextureReference().bind();

    // line width
    ofSetLineWidth( m_lineWidth );

    // mesh draw
    if( m_fill )
    {
        m_mesh.draw();
    }
    else
    {
        m_mesh.drawWireframe();
    }

    // unbind texture
    if( drawTexture ) m_texture->getTextureReference().unbind();
}




//------------------------------------------------------------------------------
// name: eval()
// desc: eval command
//------------------------------------------------------------------------------
bool VRMeshEntity::eval( const std::string & theLine )
{
    string line = lowerCase( theLine );

    // word
    string token;
    // string stream
    istringstream istr( line );
    // the command
    string command;
    // get it
    istr >> command;

    // sanity check
    if( command == "" )
    {
        // empty command
        cerr << "[VRMeshEntity]: empty EVAL command!" << endl;
        // done
        return false;
    };

    // the number
    float x, y, z;
    // string
    string str;

    // check
    if( command == "vertex" )
    {
        // loop
        if( istr >> x >> y >> z )
        {
            // push as float
            m_mesh.addVertex( ofVec3f(x,y,z) );
        }
    }
    if( command == "clear" )
    {
        // clear
        m_mesh.clear();
        // TODO: should clearing also clear the texture? then:
        // m_texture = NULL;
    }
    else if( command == "color" )
    {
        // loop
        if( istr >> x >> y >> z )
        {
            // check for alpha value
            float a;
            if( istr >> a )
            {
                // push as float with alpha
                m_mesh.addColor( ofFloatColor(x,y,z,a) );
            }
            else
            {
                // push as float
                m_mesh.addColor( ofFloatColor(x,y,z) );
            }
        }
    }
    else if( command == "normal" )
    {
        // loop
        if( istr >> x >> y >> z )
        {
            // push as float
            m_mesh.addNormal( ofVec3f(x,y,z) );
        }
    }
    else if( command == "uv" )
    {
        // loop
        if( istr >> x >> y )
        {
            // push as float
            m_mesh.addTexCoord( ofVec2f(x,y) );
        }
    }
    else if( command == "texture" )
    {
        // loop
        if( istr >> str )
        {
            // get instance
            OFCKDB * db = OFCKDB::instance();
            // get the image
            m_texture = db->getImage( str );
        }
    }
    else if( command == "draw" )
    {
        // loop
        if( istr >> str )
        {
            // check
            if( str == "points" )
                m_mesh.setMode( OF_PRIMITIVE_POINTS );
            else if( str == "lines" )
                m_mesh.setMode( OF_PRIMITIVE_LINES );
            else if( str == "linestrip" )
                m_mesh.setMode( OF_PRIMITIVE_LINE_STRIP );
            else if( str == "lineloop" )
                m_mesh.setMode( OF_PRIMITIVE_LINE_LOOP );
            else if( str == "triangles" )
                m_mesh.setMode( OF_PRIMITIVE_TRIANGLES );
            else if( str == "trianglestrip" )
                m_mesh.setMode( OF_PRIMITIVE_TRIANGLE_STRIP );
            else if( str == "trianglefan" )
                m_mesh.setMode( OF_PRIMITIVE_TRIANGLE_FAN );
            else
            {
                // error
                cerr << "[VRMeshEntity]: invalid DRAW type: '" << str << "'" << endl;
                // done
                return false;
            }
        }
    }
    else if( command == "generate" )
    {
        // get from stream
        if( istr >> str )
        {
            // check
            if( str == "sphere" )
            {
                float radius = 0;
                int res = 12;
                // get them
                if( !(istr >> radius) )
                {
                    // error
                    cerr << "[VRMeshEntity]: GENERATE SPHERE missing radius..." << endl;
                    // done
                    return false;
                }
                // get them
                if( !(istr >> res) )
                {
                    // set default
                    res = 12;
                }
                // set it
                m_mesh = ofMesh::sphere( radius, res );
            }
        }
    }
    else if( command == "mode" )
    {
        // get from stream
        if( !(istr >> str) )
        {
            // error
            cerr << "[VRMeshEntity]: MODE missing parameter..." << endl;
            // done
            return false;
        }

        m_fill = (str != "wireframe");
    }
    else if( command == "load" )
    {
        // get from stream
        if( !(istr >> str) )
        {
            // error
            cerr << "[VRMeshEntity]: LOAD missing parameter..." << endl;
            // done
            return false;
        }

        // load this
        loadOBJ( str );
    }
    else if( command == "update" )
    {
        // get from stream
        if( !(istr >> str) )
        {
            // error
            cerr << "[VRMeshEntity]: UPDATE missing parameter..." << endl;
            // done
            return false;
        }
        int index;
        if( !(istr >> index) )
        {
            // error
            cerr << "[VRMeshEntity]: UPDATE missing index..." << endl;
            // done
            return false;
        }

        // vertex
        if( str == "vertex" )
        {
            if( !(istr >> x >> y >> z) )
            {
                // error
                cerr << "[VRMeshEntity]: UPDATE vertex not enough values..." << endl;
                // done
                return false;
            }
            m_mesh.setVertex( index, ofVec3f(x,y,z) );
        }
        // color
        else if( str == "color" )
        {
            if( !(istr >> x >> y >> z) )
            {
                // error
                cerr << "[VRMeshEntity]: UPDATE color not enough values..." << endl;
                // done
                return false;
            }
            // check for alpha
            float a;
            if( istr >> a )
            {
                m_mesh.setColor( index, ofFloatColor(x,y,z,a) );
            }
            else
            {
                m_mesh.setColor( index, ofFloatColor(x,y,z) );
            }
        }
        // texture
        else if( str == "uv" )
        {
            if( !(istr >> x >> y) )
            {
                // error
                cerr << "[VRMeshEntity]: UPDATE uv not enough values..." << endl;
                // done
                return false;
            }
            m_mesh.setTexCoord( index, ofVec2f(x,y) );
        }
        // normal
        else if( str == "normal" )
        {
            if( !(istr >> x >> y >> z) )
            {
                // error
                cerr << "[VRMeshEntity]: UPDATE normal not enough values..." << endl;
                // done
                return false;
            }
            m_mesh.setNormal( index, ofVec3f(x,y,z) );
        }
        // error
        else
        {
            // error
            cerr << "[VRMeshEntity]: UPDATE unrecognized type..." << endl;
            // done
            return false;
        }
    }
}




//-----------------------------------------------------------------------------
// name: loadOBJ()
// desc: load OBJ file
//-----------------------------------------------------------------------------
bool VRMeshEntity::loadOBJ( const std::string & path )
{
    // out vectors
    vector<ofVec3f> verts;
    vector<ofVec3f> normals;
    vector<ofVec2f> uvs;

    // load it
    bool r = loadOBJFile( path, verts, normals, uvs );
    // check it
    if( !r ) return false;

    // clear
    m_mesh.clear();

    // iterate
    for( int i = 0; i < verts.size(); i++ )
    {
        // add vertex
        m_mesh.addVertex( verts[i] );
        // set normal state
        m_mesh.addNormal( normals[i] );
        // set uv
        m_mesh.addTexCoord( uvs[i] );
    }

    // done
    return true;
}




//-----------------------------------------------------------------------------
// name: loadOBJ()
// desc: obj loader
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
//-----------------------------------------------------------------------------
// (from tutorial)
// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide :
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc
//-----------------------------------------------------------------------------
bool VRMeshEntity::loadOBJFile(
    const string & path,
    vector<ofVec3f> & out_vertices,
    vector<ofVec3f> & out_normals,
    vector<ofVec2f> & out_uvs )
{
    // log
    cerr << "[ofxChucK]: loading OBJ file: " << path << endl;

    // some vectors
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<ofVec3f> temp_vertices;
    std::vector<ofVec3f> temp_normals;
    std::vector<ofVec2f> temp_uvs;

    // clear
    out_vertices.clear();
    out_normals.clear();
    out_uvs.clear();

    // open file
    FILE * file = fopen( ofToDataPath(path).c_str(), "r" );
    // check
    if( file == NULL )
    {
        // log
        cerr << "[ofxChucK]: cannot open OBJ file..." << endl;
        // done
        return false;
    }

    // start reading
    while( true )
    {
        // c string
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf( file, "%s", lineHeader );
        // end of file, break
        if( res == EOF ) break;

        // parse header
        if( strcmp( lineHeader, "v" ) == 0 )
        {
            // vertex
            ofVec3f vertex;
            // read x, y, z
            fscanf( file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            // append
            temp_vertices.push_back( vertex );
        }
        else if( strcmp( lineHeader, "vt" ) == 0 )
        {
            // texture coordinate
            ofVec2f uv;
            // read u, v
            fscanf( file, "%f %f\n", &uv.x, &uv.y );
            // invert (DDS); remove for TGA/BMP loaders
            uv.y = -uv.y;
            // append
            temp_uvs.push_back(uv);
        }
        else if( strcmp( lineHeader, "vn" ) == 0 )
        {
            // normal
            ofVec3f normal;
            // read x, y, z
            fscanf( file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            // append
            temp_normals.push_back(normal);
        }
        else if( strcmp( lineHeader, "f" ) == 0 )
        {
            // vertices
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            // read it
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                                 &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                 &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                                 &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            // check matches
            if( matches != 9 )
            {
                // log
                cerr << "[ofxChucK]: cannot parse on 'f'" << endl;
                // done
                return false;
            }

            // push it
            vertexIndices.push_back( vertexIndex[0] );
            vertexIndices.push_back( vertexIndex[1] );
            vertexIndices.push_back( vertexIndex[2] );
            uvIndices    .push_back( uvIndex[0] );
            uvIndices    .push_back( uvIndex[1] );
            uvIndices    .push_back( uvIndex[2] );
            normalIndices.push_back( normalIndex[0] );
            normalIndices.push_back( normalIndex[1] );
            normalIndices.push_back( normalIndex[2] );
        }
        else if( lineHeader[0] == '#' )
        {
            // read rest of line
            char buffer[1024];
            fgets( buffer, 1024, file);
        }
        else
        {
            // log
            cerr << "[ofxChucK]: unrecognized line header: " << lineHeader << endl;
            return false;
        }
    }

    // for each vertex of each triangle
    for( unsigned int i = 0; i < vertexIndices.size(); i++ )
    {
        // get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int normalIndex = normalIndices[i];
        unsigned int uvIndex = uvIndices[i];

        // get the attributes thanks to the index
        ofVec3f vertex = temp_vertices[ vertexIndex-1 ];
        ofVec3f normal = temp_normals[ normalIndex-1 ];
        ofVec2f uv = temp_uvs[ uvIndex-1 ];

        // put the attributes in buffers
        out_vertices.push_back(vertex);
        out_normals.push_back(normal);
        out_uvs.push_back(uv);
    }

    return true;
}




//------------------------------------------------------------------------------
// name: VRTextEntity()
// desc: constructor
//------------------------------------------------------------------------------
VRTextEntity::VRTextEntity()
{
    // default
    m_sizeToLoad = 64;
}




//------------------------------------------------------------------------------
// name: update()
// desc: update state
//------------------------------------------------------------------------------
void VRTextEntity::update( double dt )
{
    // check
    if( m_fontToLoad != "" )
    {
        // load it
        m_font.loadFont( m_fontToLoad, m_sizeToLoad );
        // clear
        m_fontToLoad = "";
    }
}




//------------------------------------------------------------------------------
// name: render()
// desc: render
//------------------------------------------------------------------------------
void VRTextEntity::render()
{
    // disable depth
    ofDisableDepthTest();
    // the size
    int size = m_font.getSize();
    // check
    if( size > 0 )
    {
        // scale down
        ofScale(1.0/size, 1.0/size, 1.0/size);
        // draw it
        m_font.drawString( m_text, 0, 0 );
    }
}




//------------------------------------------------------------------------------
// name: eval()
// desc: eval command
//------------------------------------------------------------------------------
bool VRTextEntity::eval( const std::string & theLine )
{
    // line
    string line = lowerCase( theLine );

    // token
    string token;
    // string stream
    istringstream istr(line);
    // the command
    string command;
    // get it
    istr >> command;

    // sanity check
    if( command == "" ) return false;

    // check
    if( command == "load" )
    {
        // read
        if( !(istr >> m_fontToLoad) )
        {
            // error
            cerr << "[VRTextEntity]: LOAD not enough arguments..." << endl;
            // done
            return false;
        }
        // read
        if( !(istr >> m_sizeToLoad) )
        {
            m_sizeToLoad = 64;
        }
    }
    else if( command == "set" )
    {
        // argument
        string text;
        // get rest of string
        getline( istr, text );
        // trim it
        trimStr( text );
        // read
        if( text == "" )
        {
            // error
            cerr << "[VRTextEntity]: SET not enough arguments..." << endl;
            // done
            return false;
        }
        // load it
        m_text = text;
    }

    return true;
}




//------------------------------------------------------------------------------
// name: VRLineEntity()
// desc: constructor
//------------------------------------------------------------------------------
VRLinesEntity::VRLinesEntity()
{
    // do nothing
}




//------------------------------------------------------------------------------
// name: render()
// desc: render
//------------------------------------------------------------------------------
void VRLinesEntity::render()
{
    // actual size
    int N = m_vertices.size() / 2 * 2;
    // loop over lines
    for( int i = 0; i < N; i+=2 )
    {
        // render
        ofLine( m_vertices[i].x, m_vertices[i].y, m_vertices[i].z,
                m_vertices[i+1].x, m_vertices[i+1].x, m_vertices[i+1].x );
    }
}




//------------------------------------------------------------------------------
// name: eval()
// desc: eval command
//------------------------------------------------------------------------------
bool VRLinesEntity::eval( const std::string & theLine )
{
    string line = lowerCase( theLine );

    // word
    string token;
    // string stream
    istringstream istr( line );
    // the command
    string command;
    // get it
    istr >> command;

    // sanity check
    if( command == "" )
    {
        // empty command
        cerr << "[VRLinesEntity]: empty EVAL command!" << endl;
        // done
        return false;
    };

    // check
    if( command == "add" ) // add point
    {
        // the number
        float x1, y1, z1, x2, y2, z2;

        // loop
        if( istr >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 )
        {
            // push as float
            m_vertices.push_back( Vector3D(x1,y1,z1) );
            m_vertices.push_back( Vector3D(x2,y2,z2) );
        }
    }
}




//------------------------------------------------------------------------------
// name: VRFlare()
// desc: constructor
//------------------------------------------------------------------------------
VRFlare::VRFlare()
{
    // zero out
    m_imageRef = NULL;
    // default is additive
    m_blendMode = OF_BLENDMODE_ADD;

    // add the four corners to our mesh
    m_mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

    // prepare to set color
    ofColor color;
    color.set(col.x, col.y, col.z);

    // rect width / 2
    float rw = 0.5;
    // rect height / 2: use aspect ratio of 1:1 until we get an image
    float rh = rw;

    // point 0 (tex coords are normalized for use with OculusDK2)
    m_mesh.addVertex(ofPoint(-rw, -rh));
    m_mesh.addTexCoord(ofPoint(0, 1));
    m_mesh.addColor(color);

    // point 1
    m_mesh.addVertex(ofPoint(rw, -rh));
    m_mesh.addTexCoord(ofPoint(0, 0));
    m_mesh.addColor(color);

    // point 2
    m_mesh.addVertex(ofPoint(-rw, rh));
    m_mesh.addTexCoord(ofPoint(1, 1));
    m_mesh.addColor(color);

    // point 3
    m_mesh.addVertex(ofPoint(rw, rh));
    m_mesh.addTexCoord(ofPoint(1, 0));
    m_mesh.addColor(color);
}




//------------------------------------------------------------------------------
// name: ~VRFlare()
// desc: destructor
//------------------------------------------------------------------------------
VRFlare::~VRFlare()
{
    // do nothing for now
}




//------------------------------------------------------------------------------
// name: setImage()
// desc: set image for drawing
//------------------------------------------------------------------------------
void VRFlare::setImage( ofImage * imageRef )
{
    // set the image
    m_imageRef = imageRef;

    // check reference
    if( m_imageRef )
    {
        // update mesh
        updateMeshPoints();
    }
}




//------------------------------------------------------------------------------
// name: setImage()
// desc: set image for drawing
//------------------------------------------------------------------------------
void VRFlare::setImage( const string & key )
{
    // get instance
    OFCKDB * db = OFCKDB::instance();
    // get and set the image
    setImage( db->getImage( key ) );
}




//------------------------------------------------------------------------------
// name: updateMeshPoints()
// desc: update the mesh points with the new aspect ratio
//------------------------------------------------------------------------------
void VRFlare::updateMeshPoints()
{
    // rect width / 2
    float rw = 0.5;
    // rect height / 2: use aspect ratio of image
    float rh = rw * m_imageRef->getHeight() / m_imageRef->getWidth();

    m_mesh.setVertex(0, ofPoint( -rw, -rh ));
    m_mesh.setVertex(1, ofPoint( -rw,  rh ));
    m_mesh.setVertex(2, ofPoint(  rw, -rh ));
    m_mesh.setVertex(3, ofPoint(  rw,  rh ));
}




//------------------------------------------------------------------------------
// name: eval()
// desc: set parameters
//------------------------------------------------------------------------------
bool VRFlare::eval( const std::string & theLine )
{
    string line = lowerCase( theLine );

    // string stream
    istringstream istr( line );
    // the command
    string command;
    // get it
    istr >> command;

    // set num sources
    if( command == "texture" )
    {
        // the key
        string key;

        // loop
        if( !(istr >> key) )
        {
            // empty command
            cerr << "[VRFlare]: TEXTURE missing key!" << endl;
            // done
            return false;
        }
        else
        {
            // set
            setString( "texture", key );
        }
    }
    else
    {
        // done
        return VREntity::eval( theLine );
    }

    // handled
    return true;
}




//------------------------------------------------------------------------------
// name: update()
// desc: update the flare state
//------------------------------------------------------------------------------
void VRFlare::update( double dt )
{
    // look up and set image ref
    setImage( getString("texture") );

    // reset color
    ofColor color;
    color.set(col.x, col.y, col.z, alpha);
    // UPDATE TODO: in OF 0.9.0, this can be updated to setColorForIndices()
    for( int i = 0; i < 4; i++ )
    {
        m_mesh.setColor(i, color);
    }
}




//------------------------------------------------------------------------------
// name: render()
// desc: render the flare
//------------------------------------------------------------------------------
void VRFlare::render()
{
    // check
    if( !m_imageRef ) return;

    // disable depth
    ofDisableDepthTest();
    // blending
    ofEnableBlendMode( m_blendMode );

    // bind texture and draw
    m_imageRef->getTextureReference().bind();
    m_mesh.draw();
    m_imageRef->getTextureReference().unbind();

    // blending
    ofEnableBlendMode( OF_BLENDMODE_ALPHA );
}




//------------------------------------------------------------------------------
// name: VRLightEntity()
// desc: constructor
//------------------------------------------------------------------------------
VRLightEntity::VRLightEntity()
{
    numSources = 3;
    // textureKey = "";
    // initialize vector
    // update(0);
}




//------------------------------------------------------------------------------
// name: eval()
// desc: set parameters
//------------------------------------------------------------------------------
bool VRLightEntity::eval( const std::string & theLine )
{
    string line = lowerCase( theLine );

    // string stream
    istringstream istr( line );
    // the command
    string command;
    // get it
    istr >> command;

    // set num sources
    if( command == "num" )
    {
        // the number
        int num;

        // loop
        if( !(istr >> num) )
        {
            // empty command
            cerr << "[VRLightEntity]: NUM missing number!" << endl;
            // done
            return false;
        }
        else
        {
            // set
            numSources = num;
        }
    }
    // set rotation speed
    else if( command == "rotate" || command == "rotatey" )
    {
        // the number
        float rotate;

        // loop
        if( !(istr >> rotate) )
        {
            // empty command
            cerr << "[VRLightEntity]: ROTATE missing rotation speed!" << endl;
            // done
            return false;
        }
        else
        {
            // set
            intrinsicRotation.y = rotate;
        }
    }
    else if( command == "rotatex" )
    {
        // the number
        float rotate;

        // loop
        if( !(istr >> rotate) )
        {
            // empty command
            cerr << "[VRLightEntity]: ROTATEX missing rotation speed!" << endl;
            // done
            return false;
        }
        else
        {
            // set
            intrinsicRotation.x = rotate;
        }
    }
    else if( command == "rotatez" )
    {
        // the number
        float rotate;

        // loop
        if( !(istr >> rotate) )
        {
            // empty command
            cerr << "[VRLightEntity]: ROTATEZ missing rotation speed!" << endl;
            // done
            return false;
        }
        else
        {
            // set
            intrinsicRotation.z = rotate;
        }
    }
    else
    {
        // done
        return VRFlare::eval( theLine );
    }

    // handled
    return true;
}




//------------------------------------------------------------------------------
// name: update()
// desc: update the light state
//------------------------------------------------------------------------------
void VRLightEntity::update( double dt )
{
    // rotate
    intrinsicOri.x += intrinsicRotation.x;
    intrinsicOri.y += intrinsicRotation.y;
    intrinsicOri.z += intrinsicRotation.z;

    // call parent class
    VRFlare::update( dt );
}




//------------------------------------------------------------------------------
// name: render()
// desc: render the light
//------------------------------------------------------------------------------
void VRLightEntity::render()
{
    // check
    if( !m_imageRef ) return;

    // disable depth
    ofDisableDepthTest();
    // blending
    ofEnableBlendMode( m_blendMode );

    // rotation
    ofRotate( intrinsicOri.x, 1, 0, 0 );
    ofRotate( intrinsicOri.y, 0, 1, 0 );
    ofRotate( intrinsicOri.z, 0, 0, 1 );

    // bind texture and draw
    m_imageRef->getTextureReference().bind();
    {
        // angle between each
        float angleInc = 180.0f / numSources;
        // for each flare
        for( int i = 0; i < numSources; i++ )
        {
            // draw
            m_mesh.draw();
            // rotate
            ofRotate( angleInc, 0, 1, 0 );
        }
    }
    // unbind texture
    m_imageRef->getTextureReference().unbind();

    // blending
    ofEnableBlendMode( OF_BLENDMODE_ALPHA );
}




//------------------------------------------------------------------------------
// name: VRTrailEntity()
// desc: constructor
//------------------------------------------------------------------------------
VRTrailEntity::VRTrailEntity()
{
    // fill
    m_fill = true;
    // line width
    m_lineWidth = 2;
    // set default
    setLength( 64 );
    // default mode
    eval( "draw linestrip" );
}




//------------------------------------------------------------------------------
// name: update()
// desc: update
//------------------------------------------------------------------------------
void VRTrailEntity::update( double dt )
{
    // clear mesh
    m_mesh.clear();
    // variables
    float x, y, z; Vector3D v, c = col*(1/255.0f);
    // alpha inc based on length of tail
    float inc = 1.0f/m_vertices.size(), a = alpha/255, t = 1;
    // iterate over vertices
    deque<Vector3D>::iterator it = m_vertices.begin();
    for( ; it != m_vertices.end(); it++ )
    {
        // get vertex
        v = *it;
        // add vertex
        m_mesh.addVertex( ofVec3f(v.x, v.y, v.z) );
        // add color
        m_mesh.addColor( ofFloatColor(c.x, c.y, c.z, a*t*t*t) );
        // update a
        t-= inc;
    }
}




//------------------------------------------------------------------------------
// name: render()
// desc: render
//------------------------------------------------------------------------------
void VRTrailEntity::render()
{
    // check if draw texture
    // bool drawTexture = (m_texture != NULL);
    // bind texture
    // if( drawTexture ) m_texture->getTextureReference().bind();

    // no lighting for now
    ofDisableLighting();
    // enable depth testing
    ofEnableDepthTest();
    // blending
    ofEnableBlendMode( OF_BLENDMODE_ALPHA );
    // point size
    ofSetLineWidth( m_lineWidth );

    // mesh draw
    if( m_fill ) m_mesh.draw();
    else m_mesh.drawWireframe();
    // disable blending
    ofDisableBlendMode();

    // unbind texture
    // if( drawTexture ) m_texture->getTextureReference().unbind();
}




//------------------------------------------------------------------------------
// name: eval()
// desc: command: set parameters
//------------------------------------------------------------------------------
bool VRTrailEntity::eval( const std::string & theLine )
{
    string line = lowerCase( theLine );

    // word
    string token;
    // string stream
    istringstream istr( line );
    // the command
    string command;
    // get it
    istr >> command;

    // sanity check
    if( command == "" )
    {
        // empty command
        cerr << "[VRTrailEntity]: empty EVAL command!" << endl;
        // done
        return false;
    };

    // the number
    float x, y, z;
    // string
    string str;

    // check
    if( command == "add" )
    {
        // loop
        if( istr >> x >> y >> z )
        {
            // add vertex
            addVertex( Vector3D(x,y,z) );
        }
    }
    else if( command == "length" )
    {
        // get
        if( istr >> x )
        {
            setLength( x );
        }
    }
    else if( command == "clear" )
    {
        // clear
        this->clear();
    }
    else if( command == "draw" )
    {
        // loop
        if( istr >> str )
        {
            // check
            if( str == "points" )
                m_mesh.setMode( OF_PRIMITIVE_POINTS );
            else if( str == "linestrip" )
                m_mesh.setMode( OF_PRIMITIVE_LINE_STRIP );
            else if( str == "trianglestrip" )
                m_mesh.setMode( OF_PRIMITIVE_TRIANGLE_STRIP );
            else
            {
                // error
                cerr << "[VRTrailEntity]: invalid DRAW type: '" << str << "'" << endl;
                // done
                return false;
            }
        }
    }
    else
    {
        // super class
        VREntity::eval( theLine );
    }

    return true;
}




//------------------------------------------------------------------------------
// name: clear()
// desc: clear tail points
//------------------------------------------------------------------------------
void VRTrailEntity::clear()
{
    // clear it
    m_vertices.clear();
}




//------------------------------------------------------------------------------
// name: addVertex()
// desc: add a new point
//------------------------------------------------------------------------------
void VRTrailEntity::addVertex( const Vector3D & v3 )
{
    // check size
    if( m_vertices.size() >= m_length ) m_vertices.pop_back();
    // add most recent point
    m_vertices.push_front( v3 );
}




//------------------------------------------------------------------------------
// name: setLength()
// desc: resize length of trail
//------------------------------------------------------------------------------
void VRTrailEntity::setLength( int N )
{
    // set the length
    m_length = N;
    // get rid of stuff
    while( m_vertices.size() > m_length ) m_vertices.pop_back();
}




//------------------------------------------------------------------------------
// name: VRDotEntity()
// desc: constructor
//------------------------------------------------------------------------------
VRDotEntity::VRDotEntity()
: sphere( 1, 10 )
{}




//------------------------------------------------------------------------------
// name: render()
// desc: draw the thing
//------------------------------------------------------------------------------
void VRDotEntity::render()
{
    // ofSetColor( 255 );
    sphere.draw();
}




//------------------------------------------------------------------------------
// name: VRBlowStringEntity()
// desc: constructor
//------------------------------------------------------------------------------
VRBlowStringEntity::VRBlowStringEntity()
{
    // zero out
    m_imageRef = NULL;
    // default is additive
    m_blendMode = OF_BLENDMODE_ADD;

    // form mesh
    formMesh();

    time = 0;
    animationAmount = 0;
    animationSpeed = 0;
    animationPhase = 0;
}




//------------------------------------------------------------------------------
// name: setImage()
// desc: set image for drawing
//------------------------------------------------------------------------------
void VRBlowStringEntity::setImage( ofImage * imageRef )
{
    // set the image
    m_imageRef = & imageRef->getTextureReference();
}




//------------------------------------------------------------------------------
// name: setImage()
// desc: set image for drawing
//------------------------------------------------------------------------------
void VRBlowStringEntity::setImage( const string & key )
{
    setImage(OFCKDB::instance()->getImage(key));
}

//------------------------------------------------------------------------------
// name: formMesh()
// desc: construct the mesh after the image is set
//------------------------------------------------------------------------------
void VRBlowStringEntity::formMesh()
{
    glowMesh.clear();
    glowMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

    ofColor color;
    color.set(col.x, col.y, col.z);

    // Bottom left
    glowMesh.addVertex(ofPoint(-defaultWidth/2, 0));
    glowMesh.addTexCoord(ofPoint(0, 0));
    glowMesh.addColor(color);

    // Bottom right
    glowMesh.addVertex(ofPoint(defaultWidth/2, 0));
    glowMesh.addTexCoord(ofPoint(1.0, 0));
    glowMesh.addColor(color);

    // Middle left
    glowMesh.addVertex(ofPoint(-defaultWidth/2, defaultHeight / 2));
    glowMesh.addTexCoord(ofPoint(0, 0.5));
    glowMesh.addColor(color);

    // Middle right
    glowMesh.addVertex(ofPoint(defaultWidth/2, defaultHeight / 2));
    glowMesh.addTexCoord(ofPoint(1.0, 0.5));
    glowMesh.addColor(color);

    // Top left
    glowMesh.addVertex(ofPoint(-defaultWidth/2, defaultHeight));
    glowMesh.addTexCoord(ofPoint(0, 1.0));
    glowMesh.addColor(color);

    // Top right
    glowMesh.addVertex(ofPoint(defaultWidth/2, defaultHeight));
    glowMesh.addTexCoord(ofPoint(1.0, 1.0));
    glowMesh.addColor(color);
}

void VRBlowStringEntity::updateMesh()
{
    // Compute new midpoint positions
    vector<ofVec3f> & vertices = glowMesh.getVertices();
    vertices[2].x = - defaultWidth/2 + animationAmount/20 * sin(animationSpeed * time + animationPhase);
    vertices[3].x = defaultWidth/2 + animationAmount/20 * sin(animationSpeed * time + animationPhase);

    time += 0.4;

    // Update color in case that has changed
    for (int i = 0; i < glowMesh.getVertices().size(); i++) {
        glowMesh.setColor(i, ofColor(col.x, col.y, col.z));
    }
}



//------------------------------------------------------------------------------
// name: update()
// desc: update the blowstring state
//------------------------------------------------------------------------------
void VRBlowStringEntity::update( double dt )
{
    // look up and set image ref
    setImage( getString("texture") );
    // animate the mesh
    updateMesh();
}




//------------------------------------------------------------------------------
// name: render()
// desc: render the blowstring
//------------------------------------------------------------------------------
void VRBlowStringEntity::render()
{
    // check
    if( !m_imageRef ) return;

    // blending
    ofEnableBlendMode( m_blendMode );

    // bind texture and draw
    m_imageRef->bind();
    glowMesh.draw();
    m_imageRef->unbind();

    // alpha is default
    ofEnableBlendMode( OF_BLENDMODE_ALPHA );
}

//------------------------------------------------------------------------------
// name: eval()
// desc: set the animation amount
//------------------------------------------------------------------------------
bool VRBlowStringEntity::eval( const std::string & theLine )
{
    // line
    string line = lowerCase( theLine );

    // string stream
    istringstream istr(line);
    // the command
    string command;
    // get it
    istr >> command;

    float readVal;

    // sanity check
    if( command == "" ) return false;

    // check
    if( command == "speed" )
    {
        // read
        if( !(istr >> readVal) )
        {
            // error
            cerr << "[VRBlowStringEntity]: SPEED not enough arguments..." << endl;
            // done
            return false;
        }
        else
        {
            animationSpeed = readVal;
        }
    }
    else if( command == "amount" )
    {
        // read
        if( !(istr >> readVal) )
        {
            // error
            cerr << "[VRBlowStringEntity]: AMOUNT not enough arguments..." << endl;
            // done
            return false;
        }
        else
        {
            animationAmount = readVal;
        }
    }
    else if( command == "phase" )
    {
        // read
        if( !(istr >> readVal) )
        {
            // error
            cerr << "[VRBlowStringEntity]: PHASE not enough arguments..." << endl;
            // done
            return false;
        }
        else
        {
            animationPhase = readVal;
        }
    }
    else if( command == "texture" )
    {
        string textureKey;
        // read
        if( !(istr >> textureKey) )
        {
            // error
            cerr << "[VRBlowStringEntity]: TEXTURE not enough arguments..." << endl;
            // done
            return false;
        }
        else
        {
            setString( "texture", textureKey );
        }
    }
    else
    {
        // error
        cerr << "[VRBlowStringEntity]: unrecognized command..." << endl;
        // done
        return false;
    }

    return true;
}

RingArranger::RingArranger()
{
	radius = DEFAULT_RING_RADIUS;
}

RingArranger::~RingArranger()
{
	//dtor
}

bool RingArranger::eval( const std::string & theLine )
{
	string line = lowerCase( theLine );
	// string stream
    istringstream istr( line );
    // the command
    string command;
    // get it
    istr >> command;

    if(command == "radius")
	{
        float r;
        if(istr >> r)
		{
            radius = r;
		}
	}
	else
    {
        // empty command
        cerr << "[RingArranger]: invalid EVAL command! (only 'radius [float]' is valid)" << endl;
        // done
        return false;
    };
}

void RingArranger::modifyChildren()
{
	//TODO: could probably make this more efficient by only re-arranging when radius or number of children has changed.
	//(However, that would have the side effect that children whose positions are moved from the circle would stay moved till radius or number of children change)

    for(int i = 0; i < children.size(); i++)
    {
        float angle = ((float)i/(float)children.size()) * 2 * PI;
        children[i]->loc.set(radius * cos(angle), 0, radius * sin(angle));
    }
}
