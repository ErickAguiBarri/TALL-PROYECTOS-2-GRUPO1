/* MUY IMPORTANTE: LOS TIEMPOS DE DELAY ESTAN AJUSTADOS
   A LA VELOCIDAD DE SIMULACION Y NO SON LOS TIEMPOS REALES
   QUE SE DEBAN APLICAR AL USO EN UN SISTEMA EMBEBIDO. 
*/

#include <LiquidCrystal.h> // Libreria de pantalla LCD

// INICIALIZACIÓN DE VARIABLES Y CONSTANTES
//defino constantes
const int PinPower = 13;	// Defino Pin switch.
const int PinExtBot = 2;  	// Defino Pin boton abrir puerta
const int PinIntPir = 8;	// Defino Pin PIR interior
const int PinTemp = A0;		// Defino Pin Temperatura
const int PinHumAmb = A1;	// Defino Pin Humedad Ambiente
const int PinHumSuelo = A2;	// Defino Pin Humedad Suelo
const int PinLuz = A3;		// Defino Pin iluminación.
const int PinExtBot1 = 3;	// Defino Pin Boton cerrar puerta

//defino variables
int VarPower = 0;			// variable que recibe el inicio del sistema
int VarBot1 = 0;			// variable de lectura abrir puerta ext
int VarBot2 = 0;			// variable de lectura cerrar puerta ext
int VarPir2 = 0;			// variable de lectura estado pir interior.
float Temp = 0;				// variable lectura temperatura
float HumAmb = 0;			// variable humedad ambiente
float HumSuelo = 0;			// variable humedad suelo
int VarLuz = 0;				// variable lectura iluminación
String mensaje;				// variable que contiene texto a mostrar
int Contador = 0;			// variable de usos varios

//defino variables de estado
int VarEstVent = 0; 		//para verificar estado ventilacion
int VarEstCale = 0;			//para verificar estado calefaccion x tmp 
int VarEstAspe = 0;			//para verificar estado aspersor
int VarEstCaHu = 0;			//para verificar estado calefaccion x humedad
int VarEstHuSu = 0;			//para verificar estado aspersor x humedad del suelo

//defino objetos
LiquidCrystal lcd(12, 11, 7, 6, 5, 4); //definimos conexion pantalla LCD



// RUTINA PRINCIPAL - SETUP
void setup() {
  	Serial.begin(9600);
  	pinMode(PinPower, INPUT);   //defino pin de inicio como entrada
 	pinMode(PinExtBot, INPUT);	//defino como entrada pin boton abrir
  	pinMode(PinExtBot1, INPUT); // defino como entrada pin boton cerrar
  	pinMode(PinIntPir, INPUT);	//defino como entrada pin pir interior
    while (VarPower == 0) {
  		VarPower = digitalRead(PinPower);// genero un loop en setup a la espera de 
    	MjeDefinido(5);					 // muestro mensaje para que inicie
      	delay(80);
    }									 // que se inicie el sistema a través de switch
  
    StartLcd();  //inicio el lcd
  	MjeDefinido(1); // muestro mensajes
  	delay(2000);
  	attachInterrupt(digitalPinToInterrupt(PinExtBot), AbrirPta, RISING);
  	attachInterrupt(digitalPinToInterrupt(PinExtBot1), CerrarPta, RISING);  
}

// RUTINA LOOP - REPETITIVA - PROGRAMA PINCIPAL
void loop() {
  	if (digitalRead(PinPower) == 1){
    	MjeDefinido(3); 			// leyendo sensores	
      	delay(1000);
    }
  	if (digitalRead(PinPower) == 1){
        leyendosensores(1);		//	temp
    }
    if (digitalRead(PinPower) == 1){
  		leyendosensores(2);		//  Humedad Ambiente
    }
    if (digitalRead(PinPower) == 1){
  		leyendosensores(3);		//  Humedad Suelo
    }
    if (digitalRead(PinPower) == 1){
  		leyendosensores(5);		// Pir interior
    }
    if (digitalRead(PinPower) == 1){
  		leyendosensores(6);		// sensor iluminación
    }
  	if (digitalRead(PinPower) == 0){
      	MjeDefinido(2); // Indico que el sistema esta apagado
      	PararTodo();				// aca llamo a funcion q apaga todos los actuadores
    }
  	delay(1000);
}




// PROCEDIMIENTOS ESPECIALES
// --------------------------------------------------
//Rutina de inicio de pantalla
void StartLcd() {
  	lcd.begin(16, 2);
  	lcd.noDisplay();
  	lcd.clear();
  	lcd.home();
}
//---------------------------------------------------
// Rutina para mostrar mensajes en el lcd
void MostrarMje(int c, int f,String mje,float nro1){
 	lcd.display();	
  	lcd.setCursor(c,f);
  	if (mje != "XX"){
    	lcd.print(mje);
    }
  	if (nro1 != 0) {
    	lcd.print((int) nro1);
    }
}
//---------------------------------------------------
// Rutina que indica que mensaje mostrar.
void MjeDefinido(int nro){
  switch (nro){
  	case 1:
    	lcd.clear();
    	mensaje = String("SISTEMA DE");
		MostrarMje(0,0,mensaje,0);	
  		mensaje = String("INVERNADERO");
		MostrarMje(5,1,mensaje,0);	
    	break;
    case 2:
    	lcd.clear();
    	mensaje = String("INVERNADERO");
		MostrarMje(0,0,mensaje,0);	
  		mensaje = String("DETENIDO");
		MostrarMje(5,1,mensaje,0);	
    	break;
    case 3:
    	lcd.clear();	
    	mensaje = String("LEYENDO");
		MostrarMje(0,0,mensaje,0);	
  		mensaje = String("SENSORES");
		MostrarMje(5,1,mensaje,0);	
    	break;
    case 4:
    	lcd.clear();	
    	mensaje = String("Temp: ");
    	MostrarMje(0,0,mensaje,0);	
    	break;
    case 5:
    	lcd.clear();	
    	mensaje = String("ENCIENDA SWITCH");
		MostrarMje(0,0,mensaje,0);	
    	break;
    case 6:
    	lcd.clear();	
    	mensaje = String("Hum Amb: ");
    	MostrarMje(0,0,mensaje,0);	
    	break;
	case 7:
    	lcd.clear();	
    	mensaje = String("Hum Suelo: ");
    	MostrarMje(0,0,mensaje,0);	
    	break;
	case 8:
    	lcd.clear();	
    	mensaje = String("Puerta Exterior");
    	MostrarMje(0,0,mensaje,0);	
    	break;
	case 9:
    	lcd.clear();	
    	mensaje = String("Movimiento");
    	MostrarMje(0,0,mensaje,0);	
    	mensaje = String("Interior: ");
    	MostrarMje(0,1,mensaje,0);	
    	break;
	case 10:
    	mensaje = String("ESTADO:");
    	MostrarMje(0,1,mensaje,0);	
    	break;
	case 11:
    	lcd.clear();	
    	mensaje = String("Iluminacion");
    	MostrarMje(0,0,mensaje,0);	
    	mensaje = String("Interior: ");
    	MostrarMje(0,1,mensaje,0);	
    	break;
    default:
    	lcd.clear();
    	break;
  }	
}
//---------------------------------------------------

float leyendosensores(int valor){
switch(valor){
	
  	case 1:  //sensar temperatura
  		Temp = (5.0 /1024 * analogRead(PinTemp))*100-50;
  		if (VarEstCale == 1) {
        	MjeDefinido(4);
          	MostrarMje(6,0,"XX",Temp);
          	MostrarMje(9,0,"C",0);
          	MjeDefinido(10);
          	MostrarMje(7,1,"Adecuando",0);
          	Serial.write('k'); //mantengo cale
          	if (Temp >= 25) { //hasta llegar a 25º
            	VarEstCale = 0;
            	Serial.write('l');
            }
          	delay(1000);
        }
  		if (VarEstVent == 1) {
        	MjeDefinido(4);
	        MostrarMje(6,0,"XX",Temp);
    	    MostrarMje(9,0,"C",0);
            MjeDefinido(10);
            MostrarMje(7,1,"Adecuando",0);
        	Serial.write('g'); // mantengo ventilador
          	if (Temp <= 20) { // hasta que llegue a los 20º
            	VarEstVent = 0;
            	Serial.write('h');
            }
          	delay(1000);        
        }
  		if (VarEstVent != 1 && VarEstCale != 1 && Temp >= 10 && Temp <= 30) {
  			MjeDefinido(4);
	        MostrarMje(6,0,"XX",Temp);
    	    MostrarMje(9,0,"C",0);
            MjeDefinido(10);
            MostrarMje(7,1,"Normal",0); //si todo esta normal entra aca
        	delay(1000);
        }
  		if (Temp < 10 && VarEstCale != 1) {
			MjeDefinido(4);
          	MostrarMje(6,0,"XX",Temp);
          	MostrarMje(9,0,"C",0);
          	MjeDefinido(10);
          	MostrarMje(7,1,"Baja",0);
          	Serial.write('k'); // enciende calefacción
          	Serial.write('h');
          	VarEstCale = 1;
            VarEstVent = 0;
          	delay(1000);
        }
        if (Temp > 30 && VarEstVent != 1) {
			MjeDefinido(4);
	        MostrarMje(6,0,"XX",Temp);
    	    MostrarMje(9,0,"C",0);
            MjeDefinido(10);
            MostrarMje(7,1,"Alta",0);
        	Serial.write('g'); //enciende ventilacion
            Serial.write('l');
          	VarEstCale = 0;
          	VarEstVent = 1;
            delay(1000);
        } 
  		break;
  	
  	case 2: //sensar Humedad ambiente
  		HumAmb = (5.0 /1024 * analogRead(PinHumAmb))*100-50;
  		if (VarEstAspe == 1){
   			MjeDefinido(6);
          	MostrarMje(9,0,"XX", HumAmb);
          	MostrarMje(11,0,"%",0);
          	MjeDefinido(10);
          	MostrarMje(7,1,"Adecuando",0);
          	Serial.write('i');
          	if (HumAmb >=80) {
          		VarEstAspe = 0;
             	Serial.write('j'); 	
            }
          	delay(1000);
        }    
  		if (VarEstCaHu == 1){
   			MjeDefinido(6);
          	MostrarMje(9,0,"XX", HumAmb);
          	MostrarMje(11,0,"%",0);
          	MjeDefinido(10);
          	MostrarMje(7,1,"Adecuando",0);
          	Serial.write('k');
          	if (HumAmb <=70) {
          		VarEstCaHu = 0;
             	Serial.write('l'); 	
            }
          	delay(1000);
        }          
		if (VarEstAspe != 1 && VarEstCaHu != 1 && HumAmb >= 60 && HumAmb <= 80) {          
	        MjeDefinido(6);
	      	MostrarMje(9,0,"XX", HumAmb);
         	MostrarMje(11,0,"%",0);
           	MjeDefinido(10);
    		MostrarMje(8,1,"Normal",0);          	
          	if (VarEstHuSu == 0) { //lo cruzo con humedad de suelo
          		Serial.write('j'); // para no apagar aspesor cuando se encesite
            }
  			Serial.write('l');
         	delay(1000);
        }  
  		if (HumAmb < 60 && VarEstAspe != 1) {
   			MjeDefinido(6);
          	MostrarMje(9,0,"XX", HumAmb);
          	MostrarMje(11,0,"%",0);
          	MjeDefinido(10);
          	MostrarMje(7,1,"Baja",0);
          	Serial.write('i');
          	VarEstAspe = 1;
          	delay(1000);

        }
        if (HumAmb > 80 && VarEstCaHu != 1){
   			MjeDefinido(6);
          	MostrarMje(9,0,"XX", HumAmb);
   	      	MostrarMje(11,0,"%",0);
           	MjeDefinido(10);
           	MostrarMje(7,1,"Alta",0);
           	Serial.write('k');
       		VarEstCaHu = 1;
      	  	delay(1000);
        }
		break;

  	case 3: //sensar humedad suelo
  		HumSuelo = (5.0 /1024 * analogRead(PinHumSuelo))*100-50;
  		if (VarEstHuSu == 1) {
	    		MjeDefinido(7);
	          	MostrarMje(11,0,"XX", HumSuelo);
    	      	MostrarMje(13,0,"%",0);
          		MjeDefinido(10);
          		MostrarMje(7,1,"Adecuando",0);
          		Serial.write('i');
          		if (HumSuelo >=70) {
          			VarEstHuSu = 0;
        	  		Serial.write('j');
                } 
                delay(1000);        
        }
  		if (VarEstHuSu == 0 && HumSuelo >=50){
  				MjeDefinido(7);
	          	MostrarMje(11,0,"XX", HumSuelo);
    	      	MostrarMje(13,0,"%",0);
          		MjeDefinido(10);
          		MostrarMje(7,1,"Normal",0);          	
          		if (VarEstAspe == 0) {   //aca cruzo con var de hum amb para no 
          			Serial.write('j');   // apagar aspersor cuando es necesario 
                }
        	  	delay(1000);
        }
  		if (HumSuelo < 50 && VarEstHuSu != 1) {
  			    MjeDefinido(7);
	          	MostrarMje(11,0,"XX", HumSuelo);
    	      	MostrarMje(13,0,"%",0);
          		MjeDefinido(10);
          		MostrarMje(7,1,"Baja",0);
          		Serial.write('i');
          		VarEstHuSu = 1;
        	  	delay(1000);
        }
  		break;
  	
  	case 4: //detectar pulsación boton motor puerta externa   
  		// ELIMINADO (MODIFICADO A TRAVES DE INTERRUPCIONES)
  		break;  
  	
  	case 5: //sensar movimiento interno
  		VarPir2 = digitalRead(PinIntPir);
  		if (VarPir2 == HIGH){
   		    MjeDefinido(9);
          	MostrarMje(10,1,"SI", 0);
   	      	Serial.write('c');
          	Serial.write('j');
          	delay(1000);
        }
  		else {
   		    MjeDefinido(9);
          	MostrarMje(10,1,"NO", 0);
          	Serial.write('d');
   	      	delay(1000);
        }
  		break;
  	
  	case 6: //Sensar iluminación
  		VarLuz = analogRead(PinLuz);
  		if (VarLuz < 100){
	       	MjeDefinido(11);
			MostrarMje(10,1,"BAJA", 0);
          	Serial.write('a');
        	delay(1000);		
        }
  		else {
			MjeDefinido(11);
			MostrarMje(10,1,"ALTA", 0);
          	Serial.write('b');
        	delay(1000);		
        }
  		break;  	
  	
  default:
  		break;
}
}
//---------------------------------------------------
// Procedmiento que detiene todo el funcionamiento del invernadero
void PararTodo() {
	Serial.write('b');
 	Serial.write('d');
  	Serial.write('b');
  	Serial.write('h');
  	Serial.write('j');	
  	Serial.write('l');
}

//---------------------------------------------------
// Procedimientos de interrupción
void AbrirPta() {
	MjeDefinido(8);
	MjeDefinido(10);
	MostrarMje(8,1,"ABRIR", 0);
	Serial.write('e');
  	for (Contador = 0; Contador < 50; Contador++){
  		delayMicroseconds(1000);
    }
  	
}

//---------------------------------------------------
void CerrarPta() {
	MjeDefinido(8);
	MjeDefinido(10);
	MostrarMje(8,1,"CERRAR", 0);
	Serial.write('f');
	for (Contador = 0; Contador < 50; Contador++){
  		delayMicroseconds(1000);
    }
  	
}
