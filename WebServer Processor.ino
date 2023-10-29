// ==========   SERVER DYNAMIC HTML   ================ //

/// <summary>
/// Replaces %PLACEHOLDER% elements in 
/// files served from async web server.
/// </summary>
/// <param name="var">Placeholder identifier.</param>
/// <returns>String substituted for placeholder.</returns>
String processor(const String& var) {

	/// CSS LIGHT STYLE DURING DAY  //////////////

	if (var == "CSS_LIGHT_STYLE") {
		// Switch display theme when ambient light is,
		// detected by normalized insolation %.
		if (d_Insol.valueLastAdded() > 0.01) {
			return "<link href = ""style.light.min.css"" rel = ""stylesheet"" media = ""all"" type = ""text/css"" />";
		}
		else {
			return "";
		}
	}

	///  CURRENT SENSOR READINGS  ///////////////////

	// Returns running avg of current 10-min 
	// period (except for gust and wind dir).

	if (var == "LAST_READINGS_DATETIME") {
		return gps.dateTime();
	}
	if (var == "CURRENT_TIME") {
		return gps.time();
	}
	if (var == "WEEKDAY") {
		return String(gps.dayName());
	}
	if (var == "TEMPERATURE_F")
		return String(d_Temp_F.avg_now(), 0);
	if (var == "WIND_SPEED") {
		return String(windSpeed.avg_now(), 0);	// 10-min avg
	}
	if (var == "WIND_GUST") {
		return String(windGust.max_10_min().value, 0);	// 10-min max for gusts
	}
	if (var == "WIND_DIRECTION") {
		return String(windDir.directionCardinal());		// avg since last cleared (<= 10 min)
	}
	if (var == "WIND_ANGLE") {
		return String(windDir.angleAvg_now(), 0);		// avg since last cleared (<= 10 min)
	}
	if (var == "GPS_ALTITUDE") {
		return String(gps.data.altitude(), 0);
	}
	if (var == "PRESSURE_MB_SL") {
		return String(d_Pres_seaLvl_mb.avg_now(), 0);
	}
	if (var == "PRESSURE_MB_ABS") {
		return String(d_Pres_mb.avg_now(), 0);
	}
	if (var == "WATER_BOILING_POINT") {
		return String(waterBoilingPoint_F(d_Pres_mb.avg_now()), 0);
	}
	if (var == "INSOLATION_PERCENT") {
		return String(d_Insol.avg_now(), 0);
	}
	if (var == "REL_HUMIDITY") {
		return String(d_RH.avg_now(), 0);
	}
	if (var == "UV_A") {
		if (_isGood_UV) {
			return String(d_UVA.avg_now(), 0);
		}
		else {
			return String("na");
		}
	}
	if (var == "UV_B") {
		if (_isGood_UV) {
			return String(d_UVB.avg_now(), 0);
		}
		else {
			return String("na");
		}
	}
	if (var == "UV_INDEX") {
		//if (_isGood_UV) {
		return String(d_UVIndex.avg_now(), 1);
		//}
		//else {
		//	return String("na");
		//}
	}
	if (var == "IR_T_SKY") {
		return String(d_IRSky_C.avg_now(), 0);
	}

	///  DAILY MAXIMA  ///////////////////

	if (var == "TEMPERATURE_F_HI") {
		return String(d_Temp_F.max_today().value, 0);
	}
	if (var == "WIND_SPEED_HI") {
		return String(windSpeed.max_today().value, 0);
	}
	if (var == "WIND_GUST_HI") {
		return String(windGust.max_today().value, 0);
	}
	if (var == "WIND_ANGLE_HI") {
		return "??";		// avg since last cleared (<= 10 min)
	}
	if (var == "PRESSURE_MB_SL_HI") {
		return String(d_Pres_seaLvl_mb.max_today().value, 0);
	}
	if (var == "INSOLATION_PERCENT_HI") {
		return String(d_Insol.max_today().value, 0);
	}
	if (var == "REL_HUMIDITY_HI") {
		return String(d_RH.max_today().value, 0);
	}
	if (var == "UV_A_HI") {
		if (_isGood_UV) {
			return String(d_UVA.max_today().value, 0);
		}
		else {
			return String("na");
		}
	}
	if (var == "UV_B_HI") {
		if (_isGood_UV) {
			return String(d_UVB.max_today().value, 0);
		}
		else {
			return String("na");
		}
	}
	if (var == "UV_INDEX_HI") {
		if (_isGood_UV) {
			return String(d_UVIndex.max_today().value, 1);
		}
		else {
			return String("na");
		}
	}
	if (var == "IR_T_SKY_HI") {
		return String(d_IRSky_C.max_today().value, 0);
	}

	///  DAILY MINIMA  ///////////////////

	if (var == "TEMPERATURE_F_LO") {
		return String(d_Temp_F.min_today().value, 0);
	}
	if (var == "WIND_SPEED_LO") {
		return String(windSpeed.min_today().value, 0);	// 10-min avg
	}
	if (var == "WIND_GUST_LO") {
		return String(windGust.min_today().value, 0);
	}
	if (var == "PRESSURE_MB_SL_LO") {
		return String(d_Pres_seaLvl_mb.min_today().value, 0);
	}
	if (var == "REL_HUMIDITY_LO") {
		return String(d_RH.min_today().value, 0);
	}
	if (var == "IR_T_SKY_LO") {
		return String(d_IRSky_C.min_today().value, 0);
	}

	///  GPS DATA   ////////////////////////

	if (var == "GPS_IS_SYNCED") {
		if (gps.isSynced())
			return String("Synced");
		else
			return String("Not Synced");
	}
	if (var == "GPS_LOCATIONS_UPDATE_COUNTER") {
		return String(gps.cyclesCount());
	}
	if (var == "GPS_LATITUDE") {
		return String(gps.data.latitude(), 6);
	}
	if (var == "GPS_LONGITUDE") {
		return String(gps.data.longitude(), 6);
	}
	if (var == "GPS_ALTITUDE") {
		return String(gps.data.altitude());
	}
	if (var == "GPS_DATE") {
		return String(gps.date_UTC_GPS());
	}
	if (var == "GPS_TIME") {
		return String(gps.time_UTC_GPS());
	}
	if (var == "GPS_UTC_OFFSET") {
		return String(UTC_OFFSET_HOURS);
	}
	if (var == "GPS_DAYLIGHT_TIME_USED") {
		return bool_Yes_No(IS_DAYLIGHT_TIME);
	}
	if (var == "GPS_HDOP") {
		return String(gps.data.HDOP() / 100.);
	}
	if (var == "GPS_SATELLITES") {
		return String(gps.data.satellites());
	}
	if (var == "ELAPSED_TIME_STRING") {
		return   String(gps.data.timeToSync_sec(), 2);
	}
	if (var == "FAN_RPM") {
		return String(d_fanRPM.valueLastAdded());
	}

	/// CHART FIELDS  //////////////////////////////////////////////



	if (var == "TIME_OFFSET_HOURS") {
		if (IS_DAYLIGHT_TIME) {
			return String(UTC_OFFSET_HOURS + 1);
		}
		else {
			return  String(UTC_OFFSET_HOURS);
		}
	}


	/// Y-AXIS LABEL  //////////////

	if (var == "CHART_Y_AXIS_LABEL") {
		// Based on chart requested.
		switch (_chart_request)
		{
		case CHART_NONE:
			return "Chart not specified!";
		case CHART_INSOLATION:
			return String(d_Insol.label() + ", " + d_Insol.units_html());
		case CHART_IR_SKY:
			return String(d_IRSky_C.label() + ", " + d_IRSky_C.units_html());
		case CHART_TEMPERATURE_F:
			return String(d_Temp_F.label() + ", " + d_Temp_F.units_html());
		case CHART_PRESSURE_SEA_LEVEL:
			return String(d_Pres_seaLvl_mb.label() + ", " + d_Pres_seaLvl_mb.units());
		case CHART_RELATIVE_HUMIDITY:
			return String(d_RH.label() + ", " + d_RH.units_html());
		case CHART_UV_INDEX:
			return String(d_UVIndex.label() + ", " + d_UVIndex.units());
		case CHART_WIND_DIRECTION:
			return String(windDir.label() + ", " + windDir.units_html());
		case CHART_WIND_SPEED:
			return String(windSpeed.label() + ", " + windSpeed.units());
		case CHART_WIND_GUST:
			return String("Wind Gusts, " + windGust.units());
		default:
			return "Chart not found";
		}
	}

	/// CHART TITLE  //////////////

	if (var == "CHART_TITLE") {
		// Based on chart requested.
		switch (_chart_request)
		{
		case CHART_NONE:
			return "Chart not specified!";
		case CHART_INSOLATION:
			return String(d_Insol.label());
		case CHART_IR_SKY:
			return String(d_IRSky_C.label());
		case CHART_TEMPERATURE_F:
			return String(d_Temp_F.label());
		case CHART_PRESSURE_SEA_LEVEL:
			return String(d_Pres_seaLvl_mb.label());
		case CHART_RELATIVE_HUMIDITY:
			return String(d_RH.label());
		case CHART_UV_INDEX:
			return String(d_UVIndex.label());
		case CHART_WIND_DIRECTION:
			return String(windDir.label());
		case CHART_WIND_SPEED:
			return String(windSpeed.label());
		case CHART_WIND_GUST:
			return "Wind Gusts";
		}
	}

	/// Y-AXIS MIN  //////////////

	if (var == "Y_MIN") {
		// Based on chart requested.
		switch (_chart_request)
		{
		case CHART_NONE:
			return "min: -500";
		case CHART_INSOLATION:
			return "min: 0";
		case CHART_IR_SKY:
			return "min: -50";
		case CHART_TEMPERATURE_F:
			return "min: 0";
		case CHART_PRESSURE_SEA_LEVEL:
			return "min: 950";
		case CHART_RELATIVE_HUMIDITY:
			return "min: 0";
		case CHART_UV_INDEX:
			return "min: 0";
		case CHART_WIND_DIRECTION:
			return "min: 0";
		case CHART_WIND_SPEED:
			return "min: 0";
		case CHART_WIND_GUST:
			return "min: 0";
		default:
			return "min: -2000";
		}
	}

	/// Y-AXIS MAX  //////////////

	if (var == "Y_MAX") {
		// Based on chart requested.
		switch (_chart_request)
		{
		case CHART_NONE:
			return ", max: 500";
		case CHART_INSOLATION:
			return ", max: 100";
		case CHART_IR_SKY:
			return ", max: 50";
		case CHART_TEMPERATURE_F:
			return ", max: 100";
		case CHART_PRESSURE_SEA_LEVEL:
			return ", max: 1050";
		case CHART_RELATIVE_HUMIDITY:
			return ", max: 100";
		case CHART_UV_INDEX:
			return ", max: 20";
		case CHART_WIND_DIRECTION:
			return ", max: 360";
		case CHART_WIND_SPEED:
			return ", max: 50";
		case CHART_WIND_GUST:
			return ", max: 50";
		default:
			return ", max: 2000";
		}
	}

	/// Y-AXIS TICK AMOUNT  //////////////

	if (var == "Y_TICK_AMOUNT") {
		// Based on chart requested.
		switch (_chart_request)
		{
		case CHART_IR_SKY:
			return ", tickAmount: 5";
		case CHART_TEMPERATURE_F:
			return ", tickAmount: 5";
		case CHART_PRESSURE_SEA_LEVEL:
			return ", tickAmount: 3";
		case CHART_WIND_DIRECTION:
			return ", tickAmount: 2";
		case CHART_WIND_SPEED:
			return ", tickAmount: 6";
		case CHART_WIND_GUST:
			return ", tickAmount: 6";
		default:
			return "";
		}
	}
	return var + String(" not found");
}