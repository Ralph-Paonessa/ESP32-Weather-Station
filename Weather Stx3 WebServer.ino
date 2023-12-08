//// ==========   Async Web Server   ================== //
//AsyncWebServer server(80);	// Async web server instance on port 80.

chartRequested _chart_request = CHART_NONE;	// Chart requested from server.

bool _isChart_max_min = false;	// True when chart from server is max/min.

/// <summary>
/// Defines uri routes for async web server.
/// </summary>
void serverRouteHandler() {
	/*
	Configure url routes where server will be listening. Route in url
	is: "http://[ IP Address ][Route]",	such as [Route]= "/data".

	Serving files from both LittleFS (flash memory) and SD (SD card).

	STATIC FILES:

		"Static" files are generally files that are "not changed
		by the server" such as css, js, and non-dynamic html pages.

		Usage:
			Format is server.serveStatic([ Route ], [ File system ], [ File path ]);

		Example:
		To serve the file "/dir/page.htm" when request url is "/page.htm":

			server.serveStatic("/page.htm", LittleFS, "/dir/page.htm");

	DYNAMIC FILES:

		Files can contain templates of the form	%REPLACE_ME% that are
		replaced by a processor function attached to the request.

		server.on adds a new instance of AsyncStaticWebHandler
		to the server to handle the specified file.

		(Note: ESPAsyncWebServer ReadMe says you can use template processor
		with "static" files. This seems contradictory; here, I refer to
		them as dynamic files.)

		Usage:
			server.on([ Route ], HTTP_GET, [](AsyncWebServerRequest* request)
				{
				request->send([ File system ], [ File path ] , [ Mime type ], [ isDownload ], [ template processor ]);
				});

		Example:
			server.on("/Admin", HTTP_GET, [](AsyncWebServerRequest* request)
				{
				request->send(LittleFS, "/html/Admin.html", "text/html", false, processor);
				});

		IMPORTANT: false is for bool download; otherwise browser will
		download file instead of display it!


	SETTING CACHE FOR STATIC FILES:

		Usage:
			server.serveStatic([Route], [File system], [File path]).setCacheControl("max-age=[seconds]");

		Example:
			server.serveStatic("/highcharts.css", LittleFS, "/css/highcharts.alt.css").setCacheControl("max-age=864000");

			10 days = 864,000 seconds
			 2 days = 172,800 seconds
			 1 day  =  86,400 seconds
	*/

#if defined(VM_DEBUG)
	if (!_isDEBUG_BypassWebServer) {
#endif
		// Set cache for static files.
		// css
		server.serveStatic("/highcharts.css", LittleFS, "/css/highcharts.alt.css").setCacheControl("max-age=864000");
		server.serveStatic("/highcharts-custom.css", LittleFS, "/css/highcharts-custom.css").setCacheControl("max-age=864000");
		server.serveStatic("/style.light.min.css", LittleFS, "/css/style.light.min.css").setCacheControl("max-age=864000");
		server.serveStatic("/style.min.css", LittleFS, "/css/style.min.css").setCacheControl("max-age=864000");
		// js
		server.serveStatic("/highcharts.js", LittleFS, "/js/highcharts.js").setCacheControl("max-age=864000");
		server.serveStatic("/chart.js", LittleFS, "/js/chart.js").setCacheControl("max-age=864000");
		//server.serveStatic("/chart_2.js", LittleFS, "/js/chart_2.js").setCacheControl("max-age=864000");
		// img
		server.serveStatic("/chart-icon.png", LittleFS, "/img/chart-icon-red-150px.png").setCacheControl("max-age=864000");
		server.serveStatic("/home-icon.png", LittleFS, "/img/home-icon-red-150px.png").setCacheControl("max-age=864000");
		server.serveStatic("/img/loading.gif", LittleFS, "/img/loading.gif").setCacheControl("max-age=864000");
		server.serveStatic("/favicon-32.png", LittleFS, "/img/favicon-32.png").setCacheControl("max-age=864000");
		server.serveStatic("/favicon-32.180", LittleFS, "/img/favicon-32.180").setCacheControl("max-age=864000");
		// html 
		// Our html pages are dynamic and can't be cached.

		/*****  WEB PAGES.  *****/

		// Default.
		server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/html/sensors.html", "text/html", false, processor);
			});

		// Summary data.
		server.on("/summary", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/html/summary.html", "text/html", false, processor);
			});

		// GPS info.
		server.on("/gps", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/html/gps.html", "text/html", false, processor);
			});

		// Admin page.
		server.on("/Admin", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/html/Admin.html", "text/html", false, processor);
			});

		// Log file from SD card.
		server.on("/log", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(SD, "/log.txt", "text/plain");
			});

		// Data file from SD card.
		server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(SD, "/data.txt", "text/plain");
			});

		/*****  GRAPH PAGES.  *****/

		// Pages use javascript to get data asynchronously.

		// Temperature graph page.
		server.on("/chart_T", HTTP_GET, [](AsyncWebServerRequest* request) {
			_chart_request = CHART_TEMPERATURE_F;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Wind speed graph page.
		server.on("/chart_W", HTTP_GET, [](AsyncWebServerRequest* request) {
			_chart_request = CHART_WIND_SPEED;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Wind gust graph page.
		server.on("/chart_Wgst", HTTP_GET, [](AsyncWebServerRequest* request) {
			_chart_request = CHART_WIND_GUST;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Wind direction graph page.
		server.on("/chart_Wdir", HTTP_GET, [](AsyncWebServerRequest* request) {
			_chart_request = CHART_WIND_DIRECTION;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Pressure (sea level) graph page.
		server.on("/chart_P", HTTP_GET, [](AsyncWebServerRequest* request) {
			_chart_request = CHART_PRESSURE_SEA_LEVEL;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Relative Humidity graph page.
		server.on("/chart_RH", HTTP_GET, [](AsyncWebServerRequest* request) {
			_chart_request = CHART_RELATIVE_HUMIDITY;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Sky Infrared graph page.
		server.on("/chart_IR", HTTP_GET, [](AsyncWebServerRequest* request) {
			_chart_request = CHART_IR_SKY;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// UV Index graph page.
		server.on("/chart_UVIndex", HTTP_GET, [](AsyncWebServerRequest* request) {
			_chart_request = CHART_UV_INDEX;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Insolation graph page.
		server.on("/chart_Insol", HTTP_GET, [](AsyncWebServerRequest* request) {
			_chart_request = CHART_INSOLATION;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		/*****  Images.  *****/

		server.on("/favicon-32.png", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/favicon-32.png", "image/png");
			});
		server.on("/favicon-180.png", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/favicon-180.png", "image/png");
			});
		server.on("/chart-icon.png", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/chart-icon-red-150px.png", "image/png");
			});
		server.on("/home-icon.png", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/home-icon-red-150px.png", "image/png");
			});
		server.on("/img/loading.gif", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/loading.gif", "image/gif");
			});

		/*****  CSS and Javascript.  *****/

		// CSS style sheet.
		server.on("/style.min.css",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/css/style.min.css", "text/css");
			});
		// CSS light-theme style sheet.
		server.on("/style.light.min.css",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/css/style.light.min.css", "text/css");
			});
		// Highcharts css style sheet.
		server.on("/highcharts.css",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/css/highcharts.alt.css", "text/css");
			});
		// Highcharts customized css style sheet.
		server.on("/highcharts-custom.css",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/css/highcharts-custom.css", "text/css");
			});
		// highcharts javascript file.
		server.on("/highcharts.js",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/js/highcharts.js", "text/javascript");
			});
		// highcharts custom javascript file.
		server.on("/chart.js",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/js/chart.js", "text/javascript");
			});

		/*****  DATA SOURCES FOR GRAPHS  ***********************************
		/*
			 Asynchronously Send string with data to html
			 page where Javascript parses and plots the data.
		*/

		/*****  10-MIN CHARTS  *****/

		server.on("/data_10", HTTP_GET,
			[](AsyncWebServerRequest* request) {

				_isChart_max_min = false;

				// Which chart?
				switch (_chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.data_10_min_string().c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.data_10_min_string().c_str());
					break;
				case CHART_TEMPERATURE_F:
					request->send_P(200, "text/plain", d_Temp_F.data_10_min_string().c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.data_10_min_string().c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.data_10_min_string().c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.data_10_min_string().c_str());
					break;
				case CHART_WIND_DIRECTION:
					request->send_P(200, "text/plain", windDir.data_10_min_string().c_str());
					break;
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.data_10_min_string().c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windGust.data_10_min_string().c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});

		/*****  60-MIN CHARTS  *****/

		server.on("/data_60", HTTP_GET,
			[](AsyncWebServerRequest* request) {

				_isChart_max_min = false;

				// Which chart?
				switch (_chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.data_60_min_string().c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.data_60_min_string().c_str());
					break;
				case CHART_TEMPERATURE_F:
					request->send_P(200, "text/plain", d_Temp_F.data_60_min_string().c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.data_60_min_string().c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.data_60_min_string().c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.data_60_min_string().c_str());
					break;
				case CHART_WIND_DIRECTION:
					request->send_P(200, "text/plain", windDir.data_60_min_string().c_str());
					break;
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.data_60_min_string().c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windGust.data_60_min_string().c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});

		/*****  DAILY MIN MAX CHARTS  *****/

		server.on("/data_max_min", HTTP_GET,
			[](AsyncWebServerRequest* request) {

				_isChart_max_min = true;

				// Which chart?
				switch (_chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.data_dayMaxMin_string().c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.data_dayMaxMin_string().c_str());
					break;
				case CHART_TEMPERATURE_F:
					request->send_P(200, "text/plain", d_Temp_F.data_dayMaxMin_string().c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.data_dayMaxMin_string().c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.data_dayMaxMin_string().c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.data_dayMaxMin_string().c_str());
					break;
					/*case CHART_WIND_DIRECTION:
						request->send_P(200, "text/plain", windDir.data_max_min_string_delim().c_str());
						break;*/
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.data_dayMaxMin_string().c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windGust.data_dayMaxMin_string().c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});

		/*****  DAILY MAXIMA CHARTS  *****/

		server.on("/data_max", HTTP_GET,
			[](AsyncWebServerRequest* request) {
				// Which chart?
				switch (_chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.data_dayMax_string().c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.data_dayMax_string().c_str());
					break;
				case CHART_TEMPERATURE_F:
					request->send_P(200, "text/plain", d_Temp_F.data_dayMax_string().c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.data_dayMax_string().c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.data_dayMax_string().c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.data_dayMax_string().c_str());
					break;
				case CHART_WIND_DIRECTION:
					request->send_P(200, "text/plain", windDir.data_dayMax_string().c_str());
					break;
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.data_dayMax_string().c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windGust.data_dayMax_string().c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});

		/*****  DAILY MINIMA CHARTS  *****/

		server.on("/data_min", HTTP_GET,
			[](AsyncWebServerRequest* request) {
				// Which chart?
				switch (_chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.data_dayMin_string().c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.data_dayMin_string().c_str());
					break;
				case CHART_TEMPERATURE_F:
					//request->send_P(200, "text/plain", d_Temp_F.data_dayMin_string().c_str());
					request->send_P(200, "text/plain", d_Temp_F.data_dayMin_string().c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.data_dayMin_string().c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.data_dayMin_string().c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.data_dayMin_string().c_str());
					break;
				case CHART_WIND_DIRECTION:
					request->send_P(200, "text/plain", windDir.data_dayMin_string().c_str());
					break;
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.data_dayMin_string().c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windGust.data_dayMin_string().c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});

#if defined(VM_DEBUG)
}
	else {
		Serial.println("BYPASSING WEB SERVER INITIALIZATION");
	}
#endif
}
