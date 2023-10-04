//---------------------------------------------------------------------------
//	$Id: bemmesh.h,v 1.1.1.1 2007/12/30 04:44:24 canacar Exp $
//---------------------------------------------------------------------------
/* 
 * Copyright (C) 2014 Can Erkin Acar
 * Copyright (C) 2014 Zeynep Akalin Acar
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef bemmeshH
#define bemmeshH
#include <stdio.h>
#include <assert.h>
//---------------------------------------------------------------------------
typedef double Point[3];
typedef struct{
    int start;
    int size;
    int sigin;
    int sigout;
} Boundary;

#define NUM_SIG_TYPES 20
#define MAX_BOUND 32

struct Dipole{
    double x,y,z;
    double px,py,pz;
};
//---------------------------------------------------------------------------
class BEMesh{
public:
    BEMesh(double defsig=0.2);
    virtual ~BEMesh();

    virtual int loadMesh(char *name);

    int numNodes(void){return m_cnum;}
    int numElements(void){return m_enum;}
    int numNodeElem(void){return m_node_per_elem;}
    int numBoundaries(void){return m_bnum;}

    void setSigma(int n, double sig){
        if(n>=0 && n<NUM_SIG_TYPES) m_sigma[n]=sig;
    }

//    int *getElement(int e);
//    int *getNode(int n);

    double innerSigNode(int n);
    double outerSigNode(int n);
    double averageSigNode(int n);
    double innerSigElem(int e);
    double outerSigElem(int e);
    double innerSigBnd(int b);
    double outerSigBnd(int b);

    int numBndNodes(int b);
    int numBndElem(int b);

    int **getBndElem(int bnd);
    int getBndNode(int &st, int b);

    virtual void getNodeCoord(int n, double &nx, double &ny, double &nz);
    virtual void getElemCoord(int e, double *x, double *y, double *z);

    virtual int elemNode(int el, int nd);

private:
    FILE *openFile(const char *name, const char *ext);
    char *getLine(FILE *f);
    int loadInfo(FILE *f);
    int loadElem(FILE *f);
    int loadCoord(FILE *f);
    void markNode(int nd, int bnd);

protected:    
    int m_bnum, m_enum, m_cnum;
    int m_node_per_elem;
    double m_sigma[NUM_SIG_TYPES];

    Boundary *m_bound;
    Point *m_coords;
    int **m_elements;
    unsigned *m_bndinfo;
};

class BESubMesh:public BEMesh{
public:
    BESubMesh(BEMesh &base, int bnd);
    virtual ~BESubMesh();

    virtual int loadMesh(char *name);
    virtual void getNodeCoord(int n, double &nx, double &ny, double &nz);
    virtual void getElemCoord(int e, double *x, double *y, double *z);
    virtual int elemNode(int el, int nd);

    int getForwardMapping(int n){
        assert(n>=0 && n<m_cnum);
        return m_fmap[n];
    }
    int getInverseMapping(int n){
        assert(n>=0 && n<m_base->numNodes());
        return m_imap[n];
    }

private:
    BEMesh *m_base;
    int *m_fmap, *m_imap; // forward and inverse maps
    int m_inum;
};

#endif
