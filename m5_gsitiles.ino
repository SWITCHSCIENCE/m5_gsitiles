#include <Arduino.h>
#include <M5Core2.h>
#include "TileImage.h"
#include "M5TileImageViewer.h"

TileImage::ImageSource osakaOrthoImage(8, 15, 160, 24800, 21000, "/output/osaka_ortho_files", "jpg");
M5TileImageViewer viewer(M5.Lcd, osakaOrthoImage, 64); // If tileSize is 256, specify 25.

void setup()
{
    M5.begin();
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Ortho Image Viewer for M5Stack Core2");

    viewer.willLoadImageCallback = printLoading;
    viewer.setFrame({0, 0, M5.Lcd.width(), M5.Lcd.height() - 8});
    viewer.prepareCache();
    viewer.viewport.setLevel(viewer.imageSource.minLevel);
    viewer.viewport.showCenter();
    viewer.draw();

    printTouchLabel();
}

void loop()
{
    bool redraw = false;
    bool moving = false;

    M5.update();

    Event &e = M5.Buttons.event;
    if (e & E_MOVE)
    {
        float dx, dy;
        viewer.imageSource.imageToViewportPoint(viewer.viewport.level, e.from.x - e.to.x, e.from.y - e.to.y, &dx, &dy);
        viewer.viewport.move(dx, dy);
        redraw = true;
        moving = true;
    }
    if (e & E_RELEASE)
    {
        redraw = true;
        moving = false;
    }

    if (M5.BtnA.wasReleased())
    {
        viewer.viewport.zoom(+1);
        redraw = true;
    }

    if (M5.BtnB.wasReleased())
    {
        viewer.viewport.setLevel(viewer.imageSource.minLevel);
        viewer.viewport.showCenter();
        redraw = true;
    }

    if (M5.BtnC.wasReleased())
    {
        viewer.viewport.zoom(-1);
        redraw = true;
    }
    M5.update();

    if (redraw)
    {
        if (moving)
        {
            printPoint();
        }
        viewer.draw(moving);
        if (!moving)
        {
            printTouchLabel();
        }
    }

    int wait = -millis() % 33;
    delay(wait ? wait : 33);
}

void printTouchLabel(void)
{
    M5.Lcd.setCursor(0, M5.Lcd.height() - 8 + 1);
    if (viewer.viewport.level < viewer.imageSource.maxLevel)
    {
        M5.Lcd.print("     Zoom In      ");
    }
    else
    {
        M5.Lcd.print("                  ");
    }
    M5.Lcd.print("      Reset       ");
    if (viewer.viewport.level > viewer.imageSource.minLevel)
    {
        M5.Lcd.print("     Zoom Out     ");
    }
    else
    {
        M5.Lcd.print("                  ");
    }
}

void printPoint(void)
{
    M5.Lcd.setCursor(0, M5.Lcd.height() - 8 + 1);
    int32_t x, y;
    viewer.imageSource.viewportToImagePoint(viewer.viewport.level, viewer.viewport.x, viewer.viewport.y, &x, &y);
    M5.Lcd.printf("%8d %-8d ", x, y);
}

void printLoading(const char *path, uint8_t level, int col, int row)
{
    M5.Lcd.setCursor(0, M5.Lcd.height() - 8 + 1);
    M5.Lcd.printf("%-35s ", path);
}
