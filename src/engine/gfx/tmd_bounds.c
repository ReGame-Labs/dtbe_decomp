#include "common.h"
#include "engine/gfx/tmd_bounds.h"

/* Grows bounds by the vertices of count flat triangles. */
void growBoundsF3(TMD_P_F3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count Gouraud-shaded triangles. */
void growBoundsG3(TMD_P_G3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit flat triangles. */
void growBoundsNF3(TMD_P_NF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit Gouraud-shaded triangles. */
void growBoundsNG3(TMD_P_NG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count flat quads. */
void growBoundsF4(TMD_P_F4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count Gouraud-shaded quads. */
void growBoundsG4(TMD_P_G4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit flat quads. */
void growBoundsNF4(TMD_P_NF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit Gouraud-shaded quads. */
void growBoundsNG4(TMD_P_NG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count textured flat triangles. */
void growBoundsTF3(TMD_P_TF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count textured Gouraud-shaded triangles. */
void growBoundsTG3(TMD_P_TG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit textured flat triangles. */
void growBoundsTNF3(TMD_P_TNF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit textured Gouraud-shaded triangles. */
void growBoundsTNG3(TMD_P_TNG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count textured flat quads. */
void growBoundsTF4(TMD_P_TF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count textured Gouraud-shaded quads. */
void growBoundsTG4(TMD_P_TG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit textured flat quads. */
void growBoundsTNF4(TMD_P_TNF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit textured Gouraud-shaded quads. */
void growBoundsTNG4(TMD_P_TNG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds to take in vertex. */
void boundsGrowByVertex(Bounds *bounds, SVECTOR *vertex) {
    s16 coord;

    coord = vertex->vx;
    if (coord < bounds->min[0]) {
        bounds->min[0] = coord;
    }
    if (bounds->max[0] < coord) {
        bounds->max[0] = coord;
    }
    coord = vertex->vy;
    if (coord < bounds->min[1]) {
        bounds->min[1] = coord;
    }
    if (bounds->max[1] < coord) {
        bounds->max[1] = coord;
    }
    coord = vertex->vz;
    if (coord < bounds->min[2]) {
        bounds->min[2] = coord;
    }
    if (bounds->max[2] < coord) {
        bounds->max[2] = coord;
    }
}
