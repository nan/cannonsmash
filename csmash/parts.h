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

    polyhedron(const char *filename);
    ~polyhedron();

    inline int polsize(int i) const { return (0 > polygons[i][3]) ? 3 : 4; }
private:
    void getNormal();
    void getEdges();
};

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
