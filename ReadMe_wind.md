# Making Meaningful Wind Measurements

## ***This document is only a collection of notes that needs to be re-written.***

// WindSpeed.h
// v 1.1
// Ralph Paonessa
// 3/9/2022

/*
Timer interrupt signals when to get "raw" wind speed,
which is average over brief BASE_PERIOD, which is shortest
period we can effectively measure. These small periods
are necessary to look for brief gusts.

  GOALS  //////////////////////

	1. Get "instantaneous" wind speed over a specified time of
		rotations (BASE_PERIOD, e.g., 2.5 sec).
	2. Calculate average wind speed over a specified time. E.g., 10 min.
	3. Determine wind gusts and variance during the wind averaging time.
	4. Log or report all sensor data after a specified number of wind
			speed measurements.

 Standard to follow
	- measure raw speeds over BASE_PERIOD 2-5 sec. (This will not be
			public.)
	- Get maxWindSpeed as max of speed over 10 min.
	- Get avgWindSpeed over 2 min.
	- Get maxWindSpeed over 2 min.
	- report speed as average of avgWindSpeed over a reporting
			time of 10 min.
	- report wind gust as max wind speed during current reporting time.


	HOW WIND SPEED IS MEASURED [Ref. 1]

	Defines: "WIND SPEED", "WIND GUST", "SUSTAINED WIND", "PEAK WIND"

	"WIND SPEED"
	Average speed for the most recent 2 minute time. This is considered
	the "SUSTAINED WIND."

	The "WIND SPEED" reported in each dp is an average speed for
	the most recent two-minute time prior to the dp time. This
	is also considered the "SUSTAINED WIND" for routine surface
	observations. This two minute average is calculated	from a series of
	24 five-second average values.

	A "WIND GUST" is also reported when the PEAK "INSTANTANEOUS WIND"
	during the most recent ten-minutes prior to the dp is more
	than 10 knots greater than the lowest "lull" in the wind during that
	time. If that is the case, the highest INSTANTANEOUS WIND during that
	ten minute window is reported as the GUST value.

	If the MAXIMUM INSTANTANEOUS WIND SPEED during the entire time since
	the last reported dp exceeds 25 knots (28.77 mph), that value
	is reported	as the "PEAK WIND." It will be at least equal to the
	reported gust,and may be higher than the reported gust if the "peak"
	occurred more than 10 minutes prior to the dp time.

	-----------------------------------------------------------

	WIND GUSTS [Ref. 2]

	According to U.S. weather observing practice, gusts are reported when the
	"PEAK WIND SPEED"(*) reaches at least 16 knots (18.41 mph) and the variation
	in wind speed between the peaks and lulls is at least 9 knots (10.36 mph).
	The duration of a gust is usually less than 20 seconds.

	(*)NOTE: "PEAK WIND SPEED" [2] <---> PEAK "INSTANTANEOUS WIND" [1]

________________

	[2] https://graphical.weather.gov/definitions/defineWindGust.html

**  MY TAKE FROM [1] et al.  **

There is some ambiguity or flexibility regarding how often to average
and report a wind speed. But it seems that the shortest practical period
might be 2 minutes. However, "instantaneous" wind speed measurements
are necessary to determine gusts.

Because it is impractical to maintain records of the speed at 2-min
intervals, it is up to the user to decide what frequency to save/report. For
the NWS, this appears to be 10-min or 60-min intervals, with the latter
being what is actually saved as archival weather data.

In our case, we wish to present recent data to users as graphs over the
following intervals:
	10-min
	60-min
	12-hr (which can be delimited as DAY and NIGHT)

These can be used to report trends, such as daily maximum and minimum.

2-min intervals are used internally, but not reported to the user.


1. Measure wind over short BASE_PERIOD (2-5 sec) --> "RAW SPEED"

2. Average RAW SPEEDs over 2-min period --> "SUSTAINED WIND" aka "REPORTED
	WIND SPEED" aka "2-MIN-AVG"

3. WIND GUST determined from PEAK INSTANTANEOUS WIND aka RAW WIND SPEED.

4.


HOW TO IMPLEMENT:

1. Measure wind over short BASE_PERIOD (2-5 sec) --> (speedInstant). Check for MAX (_speedMax_10_min).

2. Average RAW SPEED over 2-min period -->  2-MIN-AVG  (SUSTAINED WIND aka REPORTED WIND SPEED)

3. 10-min interval
	Get average_10_min from _speeds_2_min list.
	Get gust as max during last 10-min (_speedMax_10_min).

3. Find max every 10-min, save as gust

3. Calc 10-min avg    average_10_min from last five _speeds_2_min[]

4. 60-min, 12-hr intervals
	Avg and add to lists.



	IMPLEMENTATION
	INST_SPEED = anemometer output over 2.5 seconds. (Not reported.)
	SPEED = *2 minute* average OF INST_SPEED   (48 cycles x 2.5 sec).
	GUST: Occurs if (MAX Inst. WindSpeed) during most recent 10 minute
	time is at least 10 knots greater than (MIN Inst. WindSpeed) during time.

	If (MAX - MIN) >= 10, then GUST = MAX.   // over 10 min time

	- Report readings every *2 min*
	- Accumulate values over *10 min*
	- Look at past *10 min* to get gusts
	- Summarize data every *1hour*
	- Maintain data in memory for last N hours, to show trends.



 Data will be logged every ( countPeriod * numForAvgWindSpeed ) sec.
 Not worthwhile to report "instantaneous" wind speed measured every
 few seconds.
	- Clutters up data.
	- WindSpeed can be highly variable, therefore misleading.
 Wind is averaged over ( countPeriod * numForAvgWindSpeed ) seconds.
 Some other data should be averaged, such as wind directionCardinal, strength.

 When someone requests data, they get the last logged data, which is
 at most X min old?
 Can also include most recent measurements?
*/

/*

What is optimum wind speed sampling frequency?
	Too low		-->	Lowest speeds inaccurate.
					But, miss brief high gusts.
	Too high	--> Excess processing required.

----------------------------------------------------
SAMPLE WIND SPEED MEASUREMENT VALUES
----------------------------------------------------
WindSpeed	Freq.		Counts		Counts		Counts
mph		Hz			in 0.25s	in 2.5s		in 120s
-----	------		--------	--------	--------
1		0.44		0.1			1.1			53
2		0.89		0.2			2.2			107
3		1.33		0.3			3.3			160
4		1.78		0.4			4.4			213
5		2.22		0.6			5.6			267
6		2.67		0.7			6.7			320
7		3.11		0.8			7.8			373
8		3.56		0.9			8.9			427
9		4.00		1.0			10.0		480
10		4.44		1.1			11.1		533
11		4.89		1.2			12.2		587
12		5.33		1.3			13.3		640
13		5.78		1.4			14.4		693
14		6.22		1.6			15.6		747
15		6.67		1.7			16.7		800
20		8.89		2.2			22.2		1067
30		13.3		3.3			33.3		1600
40		17.8		4.4			44.4		2133
50		22.2		5.6			55.6		2667
60		26.7		6.7			66.7		3200
70		31.1		7.8			77.8		3733
80		35.6		8.9			88.9		4267
90		40.0		10.0		100.0		4800
100		44.4		11.1		111.1		5333
125		55.6		13.9		138.9		6667
150		66.7		16.7		166.7		8000
175		77.8		19.4		194.4		9333
200		88.9		22.2		222.2		10667
---------------------------------------------------
*/

/*
NOTE: We can compile the various average_N values into
lists that can be called by the program.

The calling program MUST HANDLE THE TIMING of processing to
produce these lists (BASE_PERIOD, 2-min, 10-min, ... ) by calling
	addReading()
	process_data_2_min()
	process_data_10_min()
	...

The calling program will call these lists when it:
	- Losg data to SD and or serial monitor at N-minute intervals.
	- Populates a web request with data.

For each N-min time, must
	- Calculate N-min avg
	- Add avg to list_N
	- Determine gust (max) in time

	NOTE: Handling GUSTS is what makes wind different from other
	sensor data. It requires examing the INSTANTANEOUS speeds over
	short time periods, and looking for maxima.



10/18/2022

SPEED
	For current data reporting, use 10-min avg.

	Can keep averaging speed for 10 minutes, save this, then reset.

	For every instantaneous reading, hold min and max (for gust calc).

GUSTS
	For current data reporting, use max over 10-min.

	Find gust when max exceeds avg (and max exceeds min by threshold).

	Save gust every 10 min, and reset max, min.

