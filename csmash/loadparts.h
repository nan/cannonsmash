/***********************************************************************
 *	z:/wata/src/a/csmash/loadparts.h
 *	$Id$
 *
 *	Copyright by ESESoft.
 *	You are granted the right to redistribute this file under
 *	the "Artistic license". See "ARTISTIC" for detail.
 *
 ***********************************************************************/
#ifndef __Yotsuya_ESESoft_07897981__loadparts_h__INCLUDED__
#define __Yotsuya_ESESoft_07897981__loadparts_h__INCLUDED__
/***********************************************************************/
#include <map>
#include <list>

/* __BEGIN__BEGIN__ */

class polyhedron_parts;
class anim_parts;
class body_parts;
class texture_parts;

class parts
{
public:
    typedef std::basic_string<char> string;
    class error {
    public:
	const char* str;
	
	inline error(const char *str) : str(str) {}
	const char* what() const { return str; }
    };
    
    enum symbol_t {
	sym_unknown = 0,
	sym_null,
	sym_load, sym_create,
	sym_polyhedron,	sym_anim, sym_texture,
	sym_body,
    };

    inline parts(const char *str) : name(str) {}
    virtual ~parts() {}

    virtual bool load(const char*) = 0;
    virtual symbol_t type() const = 0;
    virtual bool assign(parts* obejct) { return false; }

    static symbol_t getsym(const char *str);
    static const char* sym2str(symbol_t);

    static parts* getobject(const char* name);
    static void clearobjects();
    static bool loadobjects(const char *str);

protected:
    static bool addobject(const char* name, parts*);
    static bool delobject(const char *name);
    static bool loadfile(const char *str);

private:
    static bool load_create(int lineno, int argc, const char *argv[], int *optind);
    static bool load_load(int lineno, int argc, const char *argv[], int *optind);
    static bool load_polyhedron(int lineno, polyhedron_parts*, int ac, const char *av[], int *ind);

public:
    string name;
};

class parts_map : public std::map<parts::string, parts*>
{
public:
    typedef std::map<parts::string, parts*> super;

    ~parts_map() {
	clear();
    }	
    void clear() {
	for (iterator i = begin(); i != end(); ++i) delete i->second;
	super::clear();
    }
};

class texture_parts : public parts
{
public:
    typedef GLuint object_t;
    GLuint object;
    string filename;

    inline texture_parts(const char *name) : parts(name), object(0) {}
    virtual ~texture_parts() {
	if (object) {
	    glDeleteTextures(1, &object);
	}
    }
    virtual symbol_t type() const { return sym_texture; }
    virtual bool load(const char *str);

    void realize();
};

class polyhedron_parts : public parts
{
public:
    typedef polyhedron object_t;
    polyhedron *object;
    texture_parts *tex;

    inline polyhedron_parts(const char *name) : parts(name), object(NULL), tex(NULL) {}
    virtual ~polyhedron_parts() { delete object; }
    virtual symbol_t type() const { return sym_polyhedron; }
    virtual bool assign(parts*);
    virtual bool load(const char* str);

    void render() const;
    void renderWire() const;
};

class anim_parts : public parts
{
public:
    typedef affineanim object_t;
    affineanim *object;
    std::list<polyhedron_parts*> poly;

    inline anim_parts(const char *name) : parts(name), object(NULL) {}
    virtual ~anim_parts() { delete object; }
    virtual symbol_t type() const { return sym_anim; }
    virtual bool assign(parts*);
    virtual bool load(const char* str);

    void render(int frame) const;
    void renderWire(int frame) const;
};

class body_parts : public parts
{
public:
    typedef std::list<affinemotion*> object_t;
    std::list<anim_parts*> object;

    inline body_parts(const char *name) : parts(name) {}
    virtual ~body_parts() {}
    inline symbol_t type() const { return sym_body; }
    virtual bool assign(parts*);
    virtual bool load(const char* str) { return false; }

    void render(int frame) const;
    void renderWire(int frame) const;
};

/* __END__END__ */
/***********************************************************************/
#endif
/***********************************************************************
 *	END OF loadparts.h
 ***********************************************************************/