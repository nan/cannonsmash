/***********************************************************************
 *	d:/Werk/src/csmash-0.3.8.new/conv/parts.cpp
 *	$Id$
 *
 *	Copyright by ESESoft.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions
 *	are met:
 *
 *	Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer. 
 *
 *	Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer
 *	in the documentation and/or other materials provided with the
 *	distribution. 
 *
 *	The name of the author may not be used to endorse or promote
 *	products derived from this software without specific prior written
 *	permission. 
 *
 *	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 *	OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *	ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***********************************************************************/

#include "ttinc.h"
#include "LoadImage.h"

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <algorithm>

#include "float"
#include "matrix"
#include "affine"

#include "parts.h"

#define RENDEREDGE 0

vector4F Affine2Quaternion(affine4F aff);
affine4F Quaternion2Affine(vector4F v, vector4F p);

// for(;;) namescoping hack.
// VC++ 6 is not compliant with latest ANSI C++ (but VC++7 does).
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
# define for if(0);else for
#endif
#define elif else if

#define BEGIN_ANONYMOUS namespace {
#define END_ANONYMOUS }
/***********************************************************************
 *	Local data and functions
 ***********************************************************************/
BEGIN_ANONYMOUS

inline bool streq(const char *a, const char *b)
{
    return 0 == strcmp(a, b);
}

template <typename T>
inline bool between(const T& a, const T& x, const T& b) {
    return a <= x && x <= b;
}

template <typename T>
inline const T& clamp(const T& a, const T& x, const T& b) {
    if (a > x) return a;
    elif (b < x) return b;
    else return x;
}

struct plane_t {
    short poly[4];
    short cindex;
    
    inline short& operator [](int i) { return poly[i]; }
    inline int size() const { return (poly[3] < 0) ? 3 : 4; }
};

END_ANONYMOUS
/***********************************************************************
 *	Class colormap
 ***********************************************************************/
bool colormap::load(const char *file)
{
    fill(color_t(128,128,128));
    FILE *fp = fopen(file, "r");
    if (!fp) return false;
    do {
	char line[1024];
	const char *delim = " \t,;()\r\n";
	fgets(line, 1024, fp);
	if (feof(fp)) break;
	char *token = strtok(line, delim);
	if (!token || '#' == *token) continue;
	int num = atoi(token);
	int r = atoi(strtok(NULL, delim));
	int g = atoi(strtok(NULL, delim));
	int b = atoi(strtok(NULL, delim));
	token = strtok(NULL, delim);
	int a = (token && '#' != *token) ? atoi(token) : 255;

	if (0 > num) {
	    fill(color_t(r, g, b, a));
	} else {
	    map[clamp(0, num, map_size-1)] = color_t(r, g, b, a);
	}
    } while (!ferror(fp));
    fclose(fp);
    return true;
}

/***********************************************************************
 *	Class polyhedron
 ***********************************************************************/
polyhedron::polyhedron(const char* filename)
  : numPoints(0), numPolygons(0), numEdges(0),
    points(NULL), texcoord(NULL), polygons(NULL),
    cindex(NULL), normals(NULL), planeNormal(NULL), edges(NULL),
    filename(strdup(filename))
{
    FILE *in = fopen(filename, "r");
    if (NULL == in) return;

    std::vector<vector3F> vertex(1000);
    std::vector<vector3F> st(1000);
    std::vector<plane_t> loop(1000);

    initColormap();

    char line[512];
    const char *delim = " \t(,);\r\n";

    bool textureexists = false;
    while (NULL != fgets(line, sizeof(line), in)) {
	char *token = strtok(line, delim);
	if (!token) continue;
	if (streq("point", token)) {
	    int num = atoi(strtok(NULL, delim));
	    if (vertex.capacity() <= numPoints) {
		printf(_("%s:%d vertex buffer overflow\n"), __FILE__, __LINE__);
		exit(5);
	    }
	    vector3F v;
	    v[0] = strtod(strtok(NULL, delim), NULL);
	    v[1] = strtod(strtok(NULL, delim), NULL);
	    v[2] = strtod(strtok(NULL, delim), NULL);
	    vertex[numPoints] = v;
	    vector3F t(0);
	    token = strtok(NULL, delim);
	    if (token) {
		textureexists = true;
		t[0] = strtod(token, NULL);
		t[1] = 1-strtod(strtok(NULL, delim), NULL);
	    }
	    st[numPoints] = t;

	    numPoints++;
	}
	elif (streq("plane", token)) {
	    int i = 0;
	    if (loop.capacity() <= numPolygons) {
		printf(_("%s:%d loop buffer overflow\n"), __FILE__, __LINE__);
		exit(5);
	    }
	    plane_t &f = loop[numPolygons];
	    f[3] = -1;
	    f.cindex = 0;
	    while (NULL != (token = strtok(NULL, delim))) {
		if ('#' == *token) break;
		elif (isalpha(*token)) {
                    if ('C' == *token) f.cindex = atoi(token+1);
                }
		elif (isdigit(*token) && 4 > i) f[i++] = atoi(token);
	    }
	    numPolygons++;
	}
    }
    fclose(in);

    points = new vector3F[numPoints];
    polygons = new short[numPolygons][4];
    normals = new vector3F[numPolygons][4];
    planeNormal = new vector3F[numPolygons];
    cindex = new unsigned char[numPolygons];
    if (textureexists) {
	texcoord = new vector3F[numPoints];
        std::copy(&st[0], &st[numPoints], texcoord);
    }

    int i;
    std::copy(&vertex[0], &vertex[numPoints], points);

    for (i = 0; numPolygons > i; i++) {
	cindex[i] = loop[i].cindex;
	for (int j = 0; 4 > j; j++) {
	    polygons[i][j] = loop[i][j];
	}
    }
//    getNormal();
}

polyhedron::~polyhedron()
{
    delete[] points;
    delete[] polygons;
    delete[] cindex;
    delete[] normals;
    delete[] planeNormal;
    delete[] edges;
    delete[] texcoord;
    if (filename) free((void*)filename);
}

void polyhedron::initColormap()
{
    typedef colormap::color_t color_t;
    cmap.fill(color_t(128));	// default to be gray

    cmap[0] = color_t(250, 188, 137);	// C0 skin
    cmap[1] = color_t(1);		// C1 eye
    cmap[2] = color_t(42, 19, 5);	// C2 hair
    cmap[3] = color_t(250, 188, 137);	// C3 skin
//    cmap[4] = color_t(225, 7, 47);	// C4 shirts (red)
    cmap[4] = color_t(3, 87, 125);	// C4 shirts (blue)
//    cmap[5] = color_t(2, 13, 24);	// C5 pants  (green)
    cmap[5] = color_t(0);		// C5 pants  (black)
    cmap[6] = color_t(102, 7, 3);	// C6 skin/shadow
    cmap[7] = color_t(255, 0, 0);	// C7 racket/front
    cmap[8] = color_t(0);		// C8 racket/back
}

// normal vectors are destroyed
polyhedron& polyhedron::operator *=(const affine4F& m)
{
    for (int i = 0; numPoints > i; ++i) points[i] = points[i] * m;
    return *this;
}

/***********************************************************************
 *	CreateNormal
 *	Creates normal vector for each points of polygons.
 *	You should note that "each points" and "each points of polygons"
 *	are not same.
 *
 *	Normal vectors for polygon_ are returned to corresponding normal_,
 *	i.e. normal vector for polygon_[1][2] is returned to normal_[1][2].
 *
 *	This function will work on polyhedron that is NOT a 2-manifold.
 ***********************************************************************/
// MERGELIMIT = cos(angle).
#define MERGELIMIT Float(0.5)

BEGIN_ANONYMOUS
inline short round(short v, int len) { return (v + len) % len; }
inline int length(const short poly[4]) { return (0 < poly[3]) ? 4 : 3; }

int find(const short poly[4], short p) {
    int l = length(poly);
    for (int i = 0; l > i; i++) {
        if (poly[i] == p) return i;
    }
    return -1;
}

inline unsigned long key(short v0, short v1) {
    unsigned long k = v0;
    return (k << 16) | (v1);
}

struct halfedge {
    short v0, v1;
    short poly;
    
    inline halfedge() {}
    inline halfedge(short v0, short v1, short poly)
      : v0(v0), v1(v1), poly(poly) {}
    inline halfedge(const halfedge &e)
      : v0(e.v0), v1(e.v1), poly(e.poly) {}
};
END_ANONYMOUS    

void polyhedron::getNormal()
{
    int i, j;
    for (i = 0, j = 0; numPolygons > j; j++) {
	i += length(polygons[j]);
    }
    const int nhalfedges = i;
    std::vector<halfedge> hearray(nhalfedges);

    typedef std::map<unsigned long, const halfedge*> hemap_t;
    hemap_t hemap;

    for (i = 0, j = 0; numPolygons > j; j++) {
	int l = length(polygons[j]);
	for (int k = 0; l > k; k++) {
	    short v0 = polygons[j][k];
	    short v1 = polygons[j][round(k+1, l)];
	    hearray[i] = halfedge(v0, v1, j);
	    hemap[key(v0, v1)] = &hearray[i];
	    i++;
	}
    }

    for (i = 0; numPolygons > i; i++) {
	int l = length(polygons[i]);
	for (int k = 0; k < l; k++) {
	    short p0 = polygons[i][round(k-1, l)];
	    short p1 = polygons[i][k];
	    short p2 = polygons[i][round(k+1, l)];

	    const vector3F &v0 = points[p0];
	    const vector3F &v1 = points[p1];
	    const vector3F &v2 = points[p2];

	    vector3F mn, sn;
	    mn = sn = ((v2-v1)^(v0-v1)).norm();

	    unsigned long mk = key(p1, p2);	// The key of origin halfedge.

	    int count = 8;
	    hemap_t::iterator x;

	    // forward traversal
	    do {
		x = hemap.find(key(p2, p1));
		if (hemap.end() == x) break;
		int poly = x->second->poly;
		int i;
		if (0 > (i = find(polygons[poly], p1))) break;
		int l = length(polygons[poly]);
		p0 = polygons[poly][round(i-1, l)];
		p2 = polygons[poly][round(i+1, l)];
		if (mk == key(p1, p2))
		    break;	// came back to origin halfedge

		const vector3F &v0 = points[p0];
		const vector3F &v1 = points[p1];
		const vector3F &v2 = points[p2];
		
		vector3F n = ((v2-v1)^(v0-v1)).norm();
		if (mn * n > MERGELIMIT) {
		    sn += n;
		}
	    } while (0 < count--); // avoid infinite loop

	    if (hemap.end() == x) {
		// backward traversal
		// (required if there is hole(s) near the halfedge)

		p0 = polygons[i][round(k-1,l)];
		p1 = polygons[i][k];
		p2 = polygons[i][round(k+1,l)];
		mk = key(p0, p1);
		do {
		    x = hemap.find(key(p1, p0));
		    if (hemap.end() == x) break;
		    int poly = x->second->poly;
		    int i;
		    if (0 > (i = find(polygons[poly], p1))) break;
		    int l = length(polygons[poly]);
		    p0 = polygons[poly][round(i-1, l)];
		    p2 = polygons[poly][round(i+1, l)];
		    if (mk == key(p1, p0))
			break; // came back to origin

		    const vector3F &v0 = points[p0];
		    const vector3F &v1 = points[p1];
		    const vector3F &v2 = points[p2];
		    
		    vector3F n = ((v2-v1)^(v0-v1)).norm();
		    if (mn * n > MERGELIMIT) {
			sn += n;
		    }
		} while (0 < count--);
	    }
	    normals[i][k] = sn.norm();	
	    planeNormal[i] = mn.norm();
	}
    }

    typedef std::list<edge> edgelist_t;

    edgelist_t el;
    while (hemap.size() > 0) {
	const halfedge &he = *hemap.begin()->second;
	hemap_t::iterator i = hemap.find(key(he.v1, he.v0));
	edge e;
	e.v0 = he.v0;
	e.v1 = he.v1;
	e.p0 = he.poly;
	if (hemap.end() != i) {
	    e.p1 = i->second->poly;
	    hemap.erase(i);
	} else {
	    e.p1 = -1;
	}
	hemap.erase(hemap.begin());
	el.push_back(e);
    }
    numEdges = el.size();
    if (!edges) edges = new edge[numEdges];
    std::copy(el.begin(), el.end(), &edges[0]);
}

/***********************************************************************
 *	Class 
 ***********************************************************************/
affineanim::affineanim(int num)
  : numFrames(num)
{
    matrices = new affine4F[numFrames];
}

affineanim::affineanim(const char *filename)
  : numFrames(0), matrices(NULL)
{
    FILE *in = fopen(filename, "r");
    if (NULL == in) return;

    std::vector<affine4F> mat(100);

    char line[512];
    const char *delim = " \t(,);\r\n";
    while (NULL != fgets(line, sizeof(line), in)) {
	char *token = strtok(line, delim);
	if (!token) continue;
	if (streq("Affine3", token)) {
	    affine4F t(1);
	    for (int i = 0; 4 > i; i++) {
		for (int j = 0; 3 > j; j++) {
		    token = strtok(NULL, delim);
		    if (token) t[i][j] = strtod(token, NULL);
		}
	    }
	    if (mat.capacity() <= numFrames) {
		printf(_("%s:%d matrix buffer overflow\n"), __FILE__, __LINE__);
		exit(5);
	    }
	    mat[numFrames++] = t;
	}
    }
    fclose(in);
    matrices = new affine4F[numFrames];
    for (int i= 0; numFrames > i; i++) {
	matrices[i] = mat[i];
    }
}

affineanim::~affineanim()
{
    delete[] matrices;
}

/***********************************************************************
 *	Class affinemotion
 ***********************************************************************/
affinemotion::affinemotion(const char *refname, const char *animname)
  : ref(refname), anim(animname)
{
    ref.getNormal();    // create normal vectors of polyhedron
}

void
affinemotion::write(const char *basename)
{
    for (int i = 0; anim.numFrames > i; i++) {
	char buf[128];
	snprintf(buf, sizeof(buf), "%s%02d.dat", basename,i);
	FILE *fp = fopen(buf, "w");
	if (NULL == fp) exit(1);
	int j;
	for (j = 0; ref.numPoints > j; j++) {
	    vector3F v = ref.points[j] * anim.matrices[i];
	    fprintf(fp, "point %d,(%g,%g,%g);\n", j, v[0], v[1], v[2]);
	}
	for (j = 0; ref.numPolygons > j; j++) {
	    fprintf(fp, "plane ");
	    int k = 0;
	    while (k < 3) {
		fprintf(fp, "%d", ref.polygons[j][k]);
		if (2 != k) fprintf(fp, ",");
		k++;
	    }
	    if (ref.polygons[j][k] >= 0) {
		fprintf(fp, ",%d", ref.polygons[j][k]);
	    }
	    fprintf(fp, ";\n");
	}
	fclose(fp);
    }
}

/***********************************************************************
 *	Class partsmotion
 ***********************************************************************/
partsmotion::partsmotion(const char *basename)
  : numParts(0), parts(NULL)
{
    const char *partnames[] = {
	"head", "chest", "hip", "racket",
	"Rshoulder", "Rarm", "Relbow", "Rforearm", "Rhand",
	"Rthigh"/*, "Rknee"*/, "Rshin"/*, "Rankle"*/, "Rfoot",
	"Lshoulder", "Larm", "Lelbow", "Lforearm", "Lhand",
	"Lthigh"/*, "Lknee"*/, "Lshin"/*, "Lankle"*/, "Lfoot",
    };
    numParts = sizeof(partnames) / sizeof(const char *);
    parts = new affinemotion*[numParts];

    for (int i = 0; numParts > i; i++) {
	char ref[128], anim[128];
	snprintf(ref, sizeof(ref), "%s-%s01.dat", basename, partnames[i]);
	snprintf(anim, sizeof(anim), "%s-%s.affine", basename, partnames[i]);
//	printf("loading %s...", ref);
	parts[i] = new affinemotion(ref, anim);
    }
};

partsmotion::~partsmotion()
{
    if (parts) {
	for (int i = 0; numParts > i; i++) {
	    delete parts[i];
	}
	delete[] parts;
    }
}
    
bool partsmotion::render(int frame)
{
#define c(R,G,B,A) { R/255.0F, G/255.0F, B/255.0F, A/255.0F}
    static GLfloat colors[][4] = {
//	{ 0.4F, 0.4F, 0.4F, 1.0F},	// C0 default
	c(250, 188, 137, 127),		// C0 default(skin)
	c(1, 1, 1, 127),		// C1 eye
	c(42, 19, 5, 127), 		// C2 hair
	c(250, 188, 137, 127),		// C3 skin
//	c(225, 7, 47, 127),		// C4 shirts (red)
	c(3, 87, 125, 127),		// C4 shirts (blue)
//	c(2, 13, 24, 127),		// C5 pants  (green)
	c(0, 0, 0, 127),		// C5 pants  (black)
	c(102, 7, 3, 127),		// C6 skin/shadow
	c(255, 0, 0, 127),		// C7 racket/front
	c(0, 0, 0, 127),		// C8 racket/back

	{-1, -1, -1, -1}		// stop
    };
#undef c
    GLfloat BLACK[4] = { 0,0,0,1 };

    for (int i = 0; numParts > i; i++) {
	const polyhedron &ref = parts[i]->ref;
	if (0 == ref.numPolygons) continue;
	const affine4F &aff = parts[i]->anim[frame];
	glPushMatrix();
	glMultMatrixf((float*)&aff);

	for (int j = 0; ref.numPolygons > j; j++) {
	    polygon poly = ref.getPolygon(j);
#if RENDEREDGE
	    vector3F center(0);
#endif
	    glBegin(poly.glBeginSize());
	    glColor4fv(colors[poly.c()]);
	    for (int k = 0; poly.size > k; k++) {
		glNormal3fv((float*)&poly.rn(k));
#if RENDEREDGE
		center += poly.rv(k);
#endif
		glVertex3fv((float*)&poly.rv(k));
	    }
	    glEnd();

#if RENDEREDGE
	    // stop here if alpha blending is active
	    if (glIsEnabled(GL_BLEND)) continue;

	    center /= poly.size;
	    bool culling = glIsEnabled(GL_CULL_FACE)>0;
	    if (!culling) glEnable(GL_CULL_FACE);

	    glBegin(poly.glBeginSize());
	    glColor4fv(BLACK);

	    for (int k = poly.size-1; 0 <= k; --k) {
		vector3F p = poly.rv(k);
		p -= center;
		Float l = p.len();
		p *= 1 + (0.01F/l);
		p += center;
		p -= poly.n() * 1e-8F;
		glVertex3fv((float*)&p);
	    }
	    glEnd();
	    if (!culling) glDisable(GL_CULL_FACE);
#endif
	}
	glPopMatrix();
    }
    return true;
}

bool partsmotion::renderWire(int frame)
{
    for (int i = 0; numParts > i; i++) {
	const polyhedron &ref = parts[i]->ref;
	if (0 == ref.numPolygons) continue;
	const affine4F &aff = parts[i]->anim[frame];
	glPushMatrix();
	glMultMatrixf((float*)&aff);
	
	affine4F t;
	glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&t);
	vector3F origin = vector3F(0) * ~t;
	
	glBegin(GL_LINES);
	for (int j = 0; ref.numEdges > j; j++) {
	    int p0 = ref.edges[j].p0;
	    int p1 = ref.edges[j].p1;
	    bool draw = false;
	    vector3F v = ref.points[ref.edges[j].v0] - origin;
	    if (p1 >= 0) {
		Float i0 = v * ref.planeNormal[p0];
		Float i1 = v * ref.planeNormal[p1];
		if (i0 * i1 <= 0) draw = true;
	    } else {
#if 0
                // Draw if the plane is facing toward eye.
		Float i0 = v * ref.planeNormal[p0];
		if (i0 <= 0) draw = true;
#else	
                // always draw *edge* edges.
  		draw = true;
#endif
	    }

	    if (draw) {
		vector3F v0 = ref.points[ref.edges[j].v0];// - v * 0.01F;
		vector3F v1 = ref.points[ref.edges[j].v1];// - v * 0.01F;
		glVertex3fv((float*)&v0);
		glVertex3fv((float*)&v1);
	    }
	}
	glEnd();
	glPopMatrix();
    }
    return true;
}

bool partsmotion::render(double frame)
{
#define c(R,G,B,A) { R/255.0F, G/255.0F, B/255.0F, A/255.0F}
    static GLfloat colors[][4] = {
//	{ 0.4F, 0.4F, 0.4F, 1.0F},	// C0 default
	c(250, 188, 137, 127),		// C0 default(skin)
	c(1, 1, 1, 127),		// C1 eye
	c(42, 19, 5, 127), 		// C2 hair
	c(250, 188, 137, 127),		// C3 skin
//	c(225, 7, 47, 127),		// C4 shirts (red)
	c(3, 87, 125, 127),		// C4 shirts (blue)
//	c(2, 13, 24, 127),		// C5 pants  (green)
	c(0, 0, 0, 127),		// C5 pants  (black)
	c(102, 7, 3, 127),		// C6 skin/shadow
	c(255, 0, 0, 127),		// C7 racket/front
	c(0, 0, 0, 127),		// C8 racket/back

	{-1, -1, -1, -1}		// stop
    };
#undef c
    GLfloat BLACK[4] = { 0,0,0,1 };

    for (int i = 0; numParts > i; i++) {
	const polyhedron &ref = parts[i]->ref;
	if (0 == ref.numPolygons) continue;
	const affine4F &aff1 = parts[i]->anim[frame];
	affine4F aff2;

	if ( frame+1 >= 50.0 )
	  aff2 = parts[i]->anim[frame];
	else
	  aff2 = parts[i]->anim[frame+1];

	vector4F q1 = Affine2Quaternion(aff1);
	vector4F q2 = Affine2Quaternion(aff2);

	if ( q1[1]*q2[1]+q1[2]*q2[2]+q1[3]*q2[3] < 0 )
	  q2 = -q2;

	vector4F q = q1*(1-(frame-(int)frame))+q2*(frame-(int)frame);

	vector4F p;
	p[0] = aff1[3][0]*(1-(frame-(int)frame))+aff2[3][0]*(frame-(int)frame);
	p[1] = aff1[3][1]*(1-(frame-(int)frame))+aff2[3][1]*(frame-(int)frame);
	p[2] = aff1[3][2]*(1-(frame-(int)frame))+aff2[3][2]*(frame-(int)frame);
	p[3] = 1.0;

	const affine4F &aff = Quaternion2Affine(q, p);

	glPushMatrix();
	glMultMatrixf((float*)&aff);

	for (int j = 0; ref.numPolygons > j; j++) {
	    polygon poly = ref.getPolygon(j);
#if RENDEREDGE
	    vector3F center(0);
#endif
	    glBegin(poly.glBeginSize());
	    glColor4fv(colors[poly.c()]);
	    for (int k = 0; poly.size > k; k++) {
		glNormal3fv((float*)&poly.rn(k));
#if RENDEREDGE
		center += poly.rv(k);
#endif
		glVertex3fv((float*)&poly.rv(k));
	    }
	    glEnd();

#if RENDEREDGE
	    // stop here if alpha blending is active
	    if (glIsEnabled(GL_BLEND)) continue;

	    center /= poly.size;
	    bool culling = glIsEnabled(GL_CULL_FACE)>0;
	    if (!culling) glEnable(GL_CULL_FACE);

	    glBegin(poly.glBeginSize());
	    glColor4fv(BLACK);

	    for (int k = poly.size-1; 0 <= k; --k) {
		vector3F p = poly.rv(k);
		p -= center;
		Float l = p.len();
		p *= 1 + (0.01F/l);
		p += center;
		p -= poly.n() * 1e-8F;
		glVertex3fv((float*)&p);
	    }
	    glEnd();
	    if (!culling) glDisable(GL_CULL_FACE);
#endif
	}
	glPopMatrix();
    }
    return true;
}

bool partsmotion::renderWire(double frame)
{
    for (int i = 0; numParts > i; i++) {
	const polyhedron &ref = parts[i]->ref;
	if (0 == ref.numPolygons) continue;
	const affine4F &aff1 = parts[i]->anim[frame];
	affine4F aff2;

	if ( frame+1 >= 50.0 )
	  aff2 = parts[i]->anim[frame];
	else
	  aff2 = parts[i]->anim[frame+1];

	vector4F q1 = Affine2Quaternion(aff1);
	vector4F q2 = Affine2Quaternion(aff2);

	if ( q1[1]*q2[1]+q1[2]*q2[2]+q1[3]*q2[3] < 0 )
	  q2 = -q2;

	vector4F q = q1*(1-(frame-(int)frame))+q2*(frame-(int)frame);

	vector4F p;
	p[0] = aff1[3][0]*(1-(frame-(int)frame))+aff2[3][0]*(frame-(int)frame);
	p[1] = aff1[3][1]*(1-(frame-(int)frame))+aff2[3][1]*(frame-(int)frame);
	p[2] = aff1[3][2]*(1-(frame-(int)frame))+aff2[3][2]*(frame-(int)frame);
	p[3] = 1.0;

	const affine4F &aff = Quaternion2Affine(q, p);

	glPushMatrix();
	glMultMatrixf((float*)&aff);

	affine4F t;
	glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&t);
	vector3F origin = vector3F(0) * ~t;
	
	glBegin(GL_LINES);
	for (int j = 0; ref.numEdges > j; j++) {
	    int p0 = ref.edges[j].p0;
	    int p1 = ref.edges[j].p1;
	    bool draw = false;
	    vector3F v = ref.points[ref.edges[j].v0] - origin;
	    if (p1 >= 0) {
		Float i0 = v * ref.planeNormal[p0];
		Float i1 = v * ref.planeNormal[p1];
		if (i0 * i1 <= 0) draw = true;
	    } else {
#if 0
                // Draw if the plane is facing toward eye.
		Float i0 = v * ref.planeNormal[p0];
		if (i0 <= 0) draw = true;
#else	
                // always draw *edge* edges.
  		draw = true;
#endif
	    }

	    if (draw) {
		vector3F v0 = ref.points[ref.edges[j].v0];// - v * 0.01F;
		vector3F v1 = ref.points[ref.edges[j].v1];// - v * 0.01F;
		glVertex3fv((float*)&v0);
		glVertex3fv((float*)&v1);
	    }
	}
	glEnd();
	glPopMatrix();
    }
    return true;
}

// http://skal.planet-d.net/demo/matrixfaq.htm#Q46
vector4F
Affine2Quaternion(affine4F aff) {
  float t,s;
  vector4F v;

  t= aff[0][0] + aff[1][1] + aff[2][2] + 1.0f;

  if ( t > 0.0 ) {
    s = 0.5/sqrt(t);
    v[0] = 0.25/s;
    v[1] = (aff[2][1] - aff[1][2])*s;
    v[2] = (aff[0][2] - aff[2][0])*s;
    v[3] = (aff[1][0] - aff[0][1])*s;
  } else if ( aff[0][0] >= aff[1][1] && aff[0][0] >= aff[2][2] ) {
    s = sqrt(1.0 + aff[0][0] - aff[1][1] - aff[2][2])*2;
    v[0] = (aff[1][2] + aff[2][1])/s;
    v[1] = 0.5/s;
    v[2] = (aff[0][1] + aff[1][0])/s;
    v[3] = (aff[0][2] + aff[2][0])/s;
  } else if ( aff[1][1] >= aff[0][0] && aff[1][1] >= aff[2][2] ) {
    s = sqrt(1.0 + aff[1][1] - aff[0][0] - aff[2][2])*2;
    v[0] = (aff[0][2] + aff[2][0])/s;
    v[1] = (aff[0][1] + aff[1][0])/s;
    v[2] = 0.5/s;
    v[3] = (aff[1][2] + aff[2][1])/s;
  } else {
    s = sqrt(1.0 + aff[2][2] - aff[0][0] - aff[1][1])*2;
    v[0] = (aff[0][1] + aff[1][0])/s;
    v[1] = (aff[0][2] + aff[2][0])/s;
    v[2] = (aff[1][2] + aff[2][1])/s;
    v[3] = 0.5/s;
  }

  return v;
}

// http://skal.planet-d.net/demo/matrixfaq.htm#Q47
affine4F
Quaternion2Affine(vector4F v, vector4F p) {
  affine4F ret;

  ret[0][0] = 1.0 - 2*(v[2]*v[2] + v[3]*v[3]);
  ret[0][1] =       2*(v[1]*v[2] - v[0]*v[3]);
  ret[0][2] =       2*(v[1]*v[3] + v[0]*v[2]);
  ret[1][0] =       2*(v[1]*v[2] + v[0]*v[3]);
  ret[1][1] = 1.0 - 2*(v[1]*v[1] + v[3]*v[3]);
  ret[1][2] =       2*(v[2]*v[3] - v[0]*v[1]);
  ret[2][0] =       2*(v[1]*v[3] - v[0]*v[2]);
  ret[2][1] =       2*(v[2]*v[3] + v[0]*v[1]);
  ret[2][2] = 1.0 - 2*(v[1]*v[1] + v[2]*v[2]);

  ret[3][0] = p[0];
  ret[3][1] = p[1];
  ret[3][2] = p[2];
  ret[3][3] = p[3];

  ret[0][3] = 0.0;
  ret[1][3] = 0.0;
  ret[2][3] = 0.0;

  return ret;
}

#if 0
/***********************************************************************
 *	test
 ***********************************************************************/

int main(int argc, char *argv[])
{
    char ref[128], aff[128];
    snprintf(ref, sizeof(ref), "%s01.dat", argv[1]);
    snprintf(aff, sizeof(aff), "%s.affine", argv[1]);

    affinemotion pa(ref, aff);
    printf("load\n");
    pa.write(argv[2]);
    printf("write\n");
    return 0;
}
#endif

/***********************************************************************
 *	END OF parts.cpp
 ***********************************************************************/
