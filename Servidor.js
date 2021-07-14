 /************************** configuración general *************************************/    
    const express = require('express'); 
    const app = express(); //asignamos a la variable app la clase/metodo express 
    var http = require('http'); //libreria http nativa de node.js         
    const server = http.createServer(app); //creacion del servidor
 	var xorIt = require('xor-it');  //modulo de cifrado   
 /**********************configuracion del servidor websocket*************************/
    
    const WebSocket = require('ws');
    const s = new WebSocket.Server({ server });

    //Calculo de la clave: nodoCom envia un challenge a la PUF y esta devuelve una respuesta, que servira como entrada para generar la clave en el generador Trifork.
    var challenge = nodoCom();
    var respuesta = PUF(challenge);    
    var key = trifork(respuesta);
    //Debido a problemas con los tipos de datos en C/C++, se ha optado por que las claves sean enteros positivos
    if(key < 0)
    	key = Math.abs(key);

    console.log("Valores para Challenge, respuesta, key: " +challenge + " " +respuesta + " " +key);
    //Comentar estas cuatro lineas cuando se quiera recibir mensajes por parte del RPAS.
    var mensaje = "Este es un mensaje enviado desde el nodo terrestre";
    console.log("Texto original: " +mensaje);
    var ciphertext = xorIt (key.toString(), mensaje);
    console.log("Texto cifrado: " +ciphertext);
        
    
  /*************************** comunicacion websocket ********************************************************************************/
    
    s.on('connection',function(ws,req){    
	//Ejecución cuando un cliente se conecta al servidor
    
    ws.send(challenge);
    //comentar para recibir mensajes por parte del RPAS   
   	ws.send(ciphertext);
    
    ws.on('message',function(message){
	//Ejecución cuando el servidor recibe un mensaje

    console.log("Texto cifrado recibido: "+message);
    let texto_descifrado = xorIt(key.toString(), message);
    console.log("Texto descifrado: " +texto_descifrado);      
    });
    
    ws.on('close', function(){
    //Ejecución cuando un cliente cierra la conexion con el servidor

    console.log("Se ha perdido la conexion con un cliente");
    });    
    console.log("Nuevo cliente conectado");
    });
    server.listen(8080);





//Definicion de funciones

	//Funcion que simula un módulo de comunicaciones
    function PUF(input){
	var output;
	switch(input){
		case 3:
			output = input + 2;
			break;
		case 5: 
			output = input + 3;
			break;

		case 9:
			output = input + 1;
			break;
		default:
			output = 3;	
	}
	return output;
}

	//Generador Trifork
	function trifork(input){
	var xn, yn , zn;
	
	var wn;

	var xn_, yn_, zn_;
	xn = input;
	yn = input;
	zn = input;	

	var m= 4, d=5;
	
	var r1 = 4, r2 = 7, r3 = 2;
	
	var s1 = 5, s2 = 8, s3 = 6;

	xn_= ((xn-r1 + xn-s1) % m) >> d;
	yn_= ((yn-r2 + yn-s2) % m) >> d;
	zn_= ((zn-r3 + zn-s3) % m) >> d;

	xn= ((xn-r1 + xn-s1) % m) ^ zn_;
	yn= ((yn-r2 + yn-s2) % m) ^ xn_;
	zn= ((zn-r3 + zn-s3) % m) ^ yn_;

	wn = xn ^ zn;
	
	return wn;
	}
	//Nodo de comunicaciones  | Genera un entero del 1 al 10
	function nodoCom(){
	var num = Math.floor((Math.random()*10)+1);
	return num;
	}
