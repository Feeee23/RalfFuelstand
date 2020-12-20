#include <Arduino.h>
#include <string.h>
#include <TimeLib.h>
#include <EEPROM.h>

struct Fuellstand { //Struct erstellen
    int hoehe;
    int liter;
    int unix;
    int Zaehler;
    struct Fuellstand* next;
};

void erstellen(struct Fuellstand** Liste, int F, int L, unsigned int Unix, bool Speichern, int SpeicherPos){ //Element erstellen
    void *speicher=NULL;
    struct Fuellstand* F1 ;
    Speichern=false;/////////////////////////////////test
    speicher=malloc(sizeof(struct Fuellstand));
    F1=(struct Fuellstand*) speicher;
    F1->hoehe=F;
    F1->liter=L;
    F1->unix=Unix;
    if(*Liste==NULL){
        F1->next=NULL;
    } else
    {
        F1->next=*Liste;
    }
    *Liste=F1;
    if(F1->next!=NULL){ //Gib jedem eine Unike Nummer
        F1->Zaehler=F1->next->Zaehler+1;
    } else{ //wenn erster eintrag
        F1->Zaehler=1;
    }
    if(F1->next->Zaehler==NULL){ //wenn letzter eintrag keine Nummer
        F1->Zaehler=1;
    }

    if(Speichern){//die Werte auch noch in den EEPROM schreiben
        int Pos=EEPROM.read(SpeicherPos)*15; //lese die aktuelle Speicherpos aus
        int i=0;
        char buffer[20];
        itoa(F1->hoehe, buffer, 10);//itoa
        for(i=0; i<4; i++){
            EEPROM.write(Pos+i, buffer[i]);   //scheibe die höhe dahin
        }
        itoa(F1->unix, buffer, 10);//itoa
        for(i=5; i<15; i++){
            EEPROM.write(Pos+i,buffer[i-5]);   //scheibe UNIX 5 stellen weiter
        }
        EEPROM.write(SpeicherPos,EEPROM.read(SpeicherPos)+1); //erhöhe die Speicherpos um einen eintrag
        if(EEPROM.read(SpeicherPos)==21){ //Überlauf
            EEPROM.write(SpeicherPos, 1);
        }
        EEPROM.commit();
    }
}

String getDiagramWerte(struct Fuellstand* Liste ){ //Element ausgeben, bisher noch falschrum links das neuste
    String s;
    struct Fuellstand* F;
    F=Liste;
    int i=20;
    while(F->next!=NULL){
        i++;
        if(i>20){ //gib nur 20 Werte in das Diagramm
            break;
        }
        String k="['";
        if (day(Liste->unix)<10){
        k+=0;
        }
        k+=day(F->unix); //Datum
        k+=".";
        if (month(Liste->unix)<10){
        k+=0;
        }
        k+=month(F->unix);
        k+=".";
        if (year(Liste->unix)<10){
        k+=0;
        }
        k+=year(F->unix);
        k+="', ";
        k+=F->liter; //Liter
        if(F->next==NULL){
            k+="]";  //am ende einmal ohne Komma
        } else{
            k+="],";
        }
        s+=k;
        //Zielformat['19.09.2020', 200],
        F=F->next;
    }
    return s;
}
String getTabellenWerte(struct Fuellstand* Liste ){ //Element ausgeben
    String s;
    struct Fuellstand* F;
    F=Liste;
    while(F->next!=NULL){
        String k="<tr><td style='border-top:hidden;border-bottom:hidden;border-left:hidden;'> </td><td> ";
        if (day(Liste->unix)<10){
        k+=0;
        }
        k+=day(F->unix); //Datum
        k+=".";
        if (month(Liste->unix)<10){
        k+=0;
        }
        k+=month(F->unix);
        k+=".";
        if (year(Liste->unix)<10){
        k+=0;
        }
        k+=year(F->unix);
        k+=" ";
        if (hour(Liste->unix)<10){
        k+=0;
        }
        k+=hour(F->unix);//Uhrzeit
        k+=":";
        if (minute(Liste->unix)<10){
        k+=0;
        }
        k+=minute(F->unix);
        k+="</td><td>";
        k+=F->hoehe; //höhe
        k+="cm</td><td>";
        k+=F->liter; //Liter
        k+="l</td><td style='border-top:hidden;border-bottom:hidden;border-right:hidden;'> </td></tr>";
        s+=k;
        //Zielformat
//                <tr>
//                    <td style='border-top:hidden;border-bottom:hidden;border-left:hidden;'> </td>
//                    <td>19.09.2020 11:23</td>
//                    <td> 135cm</td>
//                    <td>4963l</td>
//                    <td style='border-top:hidden;border-bottom:hidden;border-right:hidden;'> </td>
//                </tr>
        F=F->next;
    }
    return s;
}

String EmailAnfrage(struct Fuellstand* Liste){
    String s= "Hallo, die Zisterne hat aktuell einen Füllstand von: ";
    s+=Liste->hoehe;
    s+="cm, das sind ca.: ";
    s+=Liste->liter;
    s+="Liter. Die Abrage war am";
    if (day(Liste->unix)<10){
        s+=0;
    }
    s+=day(Liste->unix); //Datum
    s+=".";
    if (month(Liste->unix)<10){
        s+=0;
    }
    s+=month(Liste->unix);
    s+=".";
    if (year(Liste->unix)<10){
        s+=0;
    }
    s+=year(Liste->unix);
    s+=" ";
    if (hour(Liste->unix)<10){
        s+=0;
    }
    s+=hour(Liste->unix);//Uhrzeit
    s+=":";
    if (minute(Liste->unix)<10){
        s+=0;
    }
    s+=minute(Liste->unix);

    return s;
}

int AnzahlMesswerte(struct Fuellstand* Liste){// gibt die Anzahl der Messwerte zurück //ungenutzt
     struct Fuellstand* F;
     F=Liste;
     int i=0;
     while (F->next!=NULL){
         i++;
         F=F->next;
     }
     return i;
}

void getValueEEPROM(int SpeicherPos, struct Fuellstand** Liste){
    int Z=EEPROM.read(SpeicherPos)-1; //Schaue an welcher angefangen werden muss zu lesen. 0 bedeutet leer
    int Sprung =15; //jeder eintrag benötigt 15byte
    int AktuellePos=Z*Sprung; 
    int i=0;
    int k=0;//zählt wie oft schon gelesen wurde
    char buff1[20];
    char buff2[20];
    
    for(k=0; k<20; k++){ //gehe zu jedem Eintrag
        AktuellePos=AktuellePos+Sprung; //eins hinter dem neusten müsste das älteste Stehen
        if(AktuellePos>20){ //evtl überlauf
            AktuellePos=0;
        }
        if(char(EEPROM.read(AktuellePos))==NULL){ //wenn nix mehr drinsteht geh zum nächsten
            continue;
        }
        for(i=0; i<4; i++){
            buff1[i]=char(EEPROM.read(AktuellePos+i)); //lese die höhe aus
        }
        for(i=5; i<15; i++){
            buff2[i-5]=char(EEPROM.read(AktuellePos+i)); //lese UNIX aus
        }
        int h=atoi(buff1);//mach ints aus den chars
        int UNIX=atoi(buff2); 
        erstellen(Liste,h, 0, UNIX, false, 0);//erstelle eine karte       
    }
}