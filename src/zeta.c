#include <math.h>
#include "zeta.h"
#include "linmath.h"

void generateMesh(u32* indices, u32 grid_w, u32 grid_h) {
    u32 idx = 0;

    for (u32 i = 0; i < grid_h -1; i++) {
        for (u32 j = 0; j < grid_w - 1; j++) {
            u32 topLeft = i * grid_w + j;
            u32 topRight = i * grid_w + (j +1);
            u32 bottomLeft = (i + 1) * grid_w + j;
            u32 bottomRight = (i + 1) * grid_w + (j + 1);

            //tri 1 = tl bl tr
            indices[idx++] = topLeft;
            indices[idx++] = bottomLeft;
            indices[idx++] = topRight;
            
            //tri 2 tr bl br
            indices[idx++] = topRight;
            indices[idx++] = bottomLeft;
            indices[idx++] = bottomRight;
        }
    }
}

void populateMesh(ZetaPoint* grid, ZetaVertex* gridVert, u32 w, u32 h, f32 sigma_min, f32 sigma_max, 
        f32 t_min, f32 t_max, ComplexFunc func) {
    for (u32 i = 0; i < h; i++) {
        f32 t = t_min + i * (t_max - t_min) / (h - 1);
        for (int j = 0; j < w; j++) {
            f32 sigma = sigma_min + j * (sigma_max - sigma_min) / (w - 1);
            f32 re, im;
            func(sigma, t, &re, &im);
            ZetaPoint* zp = &grid[i * w + j];
            ZetaVertex* zv = &gridVert[i * w + j];
            zp->sigma = sigma;
            zp->t = t;
            zp->re = re;
            zp->im = im;
            zp->mag = sqrtf(re * re + im * im);
            zp->arg = atan2f(im, re);
            zv->re = re;
            zv->im = im;
            zv->mag = zp->mag;
            zv->arg = zp->arg;
        }
    }
}

void zetaApprox(f32 sigma, f32 t, f32* re_out, f32* im_out) {
    u32 N = 100;
    f32 re = 0.0f;
    f32 im = 0.0f;
    for (u32 n = 1; n <= N; n++) {
        f32 logn = logf((f32)n);
        f32 amp = powf(n, -sigma);
        f32 theta = t * logn;
        re += amp * cosf(theta); 
        im -= amp * sinf(theta);
    }
    *re_out = re;
    *im_out = im;
}

void expITheta(f32 sigma, f32 t, f32* re_out, f32* im_out) {
    f32 expDecay = expf(-t);
    *re_out = expDecay * cosf(sigma);
    *im_out = expDecay * sinf(sigma);
}

void sin_complex(f32 sigma, f32 t, f32* re_out, f32* im_out) {
    *re_out = sinf(sigma) * coshf(t);
    *im_out = cosf(sigma) * sinhf(t);
}
