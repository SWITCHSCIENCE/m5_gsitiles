#ifndef M5TILEIMAGEVIEWER_H
#define M5TILEIMAGEVIEWER_H

#include <Arduino.h>
#include <M5GFX.h>
#include "TileImage.h"

class M5TileImageViewer : public TileImage::Viewer
{
public:
    const char *ca;
    void (*willLoadImageCallback)(const char *path, uint8_t level, int col, int row);
    M5Canvas spriteCache;
    M5GFX &display;
    M5TileImageViewer(M5GFX &display, TileImage::ImageSource &imgsrc, int numCaches = 28, const char *ca = NULL)
        : TileImage::Viewer(imgsrc, numCaches), spriteCache(&display), display(display), willLoadImageCallback(NULL), ca(ca){};
    void prepareCache(void);
    bool loadImageToCache(TileImage::Rect dst, uint8_t level, int column, int row);
    void drawNoImage(TileImage::Rect rect);
    void drawCachedImage(TileImage::Point dst, TileImage::Rect src);
};

#endif
