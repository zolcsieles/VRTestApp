#pragma once

#include "Config.h"

#ifdef USE_GX_OPENGL

#define GL_GLEXT_LEGACY
#include <SDL_opengl.h>
#undef GL_VERSION_1_1

/*
* Miscellaneous
*/
typedef void (APIENTRYP PFNGLCLEARINDEXPROC)(GLfloat c);
typedef void (APIENTRYP PFNGLCLEARCOLORPROC)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (APIENTRYP PFNGLCLEARPROC)(GLbitfield mask);
typedef void (APIENTRYP PFNGLINDEXMASKPROC)(GLuint mask);
typedef void (APIENTRYP PFNGLCOLORMASKPROC)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (APIENTRYP PFNGLALPHAFUNCPROC)(GLenum func, GLclampf ref);
typedef void (APIENTRYP PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);
typedef void (APIENTRYP PFNGLLOGICOPPROC)(GLenum opcode);
typedef void (APIENTRYP PFNGLCULLFACEPROC)(GLenum mode);
typedef void (APIENTRYP PFNGLFRONTFACEPROC)(GLenum mode);
typedef void (APIENTRYP PFNGLPOINTSIZEPROC)(GLfloat size);
typedef void (APIENTRYP PFNGLLINEWIDTHPROC)(GLfloat width);
typedef void (APIENTRYP PFNGLLINESTIPPLEPROC)(GLint factor, GLushort pattern);
typedef void (APIENTRYP PFNGLPOLYGONMODEPROC)(GLenum face, GLenum mode);
typedef void (APIENTRYP PFNGLPOLYGONOFFSETPROC)(GLfloat factor, GLfloat units);
typedef void (APIENTRYP PFNGLPOLYGONSTIPPLEPROC)(const GLubyte *mask);
typedef void (APIENTRYP PFNGLGETPOLYGONSTIPPLEPROC)(GLubyte *mask);
typedef void (APIENTRYP PFNGLEDGEFLAGPROC)(GLboolean flag);
typedef void (APIENTRYP PFNGLEDGEFLAGVPROC)(const GLboolean *flag);
typedef void (APIENTRYP PFNGLSCISSORPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLCLIPPLANEPROC)(GLenum plane, const GLdouble *equation);
typedef void (APIENTRYP PFNGLGETCLIPPLANEPROC)(GLenum plane, GLdouble *equation);
typedef void (APIENTRYP PFNGLDRAWBUFFERPROC)(GLenum mode);
typedef void (APIENTRYP PFNGLREADBUFFERPROC)(GLenum mode);
typedef void (APIENTRYP PFNGLENABLEPROC)(GLenum cap);
typedef void (APIENTRYP PFNGLDISABLEPROC)(GLenum cap);
typedef GLboolean (APIENTRYP PFNGLISENABLEDPROC)(GLenum cap);
typedef void (APIENTRYP PFNGLENABLECLIENTSTATEPROC)(GLenum cap);  /* 1.1 */
typedef void (APIENTRYP PFNGLDISABLECLIENTSTATEPROC)(GLenum cap);  /* 1.1 */
typedef void (APIENTRYP PFNGLGETBOOLEANVPROC)(GLenum pname, GLboolean *params);
typedef void (APIENTRYP PFNGLGETDOUBLEVPROC)(GLenum pname, GLdouble *params);
typedef void (APIENTRYP PFNGLGETFLOATVPROC)(GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETINTEGERVPROC)(GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLPUSHATTRIBPROC)(GLbitfield mask);
typedef void (APIENTRYP PFNGLPOPATTRIBPROC)(void);
typedef void (APIENTRYP PFNGLPUSHCLIENTATTRIBPROC)(GLbitfield mask);  /* 1.1 */
typedef void (APIENTRYP PFNGLPOPCLIENTATTRIBPROC)(void);  /* 1.1 */
typedef GLint (APIENTRYP PFNGLRENDERMODEPROC)(GLenum mode);
typedef GLenum (APIENTRYP PFNGLGETERRORPROC)(void);
typedef const GLubyte * (APIENTRYP PFNGLGETSTRINGPROC)(GLenum name);
typedef void (APIENTRYP PFNGLFINISHPROC)(void);
typedef void (APIENTRYP PFNGLFLUSHPROC)(void);
typedef void (APIENTRYP PFNGLHINTPROC)(GLenum target, GLenum mode);

/*
* DEPTH BUFFER
*/
typedef void (APIENTRYP PFNGLCLEARDEPTHPROC)(GLclampd depth);
typedef void (APIENTRYP PFNGLDEPTHFUNCPROC)(GLenum func);
typedef void (APIENTRYP PFNGLDEPTHMASKPROC)(GLboolean flag);
typedef void (APIENTRYP PFNGLDEPTHRANGEPROC)(GLclampd near_val, GLclampd far_val);

/*
* ACCUMulation BUFFER
*/
typedef void (APIENTRYP PFNGLCLEARACCUMPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRYP PFNGLACCUMPROC)(GLenum op, GLfloat value);

/*
* Transformation
*/
typedef void (APIENTRYP PFNGLMATRIXMODEPROC)(GLenum mode);
typedef void (APIENTRYP PFNGLORTHOPROC)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
typedef void (APIENTRYP PFNGLFRUSTUMPROC)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
typedef void (APIENTRYP PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLPUSHMATRIXPROC)(void);
typedef void (APIENTRYP PFNGLPOPMATRIXPROC)(void);
typedef void (APIENTRYP PFNGLLOADIDENTITYPROC)(void);
typedef void (APIENTRYP PFNGLLOADMATRIXDPROC)(const GLdouble *m);
typedef void (APIENTRYP PFNGLLOADMATRIXFPROC)(const GLfloat *m);
typedef void (APIENTRYP PFNGLMULTMATRIXDPROC)(const GLdouble *m);
typedef void (APIENTRYP PFNGLMULTMATRIXFPROC)(const GLfloat *m);
typedef void (APIENTRYP PFNGLROTATEDPROC)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLROTATEFPROC)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRYP PFNGLSCALEDPROC)(GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLSCALEFPROC)(GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRYP PFNGLTRANSLATEDPROC)(GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLTRANSLATEFPROC)(GLfloat x, GLfloat y, GLfloat z);

/*
* Display LISTs
*/
typedef GLboolean (APIENTRYP PFNGLISLISTPROC)(GLuint list);
typedef void (APIENTRYP PFNGLDELETELISTSPROC)(GLuint list, GLsizei range);
typedef GLuint (APIENTRYP PFNGLGENLISTSPROC)(GLsizei range);
typedef void (APIENTRYP PFNGLNEWLISTPROC)(GLuint list, GLenum mode);
typedef void (APIENTRYP PFNGLENDLISTPROC)(void);
typedef void (APIENTRYP PFNGLCALLLISTPROC)(GLuint list);
typedef void (APIENTRYP PFNGLCALLLISTSPROC)(GLsizei n, GLenum type, const GLvoid *lists);
typedef void (APIENTRYP PFNGLLISTBASEPROC)(GLuint base);

/*
* DRAWing FUNCtions
*/
typedef void (APIENTRYP PFNGLBEGINPROC)(GLenum mode);
typedef void (APIENTRYP PFNGLENDPROC)(void);
typedef void (APIENTRYP PFNGLVERTEX2DPROC)(GLdouble x, GLdouble y);
typedef void (APIENTRYP PFNGLVERTEX2FPROC)(GLfloat x, GLfloat y);
typedef void (APIENTRYP PFNGLVERTEX2IPROC)(GLint x, GLint y);
typedef void (APIENTRYP PFNGLVERTEX2SPROC)(GLshort x, GLshort y);
typedef void (APIENTRYP PFNGLVERTEX3DPROC)(GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLVERTEX3FPROC)(GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRYP PFNGLVERTEX3IPROC)(GLint x, GLint y, GLint z);
typedef void (APIENTRYP PFNGLVERTEX3SPROC)(GLshort x, GLshort y, GLshort z);
typedef void (APIENTRYP PFNGLVERTEX4DPROC)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRYP PFNGLVERTEX4FPROC)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRYP PFNGLVERTEX4IPROC)(GLint x, GLint y, GLint z, GLint w);
typedef void (APIENTRYP PFNGLVERTEX4SPROC)(GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRYP PFNGLVERTEX2DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEX2FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEX2IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLVERTEX2SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEX3DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEX3FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEX3IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLVERTEX3SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEX4DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEX4FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEX4IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLVERTEX4SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLNORMAL3BPROC)(GLbyte nx, GLbyte ny, GLbyte nz);
typedef void (APIENTRYP PFNGLNORMAL3DPROC)(GLdouble nx, GLdouble ny, GLdouble nz);
typedef void (APIENTRYP PFNGLNORMAL3FPROC)(GLfloat nx, GLfloat ny, GLfloat nz);
typedef void (APIENTRYP PFNGLNORMAL3IPROC)(GLint nx, GLint ny, GLint nz);
typedef void (APIENTRYP PFNGLNORMAL3SPROC)(GLshort nx, GLshort ny, GLshort nz);
typedef void (APIENTRYP PFNGLNORMAL3BVPROC)(const GLbyte *v);
typedef void (APIENTRYP PFNGLNORMAL3DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLNORMAL3FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLNORMAL3IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLNORMAL3SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLINDEXDPROC)(GLdouble c);
typedef void (APIENTRYP PFNGLINDEXFPROC)(GLfloat c);
typedef void (APIENTRYP PFNGLINDEXIPROC)(GLint c);
typedef void (APIENTRYP PFNGLINDEXSPROC)(GLshort c);
typedef void (APIENTRYP PFNGLINDEXUBPROC)(GLubyte c);  /* 1.1 */
typedef void (APIENTRYP PFNGLINDEXDVPROC)(const GLdouble *c);
typedef void (APIENTRYP PFNGLINDEXFVPROC)(const GLfloat *c);
typedef void (APIENTRYP PFNGLINDEXIVPROC)(const GLint *c);
typedef void (APIENTRYP PFNGLINDEXSVPROC)(const GLshort *c);
typedef void (APIENTRYP PFNGLINDEXUBVPROC)(const GLubyte *c);  /* 1.1 */
typedef void (APIENTRYP PFNGLCOLOR3BPROC)(GLbyte red, GLbyte green, GLbyte blue);
typedef void (APIENTRYP PFNGLCOLOR3DPROC)(GLdouble red, GLdouble green, GLdouble blue);
typedef void (APIENTRYP PFNGLCOLOR3FPROC)(GLfloat red, GLfloat green, GLfloat blue);
typedef void (APIENTRYP PFNGLCOLOR3IPROC)(GLint red, GLint green, GLint blue);
typedef void (APIENTRYP PFNGLCOLOR3SPROC)(GLshort red, GLshort green, GLshort blue);
typedef void (APIENTRYP PFNGLCOLOR3UBPROC)(GLubyte red, GLubyte green, GLubyte blue);
typedef void (APIENTRYP PFNGLCOLOR3UIPROC)(GLuint red, GLuint green, GLuint blue);
typedef void (APIENTRYP PFNGLCOLOR3USPROC)(GLushort red, GLushort green, GLushort blue);
typedef void (APIENTRYP PFNGLCOLOR4BPROC)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
typedef void (APIENTRYP PFNGLCOLOR4DPROC)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
typedef void (APIENTRYP PFNGLCOLOR4FPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRYP PFNGLCOLOR4IPROC)(GLint red, GLint green, GLint blue, GLint alpha);
typedef void (APIENTRYP PFNGLCOLOR4SPROC)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
typedef void (APIENTRYP PFNGLCOLOR4UBPROC)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void (APIENTRYP PFNGLCOLOR4UIPROC)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
typedef void (APIENTRYP PFNGLCOLOR4USPROC)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
typedef void (APIENTRYP PFNGLCOLOR3BVPROC)(const GLbyte *v);
typedef void (APIENTRYP PFNGLCOLOR3DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLCOLOR3FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLCOLOR3IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLCOLOR3SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLCOLOR3UBVPROC)(const GLubyte *v);
typedef void (APIENTRYP PFNGLCOLOR3UIVPROC)(const GLuint *v);
typedef void (APIENTRYP PFNGLCOLOR3USVPROC)(const GLushort *v);
typedef void (APIENTRYP PFNGLCOLOR4BVPROC)(const GLbyte *v);
typedef void (APIENTRYP PFNGLCOLOR4DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLCOLOR4FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLCOLOR4IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLCOLOR4SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLCOLOR4UBVPROC)(const GLubyte *v);
typedef void (APIENTRYP PFNGLCOLOR4UIVPROC)(const GLuint *v);
typedef void (APIENTRYP PFNGLCOLOR4USVPROC)(const GLushort *v);
typedef void (APIENTRYP PFNGLTEXCOORD1DPROC)(GLdouble s);
typedef void (APIENTRYP PFNGLTEXCOORD1FPROC)(GLfloat s);
typedef void (APIENTRYP PFNGLTEXCOORD1IPROC)(GLint s);
typedef void (APIENTRYP PFNGLTEXCOORD1SPROC)(GLshort s);
typedef void (APIENTRYP PFNGLTEXCOORD2DPROC)(GLdouble s, GLdouble t);
typedef void (APIENTRYP PFNGLTEXCOORD2FPROC)(GLfloat s, GLfloat t);
typedef void (APIENTRYP PFNGLTEXCOORD2IPROC)(GLint s, GLint t);
typedef void (APIENTRYP PFNGLTEXCOORD2SPROC)(GLshort s, GLshort t);
typedef void (APIENTRYP PFNGLTEXCOORD3DPROC)(GLdouble s, GLdouble t, GLdouble r);
typedef void (APIENTRYP PFNGLTEXCOORD3FPROC)(GLfloat s, GLfloat t, GLfloat r);
typedef void (APIENTRYP PFNGLTEXCOORD3IPROC)(GLint s, GLint t, GLint r);
typedef void (APIENTRYP PFNGLTEXCOORD3SPROC)(GLshort s, GLshort t, GLshort r);
typedef void (APIENTRYP PFNGLTEXCOORD4DPROC)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (APIENTRYP PFNGLTEXCOORD4FPROC)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (APIENTRYP PFNGLTEXCOORD4IPROC)(GLint s, GLint t, GLint r, GLint q);
typedef void (APIENTRYP PFNGLTEXCOORD4SPROC)(GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (APIENTRYP PFNGLTEXCOORD1DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLTEXCOORD1FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLTEXCOORD1IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLTEXCOORD1SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLTEXCOORD2DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLTEXCOORD2FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLTEXCOORD2IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLTEXCOORD2SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLTEXCOORD3DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLTEXCOORD3FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLTEXCOORD3IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLTEXCOORD3SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLTEXCOORD4DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLTEXCOORD4FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLTEXCOORD4IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLTEXCOORD4SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLRASTERPOS2DPROC)(GLdouble x, GLdouble y);
typedef void (APIENTRYP PFNGLRASTERPOS2FPROC)(GLfloat x, GLfloat y);
typedef void (APIENTRYP PFNGLRASTERPOS2IPROC)(GLint x, GLint y);
typedef void (APIENTRYP PFNGLRASTERPOS2SPROC)(GLshort x, GLshort y);
typedef void (APIENTRYP PFNGLRASTERPOS3DPROC)(GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLRASTERPOS3FPROC)(GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRYP PFNGLRASTERPOS3IPROC)(GLint x, GLint y, GLint z);
typedef void (APIENTRYP PFNGLRASTERPOS3SPROC)(GLshort x, GLshort y, GLshort z);
typedef void (APIENTRYP PFNGLRASTERPOS4DPROC)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRYP PFNGLRASTERPOS4FPROC)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRYP PFNGLRASTERPOS4IPROC)(GLint x, GLint y, GLint z, GLint w);
typedef void (APIENTRYP PFNGLRASTERPOS4SPROC)(GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRYP PFNGLRASTERPOS2DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLRASTERPOS2FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLRASTERPOS2IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLRASTERPOS2SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLRASTERPOS3DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLRASTERPOS3FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLRASTERPOS3IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLRASTERPOS3SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLRASTERPOS4DVPROC)(const GLdouble *v);
typedef void (APIENTRYP PFNGLRASTERPOS4FVPROC)(const GLfloat *v);
typedef void (APIENTRYP PFNGLRASTERPOS4IVPROC)(const GLint *v);
typedef void (APIENTRYP PFNGLRASTERPOS4SVPROC)(const GLshort *v);
typedef void (APIENTRYP PFNGLRECTDPROC)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
typedef void (APIENTRYP PFNGLRECTFPROC)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
typedef void (APIENTRYP PFNGLRECTIPROC)(GLint x1, GLint y1, GLint x2, GLint y2);
typedef void (APIENTRYP PFNGLRECTSPROC)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
typedef void (APIENTRYP PFNGLRECTDVPROC)(const GLdouble *v1, const GLdouble *v2);
typedef void (APIENTRYP PFNGLRECTFVPROC)(const GLfloat *v1, const GLfloat *v2);
typedef void (APIENTRYP PFNGLRECTIVPROC)(const GLint *v1, const GLint *v2);
typedef void (APIENTRYP PFNGLRECTSVPROC)(const GLshort *v1, const GLshort *v2);

/*
* VERTEX ARRAYs  (1.1)
*/
typedef void (APIENTRYP PFNGLVERTEXPOINTERPROC)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLNORMALPOINTERPROC)(GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLCOLORPOINTERPROC)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLINDEXPOINTERPROC)(GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLTEXCOORDPOINTERPROC)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLEDGEFLAGPOINTERPROC)(GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLGETPOINTERVPROC)(GLenum pname, GLvoid **params);
typedef void (APIENTRYP PFNGLARRAYELEMENTPROC)(GLint i);
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (APIENTRYP PFNGLINTERLEAVEDARRAYSPROC)(GLenum format, GLsizei stride, const GLvoid *pointer);

/*
* LIGHTing
*/
typedef void (APIENTRYP PFNGLSHADEMODELPROC)(GLenum mode);
typedef void (APIENTRYP PFNGLLIGHTFPROC)(GLenum light, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLLIGHTIPROC)(GLenum light, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLLIGHTFVPROC)(GLenum light, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLLIGHTIVPROC)(GLenum light, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLGETLIGHTFVPROC)(GLenum light, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETLIGHTIVPROC)(GLenum light, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLLIGHTMODELFPROC)(GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLLIGHTMODELIPROC)(GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLLIGHTMODELFVPROC)(GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLLIGHTMODELIVPROC)(GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLMATERIALFPROC)(GLenum face, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLMATERIALIPROC)(GLenum face, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLMATERIALFVPROC)(GLenum face, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLMATERIALIVPROC)(GLenum face, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLGETMATERIALFVPROC)(GLenum face, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETMATERIALIVPROC)(GLenum face, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLCOLORMATERIALPROC)(GLenum face, GLenum mode);

/*
* RASTER functions
*/
typedef void (APIENTRYP PFNGLPIXELZOOMPROC)(GLfloat xfactor, GLfloat yfactor);
typedef void (APIENTRYP PFNGLPIXELSTOREFPROC)(GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLPIXELSTOREIPROC)(GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLPIXELTRANSFERFPROC)(GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLPIXELTRANSFERIPROC)(GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLPIXELMAPFVPROC)(GLenum map, GLsizei mapsize, const GLfloat *values);
typedef void (APIENTRYP PFNGLPIXELMAPUIVPROC)(GLenum map, GLsizei mapsize, const GLuint *values);
typedef void (APIENTRYP PFNGLPIXELMAPUSVPROC)(GLenum map, GLsizei mapsize, const GLushort *values);
typedef void (APIENTRYP PFNGLGETPIXELMAPFVPROC)(GLenum map, GLfloat *values);
typedef void (APIENTRYP PFNGLGETPIXELMAPUIVPROC)(GLenum map, GLuint *values);
typedef void (APIENTRYP PFNGLGETPIXELMAPUSVPROC)(GLenum map, GLushort *values);
typedef void (APIENTRYP PFNGLBITMAPPROC)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
typedef void (APIENTRYP PFNGLREADPIXELSPROC)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
typedef void (APIENTRYP PFNGLDRAWPIXELSPROC)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLCOPYPIXELSPROC)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);

/*
* STENCILing
*/
typedef void (APIENTRYP PFNGLSTENCILFUNCPROC)(GLenum func, GLint ref, GLuint mask);
typedef void (APIENTRYP PFNGLSTENCILMASKPROC)(GLuint mask);
typedef void (APIENTRYP PFNGLSTENCILOPPROC)(GLenum fail, GLenum zfail, GLenum zpass);
typedef void (APIENTRYP PFNGLCLEARSTENCILPROC)(GLint s);

/*
* TEXTURE mapping
*/
typedef void (APIENTRYP PFNGLTEXGENDPROC)(GLenum coord, GLenum pname, GLdouble param);
typedef void (APIENTRYP PFNGLTEXGENFPROC)(GLenum coord, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXGENIPROC)(GLenum coord, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXGENDVPROC)(GLenum coord, GLenum pname, const GLdouble *params);
typedef void (APIENTRYP PFNGLTEXGENFVPROC)(GLenum coord, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLTEXGENIVPROC)(GLenum coord, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLGETTEXGENDVPROC)(GLenum coord, GLenum pname, GLdouble *params);
typedef void (APIENTRYP PFNGLGETTEXGENFVPROC)(GLenum coord, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXGENIVPROC)(GLenum coord, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLTEXENVFPROC)(GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXENVIPROC)(GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXENVFVPROC)(GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLTEXENVIVPROC)(GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLGETTEXENVFVPROC)(GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXENVIVPROC)(GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLTEXPARAMETERFPROC)(GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXPARAMETERFVPROC)(GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLTEXPARAMETERIVPROC)(GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERFVPROC)(GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERIVPROC)(GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETTEXLEVELPARAMETERFVPROC)(GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXLEVELPARAMETERIVPROC)(GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLTEXIMAGE1DPROC)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLGETTEXIMAGEPROC)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);

/* 1.1 functions */
typedef void (APIENTRYP PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
typedef void (APIENTRYP PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint *textures);
typedef void (APIENTRYP PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void (APIENTRYP PFNGLPRIORITIZETEXTURESPROC)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef GLboolean (APIENTRYP PFNGLARETEXTURESRESIDENTPROC)(GLsizei n, const GLuint *textures, GLboolean *residences);
typedef GLboolean (APIENTRYP PFNGLISTEXTUREPROC)(GLuint texture);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE1DPROC)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE2DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLCOPYTEXIMAGE1DPROC)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (APIENTRYP PFNGLCOPYTEXIMAGE2DPROC)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (APIENTRYP PFNGLCOPYTEXSUBIMAGE1DPROC)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (APIENTRYP PFNGLCOPYTEXSUBIMAGE2DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);


/*
* EVALuators
*/
typedef void (APIENTRYP PFNGLMAP1DPROC)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
typedef void (APIENTRYP PFNGLMAP1FPROC)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
typedef void (APIENTRYP PFNGLMAP2DPROC)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,	GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
typedef void (APIENTRYP PFNGLMAP2FPROC)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);

typedef void (APIENTRYP PFNGLGETMAPDVPROC)(GLenum target, GLenum query, GLdouble *v);
typedef void (APIENTRYP PFNGLGETMAPFVPROC)(GLenum target, GLenum query, GLfloat *v);
typedef void (APIENTRYP PFNGLGETMAPIVPROC)(GLenum target, GLenum query, GLint *v);
typedef void (APIENTRYP PFNGLEVALCOORD1DPROC)(GLdouble u);
typedef void (APIENTRYP PFNGLEVALCOORD1FPROC)(GLfloat u);
typedef void (APIENTRYP PFNGLEVALCOORD1DVPROC)(const GLdouble *u);
typedef void (APIENTRYP PFNGLEVALCOORD1FVPROC)(const GLfloat *u);
typedef void (APIENTRYP PFNGLEVALCOORD2DPROC)(GLdouble u, GLdouble v);
typedef void (APIENTRYP PFNGLEVALCOORD2FPROC)(GLfloat u, GLfloat v);
typedef void (APIENTRYP PFNGLEVALCOORD2DVPROC)(const GLdouble *u);
typedef void (APIENTRYP PFNGLEVALCOORD2FVPROC)(const GLfloat *u);
typedef void (APIENTRYP PFNGLMAPGRID1DPROC)(GLint un, GLdouble u1, GLdouble u2);
typedef void (APIENTRYP PFNGLMAPGRID1FPROC)(GLint un, GLfloat u1, GLfloat u2);
typedef void (APIENTRYP PFNGLMAPGRID2DPROC)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
typedef void (APIENTRYP PFNGLMAPGRID2FPROC)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
typedef void (APIENTRYP PFNGLEVALPOINT1PROC)(GLint i);
typedef void (APIENTRYP PFNGLEVALPOINT2PROC)(GLint i, GLint j);
typedef void (APIENTRYP PFNGLEVALMESH1PROC)(GLenum mode, GLint i1, GLint i2);
typedef void (APIENTRYP PFNGLEVALMESH2PROC)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);

/*
* FOG
*/
typedef void (APIENTRYP PFNGLFOGFPROC)(GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLFOGIPROC)(GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLFOGFVPROC)(GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLFOGIVPROC)(GLenum pname, const GLint *params);


/*
* SELECTion and FEEDBACK
*/
typedef void (APIENTRYP PFNGLFEEDBACKBUFFERPROC)(GLsizei size, GLenum type, GLfloat *buffer);
typedef void (APIENTRYP PFNGLPASSTHROUGHPROC)(GLfloat token);
typedef void (APIENTRYP PFNGLSELECTBUFFERPROC)(GLsizei size, GLuint *buffer);
typedef void (APIENTRYP PFNGLINITNAMESPROC)(void);
typedef void (APIENTRYP PFNGLLOADNAMEPROC)(GLuint name);
typedef void (APIENTRYP PFNGLPUSHNAMEPROC)(GLuint name);
typedef void (APIENTRYP PFNGLPOPNAMEPROC)(void);


#undef GL_VERSION_1_2
#undef GL_VERSION_1_3
#undef GL_ARB_imaging
#include <SDL_opengl_glext.h>
#pragma comment(lib, "opengl32.lib")

//WGL_NV_DX_interop
#define WGL_ACCESS_READ_ONLY_NV 0x0000
#define WGL_ACCESS_READ_WRITE_NV 0x0001
#define WGL_ACCESS_WRITE_DISCARD_NV 0x0002

typedef BOOL(WINAPI * PFNWGLDXCLOSEDEVICENVPROC) (HANDLE hDevice);
typedef BOOL(WINAPI * PFNWGLDXLOCKOBJECTSNVPROC) (HANDLE hDevice, GLint count, HANDLE* hObjects);
typedef BOOL(WINAPI * PFNWGLDXOBJECTACCESSNVPROC) (HANDLE hObject, GLenum access);
typedef HANDLE(WINAPI * PFNWGLDXOPENDEVICENVPROC) (void* dxDevice);
typedef HANDLE(WINAPI * PFNWGLDXREGISTEROBJECTNVPROC) (HANDLE hDevice, void* dxObject, GLuint name, GLenum type, GLenum access);
typedef BOOL(WINAPI * PFNWGLDXSETRESOURCESHAREHANDLENVPROC) (void* dxObject, HANDLE shareHandle);
typedef BOOL(WINAPI * PFNWGLDXUNLOCKOBJECTSNVPROC) (HANDLE hDevice, GLint count, HANDLE* hObjects);
typedef BOOL(WINAPI * PFNWGLDXUNREGISTEROBJECTNVPROC) (HANDLE hDevice, HANDLE hObject);

/*
#define wglDXCloseDeviceNV WGLEW_GET_FUN(__wglewDXCloseDeviceNV)
#define wglDXLockObjectsNV WGLEW_GET_FUN(__wglewDXLockObjectsNV)
#define wglDXObjectAccessNV WGLEW_GET_FUN(__wglewDXObjectAccessNV)
#define wglDXOpenDeviceNV WGLEW_GET_FUN(__wglewDXOpenDeviceNV)
#define wglDXRegisterObjectNV WGLEW_GET_FUN(__wglewDXRegisterObjectNV)
#define wglDXSetResourceShareHandleNV WGLEW_GET_FUN(__wglewDXSetResourceShareHandleNV)
#define wglDXUnlockObjectsNV WGLEW_GET_FUN(__wglewDXUnlockObjectsNV)
#define wglDXUnregisterObjectNV WGLEW_GET_FUN(__wglewDXUnregisterObjectNV)

#define WGLEW_NV_DX_interop WGLEW_GET_VAR(__WGLEW_NV_DX_interop)
*/
//


#undef _GL_PROC
#define _GL_PROC(X, x) extern PFN##X##PROC x

namespace gl
{
#include "GL_PROCS.h"
};

#undef _GL_PROC

void initGL();
void testGLFUNCs();

#endif