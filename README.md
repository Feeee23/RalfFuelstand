# RalfFuelstand

Version:
<ul>
<li>1.0 == Master: running at the moment</li>
<li>1.1 == graph:  Uptate to the HTML site. Added the Graph </li>
<li>1.2 == Email:  Addet the Email comunication</li>
<li>2.0 == P.IO:    rewritten the whole Code in PlatformIO, createt a Class for Messurments, addet some data Backup in the EEPROM, cleaning up.</li>
  </ul>

<h2>Documentaion</h2>
<p> a small Documentaion to explain what this does and how it works.</p>

<h3> 1. Functions </h3>
<p> A HC-SR04 Ultrasonic Sensor is messuring the Distance to the surface of the waterlevel inside a Watertank for Rainwater. Subtracting this Value from the height of the Sensor (relative to the bottom of the Watertank) gives us the height of the waterlevel in Centimeters. The connectet ESP8266-12E from NodeMCU can ruffly calculate the Volume of the water in Liters. The actual Time for the Messurment comes from pool.npt.org. <br>
All this data is presentet as an HTML file in the Homenetwork via WiFi, when you type it's IP-Adress into your Browser. On this Site, you can also request a new Messurment, change between summer and wintertime, request an email send to your emailadress with a new Messurment and go to the login Site for the Administrator. Also a Link to this branch of the Repoitory is providet. The 20 newest Messurments are displayed in a litte Line-Chart using Google charts. In the table below the Graph is are up to 1700 Messurments shown. <br>
Everynight at 2am (UTF Time) an automatic Messurment ist triggert. If this Messurment is below or above a certain Limit the an Automatic Email ist triggert whitch tells you the waterlevel is low/high.   </p> <br>
![Verkabelung_Steckplatine](/Verkabelung_Steckplatine.jpg)   

