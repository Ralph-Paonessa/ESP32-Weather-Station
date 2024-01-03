# How Charts are Added to Web Pages

*Charts are produced using publically-available code from Highcharts that is 
free for personal use.*

## chart.html

  - chart.html is a page with an empty chart with the following placeholders 
  for dynamic properties that are populated by the processor for AsyncWebServer:
    
    1. CHART_Y_AXIS_LABEL
    1. CHART_TITLE
    1. Y_MIN
    1. Y_MAX
    1. Y_TICK_AMOUNT

  - Upon html load, javascript function getChartData(dataRoute) is called to 
  asynchronously request and display the appropriate chart data. The chart 
  page initially requests 10-min data (dataRoute = "/data_10"). Buttons on the 
  page allow the user to request 1-hr data and daily max/min values as well.
  
  - Depending on the chart data type (temperature, pressure, etc.), the web 
  server serverRouteHandler() parses this request and supplies data from the 
  appropriate sensor as a delimited string via 
  .data_10_min_string_delim(.

  - This data string is processed by chart.js, which creates an array of
  (time, value) data points that are fed to the highcharts data series in 
  chart.html, which plots the data.

  - The data string is a series of comma-separated "time,value" pairs 
  delimited by "~". Time is in javascript format as the integer number of 
  millisec from 1/1/1970.

  - The Hourly button in chart.html asynchronously loads the 
  60-min data into the chart. SensorData::data_60_min_string_delim().

  ## chart_min_max.html -- _UNDER DEVELOPMENT_

  - _The Daily button in chart.html should asynchronously load and plot 
  **both** the daily highs and daily lows into chart_min_max.html that can 
  display TWO data series._

  ### Mechanism for getting Daily Max/Min charts.
  **This is now in development.** *One option is to create a separate 
  chart_2.html set up to display two data series, along with chart_2.js to
  parse the delimited data string from a modified* **getChartData("/data_min_max")**.