/* 
 * File:   srtmHgtReader.h
 * Author: Pavel Zbytovský <pavel@zby.cz>
 *
 * Created on April 28, 2013, 6:44 PM
 */

#ifndef SRTMHGTREADER_H
#define	SRTMHGTREADER_H

int srtmLoadTile(int latDec, int lonDec);
void srtmReadPx(int y, int x, int* height);
void setFolder(const char* f,int size);
const char* getFolder();
float srtmGetElevation(float lat, float lon);
void srtmClose();

#define INVALID_VALUE 9999999999999

struct _SrtmAscentDescent {
    float ascent;
    float descent;
    float ascentOn;
    float descentOn;
};

typedef struct _SrtmAscentDescent TSrtmAscentDescent;


TSrtmAscentDescent srtmGetAscentDescent(float lat1, float lon1, float lat2, float lon2, float dist);


#endif	/* SRTMHGTREADER_H */

