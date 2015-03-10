

const char_t* kTest1 = 
T("<?xml version=\"1.0\"?>")\
T("<SOAP-ENV:Envelope SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" ")\
T("xmlns:xsd=\"http://www.w3.org/1999/XMLSchema/\" ")\
T("xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance/\">")\
T("<SOAP-ENV:Body>")\
T("   <saveMultipleFiles>")\
T("<email xsi:type=\"xsd:string\">dave@userland.com</email>")\
T("<password xsi:type=\"xsd:string\">9dd4e461268c8034f5c8564e155c67a6</password>")\
T("      <relativePathList SOAP-ENC:arrayType=\"xsd:string[1]\" xsi:type=\"SOAP-ENC:Array\">")\
T("         <item>/test/clock2.html</item>")\
T("      </relativePathList>")\
T("      <fileTextList SOAP-ENC:arrayType=\"xsd:string[1]\" xsi:type=\"SOAP-ENC:Array\">")\
T("         <item>4/8/2001; 3:43:16 PM</item>")\
T("      </fileTextList>")\
T("   </saveMultipleFiles>")\
T("</SOAP-ENV:Body>")\
T("</SOAP-ENV:Envelope>");

const char_t* kTest2 = 
T("<?xml version=\"1.0\"?>")\
T("<SOAP-ENV:Envelope SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" ")\
T("xmlns:xsd=\"http://www.w3.org/1999/XMLSchema/\" ")\
T("xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance/\">")\
T("<SOAP-ENV:Body>")\
T("   <d:get xmlns:d=\"http://www.artlogic.com/dmf/\">")\
T("      <key xsi:type=\"xsd:string\">test.struct.char</key>")\
T("   </d:get>")\
T("</SOAP-ENV:Body>")\
T("</SOAP-ENV:Envelope>");

const char_t* kTest3 = 
T("<?xml version=\"1.0\"?>")\
T("<SOAP-ENV:Envelope SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" ")\
T("xmlns:xsd=\"http://www.w3.org/1999/XMLSchema/\" ")\
T("xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance/\">")\
T("<SOAP-ENV:Body>")\
T("   <d:get xmlns:d=\"http://www.artlogic.com/dmf/\">")\
T("      <key xsi:type=\"xsd:string\">test.struct.bytes</key>")\
T("      <index xsi:type=\"xsd:int\">1</index>")\
T("   </d:get>")\
T("</SOAP-ENV:Body>")\
T("</SOAP-ENV:Envelope>");


const char_t* kTest4 = 
T("<?xml version=\"1.0\"?>")\
T("<SOAP-ENV:Envelope SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" ")\
T("xmlns:xsd=\"http://www.w3.org/1999/XMLSchema/\" ")\
T("xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance/\">")\
T("<SOAP-ENV:Body>")\
T("   <d:get xmlns:d=\"http://www.artlogic.com/dmf/\">")\
T("      <key xsi:type=\"xsd:string\">test.struct</key>")\
T("   </d:get>")\
T("</SOAP-ENV:Body>")\
T("</SOAP-ENV:Envelope>");


const char_t* kTest5 = 
T("<?xml version=\"1.0\"?>")\
T("<SOAP-ENV:Envelope SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" ")\
T("xmlns:xsd=\"http://www.w3.org/1999/XMLSchema/\" ")\
T("xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance/\">")\
T("<SOAP-ENV:Body>")\
T("   <structTest>")\
T("      <test>")\
T("         <struct>")\
T("            <char xsi:type=\"xsd:int\">2</char>")\
T("            <int xsi:type=\"xsd:int\">101</int>")\
T("            <string xsi:type=\"xsd:string\">Howdy!</string>")\
T("         </struct>")\
T("      </test>")\
T("   </structTest>")\
T("</SOAP-ENV:Body>")\
T("</SOAP-ENV:Envelope>");


const char_t* kTest6 = 
T("<?xml version=\"1.0\"?>")\
T("<SOAP-ENV:Envelope SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" ")\
T("xmlns:xsd=\"http://www.w3.org/1999/XMLSchema/\" ")\
T("xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance/\">")\
T("<SOAP-ENV:Body>")\
T("   <d:set xmlns:d=\"http://www.artlogic.com/dmf/\">")\
T("      <key xsi:type=\"xsd:string\">test.struct.char</key>")\
T("      <value xsi:type=\"xsd:int\">5</value>")\
T("   </d:set>")\
T("</SOAP-ENV:Body>")\
T("</SOAP-ENV:Envelope>");

const char_t* kTest7 = 
T("<?xml version=\"1.0\"?>")\
T("<SOAP-ENV:Envelope SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" ")\
T("xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" ")\
T("xmlns:xsd=\"http://www.w3.org/1999/XMLSchema/\" ")\
T("xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance/\">")\
T("<SOAP-ENV:Body>")\
T("   <d:set xmlns:d=\"http://www.artlogic.com/dmf/\">")\
T("      <key xsi:type=\"xsd:string\">test.struct.bytes</key>")\
T("      <index xsi:type=\"xsd:int\">1</index>")\
T("      <value xsi:type=\"xsd:int\">42</value>")\
T("   </d:set>")\
T("</SOAP-ENV:Body>")\
T("</SOAP-ENV:Envelope>");
