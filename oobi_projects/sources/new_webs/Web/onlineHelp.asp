<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
    <title>Untitled</title>
    <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript">

    var page = '<%getOnlineHelp();%>';
<%
	var model = getVal('_familymodel_');
	var ext = getVal('_familyext_');
	var analog = getVal("system.device.kvmanalog");
	var version = getVal("system.device.olhVersion");
	var sx_prod = "undefined";
	var acs5k = getVal('_acs5k_');

	if ((model == "ACS")||(model == "AVCS")) {
		sx_prod = getLabelVal("help");
	}
	if (model == "ONS") {
		sx_prod = "ons";
	}
	if (model == "KVM") {
		sx_prod = "kvm";
		if (analog == 0) {
			sx_prod = "kvmnet";
		}
		if (ext == "KVMNETP") {
			sx_prod = "kvmnetplus";
		}
	}
%>

// If the last char in the configured link is '/', then concatenates 
// product name and version as directories and index.html. (help/en/index.html for AutoView CS)
// Otherwise just redirect to the configured file.
    if (page.charAt(page.length-1) == '/') {
        var version = '<%write(version);%>';
        var sx_ver = version.split(" ");

		version = sx_ver[0].toLowerCase();

		ix = version.lastIndexOf("-");
		if (ix > 1) {
			version = version.substring(0, ix);
		}
       	if (version.charAt(version.length-1) < '0' ||
    		version.charAt(version.length-1) > '9') {
			version = version.substring(0, version.length-1);
		}

        location.href = page + '<%write(sx_prod);%>' + "/" + version + "/en/index.html";
    } else {
        location.href = page;
    }

    </script>
</head>

<body class="bodyMain">
</body>
</html>
