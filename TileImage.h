#ifndef TILEIMAGE_H
#define TILEIMAGE_H

#include <stdint.h>

namespace TileImage
{
    typedef struct
    {
        int32_t x;
        int32_t y;
    } Point;

    typedef struct
    {
        int32_t x;
        int32_t y;
        uint32_t w;
        uint32_t h;
    } Rect;

    class ImageSource
    {
    public:
        uint8_t minLevel;
        uint8_t maxLevel;
        uint8_t numLevels;
        uint16_t tileSize;
        uint32_t imageWidth;
        uint32_t imageHeight;
        const char *imagesUrl;
        const char *imagesFmt;
        ImageSource(uint8_t min_lvl, uint8_t max_lvl, uint16_t tile_sz, uint32_t img_w, uint32_t img_h, const char *img_dir, const char *img_fmt);
        ~ImageSource() {}
        void imageToViewportPoint(uint8_t level, int32_t x, int32_t y, float *pX, float *pY);
        void viewportToImagePoint(uint8_t level, float x, float y, int32_t *pX, int32_t *pY);
        void getTileAtPoint(uint8_t level, int32_t x, int32_t y, int *pColumn, int *pRow);
        virtual size_t getImageUrl(char *str, size_t nstr, uint8_t level, int column, int row);
        int computeTiledRects(uint8_t level, Rect bounds, Point points[], Rect rects[], int max_rects, bool repeat = false);
        void printTiledRects(Point points[], Rect rects[], int count);
    };

    class XYZImageSource : public ImageSource
    {
    public:
        XYZImageSource(uint8_t min_lvl, uint8_t max_lvl, uint16_t tile_sz, const char *img_dir, const char *img_fmt);
        ~XYZImageSource() {}
        size_t getImageUrl(char *str, size_t nstr, uint8_t level, int column, int row);
    };

    class Viewport
    {
    public:
        float x;
        float y;
        uint8_t level;
        uint8_t minLevel;
        uint8_t maxLevel;
        Viewport(uint8_t min_lvl, uint8_t max_lvl) : x(0), y(0), minLevel(min_lvl), maxLevel(max_lvl), level(max_lvl) {}
        ~Viewport() {}
        void showCenter(void);
        void moveTo(float x, float y);
        void move(float delta_x, float delta_y);
        void setLevel(uint8_t new_level);
        void zoom(int delta);
    };

    class RectCache
    {
    public:
        static const uint32_t InvalidKey;
        int numCaches;
        uint32_t *keys;
        Rect *values;
        RectCache(int maxlen);
        ~RectCache();
        Rect *cacheFor(uint32_t key);
        Rect *acquireCache(uint32_t key);
        void removeCache(uint32_t key);
        static uint32_t makeKey(int level, int c, int r)
        {
            return (((level & 0x3f) << 26) | ((c & 0x1fff) << 13) | (r & 0x1fff));
        }
        static uint32_t distance(uint32_t key1, uint32_t key2)
        {
            int c1 = (key1 >> 13) & 0x1fff;
            int r1 = key1 & 0x1fff;
            int c2 = (key2 >> 13) & 0x1fff;
            int r2 = key2 & 0x1fff;
            return ((c1 - c2) * (c1 - c2)) + ((r1 - r2) * (r1 - r2));
        }
    };

    class Viewer
    {
    public:
        ImageSource &imageSource;
        Viewport viewport;
        RectCache tileCache;
        Rect frame;
        Viewer(ImageSource &imgsrc, int numCaches = 1);
        virtual ~Viewer();
        void setFrame(Rect frame);
        virtual bool loadImageToCache(Rect dst, uint8_t level, int column, int row);
        virtual void draw(bool loadDisable = false);
        virtual void drawNoImage(Rect rect);
        virtual void drawCachedImage(Point dst, Rect src);
    };

};
#endif
