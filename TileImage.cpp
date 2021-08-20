#include <stdio.h>
#include <math.h>
#include "TileImage.h"

using namespace TileImage;

#define _MY_MIN_(a, b) ((a) > (b) ? (b) : (a))
#define _MY_MAX_(a, b) ((a) > (b) ? (a) : (b))
#define _MY_ABS_(a) ((a) < 0 ? -(a) : (a))

ImageSource::ImageSource(uint8_t min_lvl, uint8_t max_lvl, uint16_t tile_sz, uint32_t img_w, uint32_t img_h, const char *img_url, const char *img_fmt)
    : minLevel(min_lvl), maxLevel(max_lvl), tileSize(tile_sz), imageWidth(img_w), imageHeight(img_h), imagesUrl(img_url), imagesFmt(img_fmt)
{
    numLevels = 1;
    uint32_t res = img_w > img_h ? img_w : img_h;
    for (; res; res >>= 1, numLevels++)
        ;
}
void ImageSource::imageToViewportPoint(uint8_t level, int32_t x, int32_t y, float *pX, float *pY)
{
    level = _MY_MIN_(level, maxLevel);
    level = _MY_MAX_(level, minLevel);
    int sf = 1 << ((numLevels - 1) - level);
    uint32_t scaled_img_w = int((imageWidth + sf - 1) / sf);
    uint32_t scaled_img_h = int((imageHeight + sf - 1) / sf);
    *pX = float(x) / float(scaled_img_w);
    *pY = float(y) / float(scaled_img_h);
}

void ImageSource::viewportToImagePoint(uint8_t level, float x, float y, int32_t *pX, int32_t *pY)
{
    level = _MY_MIN_(level, maxLevel);
    level = _MY_MAX_(level, minLevel);
    int sf = 1 << ((numLevels - 1) - level);
    uint32_t scaled_img_w = int((imageWidth + sf - 1) / sf);
    uint32_t scaled_img_h = int((imageHeight + sf - 1) / sf);
    *pX = scaled_img_w * x;
    *pY = scaled_img_h * y;
}

void ImageSource::getTileAtPoint(uint8_t level, int32_t x, int32_t y, int *pColumn, int *pRow)
{
    level = _MY_MIN_(level, maxLevel);
    level = _MY_MAX_(level, minLevel);
    int sf = 1 << ((numLevels - 1) - level);
    uint32_t scaled_img_w = int((imageWidth + sf - 1) / sf);
    uint32_t scaled_img_h = int((imageHeight + sf - 1) / sf);
    if (y < 0 || y >= scaled_img_h)
    {
        *pRow = -1;
    }
    else
    {
        int r_max = int((scaled_img_h + tileSize - 1) / tileSize);
        *pRow = int(y / tileSize) % r_max;
    }
    if (x < 0 || x >= scaled_img_w)
    {
        *pColumn = -1;
    }
    else
    {
        int c_max = int((scaled_img_w + tileSize - 1) / tileSize);
        *pColumn = int(x / tileSize) % c_max;
    }
}

size_t ImageSource::getImageUrl(char *str, size_t nstr, uint8_t l, int c, int r)
{
    return snprintf(str, nstr, "%s/%d/%d_%d.%s", imagesUrl, l, c, r, imagesFmt);
}

int ImageSource::computeTiledRects(uint8_t level, Rect bounds, Point points[], Rect rects[], int max_rects, bool repeat)
{
    level = _MY_MIN_(level, maxLevel);
    level = _MY_MAX_(level, minLevel);
    int sf = 1 << ((numLevels - 1) - level);
    uint32_t scaled_img_w = int((imageWidth + sf - 1) / sf);
    uint32_t scaled_img_h = int((imageHeight + sf - 1) / sf);
    int ret = 0;
    int32_t y2 = bounds.y + bounds.h;
    int32_t dy = 0;
    for (int32_t i = bounds.y; i < y2;)
    {
        int32_t x2 = bounds.x + bounds.w;
        int32_t dx = 0;
        int32_t yt;
        if (repeat)
        {
            yt = i < 0 ? i % -scaled_img_h : i % scaled_img_h;
        }
        else
        {
            yt = i;
        }
        uint16_t ht;
        if (yt < 0)
        {
            ht = _MY_MIN_(-yt, bounds.h);
        }
        else if (yt >= scaled_img_h)
        {
            ht = _MY_MIN_(y2 - i, bounds.h);
        }
        else
        {
            ht = _MY_MIN_(y2 - i, tileSize - (yt % tileSize));
            if (yt + ht > scaled_img_h)
            {
                ht = scaled_img_h - yt;
            }
        }
        for (int32_t j = bounds.x; j < x2;)
        {
            Point &p = points[ret];
            Rect &r = rects[ret];
            if (ret < max_rects)
            {
                if (repeat)
                {
                    r.x = j < 0 ? j % -scaled_img_w : j % scaled_img_w;
                }
                else
                {
                    r.x = j;
                }
                uint16_t wt;
                if (r.x < 0)
                {
                    wt = _MY_MIN_(-r.x, bounds.w);
                }
                else if (r.x >= scaled_img_w)
                {
                    wt = _MY_MIN_(x2 - j, bounds.w);
                }
                else
                {
                    wt = _MY_MIN_(x2 - j, tileSize - (r.x % tileSize));
                    if (r.x + wt > scaled_img_w)
                    {
                        wt = scaled_img_w - r.x;
                    }
                }
                r.y = yt;
                r.h = ht;
                r.w = wt;
                p.x = dx;
                p.y = dy;
                j += wt;
                dx += wt;
                ret++;
            }
            else
            {
                return ret;
            }
        }
        i += ht;
        dy += ht;
    }

    return ret;
}

void ImageSource::printTiledRects(Point points[], Rect rects[], int count)
{
    for (int i = 0; i < count; i++)
    {
        printf("%d (%d %d) (%d %d %d %d)\n", i, points[i].x, points[i].y, rects[i].x, rects[i].y, rects[i].w, rects[i].h);
    }
}

XYZImageSource::XYZImageSource(uint8_t min_lvl, uint8_t max_lvl, uint16_t tile_sz, const char *img_dir, const char *img_fmt)
    : ImageSource(min_lvl, max_lvl, tile_sz, tile_sz * (1 << max_lvl), tile_sz * (1 << max_lvl), img_dir, img_fmt)
{
    numLevels = max_lvl + 1;
}

size_t XYZImageSource::getImageUrl(char *str, size_t nstr, uint8_t l, int c, int r)
{
    return snprintf(str, nstr, "%s/%d/%d/%d.%s", imagesUrl, l, c, r, imagesFmt);
}

void Viewport::showCenter(void)
{
    moveTo(0.5, 0.5);
}

void Viewport::moveTo(float x, float y)
{
    this->x = x;
    this->y = y;
}

void Viewport::move(float delta_x, float delta_y)
{
    moveTo(this->x + delta_x, this->y + delta_y);
}

void Viewport::setLevel(uint8_t new_level)
{
    new_level = _MY_MIN_(new_level, maxLevel);
    new_level = _MY_MAX_(new_level, minLevel);
    level = new_level;
}

void Viewport::zoom(int delta)
{
    setLevel(level + delta);
}

const uint32_t RectCache::InvalidKey = 0xffffffff;

RectCache::RectCache(int maxlen) : numCaches(maxlen)
{
    keys = new uint32_t[numCaches];
    values = new Rect[numCaches];
    for (int i = 0; i < numCaches; i++)
    {
        keys[i] = InvalidKey;
    }
}

RectCache::~RectCache()
{
    delete[] keys;
    delete[] values;
}

Rect *RectCache::cacheFor(uint32_t key)
{
    for (int i = 0; i < numCaches; i++)
    {
        if (keys[i] == key)
        {
            return &values[i];
        }
    }
    return NULL;
}

Rect *RectCache::acquireCache(uint32_t key)
{
    int k = -1;
    for (int i = 0; i < numCaches; i++)
    {
        if (keys[i] == InvalidKey)
        {
            k = i;
            break;
        }
    }
    if (k == -1)
    {
        uint32_t max_distance = 0;
        k = 0;
        for (int i = 0; i < numCaches; i++)
        {
            uint32_t d = distance(key, keys[i]);
            if (d > max_distance)
            {
                k = i;
                max_distance = d;
            }
        }
    }
    keys[k] = key;
    return &values[k];
}

void RectCache::removeCache(uint32_t key)
{
    for (int i = 0; i < numCaches; i++)
    {
        if (keys[i] == key)
        {
            keys[i] = InvalidKey;
            break;
        }
    }
}

Viewer::Viewer(ImageSource &imgsrc, int numCaches)
    : imageSource(imgsrc), viewport(imgsrc.minLevel, imgsrc.maxLevel), tileCache(numCaches)
{
}

Viewer::~Viewer()
{
}

void Viewer::setFrame(Rect f)
{
    frame = f;
}

bool Viewer::loadImageToCache(Rect dst, uint8_t level, int column, int row)
{
    return false;
}

void Viewer::draw(bool loadDisable)
{
    int num_cols = frame.w / imageSource.tileSize + 3;
    int num_rows = frame.h / imageSource.tileSize + 3;
    int num_tiles = num_cols * num_rows;
    Point *points = new Point[num_tiles];
    Rect *rects = new Rect[num_tiles];
    Rect bounds = {0, 0, frame.w, frame.h};
    uint8_t lvl = viewport.level;
    imageSource.viewportToImagePoint(lvl, viewport.x, viewport.y, &bounds.x, &bounds.y);
    bounds.x -= bounds.w / 2;
    bounds.y -= bounds.h / 2;
    int ret = imageSource.computeTiledRects(lvl, bounds, points, rects, num_tiles);
    // imageSource.printTiledRects(points, rects, ret);

    for (int i = 0; i < ret; i++)
    {
        Point &p = points[i];
        Rect &r = rects[i];
        int col, row;
        imageSource.getTileAtPoint(lvl, r.x, r.y, &col, &row);
        Rect *cache = NULL;
        // Serial.printf("%d %d %d => %d %d\n", lvl, r.x, r.y, col, row);
        if (col >= 0 && row >= 0)
        {
            uint32_t key = RectCache::makeKey(lvl, col, row);
            cache = tileCache.cacheFor(key);
            if (cache == NULL && !loadDisable)
            {
                // Serial.printf("%d %d %d = no cache\n", lvl, col, row);
                cache = tileCache.acquireCache(key);
                // Serial.printf("%d %d %d = (%d %d)\n", lvl, col, row, cache->x, cache->y);
                if (!loadImageToCache(*cache, lvl, col, row))
                {
                    tileCache.removeCache(key);
                    cache = NULL;
                }
            }
        }
        if (cache == NULL)
        {
            // Serial.printf("fill (%d %d %d %d)\n", p.x, p.y, r.w, r.h);
            drawNoImage({p.x, p.y + frame.y, r.w, r.h});
            // display.fillRect(p.x, p.y + frame.y, r.w, r.h, NO_IMAGE_COLOR);
        }
        else
        {
            // Serial.printf("cache (%d %d %d %d) <= (%d %d)\n", p.x, p.y, r.w, r.h, cache->x, cache->y);
            int16_t sx = cache->x + r.x % cache->w;
            int16_t sy = cache->y + r.y % cache->h;
            drawCachedImage({p.x, p.y + frame.y}, {sx, sy, r.w, r.h});
        }
    }

    delete[] points;
    delete[] rects;
}

void Viewer::drawNoImage(Rect bounds)
{
}

void Viewer::drawCachedImage(Point dst, Rect src)
{
}
