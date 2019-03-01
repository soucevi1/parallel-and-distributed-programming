int eval_poi(int number) {
    // vraci maximalni cenu pro "number" nevyresenych policek
    // returns the maximal price for the "number" of unsolved squares
    int i, x;
    int max = gl_c2 * (number / gl_i2);
    int zb = number % gl_i2;
    max += gl_c1 * (zb / gl_i1);
    zb = zb % gl_i1;
    max += zb * gl_cn;
    for (i = 0; i < (number / gl_i2); i++) {
        x = gl_c2 * i;
        zb = number - i * gl_i2;
        x += gl_c1 * (zb / gl_i1);
        zb = zb % gl_i1;
        x += zb * gl_cn;
        if (x > max) max = x;
    }
    return max;
}