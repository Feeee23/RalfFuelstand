#include <Arduino.h>
#include <SR04.h>

int Trig=5; //Entspricht D1
int Echo=4; //Entspricht D2
SR04 sr04= SR04(Trig, Echo);

int Sensor(){                            //Schuckt den Sensor an
    int A = sr04.Distance(); //Messe dreimal hintereinander um Feler zu minimiern
    int B = sr04.Distance();
    int C = sr04.Distance();
    int D = (A + B + C) / 3; //Durchschnitt der 3 Werte ermitteln
    //Serial.print(D);
    //Serial.println("cm");
    int F = 240 - D; //Die Höhe des Sensors (240cm) minus den Messwert ergibt den Füllstand
    return F;
}

int Rechner(int a){                            //rechnet die Liter aus
    float k = 0.0;           //definition rueckgage variable als float zum max 1. liter rundungsfehler zu haben
    float f = (float)a / 10; //Fuelstand in dezimeter und in als float
    if (a < 200)
    {
        k = 22 * (100 * acos(1 - (f / 10)) - (10 - f) * sqrt((20 * f) - (f * f))); //macht aus der Füllhöhe die Liter (liegender Zylinder) Quelle:https://www.scalesoft.de/math/tank/tank.html
    }
    else
    {
        k = 6283 + 55.416 * (f - 20); //Uber dem Zylinder
    }
    return (int)k; //rueckgabe der wertes mit typecast zu int
}

void KartenRechner(struct Fuellstand* Liste){ //Rechnet für alle karten die Liter aus
    struct Fuellstand* F;
    F=Liste;
    while(F->next!=NULL){
        F->liter=Rechner(F->hoehe);
        F=F->next;
    }
}