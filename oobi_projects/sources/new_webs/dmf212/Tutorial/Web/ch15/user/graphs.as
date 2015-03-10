// Copyright © 2003 Art & Logic, Inc. All rights reserved.


// Hide the chart while we draw it.
hideChart();

// This is used to improve the drawing in getValuesCallback();
dataPointsUsed = 0;

// Get the URL for where the movie was served from.
// This URL is used for XML-RPC requests.
myURL = _root._url;
myURLArray = myURL.split("/user/graphs.swf");
myServer = myURLArray[0] + "/goform/RPC2";
// myServer = "http://localhost:8080/goform/RPC2"; // !!! for testing purposes

// Get the localized text strings from a DMF .asp file.
loadVariablesNum("FlashText.asp",0);

// Populate the list box with blank entries.
for (i=0; i<16; i++)
{
	myListBox0.addItemAt(i," ",i);
}

// Define the change handler for list box component.
myListBox0.setChangeHandler("listBoxChangeHandler0");

// Initialize the chart.
myChart0.setLineWeight(1);
myChart0.setLineColor(0x000000);

// The axis labels are blank for now, because the localized
// .asp file probably hasn't been recieved yet. Using a blank
// space helps to avoid shifting around during drawing.
myChart0.setXAxisTitle(" ");
myChart0.setYAxisTitle(" ");

// Set the the list box default setting to the first item.
myListBox0.setSelectedIndex(0);

// Initialization is complete. Begin polling via XML-RPC
// at regular intervals.
polling(true,750);

// This function gets called whenever the selected item
// in the list box changes.
function listBoxChangeHandler0(component)
{
	hideChart();
	myChart0.removeAll();
	myChart0.setLineColor(0x000000);
	dataPointsUsed = 0;
}

// Each time the values are polled, update the chart. This is
// the callback function for getValues();
getValuesCallback = function(errorCode,parameters)
{
	ordinary = rpc.getParameters(parameters);
	myParamTree = ordinary[0];

	listBoxValue = myListBox0.getValue();

	if (myChart0.getLength() < 34)
	{
		for (i=0; i<33; i++)
		{
			myChart0.addItemAt(i);
		}
		myChart0.addItem('',myParamTree.value[listBoxValue]);
	}

	myChart0.removeItemAt(0);
	myChart0.addItem('',myParamTree.value[listBoxValue]);

	// have any of the labels changed? If so, update the list boxes.
	for (i=0;i<16;i++)
	{
		if (myListBox0.getItemAt(i).label != myParamTree.label[i])
		{
			myListBox0.replaceItemAt(i,myParamTree.label[i],i);
		}
	}

	// Update the title and alarm fields.
	myChart0.setChartTitle(myParamTree.label[listBoxValue]);
	myChart0.setXAxisTitle(xAxisLabel);
	myChart0.setYAxisTitle(yAxisLabel);
	lowerAlarm = myParamTree.lowerAlarm[listBoxValue];
	upperAlarm = myParamTree.upperAlarm[listBoxValue];

	if(myParamTree.value[listBoxValue] < myParamTree.lowerAlarm[listBoxValue]+1)
	{
		myChart0.setLineColor(0x0000FF);
	}
	else if (myParamTree.value[listBoxValue] > myParamTree.upperAlarm[listBoxValue]-1)
	{
		myChart0.setLineColor(0xFF0000);
	}
	else
	{
		myChart0.setLineColor(0x000000);
	}
	
	// Unhide the chart when there are two data points displayed.
	// This is a workaround for a drawing problem in the LineChart
	// component. If we unhide the chart earlier, the drawing of
	// the chart is less than perfect.
	dataPointsUsed++;
	if (dataPointsUsed == 2)
	{
		_root.grad.removeMovieClip();
	}
}

// Get the values. The polling() function calls this function
// at a regular interval.
getValues = function(server)
{
	rpc = new xmlrpc(server);
	params = rpc.setParameter("system.values");
	rpc.callBack = getValuesCallback;
	rpc.send("get",[params]);
}

// Timer functions (for polling)
function pollingCallback()
{ 
		getValues(myServer);
}

function polling(start,interval)
{
	if (start)
	{
		intervalID = setInterval(pollingCallback, interval);
	}
	else
	{
		clearInterval(intervalID);
	}
}

// This function is taken from the example in the Macromedia documentation.
// It works, but there is probably a better way to hide the chart.
function hideChart()
{
	_root.createEmptyMovieClip( "grad", 1 );
	with ( _root.grad )
	{
		alphas = [ 100, 100 ];
		ratios = [ 0, 0xFF ];
		matrix = { a:200, b:0, c:0, d:0, e:200, f:0, g:200, h:200, i:1 };
		
		colors = [ 0xCCCCCC, 0xCCCCCC ];
		beginGradientFill( "linear", colors, alphas, ratios, matrix );
		moveto(169,1);
		lineto(169,238);
		lineto(478,238);
		lineto(478,1);
		lineto(169,1);
		endFill();

		colors = [ 0xFFFFFF, 0xFFFFFF ];
		beginGradientFill( "linear", colors, alphas, ratios, matrix );
		moveto(164,242);
		lineto(164,269);
		lineto(482,269);
		lineto(482,242);
		lineto(164,242);
		endFill();
	}
}
