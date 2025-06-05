#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Zarif's Room";
const char *password = "455279090d";

AsyncWebServer server(80);

String getContentType(const String &path)
{
  if (path.endsWith(".html"))
    return "text/html";
  if (path.endsWith(".css"))
    return "text/css";
  if (path.endsWith(".js"))
    return "application/javascript";
  if (path.endsWith(".png"))
    return "image/png";
  if (path.endsWith(".gif"))
    return "image/gif";
  if (path.endsWith(".jpg"))
    return "image/jpeg";
  if (path.endsWith(".ico"))
    return "image/x-icon";
  if (path.endsWith(".json"))
    return "application/json";
  if (path.endsWith(".svg"))
    return "image/svg+xml"; // SVG support added here
  return "text/plain";
}

void handleFileRequest(AsyncWebServerRequest *request)
{
  String path = request->url();

  // Handle root request
  if (path.endsWith("/"))
  {
    path = "/index.html";
  }

  // Check for gzip support
  bool gzipSupported = false;
  if (request->hasHeader("Accept-Encoding"))
  {
    const AsyncWebHeader *h = request->getHeader("Accept-Encoding");
    if (h)
    { // Added null check for safety
      gzipSupported = (h->value().indexOf("gzip") != -1);
    }
  }

  // Try gzipped version first if supported
  if (gzipSupported && LittleFS.exists(path + ".gz"))
  {
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, path + ".gz", getContentType(path));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
    return;
  }

  // Try uncompressed version
  if (LittleFS.exists(path))
  {
    request->send(LittleFS, path, getContentType(path));
    return;
  }

  // File not found - show directory listing
  String output = "File not found: " + path + "\n\nAvailable files:\n";
  Dir dir = LittleFS.openDir("/");
  while (dir.next())
  {
    output += dir.fileName();
    output += " (" + String(dir.fileSize()) + " bytes)\n";
  }
  request->send(404, "text/plain", output);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\nStarting...");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to %s", ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize filesystem
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  // Set up server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->redirect("/index.html"); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { handleFileRequest(request); });

  // Start server
  server.begin();
  Serial.println("HTTP server started");

  // Debug: List files in LittleFS
  Dir dir = LittleFS.openDir("/");
  Serial.println("\nFiles in LittleFS:");
  while (dir.next())
  {
    Serial.printf("  %s (%d bytes)\n", dir.fileName().c_str(), dir.fileSize());
  }
}

void loop()
{
  // Async server handles everything automatically
  // You might add non-blocking periodic tasks here
}