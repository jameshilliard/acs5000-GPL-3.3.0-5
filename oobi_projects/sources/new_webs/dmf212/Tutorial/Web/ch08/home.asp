<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: home.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->
<html>
<head>
<title>DMF Tutorial entry page</title>
</head>


<%
   // if the server variables we need to use are not sensibly initialized, 
   // guarantee that they've got good values before we might need them...
   getVal("_bgcolor");
   if (__error__ == "ParameterName")
   {
      set("_bgcolor", "White");
   }

   getVal("_logo");
   if (__error__ == "ParameterName")
   {
      set("_logo", "dmf_logo.png");
   }

   getVal("_rowcount");
   if (__error__ == "ParameterName")
   {
      set("_rowcount", 8);
   }
%>

<script language="JavaScript1.1">
   window.location.replace("/user/home.asp");
</script>   


</head>
