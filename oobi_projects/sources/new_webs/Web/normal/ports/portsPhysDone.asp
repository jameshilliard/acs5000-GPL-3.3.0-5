<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<style type="text/css">
<!--
a {
	text-decoration: none;
}
-->
</style>	
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

	function ActionSubmit()
	{
		parent.frames[1].document.configForm.urlOk.value = "/normal/ports/physPorts.asp";
		parent.frames[1].document.configForm.elements[5].value = 2;
		parent.frames[1].submit();
	}
    </script>
</head>
<body class="bodyMainTabs" marginheight="0" marginwidth="0">
<table border="0" cellpadding="0" cellspacing="0" width="100%" align="center" height="26">
   <tr>
      <td class="bgColor6" align="center" nowrap valign="middle">
<table align="center" cellspacing="0" cellpadding="0" border="0" height="26">
<tr>
	<td><font class="lightPorts">
		<script type="text/javascript">
		var lst = top.selectedPorts.split(',');
		var str;
		if (lst.length > 2) {
			var i, left, right;
			str = "";
			left = parseInt(lst[0]);
			for (i=1; i<lst.length; i++) {
				right = parseInt(lst[i-1]);
				if (lst[i]-right > 1) {
					if (str.length) {
						str = str + ",";
					}
					str = str + left;
					if (right > left) {
						str = str + "-" + right;
					}
					left = lst[i];
				}
			}
			right = parseInt(lst[lst.length-1]);
			if (right-left == 0) {
				str = str + "," + right;
			} else {
				if (str.length) {
					str = str + ",";
				}
				str = str + left;
				if (right > left) {
					str = str + "-" + right;
				}
			}
		} else {
			str = top.selectedPorts;
		}
		if (str.length > 50) {
			document.write('Selected ports #:' + str.substring(0,50));
			document.write('<br>' + str.substring(50,str.length));
		} else {
			document.write('Selected ports #: ' + str);
		}
		</script>	
	</font></td>
	<td>
	<a href="javascript:ActionSubmit();"><font class="lightButton">&nbsp;&nbsp;Done&nbsp;&nbsp;</font></a>
	</td>
</tr>
</table>
	</td> 
   </tr>     
</table>
</body>
</html>

