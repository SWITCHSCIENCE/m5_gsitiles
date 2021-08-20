#include <SD.h>
#include <HTTPClient.h>
#include "M5TileImageViewer.h"

#define DBG_PRINT(s)     \
    do                   \
    {                    \
        Serial.print(s); \
    } while (0);

#define DBG_PRINTLN(s)     \
    do                     \
    {                      \
        Serial.println(s); \
    } while (0);

#define DBG_PRINTF(s, ...)             \
    do                                 \
    {                                  \
        Serial.printf(s, __VA_ARGS__); \
    } while (0);

static void mkdir_r(char *path)
{
    char *r = strrchr(path, '/');
    if (r)
    {
        *r = 0;
        if (SD.exists(path))
        {
            *r = '/';
            return;
        }
        *r = '/';
        char *p = strchr(&path[1], '/');
        while (p)
        {
            *p = 0;
            if (!SD.exists(path))
            {
                DBG_PRINTF("mkdir %s\n", path)
                SD.mkdir(path);
            }
            *p = '/';
            p = strchr(p + 1, '/');
        }
    }
}

static HTTPClient http;
static uint8_t http_buff[4096];

static bool httpget(const char *url, const char *ca, const char *save_to)
{
    bool result = true;

    DBG_PRINT("[HTTP] begin...\n");
    http.setReuse(true);
    http.begin(url, ca);

    DBG_PRINTF("[HTTP] GET %s\n", url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        DBG_PRINTF("[HTTP] GET... code: %d\n", httpCode);

        WiFiClient *stream = http.getStreamPtr();
        int data_size = http.getSize();
        int read_size = 0;

        DBG_PRINTF("[HTTP] Save to %s\n", save_to);

        File f = SD.open(save_to, FILE_WRITE);
        if (f)
        {
            while (http.connected() && read_size < data_size)
            {
                size_t sz = stream->available();
                if (sz > 0)
                {
                    int rd = stream->readBytes(http_buff, min(sz, sizeof(http_buff)));
                    read_size += rd;
                    DBG_PRINTF("[HTTP] read: %d/%d\n", read_size, data_size);
                    f.write(http_buff, rd);
                }
                else
                {
                    delay(1);
                }
            }
            f.close();
        }
        else
        {
            DBG_PRINTLN("ERROR: OPEN FILE.");
            result = false;
        }
    }
    else
    {
        DBG_PRINTF("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        result = false;
    }
    http.end();
    return result;
}

static const uint16_t NO_IMAGE_COLOR = BLUE;

void M5TileImageViewer::prepareCache(void)
{
    uint32_t tsz = imageSource.tileSize;
    spriteCache.createSprite(tileCache.numCaches * tsz, tsz);
    for (int i = 0; i < tileCache.numCaches; i++)
    {
        tileCache.values[i] = {int32_t(i * tsz), 0, tsz, tsz};
    }
}

bool M5TileImageViewer::loadImageToCache(TileImage::Rect dst, uint8_t level, int column, int row)
{
    bool result = false;
    char url[256];
    char *filepath = url;

    DBG_PRINTF("load %d %d %d\n", level, column, row);

    imageSource.getImageUrl(url, sizeof(url), level, column, row);
    if (strncmp(url, "https://", 8) == 0)
    {
        filepath = url + 7;
        if (!SD.exists(filepath))
        {
            if (willLoadImageCallback)
            {
                willLoadImageCallback(filepath, level, column, row);
            }
            mkdir_r(filepath);
            httpget(url, ca, filepath);
        }
    }
    if (SD.exists(filepath))
    {
        if (willLoadImageCallback)
        {
            willLoadImageCallback(filepath, level, column, row);
        }
        spriteCache.fillRect(dst.x, dst.y, dst.w, dst.h, 0);
        spriteCache.drawPngFile(SD, filepath, dst.x, dst.y, dst.w, dst.h, 0, 0);
        result = true;
    }
    return result;
}

void M5TileImageViewer::drawNoImage(TileImage::Rect rect)
{
    display.fillRect(rect.x, rect.y, rect.w, rect.h, NO_IMAGE_COLOR);
}

void M5TileImageViewer::drawCachedImage(TileImage::Point dst, TileImage::Rect src)
{
    uint16_t iw = spriteCache.width();
    uint16_t *fb = (uint16_t *)spriteCache.frameBuffer(16) + iw * src.y + src.x;
    bool oldSwapBytes = display.getSwapBytes();
    display.setSwapBytes(false);
    for (int i = 0; i < src.h; i++)
    {
        display.pushImage(dst.x, dst.y + i, src.w, 1, fb);
        fb += iw;
    }
    display.setSwapBytes(oldSwapBytes);
}
