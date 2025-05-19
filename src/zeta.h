#ifndef zeta_ZETA_H
#define zeta_ZETA_H

#include "common_types.h"

typedef void (*ComplexFunc)(f32 sigma, f32 t, f32 *re_out, f32* im_out);

typedef struct ZetaPoint {
    f32 sigma;
    f32 t;
    f32 re;
    f32 im;
    f32 mag;
    f32 arg;
} ZetaPoint;

typedef struct ZetaVertex {
    f32 re;
    f32 im;
    f32 mag;
    f32 arg;
} ZetaVertex;

void generateMesh(u32* indices, u32 grid_h, u32 grid_w);
void populateMesh(ZetaPoint* grid, ZetaVertex* vertexGrid, u32 w, u32 h, f32 sigma_min, f32 sigma_max, 
        f32 t_min, f32 t_max, ComplexFunc func);

void zetaApprox(f32 sigma, f32 t, f32* re_out, f32* im_out); 
void expITheta(f32 sigma, f32 t, f32* re_out, f32* im_out); 
void sin_complex(f32 sigma, f32 t, f32* re_out, f32* im_out); 

#endif
