/***********************************************************************
 *	d:/Werk/src/csmash-0.3.8.new/conv/parts.h
 *	$Id$
 *
 *	Copyright by ESESoft.
 *	You are granted the right to redistribute this file under
 *	the "Artistic license". See "ARTISTIC" for detail.
 *
 ***********************************************************************/
#ifndef __wata_ESESoft_9465__parts_h__INCLUDED__
#define __wata_ESESoft_9465__parts_h__INCLUDED__
/***********************************************************************/
#include "float"
#include "matrix"
#include "affine"
/* __BEGIN__BEGIN__ */

class edge {
public:
    short v0, v1;
    short p0, p1;
};

class polygon;

class polyhedron
{
public:
    int numPoints, numPolygons, numEdges;
    vector3F *points;
    short (*polygons)[4];
    unsigned char *cindex;
    vector3F (*normals)[4];
    vector3F *planeNormal;
    edge *edges;
    char *filename;

    polyhedron(const char *filename);
    ~polyhedron();

    inline int polsize(int i) const { return (0 > polygons[i][3]) ? 3 : 4; }
    polygon getPolygon(int i) const;

private:
    void getNormal();
    void getEdges();
};

/// polygon access object
class polygon
{
    friend class polyhedron;
protected:
    inline polygon(const polyhedron& parent, int polynum)
      : num(polynum), p(parent) {
	size = (p.polygons[num][3] < 0) ? 3 : 4;
    }

public:
    inline int round(int idx) const { return (idx+size)%size; }
    inline int pround(int idx) const { return idx%size; }

    inline short ri(int idx) const { return p.polygons[num][idx]; }
    inline short i(int idx) const { return p.polygons[num][round(idx)]; }

    inline const vector3F& rv(int idx) const { return p.points[ri(idx)]; }
    inline const vector3F& v(int idx) const { return p.points[i(idx)]; }

    inline const vector3F& rn(int idx) const { return p.normals[num][idx]; }
    inline const vector3F& n(int idx) const { return p.normals[num][round(idx)]; }
    inline const vector3F& n(void) const { return p.planeNormal[num]; }

    inline const unsigned char c() const { return p.cindex[num]; }

    inline short getv(short vidx) const {
	for (int k = 0; size > k; ++k) if (vidx == ri(k)) return k;
	return -1;
    }
    inline short gete(const edge&e, int* way) const {
	return gete(e.v0, e.v1, way);
    }
    inline short gete(short v0, short v1, int* way) const {
	for (int k = 0; size > k; ++k) {
	    if (v0 == ri(k)) {
		if (v1 == ri(pround(k+1))) {
		    *way = +1;
		    return k;
		}
		else if (v1 == i(k-1)) {
		    *way = -1;
		    return k;
		}
	    }
	}
	return -1;
    }

    inline GLenum glBeginSize() const {
	return (3 == size) ? GL_TRIANGLES : GL_QUADS;
    }

public:
    const polyhedron& p;
    int num;
    int size;
};

inline polygon polyhedron::getPolygon(int i) const {
    return polygon(*this, i);
}

class affineanim {
public:
    int numFrames;
    affine4F *matrices;

    affineanim(int num);
    affineanim(const char *filename);
    ~affineanim();

    inline const affine4F& operator[](int i) const {
	return matrices[i];
    }
    inline affine4F& operator [](int i) {
	return matrices[i];
    }
};

class affinemotion {
public:
    polyhedron ref;
    affineanim anim;

    affinemotion(const char *ref, const char *anim);
    void write(const char *basename);
    inline bool valid() const {
	return  ref.numPoints > 0 && anim.numFrames > 0;
    }
};

class partsmotion
{
public:
    int numParts;
    affinemotion **parts;

    partsmotion(const char *basename);
    virtual ~partsmotion();

    virtual bool render(int frame);
    virtual bool renderWire(int frame);
};

/* __END__END__ */

/***********************************************************************/
#endif
/***********************************************************************
 *	END OF parts.h
 ***********************************************************************/
