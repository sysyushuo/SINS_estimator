#include "lib/vector.h"
#include "lib/basis.h"
#include <stdio.h>
#include <math.h>
#include "lib/SINS.h"
#include "lib/filereader_sins.h"
#include "lib/progbar.h"
#include "lib/params.h"

using namespace Vectors;
using namespace std;

double degtorad(double deg) {
    return deg / 180 * M_PI;
}

double radtodeg(double rad) {
    return rad / M_PI * 180;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Wong amount of arguments! Exiting...\n");
        return -1;
    }
    FILE *infile;
    FILE *outfile;
    infile  = fopen(argv[1], "r");
    outfile = fopen("out.txt", "w");
    int i;
    const int linemaxlen = 1024;
    char dummy[linemaxlen];
    printf("Pre-scanning file ...\n");
    for (i = 0; fgets(dummy, linemaxlen, infile) != NULL; ) {
        i++;
    }
    printf("%d lines found\n", i);
    progbar_t progb(i, PROGB_LEN);
    progb.upd(0);
    rewind(infile);
    vector Ucal(0, 0, 0),
           fcal = Ucal;
    basis B0;
    double indata[COLS_CNT];
    for (i = 0; i <= CAL_TIME && freadline(infile, indata, COLS_CNT); ) {
        i++;
        fcal += vector(indata[ACC_X] , indata[ACC_Y] , indata[ACC_Z]);
        Ucal += vector(indata[GYRO_X] , indata[GYRO_Y] , indata[GYRO_Z]);
        progb.upd(i);
    }
    printf("\nCalibration finished at i = %d\n", i);
/*    B0.rotate(vector(0, degtorad(-1.954215945840), 0));
    B0.rotate(vector(degtorad(-3.172652538105), 0, 0));
    B0.rotate(vector(0, 0, -degtorad(87.123140486447)));*/
    SINS_t SINS(degtorad(indata[LATIT_SNS]), degtorad(indata[LONGIT_SNS]), indata[H_SNS], fcal / i, Ucal / i, UPD_PERIOD);
    for (i; freadline(infile, indata, COLS_CNT); ) {
        i++;
        vector acc_raw  (indata[ACC_X] , indata[ACC_Y] , indata[ACC_Z] ),
               omega_raw(indata[GYRO_X], indata[GYRO_Y], indata[GYRO_Z]);
        SINS.upd(acc_raw, omega_raw);
        progb.upd(i);
        if ((i % OUT_INTERVAL) == 0) {
            vector v = SINS.getv();
            basis B = SINS.getb();
            double vsns      = indata[V_SNS],
                   coursesns = degtorad(indata[COURSE_SNS]),
                   vesns     = vsns * sin(coursesns),
                   vnsns     = vsns * cos(coursesns),
                   latit     = radtodeg(SINS.getlatit()),
                   longit    = radtodeg(SINS.getlongit()),
                   courseins = radtodeg(vector::Angle(vector(0, 1, 0), B.j));
                   if (B.j.x < 0) {
                       courseins = 360 - courseins;
                   }
//                   dlatit     = latit - latit_sns,
//                   dlongit    = longit - longit_sns;
            fprintf(outfile,
                    "%20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf %20.10lf\n",
                    vesns,    vnsns, vsns, indata[LATIT_SNS], indata[LONGIT_SNS], indata[H_SNS], indata[COURSE_SNS],
                    indata[VE_INS], indata[VN_INS],
                    v.x, v.y, latit, longit, SINS.geth(), courseins);
        }
    }
    printf("\n");
    fclose(infile);
    fclose(outfile);

	return 0;
}
