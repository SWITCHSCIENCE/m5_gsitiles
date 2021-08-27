#include <WiFi.h>
#include <Arduino.h>
#include <M5Core2.h>
#include <M5GFX.h>
#include "TileImage.h"
#include "M5TileImageViewer.h"

const char *ssid = ""; //Enter SSID
const char *password = "";  //Enter Password

const char *ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4G\n"
    "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNp\n"
    "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4\n"
    "MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEG\n"
    "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n"
    "hvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2EcWtiHL8\n"
    "RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUhhB5uzsT\n"
    "gHeMCOFJ0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL0gRgykmm\n"
    "KPZpO/bLyCiR5Z2KYVc3rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65TpjoWc4zd\n"
    "QQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjlOCZgdbKfd/+RFO+uIEn8rUAVSNECMWEZ\n"
    "XriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2xmmFghcCAwEAAaNCMEAw\n"
    "DgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFI/wS3+o\n"
    "LkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNvAUKr+yAzv95ZU\n"
    "RUm7lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8dEe3jgr25sbwMp\n"
    "jjM5RcOO5LlXbKr8EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw8lo/s7awlOqzJCK\n"
    "6fBdRoyV3XpYKBovHd7NADdBj+1EbddTKJd+82cEHhXXipa0095MJ6RMG3NzdvQX\n"
    "mcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18YIvDQVETI53O9zJrlAGomecs\n"
    "Mx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02JQZR7rkpeDMdmztcpH\n"
    "WD9f\n"
    "-----END CERTIFICATE-----\n";

// https://cyberjapandata.gsi.go.jp/xyz/std/18/232801/103215.png
// https://maps.gsi.go.jp/development/ichiran.html#std2
// https://maps.gsi.go.jp/development/siyou.html

M5GFX display;
TileImage::XYZImageSource gsiImage(1, 18, 256, "https://cyberjapandata.gsi.go.jp/xyz/std", "png");
// TileImage::XYZImageSource gsiImage(2, 18, 256, "https://cyberjapandata.gsi.go.jp/xyz/seamlessphoto", "jpg");

M5TileImageViewer viewer(display, gsiImage, 25, ca);

double default_x = 0;
double default_y = 0;
int default_z = 1;

void setup()
{
    M5.begin();

    WiFi.begin(ssid, password);

    Serial.println("WiFi connecting");

    // Wait some time to connect to wifi
    for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++)
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nGSI Map Viewer for M5Stack Core2");

    if (WiFi.isConnected())
    {
        Serial.println("Time sync...");

        configTzTime("JST-9", "ntp.nict.jp");
        struct tm timeinfo;
        getLocalTime(&timeinfo, 60000);
    }

    display.begin();
    if (display.width() < display.height())
    {
        display.setRotation(display.getRotation() ^ 1);
    }

    latLonToTile(&default_x, &default_y, 35.689440, 139.691670, 0); // 東京都庁
    default_z = 7;

    viewer.willLoadImageCallback = printLoading;
    viewer.setFrame({0, 0, display.width(), display.height() - 8});
    viewer.prepareCache();
    viewer.viewport.setLevel(default_z);
    viewer.viewport.moveTo(default_x, default_y);
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
        viewer.viewport.setLevel(default_z);
        viewer.viewport.moveTo(default_x, default_y);
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
    display.setCursor(0, display.height() - 8 + 1);
    if (viewer.viewport.level < viewer.imageSource.maxLevel)
    {
        display.print("     Zoom In      ");
    }
    else
    {
        display.print("                  ");
    }
    display.print("      Reset       ");
    if (viewer.viewport.level > viewer.imageSource.minLevel)
    {
        display.print("     Zoom Out     ");
    }
    else
    {
        display.print("                  ");
    }
}

void printPoint(void)
{
    display.setCursor(0, display.height() - 8 + 1);
    int32_t x, y;
    viewer.imageSource.viewportToImagePoint(viewer.viewport.level, viewer.viewport.x, viewer.viewport.y, &x, &y);
    display.printf("%10d %-10d   ", x, y);
}

void printLoading(const char *path, uint8_t level, int col, int row)
{
    display.setCursor(0, display.height() - 8 + 1);
    display.printf("%-35s   ", path);
}

// 参考記事
// https://www.trail-note.net/tech/coordinate/
// https://note.sngklab.jp/?p=72

void tileToLatLon(double *lat /*緯度*/, double *lon /*経度*/, double x, double y, int z)
{
    double zz = pow(2.0, z);
    *lon = (x / zz) * 360.0 - 180.0;
    double mapy = (y / zz) * 2.0 * M_PI - M_PI;
    *lat = 2.0 * atan(pow(M_E, -mapy)) * 180.0 / M_PI - 90.0;
}

void latLonToTile(double *x, double *y, double lat, double lon, int z)
{
    double zz = pow(2.0, z);
    *x = ((lon / 180.0 + 1) * zz / 2.0);
    *y = (((-log(tan((45.0 + lat / 2.0) * M_PI / 180.0)) + M_PI) * zz / (2.0 * M_PI)));
}
