// Gets high and low data_point (time, value) pairs for a chart from 
// the server at dataRoute and adds them to a chart.
function getChartData(dataRoute, intervalMins, timeOffsetHrs) {
    
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {

            // Split into arrary of high and low delimited list strings.
            const lists = this.responseText.split("|");

            console.log(lists);

            // Parse each list and add it to a series in the chart.
            for (var i_list = 0; i_list < lists.length; i_list++)
            {

                // Reset all data in series[i_list]
                chart_1.series[i_list].setData([]);

                // Split into array of (time,value) strings.
                const data_points = lists[i_list].split("~");
                console.log(data_points);
                // JS time is in millisec from 1/1/1970.
                // Data is in seconds.
                const MILLISECONDS_PER_SECOND = 1000;
                for (let i_point = 0; i_point < data_points.length; i_point++)
                {
                    const data_point = data_points[i_point].split(","); // split into time, value
                    //console.log("data_point[" + i_point + "] " + data_point);
                    chart_1.series[i_list].addPoint([parseInt(data_point[0] * MILLISECONDS_PER_SECOND), parseFloat(data_point[1])], true, false, true);
                }
            }
        }
    };
    xhttp.open("GET", dataRoute, true);
    xhttp.send();
}
