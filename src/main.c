/*
 * MINI KART - jeu de kart pour NumWorks (application .nwa, EADK)
 * Rendu "Mode 7" (sol en perspective) comme sur Super Nintendo.
 *
 * Fleches : accelerer / freiner / tourner     SHIFT : deraper
 * OK ou EXE : utiliser l'objet                BACK : quitter
 *
 * Tout est en entiers (pas de float, pas de libm, pas de memcpy).
 */
#include "eadk.h"

#pragma GCC optimize ("O2", "no-tree-loop-distribute-patterns")

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Mini Kart";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

#ifdef TEST
#include <stdio.h>
void test_log(const char *s, int a, int b);
#define TLOG(s, a, b) test_log(s, a, b)
#else
#define TLOG(s, a, b)
#endif

#define SW 320
#define SHGT 240
#define BAND 16
#define HZ 92
#define FOC 210
#define CH 26
#define CHF (CH * FOC)
#define FARR 10
#define RMAX 147
#define BACKD 20
#define FARU 340
#define NCP 18
#define SUBD 10
#define NW (NCP * SUBD)
#define PL 1280
#define NRIV 6
#define NBOX 15
#define NTREE 150
#define NBAN 4
#define LAPS 3
#define MAXSPR 160

#define RGB(r, g, b) ((uint16_t)((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3)))

#define C_BLACK RGB(0, 0, 0)
#define C_WHITE RGB(255, 255, 255)
#define C_RED RGB(220, 30, 30)
#define C_YEL RGB(255, 215, 0)
#define C_ORA RGB(255, 140, 0)
#define C_BLU RGB(70, 150, 255)
#define C_SHADOW RGB(40, 90, 40)
#define C_BRN RGB(110, 70, 30)
#define C_FOL RGB(24, 112, 36)
#define C_FOL2 RGB(52, 160, 52)
#define C_SKIN RGB(255, 205, 160)
#define C_DARK RGB(40, 40, 48)
#define C_HILLA RGB(138, 164, 200)
#define C_HILLB RGB(80, 150, 98)

enum { ST_TITLE, ST_COUNT, ST_RACE, ST_FINISH };

/* ------------------------------------------------------------------ */
/* donnees                                                            */
/* ------------------------------------------------------------------ */
static const int16_t cp[NCP][2] = {
    {260, 870}, {480, 870}, {700, 860}, {860, 780}, {900, 640}, {820, 520},
    {660, 500}, {560, 420}, {600, 300}, {720, 230}, {800, 130}, {640, 90},
    {440, 130}, {330, 240}, {230, 350}, {150, 480}, {130, 640}, {150, 780}};

static const uint8_t font[42][7] = {
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, /* 0 */
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, /* 1 */
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, /* 2 */
    {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}, /* 3 */
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, /* 4 */
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, /* 5 */
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, /* 6 */
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, /* 7 */
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, /* 8 */
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, /* 9 */
    {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, /* A */
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, /* B */
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, /* C */
    {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}, /* D */
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, /* E */
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, /* F */
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, /* G */
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, /* H */
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, /* I */
    {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, /* J */
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, /* K */
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, /* L */
    {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, /* M */
    {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, /* N */
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, /* O */
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, /* P */
    {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, /* Q */
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, /* R */
    {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, /* S */
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, /* T */
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, /* U */
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, /* V */
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}, /* W */
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, /* X */
    {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, /* Y */
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, /* Z */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* espace */
    {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10}, /* / */
    {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00}, /* : */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}, /* . */
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}, /* ! */
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}  /* - */
};

static const uint16_t rcol[NRIV] = {RGB(40, 170, 60), RGB(250, 200, 20), RGB(50, 110, 240),
                                    RGB(250, 130, 20), RGB(160, 60, 200), RGB(30, 200, 210)};
static const uint16_t rhelm[NRIV] = {RGB(20, 100, 30), RGB(200, 90, 10), RGB(230, 230, 250),
                                     RGB(240, 240, 240), RGB(250, 220, 60), RGB(20, 60, 140)};

/* tables */
static int16_t sint[1024];
static uint8_t df[64 * 64];
static int32_t wx[NW], wy[NW], seglen[NW], cum[NW + 1];
static int16_t ndx[NW], ndy[NW];
static int8_t curv[NW];
static int32_t TL;
static uint16_t pal[RMAX + 1][8];
static uint16_t skyc[HZ];
static uint8_t hillA[PL], hillB[PL];
static uint16_t band[SW * BAND];
static uint8_t topA[SW], topB[SW];
static int16_t mmx[NW], mmy[NW];
static uint32_t rng;

/* decor */
static int32_t trx[NTREE], try_[NTREE];
static uint8_t trv[NTREE];
static int ntree;
static int32_t bxx[NBOX], bxy[NBOX];
static int bxcd[NBOX];
static int32_t archLx, archLy, archRx, archRy;

/* camera */
static int32_t camx16, camy16, cdx, cdy, camAng;
static int by0;

/* kart du joueur */
static int32_t kx8, ky8;
static int32_t hd, vd, v4, stq;
static int boostT, spinT, stallT, drifting, driftDir, driftT, colCD;
static int item, itemRoll, itemFinal;
static int pidx, lc;
static int32_t ptot4;
static int leanv;
static int camShift;

/* rivaux */
static int32_t rs[NRIV], rvq4[NRIV];
static int rlat[NRIV], rlt[NRIV], rtt[NRIV], rstun[NRIV], rvb[NRIV];
static int32_t rpx[NRIV], rpy[NRIV];
static int rlean[NRIV];

/* objets */
static int32_t banx[NBAN], bany[NBAN];
static int banon[NBAN];
static int32_t shx8, shy8;
static int shc, shs, shlife;

/* course */
static int state;
static int cdT, startBoost, stallOnce;
static int raceT, lapStart, bestLap, finalMs, finalRank;
static char msg[24];
static int msgT;
static int okprev;
static int in_up, in_down, in_left, in_right, in_drift, in_ok;
static uint32_t lastms;
static int frameCount;

/* sprites */
#define SP_PINE 0
#define SP_ROUND 1
#define SP_BOX 2
#define SP_BAN 3
#define SP_SHELL 4
#define SP_KART 5
#define SP_ARCH 6
static int spt[MAXSPR], spx[MAXSPR], spy[MAXSPR], spS[MAXSPR], spf[MAXSPR];
static int spa[MAXSPR], spb[MAXSPR], spc[MAXSPR], spd[MAXSPR];
static int order[MAXSPR];
static int nspr;

/* ------------------------------------------------------------------ */
/* utilitaires                                                        */
/* ------------------------------------------------------------------ */
static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }
static inline int iabs(int a) { return a < 0 ? -a : a; }
static inline int iclamp(int a, int lo, int hi) { return a < lo ? lo : (a > hi ? hi : a); }

static uint32_t rnd(void) {
    rng = rng * 1664525u + 1013904223u;
    return rng >> 16;
}

static int32_t isqrt32(uint32_t n) {
    uint32_t r = 0, bit = 1u << 30;
    while (bit > n) bit >>= 2;
    while (bit) {
        if (n >= r + bit) {
            n -= r + bit;
            r = (r >> 1) + bit;
        } else
            r >>= 1;
        bit >>= 2;
    }
    return (int32_t)r;
}

static inline int sinq(int32_t a6) { return sint[(a6 >> 6) & 1023]; }
static inline int cosq(int32_t a6) { return sint[((a6 >> 6) + 256) & 1023]; }
static inline int32_t angdiff(int32_t a, int32_t b) {
    int32_t d = (a - b) & 65535;
    if (d >= 32768) d -= 65536;
    return d;
}

static uint16_t blend(uint16_t c, int cr, int cg, int cb, int a) {
    int r = ((c >> 11) << 3) | 4, g = (((c >> 5) & 63) << 2) | 2, b = ((c & 31) << 3) | 4;
    r = (r * (256 - a) + cr * a) >> 8;
    g = (g * (256 - a) + cg * a) >> 8;
    b = (b * (256 - a) + cb * a) >> 8;
    return RGB(r, g, b);
}

/* ------------------------------------------------------------------ */
/* initialisation des tables                                          */
/* ------------------------------------------------------------------ */
static void init_sin(void) {
    int a;
    for (a = 0; a < 512; a++) {
        uint32_t x = (uint32_t)a * (uint32_t)(512 - a);
        uint32_t x4 = x >> 2;
        uint32_t v = (16u * x4 * 8192u) / (327680u - x);
        sint[a] = (int16_t)v;
        sint[a + 512] = (int16_t)(-(int32_t)v);
    }
}

static int32_t sex[NW], sey[NW], sl2[NW];

static void build_track(void) {
    static int32_t tx[NW], ty[NW];
    int i, j, k;
    for (i = 0; i < NCP; i++) {
        int p0x = cp[(i + NCP - 1) % NCP][0], p0y = cp[(i + NCP - 1) % NCP][1];
        int p1x = cp[i][0], p1y = cp[i][1];
        int p2x = cp[(i + 1) % NCP][0], p2y = cp[(i + 1) % NCP][1];
        int p3x = cp[(i + 2) % NCP][0], p3y = cp[(i + 2) % NCP][1];
        for (k = 0; k < SUBD; k++) {
            int32_t kk = k;
            int32_t ax = 2 * p1x * 1000 + (p2x - p0x) * kk * 100 +
                         (2 * p0x - 5 * p1x + 4 * p2x - p3x) * kk * kk * 10 +
                         (-p0x + 3 * p1x - 3 * p2x + p3x) * kk * kk * kk;
            int32_t ay = 2 * p1y * 1000 + (p2y - p0y) * kk * 100 +
                         (2 * p0y - 5 * p1y + 4 * p2y - p3y) * kk * kk * 10 +
                         (-p0y + 3 * p1y - 3 * p2y + p3y) * kk * kk * kk;
            tx[i * SUBD + k] = ax / 125;
            ty[i * SUBD + k] = ay / 125;
        }
    }
    for (j = 0; j < NW; j++) {
        wx[j] = tx[(j + 5) % NW];
        wy[j] = ty[(j + 5) % NW];
    }
    cum[0] = 0;
    for (i = 0; i < NW; i++) {
        int32_t dx, dy, len;
        j = (i + 1) % NW;
        dx = wx[j] - wx[i];
        dy = wy[j] - wy[i];
        len = isqrt32((uint32_t)(dx * dx + dy * dy));
        if (len < 1) len = 1;
        seglen[i] = len;
        ndx[i] = (int16_t)((dx * 8192) / len);
        ndy[i] = (int16_t)((dy * 8192) / len);
        sex[i] = dx;
        sey[i] = dy;
        sl2[i] = dx * dx + dy * dy;
        if (sl2[i] < 1) sl2[i] = 1;
        cum[i + 1] = cum[i] + len;
    }
    TL = cum[NW];
    for (i = 0; i < NW; i++) {
        int32_t c;
        j = (i + 5) % NW;
        c = ((int32_t)ndx[i] * ndy[j] - (int32_t)ndy[i] * ndx[j]) >> 13;
        c >>= 6;
        curv[i] = (int8_t)iclamp(c, -127, 127);
    }
}

static void build_df(void) {
    int i, j, k;
    for (j = 0; j < 64; j++) {
        for (i = 0; i < 64; i++) {
            int32_t nx = i * 256, ny = j * 256, best = 0x7fffffff;
            int hv, flag = 0;
            for (k = 0; k < NW; k++) {
                int32_t ax = wx[k], ay = wy[k];
                int32_t px = nx - ax, py = ny - ay, t, qx, qy, dx, dy, d2;
                if (px > 2600 || px < -2600 || py > 2600 || py < -2600) continue;
                t = px * sex[k] + py * sey[k];
                if (t <= 0) {
                    qx = ax;
                    qy = ay;
                } else if (t >= sl2[k]) {
                    qx = ax + sex[k];
                    qy = ay + sey[k];
                } else {
                    int32_t fr = (t * 256) / sl2[k];
                    qx = ax + ((sex[k] * fr) >> 8);
                    qy = ay + ((sey[k] * fr) >> 8);
                }
                dx = nx - qx;
                dy = ny - qy;
                d2 = dx * dx + dy * dy;
                if (d2 < best) best = d2;
            }
            hv = (best == 0x7fffffff) ? 127 : (isqrt32((uint32_t)best) >> 3);
            if (hv > 127) hv = 127;
            {
                int32_t al = ((nx + 128 - wx[0]) * ndx[0] + (ny + 128 - wy[0]) * ndy[0]) >> 13;
                int32_t la = ((nx + 128 - wx[0]) * (-ndy[0]) + (ny + 128 - wy[0]) * ndx[0]) >> 13;
                if (iabs(al) < 128 && iabs(la) < 520 && hv < 48) flag = 1;
            }
            df[j * 64 + i] = (uint8_t)(hv | (flag << 7));
        }
    }
}

static void build_palettes(void) {
    static const uint16_t base[8] = {RGB(52, 150, 52), RGB(64, 168, 60), RGB(104, 104, 114),
                                     RGB(118, 118, 128), RGB(214, 40, 40), RGB(244, 244, 244),
                                     RGB(250, 250, 250), RGB(20, 20, 20)};
    int r, k, y;
    for (r = 0; r <= RMAX; r++) {
        int a = 0;
        if (r <= FARR)
            a = 256;
        else if (r < 100) {
            a = (256 * (100 - r)) / (100 - FARR);
            a = (a * a) >> 8;
        }
        for (k = 0; k < 8; k++) pal[r][k] = blend(base[k], 196, 226, 244, a);
    }
    for (y = 0; y < HZ; y++) {
        int t = (y * 256) / HZ;
        int rr, gg, bb;
        t = (t * t) >> 8;
        rr = 56 + (((196 - 56) * t) >> 8);
        gg = 124 + (((226 - 124) * t) >> 8);
        bb = 226 + (((244 - 226) * t) >> 8);
        skyc[y] = RGB(rr, gg, bb);
    }
}

static inline int sinp(int k, int x, int ph) { return sint[((k * x * 4) / 5 + ph) & 1023]; }

static void build_hills(void) {
    int x;
    for (x = 0; x < PL; x++) {
        int a = 24 + ((sinp(2, x, 0) * 8) >> 13) + ((sinp(6, x, 300) * 5) >> 13) + ((sinp(14, x, 100) * 3) >> 13);
        int b = 10 + ((sinp(3, x, 50) * 5) >> 13) + ((sinp(9, x, 400) * 3) >> 13) + ((sinp(17, x, 700) * 2) >> 13);
        hillA[x] = (uint8_t)iclamp(a, 2, 44);
        hillB[x] = (uint8_t)iclamp(b, 1, 22);
    }
}

/* ------------------------------------------------------------------ */
/* piste : position le long de la ligne centrale                      */
/* ------------------------------------------------------------------ */
static int seg_of(int32_t s4) {
    int lo = 0, hi = NW - 1;
    while (lo < hi) {
        int mid = (lo + hi + 1) >> 1;
        if (cum[mid] <= s4) lo = mid;
        else hi = mid - 1;
    }
    return lo;
}

/* s4 : abscisse (Q4), lat4 : decalage lateral (Q4 unites, + = droite) */
static int seg_pos(int32_t s4, int lat4, int32_t *x16, int32_t *y16) {
    int i, j;
    int32_t fr, px4, py4;
    s4 %= TL;
    if (s4 < 0) s4 += TL;
    i = seg_of(s4);
    j = (i + 1) % NW;
    fr = ((s4 - cum[i]) * 256) / seglen[i];
    px4 = wx[i] + (((wx[j] - wx[i]) * fr) >> 8);
    py4 = wy[i] + (((wy[j] - wy[i]) * fr) >> 8);
    px4 += ((-ndy[i]) * lat4) >> 13;
    py4 += (ndx[i] * lat4) >> 13;
    *x16 = px4 << 12;
    *y16 = py4 << 12;
    return i;
}

static int df_at(int32_t px16, int32_t py16) {
    uint32_t ci = (uint32_t)(px16 >> 20), cj = (uint32_t)(py16 >> 20);
    int fx, fy, a, b, c, d, top, bot;
    const uint8_t *p;
    if (ci >= 63u || cj >= 63u) return 127;
    fx = (px16 >> 12) & 255;
    fy = (py16 >> 12) & 255;
    p = df + cj * 64 + ci;
    a = p[0] & 127;
    b = p[1] & 127;
    c = p[64] & 127;
    d = p[65] & 127;
    top = a * 256 + (b - a) * fx;
    bot = c * 256 + (d - c) * fx;
    return (top * 256 + (bot - top) * fy) >> 16;
}

/* ------------------------------------------------------------------ */
/* decor                                                              */
/* ------------------------------------------------------------------ */
static void build_scenery(void) {
    int k, side, n = 0, b, row;
    static const int rows[5] = {16, 50, 84, 118, 152};
    rng = 777;
    for (k = 0; k < NW && n < NTREE; k += 3) {
        for (side = -1; side <= 1; side += 2) {
            int lat, tries;
            for (tries = 0; tries < 3; tries++) {
                int32_t x16, y16;
                if ((rnd() & 7) == 0) break;
                lat = side * (46 + (int)(rnd() % 44));
                seg_pos(cum[k] + ((int)(rnd() % 160) - 80), lat * 16, &x16, &y16);
                if (df_at(x16, y16) >= 84 && n < NTREE) {
                    trx[n] = x16;
                    try_[n] = y16;
                    trv[n] = (uint8_t)(rnd() & 1);
                    n++;
                    break;
                }
            }
        }
    }
    ntree = n;
    b = 0;
    for (row = 0; row < 5; row++) {
        for (k = -1; k <= 1; k++) {
            seg_pos(cum[rows[row]], k * 16 * 16, &bxx[b], &bxy[b]);
            b++;
        }
    }
    seg_pos(0, -30 * 16, &archLx, &archLy);
    seg_pos(0, 30 * 16, &archRx, &archRy);
}

/* ------------------------------------------------------------------ */
/* dessin dans le tampon de bande                                     */
/* ------------------------------------------------------------------ */
static void rect(int x, int y, int w, int h, uint16_t c) {
    int x1 = x + w, y1 = y + h, yy, xx;
    uint16_t *p;
    if (x < 0) x = 0;
    if (x1 > SW) x1 = SW;
    if (y < by0) y = by0;
    if (y1 > by0 + BAND) y1 = by0 + BAND;
    if (x >= x1 || y >= y1) return;
    for (yy = y; yy < y1; yy++) {
        p = band + (yy - by0) * SW + x;
        for (xx = x; xx < x1; xx++) *p++ = c;
    }
}

static int gidx(char c) {
    static const char t[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ /:.!-";
    int i;
    for (i = 0; t[i]; i++)
        if (t[i] == c) return i;
    return 36;
}

static int tlen(const char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

static void text(int x, int y, int sc, uint16_t col, const char *s) {
    int row, cx, bit;
    if (y >= by0 + BAND || y + 7 * sc <= by0) return;
    for (; *s; s++) {
        const uint8_t *g = font[gidx(*s)];
        for (row = 0; row < 7; row++) {
            int yy = y + row * sc;
            uint8_t bits = g[row];
            if (!bits || yy >= by0 + BAND || yy + sc <= by0) continue;
            for (bit = 0; bit < 5; bit++) {
                if (bits & (16 >> bit)) {
                    cx = x + bit * sc;
                    rect(cx, yy, sc, sc, col);
                }
            }
        }
        x += 6 * sc;
    }
}

static void text_sh(int x, int y, int sc, uint16_t col, const char *s) {
    int o = (sc + 1) / 2;
    text(x + o, y + o, sc, C_BLACK, s);
    text(x, y, sc, col, s);
}

static void text_center(int y, int sc, uint16_t col, const char *s) {
    text_sh((SW - tlen(s) * 6 * sc + sc) / 2, y, sc, col, s);
}

static int itoa_(char *b, int v) {
    char t[12];
    int n = 0, i, o = 0;
    if (v < 0) v = 0;
    if (v == 0) t[n++] = '0';
    while (v > 0) {
        t[n++] = (char)('0' + v % 10);
        v /= 10;
    }
    for (i = n - 1; i >= 0; i--) b[o++] = t[i];
    b[o] = 0;
    return o;
}

static void fmt_time(char *b, int ms) {
    int m = ms / 60000, s = (ms / 1000) % 60, t = (ms / 100) % 10, o;
    o = itoa_(b, m);
    b[o++] = ':';
    b[o++] = (char)('0' + s / 10);
    b[o++] = (char)('0' + s % 10);
    b[o++] = '.';
    b[o++] = (char)('0' + t);
    b[o] = 0;
}

#define U(n) (((n) * S) >> 8)

static void kart_sprite(int cx, int gy, int S, uint16_t col, uint16_t helm, int lean, int fl) {
    int sh = lean * U(1) / 1;
    int sh2 = sh / 2;
    uint16_t pipe = C_DARK;
    rect(cx - U(10), gy - U(1), U(20), U(2), C_SHADOW);
    rect(cx - U(9), gy - U(8), U(4), U(8), C_BLACK);
    rect(cx + U(5), gy - U(8), U(4), U(8), C_BLACK);
    rect(cx - U(8), gy - U(6), U(2), U(3), RGB(150, 150, 150));
    rect(cx + U(6), gy - U(6), U(2), U(3), RGB(150, 150, 150));
    rect(cx - U(7), gy - U(9), U(14), U(6), col);
    if (fl & 1) pipe = (frameCount & 2) ? C_YEL : C_ORA;
    rect(cx - U(5), gy - U(6), U(3), U(3), pipe);
    rect(cx + U(2), gy - U(6), U(3), U(3), pipe);
    if (fl & 1) {
        rect(cx - U(5) + sh2, gy - U(2), U(3), U(3), C_ORA);
        rect(cx + U(2) + sh2, gy - U(2), U(3), U(3), C_ORA);
    }
    rect(cx - U(6) + sh2, gy - U(13), U(12), U(5), col);
    rect(cx - U(8) + sh2, gy - U(15), U(16), U(2), C_DARK);
    rect(cx - U(3) + sh, gy - U(19), U(6), U(5), col);
    rect(cx - U(3) + sh, gy - U(24), U(6), U(5), C_SKIN);
    rect(cx - U(4) + sh, gy - U(27), U(8), U(4), helm);
    rect(cx - U(4) + sh, gy - U(24), U(1), U(3), helm);
    rect(cx + U(3) + sh, gy - U(24), U(1), U(3), helm);
    if (fl & 2) {
        uint16_t sc = (fl & 4) ? ((fl & 8) ? C_ORA : C_BLU) : C_WHITE;
        rect(cx - U(12), gy - U(4), U(3), U(2), sc);
        rect(cx + U(9), gy - U(4), U(3), U(2), sc);
        rect(cx - U(14), gy - U(7), U(2), U(2), sc);
        rect(cx + U(12), gy - U(7), U(2), U(2), sc);
    }
}

static void draw_sprite(int k) {
    int x = spx[k], gy = spy[k], S = spS[k];
    switch (spt[k]) {
    case SP_PINE: {
        int tw = imax(1, U(3)), th = U(7), h1 = U(9);
        rect(x - tw / 2, gy - th, tw, th, C_BRN);
        rect(x - U(9), gy - th - h1, U(18), h1, C_FOL);
        rect(x - U(7), gy - th - 2 * h1 + U(2), U(14), h1, C_FOL);
        rect(x - U(5), gy - th - 3 * h1 + U(4), U(10), h1, C_FOL2);
        break;
    }
    case SP_ROUND: {
        int tw = imax(1, U(3)), th = U(8);
        rect(x - tw / 2, gy - th, tw, th, C_BRN);
        rect(x - U(9), gy - th - U(16), U(18), U(16), C_FOL);
        rect(x - U(6), gy - th - U(21), U(12), U(6), C_FOL2);
        break;
    }
    case SP_BOX: {
        uint16_t c;
        int ph = (frameCount >> 2) % 3;
        c = ph == 0 ? C_RED : (ph == 1 ? C_YEL : C_BLU);
        rect(x - U(6), gy - U(16), U(12), U(12), C_DARK);
        rect(x - U(5), gy - U(15), U(10), U(10), c);
        rect(x - U(1), gy - U(13), U(3), U(5), C_WHITE);
        rect(x - U(1), gy - U(7), U(3), U(2), C_WHITE);
        break;
    }
    case SP_BAN:
        rect(x - U(5), gy - U(4), U(10), U(4), C_YEL);
        rect(x - U(6), gy - U(7), U(4), U(4), C_YEL);
        rect(x + U(2), gy - U(7), U(4), U(4), C_YEL);
        rect(x - U(6), gy - U(8), U(2), U(2), C_BRN);
        rect(x + U(4), gy - U(8), U(2), U(2), C_BRN);
        break;
    case SP_SHELL:
        rect(x - U(5), gy - U(8), U(10), U(7), RGB(30, 170, 60));
        rect(x - U(5), gy - U(3), U(10), U(3), C_WHITE);
        rect(x - U(3), gy - U(7), U(3), U(2), RGB(150, 240, 160));
        break;
    case SP_KART:
        kart_sprite(x, gy, S, (uint16_t)spa[k], (uint16_t)spb[k], spc[k], spd[k]);
        break;
    case SP_ARCH: {
        int xr = spa[k], gyr = spb[k], Sr = spc[k], xx, n, hl, hr, x0, x1;
        int tl = gy - ((50 * S) >> 8), tr_ = gyr - ((50 * Sr) >> 8);
        int th = imax(3, U(7));
        rect(x - imax(1, U(2)), tl, imax(2, U(4)), gy - tl, C_WHITE);
        rect(xr - imax(1, (2 * Sr) >> 8), tr_, imax(2, (4 * Sr) >> 8), gyr - tr_, C_WHITE);
        x0 = x;
        x1 = xr;
        if (x0 > x1) {
            int t = x0;
            x0 = x1;
            x1 = t;
        }
        if (x1 - x0 > 0 && x1 - x0 < 1200) {
            n = 0;
            for (xx = x0; xx < x1; xx += 3) {
                int yy;
                hl = tl;
                hr = tr_;
                yy = (x1 == x0) ? hl : hl + (((hr - hl) * ((x == x0) ? (xx - x0) : (x1 - xx))) / (x1 - x0));
                rect(xx, yy, 3, th, (n & 1) ? C_BLACK : C_WHITE);
                rect(xx, yy + th, 3, th, (n & 1) ? C_WHITE : C_BLACK);
                n++;
            }
        }
        break;
    }
    }
}

/* ------------------------------------------------------------------ */
/* projection des objets                                              */
/* ------------------------------------------------------------------ */
static int project(int32_t x16, int32_t y16, int *sx, int *gy, int *S, int *fout) {
    int32_t rx4 = (x16 - camx16) >> 12, ry4 = (y16 - camy16) >> 12;
    int32_t f4 = (rx4 * cdx + ry4 * cdy) >> 13;
    int32_t rt4;
    if (f4 < 26 * 16 || f4 > FARU * 16) return 0;
    rt4 = (-rx4 * cdy + ry4 * cdx) >> 13;
    if (iabs(rt4) > f4 + 700) return 0;
    *S = (FOC * 4096) / f4;
    *sx = 160 + ((rt4 * *S) >> 12);
    *gy = HZ + ((CH * *S) >> 8);
    *fout = f4;
    return 1;
}

static void add_sprite(int type, int32_t x16, int32_t y16, int a, int b, int c, int d) {
    int sx, gy, S, f;
    if (nspr >= MAXSPR) return;
    if (!project(x16, y16, &sx, &gy, &S, &f)) return;
    spt[nspr] = type;
    spx[nspr] = sx;
    spy[nspr] = gy;
    spS[nspr] = S;
    spf[nspr] = f;
    spa[nspr] = a;
    spb[nspr] = b;
    spc[nspr] = c;
    spd[nspr] = d;
    nspr++;
}

static void build_sprites(void) {
    int i, j, k, sx, gy, S, f;
    nspr = 0;
    for (i = 0; i < ntree; i++) add_sprite(trv[i] ? SP_ROUND : SP_PINE, trx[i], try_[i], 0, 0, 0, 0);
    for (i = 0; i < NBOX; i++)
        if (bxcd[i] <= 0) add_sprite(SP_BOX, bxx[i], bxy[i], 0, 0, 0, 0);
    for (i = 0; i < NBAN; i++)
        if (banon[i]) add_sprite(SP_BAN, banx[i], bany[i], 0, 0, 0, 0);
    if (shlife > 0) add_sprite(SP_SHELL, shx8 << 8, shy8 << 8, 0, 0, 0, 0);
    for (i = 0; i < NRIV; i++) {
        int fl = 0;
        if (rstun[i] > 0) fl = 2 | 4 | 8;
        add_sprite(SP_KART, rpx[i], rpy[i], rcol[i], rhelm[i], rstun[i] > 0 ? ((frameCount & 4) ? 4 : -4) : rlean[i], fl);
    }
    {
        int xr, gyr, Sr, fr_;
        if (project(archLx, archLy, &sx, &gy, &S, &f) && project(archRx, archRy, &xr, &gyr, &Sr, &fr_)) {
            if (nspr < MAXSPR) {
                spt[nspr] = SP_ARCH;
                spx[nspr] = sx;
                spy[nspr] = gy;
                spS[nspr] = S;
                spf[nspr] = imin(f, fr_);
                spa[nspr] = xr;
                spb[nspr] = gyr;
                spc[nspr] = Sr;
                spd[nspr] = 0;
                nspr++;
            }
        }
    }
    for (i = 0; i < nspr; i++) order[i] = i;
    for (i = 1; i < nspr; i++) {
        k = order[i];
        j = i - 1;
        while (j >= 0 && spf[order[j]] < spf[k]) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = k;
    }
}

/* ------------------------------------------------------------------ */
/* rendu                                                              */
/* ------------------------------------------------------------------ */
static void floor_row(uint16_t *dst, int r) {
    int32_t f4 = (CHF * 16) / r;
    int32_t st = (CH << 12) / r;
    int32_t ix = ((-cdy) * st) >> 9, iy = (cdx * st) >> 9;
    int32_t px = camx16 + ((cdx * f4) >> 1) - ix * 160;
    int32_t py = camy16 + ((cdy * f4) >> 1) - iy * 160;
    const uint16_t *pl = pal[r];
    int x;
    ix *= 2;
    iy *= 2;
    for (x = 0; x < SW; x += 2) {
        uint32_t ci = (uint32_t)(px >> 20), cj = (uint32_t)(py >> 20);
        uint16_t c;
        if (ci >= 63u || cj >= 63u) {
            c = pl[((px >> 20) ^ (py >> 20)) & 1];
        } else {
            int fx = (px >> 12) & 255, fy = (py >> 12) & 255, v;
            const uint8_t *p = df + cj * 64 + ci;
            int a = p[0] & 127, b = p[1] & 127, cc = p[64] & 127, d = p[65] & 127;
            int top = a * 256 + (b - a) * fx, bot = cc * 256 + (d - cc) * fx;
            v = (top * 256 + (bot - top) * fy) >> 16;
            if (v < 48) {
                if (p[0] & 128) c = pl[6 + (((px >> 18) ^ (py >> 18)) & 1)];
                else c = pl[2 + (((px >> 19) ^ (py >> 19)) & 1)];
            } else if (v < 62) {
                c = pl[4 + (((px >> 19) ^ (py >> 19)) & 1)];
            } else {
                c = pl[((px >> 20) ^ (py >> 20)) & 1];
            }
        }
        dst[x] = c;
        dst[x + 1] = c;
        px += ix;
        py += iy;
    }
}

static void icon(int x, int y, int it) {
    /* cadre 36x36 */
    rect(x, y, 36, 36, C_DARK);
    rect(x + 2, y + 2, 32, 32, RGB(235, 235, 245));
    if (it == 1) { /* champignon */
        rect(x + 8, y + 7, 20, 6, C_RED);
        rect(x + 5, y + 13, 26, 8, C_RED);
        rect(x + 10, y + 9, 5, 5, C_WHITE);
        rect(x + 21, y + 9, 5, 5, C_WHITE);
        rect(x + 15, y + 15, 6, 4, C_WHITE);
        rect(x + 11, y + 21, 14, 9, RGB(250, 225, 180));
        rect(x + 13, y + 24, 3, 3, C_DARK);
        rect(x + 20, y + 24, 3, 3, C_DARK);
    } else if (it == 2) { /* banane */
        rect(x + 8, y + 22, 20, 6, C_YEL);
        rect(x + 6, y + 14, 6, 10, C_YEL);
        rect(x + 24, y + 14, 6, 10, C_YEL);
        rect(x + 5, y + 9, 4, 6, C_BRN);
        rect(x + 27, y + 9, 4, 6, C_BRN);
    } else if (it == 3) { /* carapace */
        rect(x + 7, y + 8, 22, 15, RGB(30, 170, 60));
        rect(x + 9, y + 6, 18, 3, RGB(30, 170, 60));
        rect(x + 5, y + 23, 26, 6, C_WHITE);
        rect(x + 12, y + 11, 6, 4, RGB(150, 240, 160));
        rect(x + 18, y + 16, 7, 4, RGB(20, 120, 40));
    }
}

static void draw_hud(void) {
    int i, rank;
    char b[24];
    int o;
    if (state == ST_TITLE) return;
    /* rang */
    rank = 1;
    for (i = 0; i < NRIV; i++)
        if (rs[i] > ptot4) rank++;
    b[0] = (char)('0' + rank);
    b[1] = 0;
    text_sh(8, 6, 5, C_YEL, b);
    text_sh(8 + 34, 22, 2, C_WHITE, "/7");
    /* objet */
    rect(139, 3, 42, 42, C_DARK);
    {
        int show = item;
        if (itemRoll > 0) show = 1 + ((frameCount >> 1) % 3);
        icon(142, 6, show);
    }
    /* tour et temps */
    b[0] = 'T';
    b[1] = 'O';
    b[2] = 'U';
    b[3] = 'R';
    b[4] = ' ';
    b[5] = (char)('0' + iclamp(lc + 1, 1, LAPS));
    b[6] = '/';
    b[7] = (char)('0' + LAPS);
    b[8] = 0;
    text_sh(8, 196, 2, C_WHITE, b);
    fmt_time(b, raceT);
    text_sh(8, 214, 2, C_WHITE, b);
    /* vitesse */
    o = itoa_(b, (v4 >> 4) * 2);
    text_sh(SW - 8 - o * 18 - 38, 204, 3, C_WHITE, b);
    text_sh(SW - 8 - 38, 212, 2, C_YEL, "KMH");
    /* mini-carte */
    rect(252, 3, 66, 66, C_DARK);
    rect(254, 5, 62, 62, RGB(60, 120, 70));
    for (i = 0; i < NW; i += 2) rect(255 + mmx[i], 6 + mmy[i], 4, 4, RGB(200, 200, 212));
    rect(255 + mmx[0] - 1, 6 + mmy[0] - 1, 6, 6, C_WHITE);
    for (i = 0; i < NRIV; i++) {
        int mx = (int)(rpx[i] >> 16) * 15 >> 8, my = (int)(rpy[i] >> 16) * 15 >> 8;
        rect(255 + mx - 1, 6 + my - 1, 6, 6, rcol[i]);
    }
    {
        int mx = (kx8 >> 8) * 15 >> 8, my = (ky8 >> 8) * 15 >> 8;
        rect(255 + mx - 2, 6 + my - 2, 8, 8, C_BLACK);
        rect(255 + mx - 1, 6 + my - 1, 6, 6, C_RED);
    }
    /* message */
    if (msgT > 0 && msg[0]) text_center(70, 3, C_WHITE, msg);
}

static void draw_overlay(void) {
    char b[24];
    if (state == ST_TITLE) {
        text_center(30, 5, C_YEL, "MINI KART");
        if ((frameCount >> 4) & 1) text_center(110, 2, C_WHITE, "APPUIE SUR OK");
        text_center(150, 2, C_WHITE, "FLECHES: CONDUIRE");
        text_center(168, 2, C_WHITE, "SHIFT: DERAPER");
        text_center(186, 2, C_WHITE, "OK: UTILISER OBJET");
        text_center(212, 2, C_YEL, "BACK: QUITTER");
    } else if (state == ST_COUNT) {
        int n = (cdT + 999) / 1000;
        if (cdT > 0 && n >= 1 && n <= 3) {
            b[0] = (char)('0' + n);
            b[1] = 0;
            text_center(70, 9, C_YEL, b);
        }
    } else if (state == ST_FINISH) {
        int w;
        rect(40, 60, 240, 130, C_DARK);
        rect(44, 64, 232, 122, RGB(250, 250, 255));
        text(70, 72, 3, C_RED, "ARRIVEE!");
        b[0] = (char)('0' + finalRank);
        b[1] = 0;
        text(60, 100, 6, C_DARK, b);
        text(100, 116, 3, C_DARK, "/7");
        text(60, 146, 2, C_DARK, "TEMPS");
        fmt_time(b, finalMs);
        text(140, 146, 2, C_DARK, b);
        text(60, 164, 2, C_DARK, "MEILLEUR");
        fmt_time(b, bestLap);
        text(172, 164, 2, C_DARK, b);
        w = (frameCount >> 4) & 1;
        if (w) text(120, 100, 2, C_RED, "OK: REJOUER");
    }
}

static void render_band(int y0) {
    int y, i;
    eadk_rect_t rc;
    by0 = y0;
    for (y = y0; y < y0 + BAND; y++) {
        uint16_t *row = band + (y - y0) * SW;
        if (y < HZ) {
            uint16_t sk = skyc[y];
            int x;
            for (x = 0; x < SW; x++) row[x] = (y >= topB[x]) ? C_HILLB : ((y >= topA[x]) ? C_HILLA : sk);
        } else {
            int r = y - HZ, x;
            if (r < FARR) {
                uint16_t c = pal[0][0];
                for (x = 0; x < SW; x++) row[x] = c;
            } else {
                floor_row(row, r);
            }
        }
    }
    for (i = 0; i < nspr; i++) {
        int k = order[i];
        if (spy[k] - 400 > y0 + BAND) continue;
        draw_sprite(k);
    }
    /* joueur */
    if (state != ST_TITLE) {
        int fl = 0, cx = 160 + camShift;
        if (boostT > 0) fl |= 1;
        if (drifting) {
            fl |= 2;
            if (driftT > 700) fl |= 4;
            if (driftT > 1500) fl |= 8;
        }
        kart_sprite(cx, 238, 760, C_RED, RGB(235, 30, 30), leanv, fl);
    }
    draw_hud();
    draw_overlay();
    rc.x = 0;
    rc.y = (uint16_t)y0;
    rc.width = SW;
    rc.height = BAND;
    eadk_display_push_rect(rc, band);
}

static void render_frame(void) {
    int x, y0;
    int offB = (int)((camAng * 1280) >> 16), offA = offB >> 1;
    for (x = 0; x < SW; x++) {
        topB[x] = (uint8_t)(HZ - hillB[(offB + x) % PL]);
        topA[x] = (uint8_t)(HZ - hillA[(offA + x) % PL]);
    }
    cdx = cosq(camAng);
    cdy = sinq(camAng);
    camx16 = (kx8 << 8) - cdx * BACKD * 8;
    camy16 = (ky8 << 8) - cdy * BACKD * 8;
    build_sprites();
    for (y0 = 0; y0 < SHGT; y0 += BAND) render_band(y0);
}

/* ------------------------------------------------------------------ */
/* jeu                                                                */
/* ------------------------------------------------------------------ */
static void set_msg(const char *s, int ms) {
    int i = 0;
    while (s[i] && i < 22) {
        msg[i] = s[i];
        i++;
    }
    msg[i] = 0;
    msgT = ms;
}

static int angle_of(int dx, int dy) {
    int a, best = 0, bd = -0x7fffffff;
    for (a = 0; a < 1024; a++) {
        int d = sint[(a + 256) & 1023] * dx + sint[a] * dy;
        if (d > bd) {
            bd = d;
            best = a;
        }
    }
    return best;
}

static void reset_race(void) {
    static const int ridx[NRIV] = {178, 178, 175, 175, 172, 172};
    static const int rlat0[NRIV] = {-10, 10, 10, -10, -10, 10};
    int i;
    int32_t x16, y16;
    seg_pos(cum[168], 0, &x16, &y16);
    kx8 = x16 >> 8;
    ky8 = y16 >> 8;
    hd = (int32_t)angle_of(ndx[168], ndy[168]) << 6;
    vd = hd;
    camAng = hd;
    v4 = 0;
    stq = 0;
    boostT = spinT = stallT = drifting = driftDir = driftT = colCD = 0;
    item = itemRoll = itemFinal = 0;
    pidx = 168;
    lc = -1;
    leanv = 0;
    camShift = 0;
    for (i = 0; i < NRIV; i++) {
        rs[i] = cum[ridx[i]] - TL;
        rlat[i] = rlat0[i] * 16;
        rlt[i] = rlat[i];
        rtt[i] = 1500 + (int)(rnd() % 2000);
        rvq4[i] = 0;
        rstun[i] = 0;
        rvb[i] = 84 + (int)(rnd() % 12);
        seg_pos(rs[i], rlat[i], &rpx[i], &rpy[i]);
        rlean[i] = 0;
    }
    for (i = 0; i < NBOX; i++) bxcd[i] = 0;
    for (i = 0; i < NBAN; i++) banon[i] = 0;
    shlife = 0;
    raceT = 0;
    lapStart = 0;
    bestLap = 99 * 60000;
    msgT = 0;
    msg[0] = 0;
    ptot4 = -TL + cum[168];
}

static void start_countdown(void) {
    reset_race();
    state = ST_COUNT;
    cdT = 3400;
    startBoost = 0;
    stallOnce = 0;
}

static void get_input(uint32_t dummy);

static void fire_item(void) {
    int i;
    if (item == 1) {
        boostT = imax(boostT, 1600);
        set_msg("TURBO!", 1200);
    } else if (item == 2) {
        for (i = 0; i < NBAN; i++)
            if (!banon[i]) break;
        if (i == NBAN) i = 0;
        banx[i] = (kx8 << 8) - cosq(hd) * 14 * 8;
        bany[i] = (ky8 << 8) - sinq(hd) * 14 * 8;
        banon[i] = 1;
        set_msg("BANANE!", 900);
    } else if (item == 3) {
        shx8 = kx8 + (cosq(hd) * 14 >> 5);
        shy8 = ky8 + (sinq(hd) * 14 >> 5);
        shc = cosq(hd);
        shs = sinq(hd);
        shlife = 2400;
        set_msg("CARAPACE!", 900);
    }
    item = 0;
}

static int pick_item(int rank) {
    int r = (int)(rnd() % 100);
    if (rank <= 2) return r < 50 ? 2 : (r < 85 ? 3 : 1);
    if (rank <= 4) return r < 30 ? 2 : (r < 65 ? 3 : 1);
    return r < 15 ? 2 : (r < 45 ? 3 : 1);
}

static void update_player(int dt) {
    int up = in_up, down = in_down, s = in_right - in_left;
    int vkm = v4 >> 4, dh, offroad, vcap, rate, ds, grip;
    int32_t c, sn;
    if (state == ST_FINISH) {
        up = 0;
        down = 1;
        s = 0;
    }
    if (stallT > 0) {
        stallT -= dt;
        up = 0;
    }
    dh = df_at(kx8 << 8, ky8 << 8);
    offroad = dh > 64;
    vcap = 100 * 16;
    if (boostT > 0) vcap = 138 * 16;
    else if (offroad) vcap = 42 * 16;
    if (spinT > 0) vcap = 28 * 16;
    if (up) v4 += (75 * 16 * dt) / 1000;
    else if (down) v4 -= (170 * 16 * dt) / 1000;
    else v4 -= (22 * 16 * dt) / 1000;
    if (boostT > 0) {
        boostT -= dt;
        v4 += (200 * 16 * dt) / 1000;
    }
    if (v4 > vcap) {
        v4 -= (170 * 16 * dt) / 1000;
        if (v4 < vcap) v4 = vcap;
    }
    if (v4 < 0) v4 = 0;
    vkm = v4 >> 4;
    /* derapage */
    if (in_drift && s != 0 && vkm > 55 && !drifting && spinT <= 0 && state == ST_RACE) {
        drifting = 1;
        driftDir = s;
        driftT = 0;
    }
    if (drifting) {
        if (!in_drift || vkm < 40 || spinT > 0) {
            if (driftT > 2400) boostT = imax(boostT, 1700), set_msg("SUPER TURBO!", 900);
            else if (driftT > 1500) boostT = imax(boostT, 1100), set_msg("TURBO!", 800);
            else if (driftT > 700) boostT = imax(boostT, 600), set_msg("MINI TURBO", 700);
            drifting = 0;
            driftT = 0;
        } else {
            driftT += (s == driftDir) ? dt : dt / 2;
            v4 -= (10 * 16 * dt) / 1000;
        }
    }
    stq += ((s * 256 - stq) * imin(dt * 10, 1000)) / 1000;
    rate = 4000 + vkm * 70;
    if (spinT > 0) {
        spinT -= dt;
        ds = (frameCount & 8) ? 256 : -256;
        rate = 9000;
        hd += (ds * rate / 256 * dt) / 1000;
    } else if (drifting) {
        ds = driftDir * 160 + s * 96;
        rate = rate * 13 / 10;
        hd += (ds * rate / 256 * dt) / 1000;
    } else {
        hd += (stq * rate / 256 * dt) / 1000;
    }
    hd &= 65535;
    grip = drifting ? 18 : 140;
    vd += (angdiff(hd, vd) * (grip * dt)) / 10000;
    vd &= 65535;
    c = cosq(vd);
    sn = sinq(vd);
    kx8 += (v4 * c * dt) / 512000;
    ky8 += (v4 * sn * dt) / 512000;
    kx8 = iclamp(kx8, 24 * 256, 1000 * 256);
    ky8 = iclamp(ky8, 24 * 256, 1000 * 256);
    /* camera */
    {
        int k = drifting ? 40 : 85;
        camAng += (angdiff(hd, camAng) * (k * dt)) / 10000;
        camAng &= 65535;
    }
    camShift = iclamp((angdiff(hd, camAng) * 1280) >> 16, -60, 60);
    {
        int tl = (stq * 3) >> 8;
        if (drifting) tl += driftDir * 2;
        if (spinT > 0) tl = (frameCount & 4) ? 4 : -4;
        leanv = iclamp(tl, -4, 4);
    }
}

static void track_player(void) {
    int k, best = pidx, prev = pidx;
    int32_t bd = 0x7fffffff, kx4 = kx8 >> 4, ky4 = ky8 >> 4, pr;
    for (k = -3; k <= 8; k++) {
        int i = (pidx + k + NW) % NW;
        int32_t dx = kx4 - wx[i], dy = ky4 - wy[i], d = dx * dx + dy * dy;
        if (d < bd) {
            bd = d;
            best = i;
        }
    }
    if (prev > NW - 25 && best < 25) {
        lc++;
        if (lc >= 0 && lc < LAPS) {
            int lt = raceT - lapStart;
            if (lc >= 1 && lt < bestLap) bestLap = lt;
            lapStart = raceT;
            if (lc >= 1) {
                char t[8];
                t[0] = 'T';
                t[1] = 'O';
                t[2] = 'U';
                t[3] = 'R';
                t[4] = ' ';
                t[5] = (char)('0' + lc + 1);
                t[6] = 0;
                set_msg(t, 1800);
            }
        }
    } else if (prev < 25 && best > NW - 25) {
        lc--;
    }
    pidx = best;
    pr = ((kx4 - wx[best]) * ndx[best] + (ky4 - wy[best]) * ndy[best]) >> 13;
    pr = iclamp(pr, -seglen[(best + NW - 1) % NW], seglen[best]);
    ptot4 = lc * TL + cum[best] + pr;
}

static void update_rivals(int dt) {
    int i, j;
    for (i = 0; i < NRIV; i++) {
        int seg = seg_of(((rs[i] % TL) + TL) % TL);
        int cv = curv[seg], slowf = (iabs(cv) * 90) >> 7;
        int vt = rvb[i] * (256 - slowf) / 256, rub;
        int32_t gap = ptot4 - rs[i];
        rub = iclamp(gap / 64, -10, 14);
        vt += rub;
        if (rstun[i] > 0) {
            rstun[i] -= dt;
            vt /= 4;
        }
        rvq4[i] += ((vt * 16 - rvq4[i]) * imin(dt * 3, 1000)) / 1000;
        rs[i] += (rvq4[i] * dt) / 1000;
        rtt[i] -= dt;
        if (rtt[i] < 0) {
            rtt[i] = 1500 + (int)(rnd() % 2500);
            rlt[i] = ((int)(rnd() % 5) - 2) * 7 * 16;
        }
        rlat[i] += ((rlt[i] - rlat[i]) * imin(dt, 1000)) / 1000 / 1;
        rlat[i] += ((rlt[i] - rlat[i]) * imin(dt, 1000)) / 1500;
        rlean[i] = iclamp(cv >> 5, -4, 4);
        seg_pos(rs[i], rlat[i], &rpx[i], &rpy[i]);
        for (j = 0; j < NBAN; j++) {
            if (banon[j]) {
                int32_t dx = (banx[j] - rpx[i]) >> 12, dy = (bany[j] - rpy[i]) >> 12;
                if (dx * dx + dy * dy < 144 * 144 / 4) {
                    banon[j] = 0;
                    rstun[i] = 1500;
                    rvq4[i] /= 3;
                }
            }
        }
    }
}

static void update_items(int dt) {
    int i, rank;
    for (i = 0; i < NBOX; i++) {
        if (bxcd[i] > 0) {
            bxcd[i] -= dt;
            continue;
        }
        {
            int32_t dx = ((kx8 << 8) - bxx[i]) >> 12, dy = ((ky8 << 8) - bxy[i]) >> 12;
            if (dx * dx + dy * dy < 170 * 170 / 4 * 4 && item == 0 && itemRoll <= 0) {
                bxcd[i] = 4000;
                rank = 1;
                {
                    int j;
                    for (j = 0; j < NRIV; j++)
                        if (rs[j] > ptot4) rank++;
                }
                itemFinal = pick_item(rank);
                itemRoll = 1100;
            }
        }
    }
    if (itemRoll > 0) {
        itemRoll -= dt;
        if (itemRoll <= 0) item = itemFinal;
    }
    for (i = 0; i < NBAN; i++) {
        if (banon[i]) {
            int32_t dx = ((kx8 << 8) - banx[i]) >> 12, dy = ((ky8 << 8) - bany[i]) >> 12;
            if (dx * dx + dy * dy < 150 * 150 / 4 && spinT <= 0) {
                banon[i] = 0;
                spinT = 1000;
                v4 /= 2;
                drifting = 0;
                set_msg("AIE!", 900);
            }
        }
    }
    if (shlife > 0) {
        shlife -= dt;
        shx8 += (shc * 190 * dt) / 32000;
        shy8 += (shs * 190 * dt) / 32000;
        for (i = 0; i < NRIV; i++) {
            int32_t dx = (shx8 >> 4) - (rpx[i] >> 12), dy = (shy8 >> 4) - (rpy[i] >> 12);
            if (dx * dx + dy * dy < 170 * 170 / 4) {
                rstun[i] = 2200;
                rvq4[i] /= 4;
                shlife = 0;
                set_msg("TOUCHE!", 900);
            }
        }
    }
    /* collisions joueur - rivaux */
    if (colCD > 0) colCD -= dt;
    for (i = 0; i < NRIV; i++) {
        int32_t dx = ((kx8 << 8) - rpx[i]) >> 12, dy = ((ky8 << 8) - rpy[i]) >> 12;
        int32_t d2 = dx * dx + dy * dy;
        if (d2 < 30976) {
            kx8 += dx / 4;
            ky8 += dy / 4;
            if (colCD <= 0) {
                v4 = v4 * 7 / 8;
                rvq4[i] = rvq4[i] * 7 / 8;
                colCD = 250;
            }
        }
    }
}

static void get_input(uint32_t dummy) {
    eadk_keyboard_state_t ks = eadk_keyboard_scan();
    int ok;
    (void)dummy;
    in_up = eadk_keyboard_key_down(ks, eadk_key_up);
    in_down = eadk_keyboard_key_down(ks, eadk_key_down);
    in_left = eadk_keyboard_key_down(ks, eadk_key_left);
    in_right = eadk_keyboard_key_down(ks, eadk_key_right);
    in_drift = eadk_keyboard_key_down(ks, eadk_key_shift);
    ok = eadk_keyboard_key_down(ks, eadk_key_ok) || eadk_keyboard_key_down(ks, eadk_key_exe);
    in_ok = ok && !okprev;
    okprev = ok;
#ifdef AUTOPILOT
    {
        int32_t tx = wx[(pidx + 5) % NW], ty = wy[(pidx + 5) % NW];
        int32_t hx = cosq(hd), hy = sinq(hd);
        int32_t vx = tx - (kx8 >> 4), vy = ty - (ky8 >> 4);
        int32_t cr = (hx * vy - hy * vx) >> 8;
        in_up = 1;
        in_down = 0;
        in_left = cr < -300;
        in_right = cr > 300;
        in_drift = (iabs(cr) > 2500) && (v4 > 70 * 16);
        in_ok = (state == ST_TITLE && frameCount > 3) || (item > 0 && itemRoll <= 0 && (frameCount & 7) == 0);
    }
#endif
    if (eadk_keyboard_key_down(ks, eadk_key_back)) {
        in_ok = -1;
    }
}

int main(int argc, char *argv[]) {
    int quit = 0;
    (void)argc;
    (void)argv;
    init_sin();
    build_track();
    build_df();
    build_palettes();
    build_hills();
    build_scenery();
    {
        int i;
        for (i = 0; i < NW; i++) {
            mmx[i] = (int16_t)((((wx[i] >> 4) * 15) >> 8) - 2);
            mmy[i] = (int16_t)((((wy[i] >> 4) * 15) >> 8) - 2);
        }
    }
    rng = 4242;
    reset_race();
    state = ST_TITLE;
    okprev = 1;
    lastms = (uint32_t)eadk_timing_millis();
    frameCount = 0;
#ifdef TEST
    {
        FILE *f = fopen("/tmp/kart_c/df.pgm", "wb");
        int i;
        fprintf(f, "P5\n64 64\n255\n");
        for (i = 0; i < 4096; i++) fputc((df[i] & 127) * 2, f);
        fclose(f);
        fprintf(stderr, "TL=%d (units %d) NTREE=%d\n", (int)TL, (int)(TL / 16), ntree);
    }
#endif
    while (!quit) {
        uint32_t now = (uint32_t)eadk_timing_millis();
        int dt = (int)(now - lastms);
        lastms = now;
        dt = iclamp(dt, 1, 60);
        frameCount++;
        get_input(now);
        if (in_ok == -1) break;
        if (msgT > 0) msgT -= dt;
        switch (state) {
        case ST_TITLE:
            camAng = (camAng + dt * 6) & 65535;
            if (in_ok) {
                start_countdown();
                TLOG("countdown", 0, 0);
            }
            break;
        case ST_COUNT:
            cdT -= dt;
            if (in_up) {
                if (cdT > 700 && !stallOnce) {
                    stallOnce = 1;
                    stallT = 1500;
                    set_msg("TROP TOT!", 1200);
                } else if (cdT <= 700 && !stallOnce) {
                    startBoost = 1;
                }
            }
            camAng += (angdiff(hd, camAng) * imin(dt * 8, 1000)) / 1000;
            camAng &= 65535;
            camShift = 0;
            if (cdT <= 0) {
                state = ST_RACE;
                set_msg("GO!", 1000);
                if (startBoost) {
                    boostT = 1000;
                    set_msg("DEPART TURBO!", 1200);
                }
                TLOG("go", 0, 0);
            }
            break;
        case ST_RACE:
            raceT += dt;
            if (in_ok > 0 && item > 0 && itemRoll <= 0) fire_item();
            update_player(dt);
            update_rivals(dt);
            update_items(dt);
            track_player();
            if (lc >= LAPS) {
                int i;
                state = ST_FINISH;
                finalMs = raceT;
                finalRank = 1;
                for (i = 0; i < NRIV; i++)
                    if (rs[i] > ptot4) finalRank++;
                if (raceT - lapStart < bestLap) bestLap = raceT - lapStart;
                set_msg("", 0);
                TLOG("finish rank/time", finalRank, finalMs);
                TLOG("best lap", bestLap, 0);
            }
            break;
        case ST_FINISH:
            update_player(dt);
            update_rivals(dt);
            update_items(dt);
            if (in_ok > 0) {
                start_countdown();
            }
            break;
        }
#ifdef TEST
        if ((frameCount % 100) == 0 && state == ST_RACE)
            TLOG("race lc/pidx", lc, pidx);
        if (state == ST_FINISH && frameCount > 0) {
            static int fin_frames;
            fin_frames++;
            if (fin_frames > 60) return 0;
        }
#endif
        render_frame();
    }
    return 0;
}
