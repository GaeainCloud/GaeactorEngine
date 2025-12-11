/* 
 * File:   srtmHgtReader.cpp
 * Author: Pavel Zbytovský <pavel@zby.cz>
 *
 * Created on April 28, 2013, 12:01 AM
 */
//#define SRTMSLIM 1

#include <stdio.h> 
#include <stdlib.h> //exit
#include <stdint.h> //int16_t
#include <math.h>

#include "srtmHgtReader.h" //fmod

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
static CRITICAL_SECTION srtmCacheMutex;
#else
#include <pthread.h>
static pthread_mutex_t srtmCacheMutex = PTHREAD_MUTEX_INITIALIZER;
#endif
static int srtmCacheMutexInitialized = 0;

//const int secondsPerPx = 1;  //arc seconds per pixel (3 equals cca 90m)
//const int totalPx = 3601;
//const char* folder = "aster";

const int secondsPerPx = 3;  //arc seconds per pixel (3 equals cca 90m)
const int totalPx = 1201;
char folder[1024] = "srtmsss";

#define MAX_CACHE_TILES 8

int srtmLat = 255; //default never valid
int srtmLon = 255;

typedef struct {
    int lat;
    int lon;
    FILE* fd;     
    unsigned char *tile;
} SrtmCache;

SrtmCache srtmCache[MAX_CACHE_TILES] = {0};

void setFolder(const char* f,int size)
{
    strcpy(folder,f);
    folder[size] = '\0';
}

const char *getFolder()
{
    return folder;
}

/** Prepares corresponding file if not opened */
int srtmLoadTile(int latDec, int lonDec){
    if (!srtmCacheMutexInitialized) {
#if defined(_WIN32) || defined(_WIN64)
        InitializeCriticalSection(&srtmCacheMutex);
#else
        pthread_mutex_init(&srtmCacheMutex, NULL);
#endif
        srtmCacheMutexInitialized = 1;
    }
    
    // Use single critical section for the entire cache operation
#if defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&srtmCacheMutex);
#else
    pthread_mutex_lock(&srtmCacheMutex);
#endif
    
    // Check if tile is already cached
    for (int i = 0; i < MAX_CACHE_TILES; i++) {
        if (srtmCache[i].lat == latDec && srtmCache[i].lon == lonDec) {
#if defined(_WIN32) || defined(_WIN64)
            LeaveCriticalSection(&srtmCacheMutex);
#else
            pthread_mutex_unlock(&srtmCacheMutex);
#endif
            return 1;
        }
    }

    // Find empty slot
    int slot = -1;
    for (int i = 0; i < MAX_CACHE_TILES; i++) {
        if (srtmCache[i].fd == NULL) {
            slot = i;
            break;
        }
    }
    
    // If no empty slot, use the first one (simple replacement)
    if (slot == -1) {
        slot = 0;
    }

    // Close existing fd if needed
    if (srtmCache[slot].fd != NULL) {
        fclose(srtmCache[slot].fd);
        srtmCache[slot].fd = NULL;
#if !SRTMSLIM
        if (srtmCache[slot].tile != NULL) {
            free(srtmCache[slot].tile);
            srtmCache[slot].tile = NULL;
        }
#endif
    }

    // Set new cache entry and open file
    srtmCache[slot].lat = latDec;
    srtmCache[slot].lon = lonDec;
    
    char filename[1024];
#if defined (Q_OS_WIN)
    sprintf_s(filename,1024, "%s/%c%02d%c%03d.hgt", folder,
                latDec>0?'N':'S', abs(latDec),
                lonDec>0?'E':'W', abs(lonDec));
#else
    sprintf(filename, "%s/%c%02d%c%03d.hgt", folder,
                latDec>0?'N':'S', abs(latDec),
                lonDec>0?'E':'W', abs(lonDec));
#endif

//    printf("Opening %s\n", filename);
#if defined (Q_OS_WIN)
    int err = fopen_s(&srtmCache[slot].fd, filename, "rb");  // use binary mode for consistent behavior
    if(err != 0) {
        printf("Error opening %s\n",  filename);
        srtmCache[slot].fd = NULL;
        srtmCache[slot].lat = 255; // Mark as invalid
        srtmCache[slot].lon = 255;
#if defined(_WIN32) || defined(_WIN64)
        LeaveCriticalSection(&srtmCacheMutex);
#else
        pthread_mutex_unlock(&srtmCacheMutex);
#endif
        return  0;
    }
#else
    srtmCache[slot].fd = fopen(filename, "rb");  // use binary mode for consistent behavior
    if(srtmCache[slot].fd == NULL) {
        srtmCache[slot].lat = 255; // Mark as invalid
        srtmCache[slot].lon = 255;
#if defined(_WIN32) || defined(_WIN64)
        LeaveCriticalSection(&srtmCacheMutex);
#else
        pthread_mutex_unlock(&srtmCacheMutex);
#endif
        return  0;
    }
#endif

#if !SRTMSLIM
    if (srtmCache[slot].tile == NULL) {
        srtmCache[slot].tile = (unsigned char*) malloc(totalPx * totalPx * 2);
        if (srtmCache[slot].tile == NULL) {
            fclose(srtmCache[slot].fd);
            srtmCache[slot].fd = NULL;
            srtmCache[slot].lat = 255; // Mark as invalid
            srtmCache[slot].lon = 255;
#if defined(_WIN32) || defined(_WIN64)
            LeaveCriticalSection(&srtmCacheMutex);
#else
            pthread_mutex_unlock(&srtmCacheMutex);
#endif
            return 0;
        }
    }
    
    // Check file and buffer integrity before fread
    if (srtmCache[slot].fd == NULL || srtmCache[slot].tile == NULL) {
#if defined(_WIN32) || defined(_WIN64)
        LeaveCriticalSection(&srtmCacheMutex);
#else
        pthread_mutex_unlock(&srtmCacheMutex);
#endif
        return 0;
    }
    
    // Read the whole tile
    size_t bytesRead = fread(srtmCache[slot].tile, 1, (2 * totalPx * totalPx), srtmCache[slot].fd);
    if (bytesRead != (2 * totalPx * totalPx)) {
        printf("Error reading tile data from %s, bytes read: %zu\n", filename, bytesRead);
        fclose(srtmCache[slot].fd);
        srtmCache[slot].fd = NULL;
        free(srtmCache[slot].tile);
        srtmCache[slot].tile = NULL;
        srtmCache[slot].lat = 255; // Mark as invalid
        srtmCache[slot].lon = 255;
#if defined(_WIN32) || defined(_WIN64)
        LeaveCriticalSection(&srtmCacheMutex);
#else
        pthread_mutex_unlock(&srtmCacheMutex);
#endif
        return 0;
    }
#endif

#if defined(_WIN32) || defined(_WIN64)
    LeaveCriticalSection(&srtmCacheMutex);
#else
    pthread_mutex_unlock(&srtmCacheMutex);
#endif

    return 1;
}

void srtmClose(){
    if (!srtmCacheMutexInitialized) {
#if defined(_WIN32) || defined(_WIN64)
        InitializeCriticalSection(&srtmCacheMutex);
#else
        pthread_mutex_init(&srtmCacheMutex, NULL);
#endif
        srtmCacheMutexInitialized = 1;
    }
#if defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&srtmCacheMutex);
#else
    pthread_mutex_lock(&srtmCacheMutex);
#endif
    for (int i = 0; i < MAX_CACHE_TILES; i++) {
        if (srtmCache[i].fd != NULL) {
            fclose(srtmCache[i].fd);
            srtmCache[i].fd = NULL;
        }
#if !SRTMSLIM
        if (srtmCache[i].tile != NULL) {
            free(srtmCache[i].tile);
            srtmCache[i].tile = NULL;
        }
#endif
    }
#if defined(_WIN32) || defined(_WIN64)
    LeaveCriticalSection(&srtmCacheMutex);
#else
    pthread_mutex_unlock(&srtmCacheMutex);
#endif
}

/** Pixel idx from left bottom corner (0-1200) */
void srtmReadPx(int y, int x, int* height){
    int row = (totalPx-1) - y;
    int col = x;
    int pos = (row * totalPx + col) * 2;
    
#if SRTMSLIM
    
    //seek and read 2 bytes short - must be protected by mutex
    unsigned char buff[2];// = {0xFF, 0xFB}; //-5 (bigendian)
    FILE* currentFd = NULL;
    
    // Protect cache access with mutex to prevent race conditions
#if defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&srtmCacheMutex);
#else
    pthread_mutex_lock(&srtmCacheMutex);
#endif
    for (int i = 0; i < MAX_CACHE_TILES; i++) {
        if (srtmCache[i].lat == srtmLat && srtmCache[i].lon == srtmLon) {
            currentFd = srtmCache[i].fd;
            break;
        }
    }
    
    if (currentFd == NULL) {
#if defined(_WIN32) || defined(_WIN64)
        LeaveCriticalSection(&srtmCacheMutex);
#else
        pthread_mutex_unlock(&srtmCacheMutex);
#endif
        *height = 0;
        return;
    }
    
    fseek(currentFd, pos, SEEK_SET);
    size_t read = fread(&buff, 2, 1, currentFd);
    
#if defined(_WIN32) || defined(_WIN64)
    LeaveCriticalSection(&srtmCacheMutex);
#else
    pthread_mutex_unlock(&srtmCacheMutex);
#endif
    
    if (read != 1) {
        // Reset file pointer if read failed
        rewind(currentFd);
        *height = 0;
        return;
    }
    // Reset file pointer after successful read
    rewind(currentFd);
    
#else
    
    // Find the correct cache entry - must be protected by mutex
    unsigned char *buff = NULL;
#if defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&srtmCacheMutex);
#else
    pthread_mutex_lock(&srtmCacheMutex);
#endif
    for (int i = 0; i < MAX_CACHE_TILES; i++) {
        if (srtmCache[i].lat == srtmLat && srtmCache[i].lon == srtmLon) {
            if (srtmCache[i].tile == NULL) {
#if defined(_WIN32) || defined(_WIN64)
                LeaveCriticalSection(&srtmCacheMutex);
#else
                pthread_mutex_unlock(&srtmCacheMutex);
#endif
                *height = 0;
                return;
            }
            buff = &srtmCache[i].tile[pos];
            break;
        }
    }
#if defined(_WIN32) || defined(_WIN64)
    LeaveCriticalSection(&srtmCacheMutex);
#else
    pthread_mutex_unlock(&srtmCacheMutex);
#endif
    
    if (buff == NULL) {
        *height = 0;
        return;
    }
    
#endif
    
    //solve endianity (using int16_t)
    int16_t hgt = 0 | (buff[0] << 8) | (buff[1] << 0);
    
    if(hgt == -32768) {
        printf("ERROR: Void pixel found on xy(%d,%d) in latlon(%d,%d) tile.\n", x,y, srtmLat, srtmLon);
        //exit(1);
        hgt = 0;
    }
    
    *height = (int) hgt;
}       

/** Returns interpolated height from four nearest points */
float srtmGetElevation(float lat, float lon){
    int latDec = (int)floor(lat);
    int lonDec = (int)floor(lon);

    float secondsLat = (lat-latDec) * 60 * 60;
    float secondsLon = (lon-lonDec) * 60 * 60;
    
    auto loaded = srtmLoadTile(latDec, lonDec);
    if(!loaded)
    {
        return  INVALID_VALUE;
    }

    //X coresponds to x/y values,
    //everything easter/norhter (< S) is rounded to X.
    //
    //  y   ^
    //  3   |       |   S
    //      +-------+-------
    //  0   |   X   |
    //      +-------+-------->
    // (sec)    0        3   x  (lon)
    
    //both values are 0-1199 (1200 reserved for interpolating)
    int y = secondsLat/secondsPerPx;
    int x = secondsLon/secondsPerPx;
    
    srtmLat = latDec;
    srtmLon = lonDec;
    //get norther and easter points
    int height[4];
    srtmReadPx(y,   x, &height[2]);
    srtmReadPx(y+1, x, &height[0]);
    srtmReadPx(y,   x+1, &height[3]);
    srtmReadPx(y+1, x+1, &height[1]);

    //ratio where X lays
    float dy = fmod(secondsLat, secondsPerPx) / secondsPerPx;
    float dx = fmod(secondsLon, secondsPerPx) / secondsPerPx;
    
    // Bilinear interpolation
    // h0------------h1
    // |
    // |--dx-- .
    // |       |
    // |      dy
    // |       |
    // h2------------h3   
    return  height[0] * dy * (1 - dx) +
            height[1] * dy * (dx) +
            height[2] * (1 - dy) * (1 - dx) +
            height[3] * (1 - dy) * dx;
}


/** Returns amount of ascent and descent between points */
TSrtmAscentDescent srtmGetAscentDescent(float lat1, float lon1, float lat2, float lon2, float dist){
    TSrtmAscentDescent ret = {0};
    
    //segment we need to devide in "pixels"
    double latDiff = lat2 - lat1;
    double lonDiff = lon2 - lon1;
    
    //how many pixels there are both in y and x axis
    double latSteps = latDiff * (3600 / 3); // 1/pixelDistance = cca 0.00083
    double lonSteps = lonDiff * (3600 / 3);
    
    //we use the max of both
    int steps = fmax(fabs(latSteps), fabs(lonSteps));

    //just in case both points are inside one pixel (we need interpolation!)
    if(steps == 0) steps = 1;
    
    
    //set the delta of each step
    double latStep = latDiff / steps;
    double lonStep = lonDiff / steps;
    double distStep = dist/steps;
      //printf("steps %d: %f %f %f\n", steps, latStep, lonStep, distStep);
    
    int i;
    double lat = lat1, lon = lon1;
    float height, lastHeight, eleDiff;

    //get first elevation -> we need eleDiff then
    height = srtmGetElevation(lat, lon);
      //printf("first: %f %f hgt:%f\n", lat, lon, height);
    
    for(i=0; i<steps; ++i){
        lat += latStep;
        lon += lonStep;
        lastHeight = height;
        
        height = srtmGetElevation(lat, lon);
        eleDiff = height - lastHeight;
        
        if(eleDiff > 0){
            ret.ascent += eleDiff;
            ret.ascentOn += distStep;
        }
        else{
            ret.descent += -eleDiff;
            ret.descentOn += distStep;
        }
        
        //printf("LL(%d): %f %f hgt: %0.1f, diff %0.1f\n", i, lat, lon, height, eleDiff);
    }
    
    // printf("last: %f %f\n", i, lat, lon); ==   printf("ll2: %f %f\n", i, lat2, lon2);
    
    return ret;
}
