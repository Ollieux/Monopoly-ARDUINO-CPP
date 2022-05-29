#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel plansza(24, 6, NEO_GRB + NEO_KHZ800);

// [koszt, doKupienia, idWlasciciel]
const int iloscPlansza = 24;
const int iloscAtrybutPlansza = 3;
int polePlansza[iloscPlansza][iloscAtrybutPlansza] = {
  {-1, -1, -1}, // start
  {100, 1, -1},
  {125, 1, -1},
  {-1, -1, -1},
  {175, 1, -1},
  {200, 1, -1},
  {-1, -1, -1}, // visit
  {250, 1, -1},
  {275, 1, -1},
  {-1, -1, -1},
  {325, 1, -1},
  {350, 1, -1},
  {-1, -1, -1}, // parking
  {400, 1, -1},
  {425, 1, -1},
  {-1, -1, -1},
  {475, 1, -1},
  {500, 1, -1},
  {-1, -1, -1}, // jail
  {550, 1, -1},
  {575, 1, -1},
  {-1,  -1, -1},
  {625, 1, -1},
  {650, 1, -1}
};

// [id, pole, budzet, iloscNieruchomosc, kolor]
int iloscGracz = 4;
const int iloscAtrybutGracz = 7;
int gracz[4][iloscAtrybutGracz] = {
  {1, 0, 600, 0, 255, 0, 0},
  {2, 0, 600, 0, 255, 255, 0},
  {3, 0, 600, 0, 100, 100, 100},
  {4, 0, 600, 0, 255, 0, 255}
};

int kolejkaGracz[4] = {1, 2, 3, 4};

void czekaj();
void zmienAtrybutIlosc(int pos, bool inc);
int obliczDebet(int pos);
void bankrutGracz(int pos_guest, int ilosc_guest, int pos_host);
int zmienWlascicielNieruchomosc(int pos_z_gracz, int pos_na_gracz);
int przeszukajNieruchomosc(int id_gracz);
void kolejkaRuch();
void kolejkaGraczUsun();
void zwyciestwoGracz();
int znajdzGraczPoId(int id_gracz);
void zarzadzajPosiadane(int pos_guest, int ilosc, int pos_host, int kwota=0);
void cls(int N);
void ruchGracz();
void transakcjaGracz(int dawca_gracz, int gotowka, int biorca_gracz=-1);
int pokazBudzet(int idx);
void niespodzianka();
void posiadane(int posiadacz);
void dostepne();

void setup() {
  Serial.begin(9600);
  delay(2000);
  plansza.begin();
  Serial.println("started");
}

void loop() {
  while (iloscGracz > 1){
  czekaj();
  ruchGracz();
  kolejkaRuch();
  cls(6);
  }
  zwyciestwoGracz();
}

void kolejkaRuch() {
    int temp = kolejkaGracz[0];
    int i = 1;
    while (i < iloscGracz){
      kolejkaGracz[i - 1] = kolejkaGracz[i];
      i++;
    }
    kolejkaGracz[i - 1] = temp;
}

void kolejkaGraczUsun() {
  iloscGracz--;
}

void czekaj() {
  int posGracz = znajdzGraczPoId(kolejkaGracz[0]);
  plansza.setPixelColor(gracz[posGracz][1], plansza.Color(gracz[posGracz][4], gracz[posGracz][5], gracz[posGracz][6]));
  Serial.print("Tura gracza nr: ");
  Serial.println(kolejkaGracz[0]);
  Serial.println("1 - Pokaz do kupienia");
  Serial.println("2 - Pokaz posiadane");
  Serial.println("3 - Rzut kostka ");
  Serial.println("4 - Wyswietl budzet");
  Serial.println();
  while (Serial.available() == 0){
    
  }
  plansza.clear();
  int faktor = Serial.read() - '0';
  switch(faktor){
    case 1:
      dostepne();
      czekaj();
      break;

    case 2:
      posiadane(kolejkaGracz[0]);
      czekaj();
      break;
    case 3:
      plansza.show();
      break; 
    case 4:
      int _ = znajdzGraczPoId(kolejkaGracz[0]);
      Serial.print("Budzet: ");
      Serial.println(pokazBudzet(_)); 
      czekaj();
      break;
  }
}


void ruchGracz() {
  int idGracz = kolejkaGracz[0];
  int pozycjaGracz = znajdzGraczPoId(idGracz); // indeks gracza w dwuwymiarowej tablicy
  int* _poleGracz = &gracz[pozycjaGracz][1]; 

  Serial.print("Gracz przed rzutem na polu: ");
  Serial.println(*_poleGracz);
  
  int rzut = millis() % 6 + 1;
  
  Serial.print("Gracz wyrzucil na kostce: ");
  Serial.println(rzut);

  int poleNowe = (*_poleGracz + rzut) % iloscPlansza;
  trawersal(*_poleGracz, poleNowe, gracz[pozycjaGracz][4], gracz[pozycjaGracz][5], gracz[pozycjaGracz][4]);
  *_poleGracz = poleNowe;

  
  
  Serial.print("Gracz po rzucie na polu: ");
  Serial.println(*_poleGracz);

  

  

  int* _budzetGracz = _poleGracz + 1;
  int* _posiadaneGracz = _poleGracz + 2;
  int* _wlascicielNieruchomosc = &polePlansza[*_poleGracz][2];
  int* _stanNieruchomosc = _wlascicielNieruchomosc - 1;
  int kosztNieruchomosc = *(_wlascicielNieruchomosc - 2);
  
  
  switch(*_stanNieruchomosc) { // sprawdzany stan posiadania nieruchomosci
    case 1: // nieruchomosc jest do kupienia
      if (*_budzetGracz > kosztNieruchomosc) { // jezeli gracza stac na nieruchomosc
        *_budzetGracz -= kosztNieruchomosc;
        *_posiadaneGracz = *_posiadaneGracz + 1;
        *_stanNieruchomosc = 0;
        *_wlascicielNieruchomosc = idGracz;
      }
      else {
        Serial.print("Gracz ");
        Serial.print(idGracz);
        Serial.println(" nie zgromadzil wystarczajacych srodkow do zakupu nieruchomosci");
      }
      break;
      
    case 0: // nieruchomosc jest zajeta
      Serial.println("Nieruchomosc sprzedana."); 
      int mnoznik;
      if (*_wlascicielNieruchomosc == idGracz) {
        Serial.print("Gracz ");
        Serial.print(idGracz);
        Serial.println(" jest u siebie.");
      }
      else {
        if (*_poleGracz % 2 == 0) {
          if (*_wlascicielNieruchomosc == *(_wlascicielNieruchomosc - 1)) {
            mnoznik = 2;
          }
          else {
            mnoznik = 1;
          }
        }
        else {
          if (*_wlascicielNieruchomosc == *(_wlascicielNieruchomosc + 1)) {
            mnoznik = 2;
          }
          else {
            mnoznik = 1;
          }
        }
      }
      int kosztWizyta = kosztNieruchomosc * mnoznik;
      int pozycjaOdwiedzanyGracz = znajdzGraczPoId(*_wlascicielNieruchomosc); // Indeks gracz u ktorego znalazl sie gracz
      if (*_budzetGracz > kosztWizyta) {
        Serial.print("Placisz: ");
        Serial.println(kosztWizyta);
        transakcjaGracz(pozycjaGracz, kosztWizyta, pozycjaOdwiedzanyGracz);
      }
      else {
        if (*_posiadaneGracz != 0){ // gracz posiada nieruchomosci ktore moze sprzedac
          zarzadzajPosiadane(pozycjaGracz, *_posiadaneGracz, pozycjaOdwiedzanyGracz, kosztWizyta); 
        }
        else {
          bankrutGracz(idGracz, 0, *_wlascicielNieruchomosc);
          return;
        }
      }
      break;
    case -1: // nieruchomosc nie jest na sprzedaz
//      for (int i = 3; i <= 21;  i += 6){
//        if (i == *_poleGracz){
//          niespodzianka();
//          break;
//        }
        if (*_poleGracz == 18){
          police();
          return;
        }
        if (*_poleGracz == 6 || *_poleGracz == 12){
          Serial.println("Nic tutaj nie kupisz, ale tez nie zaplacisz!");
        }
        break;
      }
      
  
   
  Serial.print("Budzet gracza: ");
  Serial.print(*_budzetGracz);
  Serial.println();
  
  if(rzut == 6){ // gdy gracz wyrzuci 6 przysluguje mu dodatkowy rzut
    delay(750);
    cls(6);
    ruchGracz();
  }
}




bool czyDebet(float budzet, int obciazenie) {
  return budzet - obciazenie < 0;
}

void transakcjaGracz(int dawca_gracz,int gotowka, int biorca_gracz=-1){
  gracz[dawca_gracz][2] -= gotowka;
  if (biorca_gracz > 0){
  gracz[biorca_gracz][2] += gotowka;
  }
}

int znajdzGraczPoId(int id_gracz) {
  for (int i = 0; i < iloscGracz; i++){
    if (gracz[i][0] == id_gracz){
      return i;
    }
  }
}
void zarzadzajPosiadane(int guest_gracz, int ilosc, int host_gracz, int kwota=0) {
  int debetGracz = obliczDebet(guest_gracz, kwota);
  Serial.print("Debet :");
  Serial.println(debetGracz);
  Serial.print("Gracz ");
  Serial.print(gracz[guest_gracz][0]); //Id goscia
   Serial.print(" posiada ");
  while (ilosc > 0 || debetGracz > 0){
    Serial.println("Nieruchomosc/i: ");
    for (int j = 0; j < iloscPlansza; j++){
      if (polePlansza[j][2] == gracz[guest_gracz][0]){
        Serial.print(j); // numer nieruchomosci
        Serial.print(" cena: ");
        Serial.println(polePlansza[j][0]); // koszt nieruchomosci
      }
    }
    Serial.println();
    Serial.print("Do zaplaty: ");
    Serial.println(debetGracz);
    Serial.print("Budzet");
    Serial.println(pokazBudzet(guest_gracz));
    Serial.println("0 - Bankrut");
    Serial.println("Wybierz nieruchomosc do sprzedania: ");
    while (Serial.available() == 0){
    }
    int numerNieruchomosc = Serial.parseInt();
    if (numerNieruchomosc == 0){
      bankrutGracz(guest_gracz, ilosc, host_gracz);
      return;
    }
    else {
      debetGracz -= polePlansza[numerNieruchomosc][0]; // zmniejsz aktualny debet
      gracz[guest_gracz][2] += polePlansza[numerNieruchomosc][0]; // zasil konto gr. o cene sprzedanej nier.
      polePlansza[numerNieruchomosc][2] = gracz[host_gracz][0]; // zmien id wlasciciela na gr. goszczacego odwiedzajacego gr.
      zmienAtrybutIlosc(guest_gracz, 0);
      zmienAtrybutIlosc(host_gracz, 1);
      Serial.print("Aktualny debet: ");
      Serial.println(debetGracz);
      ilosc--;
    }
    
  }
  if (debetGracz > 0){ // gracz pomimo sprzedania wszystkich nieruchomosci jest na debecie = bankrut
    bankrutGracz(guest_gracz, 0, host_gracz);
    return;
  }
}

void zmienAtrybutIlosc(int pos, bool inc){
  //int pos = znajdzGraczPoId(idGracz);
  if (inc){
    //gracz[pos][3]++;
    gracz[pos][3]++;
  }
  else {
    gracz[pos][3]--;
  }
}

int obliczDebet(int pos, int oplata){
  int debet = oplata - gracz[pos][2];
  if (debet > 0){
    return debet;
  }
  return 0;
}

void bankrutGracz(int pos_guest, int ilosc_guest, int pos_host) {
  if (polePlansza[pos_guest][1] == 0){// gdy gracz zbankrutuje wchodzac na posiadlosc 
    for (int i = 0; i < ilosc_guest; i++){
      zmienAtrybutIlosc(pos_guest, 0);
      //gracz[pos_guest][3]--; ooo
      //gracz[pos_guest][3]--; ooo
      zmienAtrybutIlosc(pos_host, 1);
      zmienWlascicielNieruchomosc(pos_guest, pos_host);   
    }
  }
  else {
    for (int i = 0; i < ilosc_guest; i++){
      zmienWlascicielNieruchomosc(pos_guest, -1);
      
    }
  }
  kolejkaRuch();
  kolejkaGraczUsun();
  cls(6);
  ruchGracz();
}


int zmienWlascicielNieruchomosc(int pos_z_gracz, int pos_na_gracz) {
  int nr_posesja = przeszukajNieruchomosc(gracz[pos_z_gracz][0]); // (id)
  polePlansza[nr_posesja][2] = gracz[pos_na_gracz][0];  
}

int przeszukajNieruchomosc(int id_gracz) {
  for (int i = 0; i < iloscPlansza; i++){
    if (polePlansza[i][2] == id_gracz){
      return i;
    }
  }
}


/*
String kartaNiespodzianka[] = {
  "Wracasz na poprzednie pole",
  "Cofasz sie o 3 pola wstecz",
  "Idziesz na start",
  "Idziesz do wiezienie",
  "Otrzymujesz po x z konta innych gracz (czyGra)",
  "Wyplacasz po x na konta pozostalych graczy",
  
}*/

void zwyciestwoGracz(){
  //millis
  Serial.print("Wygral gracz: ");
  Serial.print(kolejkaGracz[0]);
  Serial.println(" brawo!");
}

void cls(int N){
  for (int i =0; i < N; i++){
    Serial.println();
  }
}

int pokazBudzet(int idx){
  return gracz[idx][2];
}

void niespodzianka() {
int _ = millis() % 5;
  switch(_){
   case 1:
    //graczStart();
    break;
   case 2:
    //graczWiezienie();
    break;
   case 3:
    //graczWstecz();
    break;
  }
}

void posiadane(int posiadacz){
  for (int i = 0; i < iloscPlansza; i++){
    if (polePlansza[i][2] == posiadacz){
      plansza.setPixelColor(i, plansza.Color(0, 125, 125));
      delay(100);
    }
    plansza.show();
  }
}

void dostepne(){
  for (int i = 0; i < iloscPlansza; i++){
    if (polePlansza[i][2] == -1){ // id posiadacza jest nieokreslone
      if (polePlansza[i][1] == 1){ // jest kupowalna
        plansza.setPixelColor(i, plansza.Color(0, 125, 0));
        
      }
    }
    plansza.show();
  }
}

void trawersal(int start_pole, int stop_pole, int R, int G, int B){
    for (int i = start_pole; i <= stop_pole; i++){
      plansza.setPixelColor(i, plansza.Color(R, G, B));
      plansza.show();
      delay(500);
      plansza.clear();
    }
    plansza.show();
}

 void police(){
  for (int j = 0; j <= 10; j++){
    for (int i = 0; i <= 24; i++){
      if (i % 2 == 0){
        plansza.setPixelColor(i, plansza.Color(125, 0, 0));
        plansza.show();
      }
      else{
        plansza.setPixelColor(i, plansza.Color(0, 0, 255));
        plansza.show();
      }
      //delay(500);
      //plansza.clear();
    }
    //plansza.show();
    delay(300);
    plansza.clear();
  }
}
