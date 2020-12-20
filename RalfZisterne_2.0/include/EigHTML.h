#include <Arduino.h>
#include <string.h>

const char StandartSeite1[] = R"rawliteral(
        <!DOCTYPE html>
        <html>

        <head>
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <meta name='author' content='Felix Girke'>
            <link rel="icon" href='https://tse2.mm.bing.net/th?id=OIP.-rQg8T0gZNSX-8BivY33vgHaHa&o=6&pid=Api'>
            <script src="https://kit.fontawesome.com/7d927496f9.js" crossorigin="anonymous"></script><!--Zum logos bekommen-->
            <script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>
            <script type='text/javascript'>
                google.charts.load('current', { 'packages': ['corechart'] });
                google.charts.setOnLoadCallback(drawChart);

                function drawChart() {
                    var data = google.visualization.arrayToDataTable([
                        ['Datum', 'Liter'],
    )rawliteral";
//                        ['19.09.2020', 200],
//                        ['20.09.2020', 200],
//                        ['21.09.2020', 660],
//                        ['22.09.2020', 400],
//                        ['25.09.2020', 199]
const char StandartSeite2[] = R"rawliteral(
                    ]);

                    var options = {
                        curveType: 'none',
                        legend: 'none',
                    };
                    var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));

                    chart.draw(data, options);
                }
            </script>
            <style>
                html {
                    font-family: Helvetica;
                    display: inline-block;
                    margin: 0px auto;
                    text-align: center;
                }

                .button {
                    background-color: #00008B;
                    border: none;
                    color: white;
                    padding: 16px 40px;
                    text-decoration: none;
                    font-size: 30px;
                    margin: 2px;
                    cursor: pointer;
                }

                table,
                th,
                td {
                    border: 1px solid black;
                    border-collapse: collapse;
                }
            </style>
            <title>Ralf's Zisternen Fuellstand</title>
        </head>

        <body>
            <h1>Ralf's Zisternen Fuellstand</h1>
            <p>neue Abfrage:</p>
            <p><a href="/5/on"><button class="button">Start</button></a><a href='/'><button class="button" style='background-color:#228B22'>aktualisieren</button></a></p>
                )rawliteral";
            //<p><a href="/6/off"><button class="button" style='background-color:#3399ff'>Winterzeit aktivieren</button></a></p>
const char StandartSeite3[] = R"rawliteral(
            <p>Der letzte gemessene Fuellstand sind<b> 135cm</b></p>
            <p>Dies entspricht ungefaehr<b> 4963Liter</b></p> <br>
            <div style=' width: auto; height: 500px' id='curve_chart'></div>
            <table style='width:100%'>
                <tr>
                    <th style='border-top:hidden;border-bottom:hidden;'></th>
                    <th style='width:150px'>Datum</th>
                    <th style='width:100px'>Fuellhoehe</th>
                    <th style='width:100px'>Liter</th>
                    <th style='border-top:hidden;border-bottom:hidden;border-right:hidden;'></th>
                </tr>
    )rawliteral";
//                <tr>
//                    <td style='border-top:hidden;border-bottom:hidden;border-left:hidden;'> </td>
//                    <td>19.09.2020 11:23</td>
//                    <td> 135cm</td>
//                    <td>4963l</td>
//                    <td style='border-top:hidden;border-bottom:hidden;border-right:hidden;'> </td>
//                </tr>
//                <tr>
//                    <td style='border-top:hidden;border-bottom:hidden;border-left:hidden;'> </td>
//                    <td>19.09.2020 11:23</td>
//                    <td> 135cm</td>
//                    <td>4963l</td>
//                    <td style='border-top:hidden;border-bottom:hidden;border-right:hidden;'> </td>
//                </tr>
const char StandartSeite4[] = R"rawliteral(
        </table> <br>
            <div align='right'>
                <a href='/7/on'><i class='fas fa-envelope-open-text fa-2x' ;></i></a>
                <a href='/8/on'><i class="fas fa-unlock-alt fa-2x" style="color: black;"></i></a>
                <p align='right'>Die aktuelle Version 2.0 des Quellcodes gibt es <a href='https://github.com/Feeee23/RalfFuelstand/tree/2.0_PlatformIO'>hier</a></p>
            </div>
        </body>

        </html>
    )rawliteral";

const char AnmeldeSeite[] = R"rawliteral(

    )rawliteral";  

const char AdminBereich[] = R"rawliteral(

    )rawliteral";

String SendStandartseite(String Dia, String Tab, int j){ //baut die Standart Seite zusammen
    String s=StandartSeite1;
    s+=Dia;
    s+=StandartSeite2;
    if (j==3600) { //Sommerzeit
        s+="<p><a href=\"/6/off\"><button class=\"button\" style='background-color:#3399ff'>Winterzeit aktivieren</button></a></p>";
    }else{ //winterzeit
        s+="<p><a href=\"/6/on\"><button class=\"button\" style='background-color:#ffb84d'>Sommerzeit aktivieren</button></a></p>"; 
    }
    s+=StandartSeite3;
    s+=Tab;
    s+=StandartSeite4;
    return s;
}