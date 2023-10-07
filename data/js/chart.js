// Gets data_point (time, value) pairs for a chart from 
// the server at dataRoute and adds them to a chart.
function getChartData(dataRoute, intervalMins, timeOffsetHrs) {
    // Reset all data in series[0]
    chart_1.series[0].setData([]);
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            // Split into array of (time, value)
            const data_points = this.responseText.split("~");
            console.log(data_points);
            // JS time is in millisec from 1/1/1970.
            // Data is in seconds.
            const MILLISECONDS_PER_SECOND = 1000;
            for (let i = 0; i < data_points.length; i++) {
                const data_point = data_points[i].split(","); // split into time, value
                //console.log("data_point[" + i + "] " + data_point);
                chart_1.series[0].addPoint([parseInt(data_point[0] * MILLISECONDS_PER_SECOND), parseFloat(data_point[1])], true, false, true);
            }
        }
    };
    xhttp.open("GET", dataRoute, true);
    xhttp.send();
}
