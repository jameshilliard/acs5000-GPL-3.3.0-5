// Copyright © 2003 Art & Logic, Inc. All rights reserved.


// Get the URL for where the movie was served from.
// This URL is used for XML-RPC requests.
myURL = _root._url;
myURLArray = myURL.split("/user/graphs.swf");
myServer = myURLArray[0] + "/goform/RPC2";
// myServer = "http://localhost:8080/goform/RPC2"; // !!! for testing purposes

// Get the parameter labels
rpc = new xmlrpc(myServer);
params = rpc.setParameter("system.values.label");
rpc.callBack = getLabelsCallback;
rpc.send("get",[params]);

// Once the labels have been returned, initialize the charts.
function getLabelsCallback(errorCode,parameters)
{
	ordinary = rpc.getParameters(parameters);

	// Populate the combo boxes.
	for (i=0; i<16; i++)
	{
		myComboBox0.addItemAt(i,ordinary[0][i],i);
		myComboBox1.addItemAt(i,ordinary[0][i],i);
		myComboBox2.addItemAt(i,ordinary[0][i],i);
		myComboBox3.addItemAt(i,ordinary[0][i],i);
	}

	// Set the default combo boxes to 0,1,2,3.
	myComboBox0.setSelectedIndex(0);
	myComboBox1.setSelectedIndex(1);
	myComboBox2.setSelectedIndex(2);
	myComboBox3.setSelectedIndex(3);

	// Set up the change handlers for combo box components.
	myComboBox0.setChangeHandler("comboBoxChangeHandler0");
	myComboBox1.setChangeHandler("comboBoxChangeHandler1");
	myComboBox2.setChangeHandler("comboBoxChangeHandler2");
	myComboBox3.setChangeHandler("comboBoxChangeHandler3");

	// Initialize the default charts (in this case, 0,1,2,3)
	initChart(myChart0,ordinary[0][0]);
	initChart(myChart1,ordinary[0][1]);
	initChart(myChart2,ordinary[0][2]);
	initChart(myChart3,ordinary[0][3]);

	// Everything is set up. Begin polling.
	polling(true,2000);
}

// Add a new data point to a chart.
function updateChart(chart,index,ordinary)
{
	chart.removeItemAt(0);
	chart.addItem('',ordinary[0].value[index]);

	// just in case the chart title was modified in another window.
	chart.setChartTitle(ordinary[0].label[index]);

	if(ordinary[0].value[index] < ordinary[0].lowerAlarm[index]+1)
	{
		chart.setLineColor(0x0000FF);
	}
	else if (ordinary[0].value[index] > ordinary[0].upperAlarm[index]-1)
	{
		chart.setLineColor(0xFF0000);
	}
	else
	{
		chart.setLineColor(0x000000);
	}
}

// Each time the values are polled, update each chart.
getValuesCallback = function(errorCode,parameters)
{
	ordinary = rpc.getParameters(parameters);
	processChartData(myChart0,myComboBox0,index,ordinary);
	processChartData(myChart1,myComboBox1,index,ordinary);
	processChartData(myChart2,myComboBox2,index,ordinary);
	processChartData(myChart3,myComboBox3,index,ordinary);
}

// check whether the chart is still mapping the same parameter, then
// update the chart or re-initialize it.
function processChartData(chart,comboBox,index,ordinary)
{
	for (i=0;i<16;i++)
	{
		// have any of the labels changed? If so, update the comboBoxes.
		if (comboBox.getItemAt(i).label != ordinary[0].label[i])
		{
			comboBox.replaceItemAt(i,ordinary[0].label[i],i);
		}
	}
	updateChart(chart,comboBox.getValue(),ordinary)
}

// Get the values. Call this function at a regular interval.
getValues = function(server)
{
	rpc = new xmlrpc(server);
	params = rpc.setParameter("system.values");
	rpc.callBack = getValuesCallback;
	rpc.send("get",[params]);
}

function comboBoxChangeHandler0(component)
{
	initChart(myChart0,component.getSelectedItem().label);
}

function comboBoxChangeHandler1(component)
{
	initChart(myChart1,component.getSelectedItem().label);
}

function comboBoxChangeHandler2(component)
{
	initChart(myChart2,component.getSelectedItem().label);
}

function comboBoxChangeHandler3(component)
{
	initChart(myChart3,component.getSelectedItem().label);
}

// General set-up for the chart.
function initChart(chart,chartTitle)
{
	chart.setChartTitle(chartTitle);
	chart.setXAxisTitle("Time");
	chart.setYAxisTitle("Value");
	chart.setLineWeight(1);
	chart.removeAll(); // !!! move to for?
	chart.setLineColor(0x000000); // !!! move to for?
	for (i=0; i<24; i++)
	{
		// set or reset all points in the chart to zero.
		chart.addItemAt(i);
	}
}

// Timer functions (for polling)
function timerCallback()
{ 
		getValues(myServer);
}

function polling(start,interval)
{
	if (start)
	{
		intervalID = setInterval(timerCallback, interval);
	}
	else
	{
		clearInterval(intervalID);
	}
}
