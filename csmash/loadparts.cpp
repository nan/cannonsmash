/***********************************************************************
 *	z:/wata/src/a/csmash/loadparts.cpp
 *	$Id$
 *
 *	Copyright by ESESoft.
 *	You are granted the right to redistribute this file under
 *	the "Artistic license". See "ARTISTIC" for detail.
 *
 ***********************************************************************/
#include "ttinc.h"

#include <iostream>
#include <string>
#include <map>
#include <list>
#include <algorithm>

#include "float"
#include "matrix"
#include "affine"


#include "LoadImage.h"
#include "parts.h"
#include "loadparts.h"

#define elif else if

// ANSI C++ for () namescoping hack :-)
#if defined(_MSC_VER) && _MSC_VER <= 1200
# define for if(false);else for
#endif

#define BEGIN_ANONYMOUS namespace {
#define END_ANONYMOUS }

/***********************************************************************
 *	Local data and functions
 ***********************************************************************/
BEGIN_ANONYMOUS
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

inline bool streq(const char *a, const char *b) {
    return 0 == strcmp(a, b);
}
END_ANONYMOUS

bool loadAffine4F(const char *str, affine4F *pm)
{
    FILE *fp = fopen(str, "r");
    if (!fp) return false;

    affine4F &m = *pm;
    m = affine4F(1);
    const char *delim = " \t(,);\r\n";
    char line[256];
    int i = 0;
    while (NULL != fgets(line, sizeof(line), fp)) {
	const char *token = strtok(line, delim);
	if (!token) continue;
	do {
	    if (streq("Affine3", token)) continue;
	    Float f = (Float)strtod(token, NULL);
	    int x = i / 3;
	    int y = i % 3;
	    m[x][y] = f;
	    if (12 == ++i) break;
	}
	while (token = strtok(NULL, delim));
    }
    fclose(fp);
    if (12 != i) return false;

    return true;
}


/***********************************************************************
 *	Class parts
 ***********************************************************************/

static struct symtab_t {
    parts::symbol_t sym;
    const char *str;
} symtab[] = {
#define SYM(A) { parts::sym_##A, #A }
    SYM(null),
    SYM(load),
    SYM(create),
    SYM(polyhedron),
    SYM(anim),
    SYM(texture),
    SYM(body),
#undef SYM
    { parts::sym_unknown, NULL },
    { parts::sym_unknown, "NanTheBLACK, our guru." }
};

parts::symbol_t parts::getsym(const char *str)
{
    if (!str) return sym_unknown;
    for (symtab_t *p = symtab; p->str; ++p) {
	if (streq(p->str, str)) return p->sym;
    }
    return sym_unknown;
}

const char* parts::sym2str(parts::symbol_t sym)
{
    for (symtab_t *p = symtab; p->str; ++p) {
	if (p->sym == sym) return p->str;
    }
    return "sym_unknown";
}

static parts_map partsmap;

parts* parts::getobject(const char* name)
{
    const parts_map::iterator i = partsmap.find(name);
    if (partsmap.end() == i) return NULL;
    else return i->second;
}

bool parts::addobject(const char *name, parts* p)
{
    const parts_map::iterator i = partsmap.find(name);
    if (partsmap.end() != i) return false;

    p->name = name;
    partsmap[name] = p;
    return true;
}

bool parts::delobject(const char *name)
{
    parts_map::iterator i = partsmap.find(name);
    if (partsmap.end() == i) return false;
    delete i->second;
    partsmap.erase(i);
    return true;
}

void parts::clearobjects()
{
    partsmap.clear();
}

struct auto_fp
{
    FILE *fp;
    bool needclose;
    inline auto_fp(FILE *fp, bool b = true) : fp(fp), needclose(b) {}
    inline ~auto_fp() { if (needclose && fp) fclose(fp); }
    inline operator FILE*() { return fp; }
    inline bool operator!() { return !fp; }
};

bool parts::loadobjects(const char *str)
{
    try {
	loadfile(str);
    }
    catch (const error &e) {
	printf("loadfile failed\n");
	printf(e.what());
	return false;
    }
    return true;
}
    
bool parts::loadfile(const char *str)
{
    auto_fp fp(fopen(str, "r"));
    if (!fp) return false;

    int lineno = 0;
    do {
	const char *delim = " \t\r\n;";
	char line[4096];
	fgets(line, sizeof(line), fp);
	if (feof((FILE*)fp)) break;
	++lineno;
	int l = strlen(line);
	int addline = 0;
	while ('\\' == line[l-2]) {	// "\\\n"
	    int bufsize = clamp(0U, sizeof(line)-l, sizeof(line)-1);
	    fgets(&line[l-2], bufsize, fp);
	    if (feof((FILE*)fp)) break;
	    l = strlen(line);
	    ++addline;
	}


	int argc = 0;
	const char *argv[128];
	const char *token = strtok(line, delim);
	if (!token || '#' == *token) continue;
	do {
	    argv[argc++] = token;
	    if (64 == argc) {
		snprintf(line, sizeof(line),
			 "%d: has 64 or more argv\n", lineno);
		throw error(line);
	    }	
	} while (token = strtok(NULL, delim));
	argv[argc] = NULL;
	int optind = 0;

	token = argv[optind++];
	symbol_t sym = getsym(token);
	switch (sym) {
	case sym_load:
	    load_load(lineno, argc, argv, &optind); break;
	case sym_create:
	    load_create(lineno, argc, argv, &optind); break;
	default:
	    snprintf(line, sizeof(line), "%d: error unknown command %s\n",
		     lineno, token);
	    throw error(line);
	}
	lineno += addline;
    } while (!ferror((FILE*)fp));

    return true;
}
	
bool parts::load_load(int lineno, int argc, const char *argv[], int* poptind)
{
    int& optind = *poptind;

    char line[256];
    const char *token = argv[optind++];
    if (!token) {
	snprintf(line, sizeof(line), "%d: error type not specified\n",
		 lineno);
	throw error(line);
    }
    symbol_t sym = getsym(token);

    const char *objectname = argv[optind++];
    if (!objectname) {
	snprintf(line, sizeof(line),
		 "%d: object name is not specified\n", lineno);
	throw error(line);
    }
    
    const char *filename = argv[optind++];
    switch (sym) {
    case sym_texture: {
	texture_parts* object = new texture_parts(objectname);
	if (!addobject(objectname, object)) {
	    delete object;
	    snprintf(line, sizeof(line),
		     "%d: %s is already loaded\n", lineno, objectname);
	    throw error(line);
	}
	object->load(filename);
	break;
    } /* texture */
    
    case sym_polyhedron: {
	polyhedron_parts *object = new polyhedron_parts(objectname);
	if (!addobject(objectname, object)) {
	    delete object;
	    snprintf(line, sizeof(line),
		     "%d: %s is already loaded\n", lineno, objectname);
	    throw error(line);
	}
	object->load(filename);
	load_polyhedron(lineno, object, argc, argv, &optind);

	break;
    }	/* polyhedron */

    case sym_anim: {
	anim_parts *object = new anim_parts(objectname);
	if (!addobject(objectname, object)) {
	    delete object;
	    snprintf(line, sizeof(line),
		     "%d: %s is already loaded\n", lineno, objectname);
	    throw error(line);
	}
	object->load(filename);
	int i = 0;
	while (const char *name = argv[optind++]) {
	    parts *poly = getobject(name);
	    if (!poly) {
		snprintf(line, sizeof(line),
			 "%d: %s not loaded\n", lineno, name);
		throw error(line);
	    }
	    if (!object->assign(poly)) {
		snprintf(line, sizeof(line),
			 "%d: %s is not assignable\n", lineno, name);
		throw error(line);
	    }
	    ++i;
	}
	if (!i) {
	    snprintf(line, sizeof(line),
		     "%d: %s is empty object\n", lineno, objectname);
	    printf(line);
	}
	break;
    }	 /* anim */

    default:
	snprintf(line, sizeof(line),
		 "%d: error unknown type specified\n", lineno, token);
	throw error(line);
    }

    return true;
}

bool parts::load_polyhedron(int lineno, polyhedron_parts *object,
			    int argc, const char* argv[], int *poptind)
{
    int &optind = *poptind;
    char line[256];
    const char *option;
    while (option = argv[optind++]) {
	if ('-' != *option) {
	    snprintf(line, sizeof(line),
		     "%d: unknown option %s\n", lineno, option);
	    throw error(line);
	}
	const char *operand = argv[optind++];
	if (!operand) {
	    snprintf(line, sizeof(line),
		     "%d: no operadnd for %s\n", lineno, option);
	    throw error(line);
	}
	switch (option[1]) {
	case 'c': {	/* colormap */
	    colormap cmap;
	    if (!cmap.load(operand)) {
		snprintf(line, sizeof(line),
			 "%d: could not load colormap %s\n", lineno, operand);
		throw error(line);
	    }
	    object->object->cmap = cmap;	// copy all
	    break;
	}
	case 't': {	/* texture */
	    parts *tex = getobject(operand);
	    if (!tex) {
		snprintf(line, sizeof(line),
			 "%d: texture %s not loaded\n", lineno, operand);
		throw error(line);
	    }
	    if (!object->assign(tex)) {
		snprintf(line, sizeof(line),
			 "%d: %s is not assignable\n", lineno, operand);
		throw error(line);
	    }
	    break;
	}
	case 'm': {
	    affine4F m;
	    loadAffine4F(operand, &m);
	    *object->object *= m;
	    object->object->getNormal();

	    break;
	}
	default:
	    snprintf(line, sizeof(line),
		     "%d: unknown option %s\n", lineno, option);
	    throw error(line);
	}
    }
    return true;
}

bool parts::load_create(int lineno, int argc, const char *argv[], int *poptind)
{
    int &optind = *poptind;

    char buf[256];
    const char *token = argv[optind++];
    if (!token) {
	snprintf(buf, sizeof(buf),
		 "%d: type is not specified\n", lineno);
	throw error(buf);
    }
    switch (getsym(token)) {
    case sym_body: {
	const char *objectname = argv[optind++];
	if (!objectname) {
	    snprintf(buf, sizeof(buf),
		     "%d: object name is not specified\n", lineno);
	    throw error(buf);
	}
	body_parts *object = new body_parts(objectname);
	if (!addobject(objectname, object)) {
	    snprintf(buf, sizeof(buf),
		     "%d: %s is already loaded\n", lineno, objectname);
	    throw error(buf);
	}
	int i = 0;
	while (token = argv[optind++]) {
	    parts* p = getobject(token);
	    if (!p) {
		snprintf(buf, sizeof(buf),
			 "%d: %s is not loaded\n", lineno, token);
		throw error(buf);
	    }
	    object->assign(p);
	    ++i;
	}
	if (!i) {
	    snprintf(buf, sizeof(buf),
		     "%d: %s is empty\n", lineno, objectname);
	    printf(buf);
	}
	break;
    }
    default:
	snprintf(buf, sizeof(buf),
		 "%d: type %s cannot be created\n", lineno, token);
	throw error(buf);
    }
    return true;
}

/***********************************************************************
 *	Class texture_parts
 ***********************************************************************/
bool texture_parts::load(const char *str)
{
    filename = "Parts/";
    filename += str;
    return true;
}

void texture_parts::realize()
{
    if (object) return;

    static int allowedsize[] = {
	64, 128, 130, 256, 512, 0
    };

    ImageData img;
    if (!img.LoadFile(filename.c_str())) {
	char buf[256];
	snprintf(buf, sizeof(buf), "could not load texture %s\n", filename.c_str);
	throw error(buf);
    }
    int width = img.GetWidth();
    int height = img.GetHeight();
    int i, j;
    for (i = 0; 0 != allowedsize[i]; ++i) {
	if (width == allowedsize[i]) break;
    }
    for (j = 0; 0 != allowedsize[i]; ++j) {
	if (height == allowedsize[i]) break;
    }
    if (0 == allowedsize[i] || 0 == allowedsize[j]) {
	char buf[256];
	snprintf(buf, sizeof(buf), "texture %s has illegal size(%d,%d)\n",
		 filename.c_str(), width, height);
	throw error(buf);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &object);
    glBindTexture(GL_TEXTURE_2D, object);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0,
		 GL_RGBA, GL_UNSIGNED_BYTE, img.GetImage());

    printf("texture %s is %d\n", filename.c_str(), object);
    if (0 == object) {
	char buf[256];
	snprintf(buf, sizeof(buf), "texture %s cannot be realized\n", filename.c_str());
	printf(buf);
	throw error(buf);
    }
}

/***********************************************************************
 *	Class polyhedron_parts
 ***********************************************************************/
bool polyhedron_parts::load(const char *str)
{
    object = new polyhedron(str);
    if (!object->points) {
	delete object;
	object = NULL;
	char buf[256];
	snprintf(buf, sizeof(buf), "polyhedron %s cannot be loaded\n", str);
	throw error(buf);
    }
    return true;
}

bool polyhedron_parts::assign(parts* a)
{
    char buf[256];
    switch (a->type()) {
    case sym_texture:
//	object->texturename = reinterpret_cast<texture_parts*>(a)->object;
	tex = reinterpret_cast<texture_parts*>(a);
	break;
    default:
	snprintf(buf, sizeof(buf),
		 "%s(%s) cannot be assigned to polyhedron(%s)\n",
		 a->name.c_str(), sym2str(a->type()), name.c_str());
	throw error(buf);
    }
    return true;
}

void polyhedron_parts::render() const
{
    GLbyte NanTheBLACK[4] = { 0, 0, 0, 1 };
    GLbyte ManOfVirtue[4] = { (GLbyte)255, (GLbyte)255, (GLbyte)255, 1 };

    polyhedron &poly = *object;
    if (tex) tex->realize();
    if (tex && poly.texcoord && tex->object) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->object);
	glColor4bv(ManOfVirtue);
	for (int i = 0; poly.numPolygons > i; ++i) {
	    const polygon &face = poly.getPolygon(i);
	    glBegin(face.glBeginSize());
	    for (int j = 0; face.size > j; ++j) {
		glNormal3fv((float*)&face.rn(j));
		glTexCoord2fv((float*)&face.rst(j));
		glVertex3fv((float*)&face.rv(j));
	    }
	    glEnd();
	}
    } else {
	glDisable(GL_TEXTURE_2D);
	for (int i = 0; poly.numPolygons > i; ++i) {
	    const polygon &face = poly.getPolygon(i);
	    glBegin(face.glBeginSize());
	    for (int j = 0; face.size > j; ++j) {
		glColor4bv((const GLbyte*)&poly.cmap[face.c()]);
		glNormal3fv((float*)&face.rn(j));
		glVertex3fv((float*)&face.rv(j));
	    }
	    glEnd();
	}
    }
}

void polyhedron_parts::renderWire() const
{
    polyhedron &poly = *object;

    affine4F t;
    glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&t);
    vector3F origin = vector3F(0) * ~t;

    glBegin(GL_LINES);
    for (int i = 0; poly.numEdges > i; ++i) {
	int p0 = poly.edges[i].p0;
	int p1 = poly.edges[i].p1;
	bool draw = false;
	vector3F v = poly.points[poly.edges[i].v0] - origin;
	if (p1 >= 0) {
	    Float i0 = v * poly.planeNormal[p0];
	    Float i1 = v * poly.planeNormal[p1];
	    if (i0 * i1 <= 0) draw = true;
	} else {
	    draw = true;
	}
	if (draw) {
	    vector3F &v0 = poly.points[poly.edges[i].v0];
	    vector3F &v1 = poly.points[poly.edges[i].v1];
	    glVertex3fv((float*)&v0);
	    glVertex3fv((float*)&v1);
	}
    }
    glEnd();
}
/***********************************************************************
 *	Class anim_parts
 ***********************************************************************/
bool anim_parts::load(const char *str)
{
    object = new affineanim(str);
    if (!object->matrices) {
	char buf[256];
	snprintf(buf, sizeof(buf), "could not load anim %s\n", str);
	throw error(buf);
    }
    return true;
}

bool anim_parts::assign(parts* a)
{
    char buf[256];
    switch (a->type()) {
    case sym_polyhedron:
	poly.push_back(reinterpret_cast<polyhedron_parts*>(a));
	break;
    default:
	snprintf(buf, sizeof(buf),
		 "%s(%s) cannot be assigned to anim(%s)\n",
		 a->name.c_str(), sym2str(a->type()), name.c_str());
	throw error(buf);
    }
    return true;
}

void anim_parts::render(int frame) const
{
    affineanim &anim = *object;
    glPushMatrix();
    glMultMatrixf((float*)&anim[frame]);
    for (std::list<polyhedron_parts*>::const_iterator i = poly.begin();
	 poly.end() != i; ++i) {
	(*i)->render();
    }
    glPopMatrix();
}

void anim_parts::renderWire(int frame) const
{
    affineanim &anim = *object;
    glPushMatrix();
    glMultMatrixf((float*)&anim[frame]);
    for (std::list<polyhedron_parts*>::const_iterator i = poly.begin();
	 poly.end() != i; ++i) {
	(*i)->renderWire();
    }
    glPopMatrix();
}

/***********************************************************************
 *	Class body_parts
 ***********************************************************************/
bool body_parts::assign(parts* a)
{
    char buf[256];
    switch (a->type()) {
    case sym_anim:
	object.push_back(reinterpret_cast<anim_parts*>(a));
	break;
    default:
	snprintf(buf, sizeof(buf),
		 "%s(%s) cannot be assigned to body (%s)\n",
		 a->name.c_str(), sym2str(a->type()), name.c_str());
	throw error(buf);
    }
    return true;
}

void body_parts::render(int frame) const
{
    for (std::list<anim_parts*>::const_iterator i = object.begin();
	 object.end() != i; ++i) {
	(*i)->render(frame);
    }
}

void body_parts::renderWire(int frame) const
{
    for (std::list<anim_parts*>::const_iterator i = object.begin();
	 object.end() != i; ++i) {
	(*i)->renderWire(frame);
    }
}

/***********************************************************************
 *	END OF loadparts.cpp
 ***********************************************************************/
