/**
 * Servidor HTTP simple.
 * @author: Miguel Angel Piña Avelino
 * @version: 0.1
 * Compilación: gcc -W -o servidorhttp server.c
 * Ejecución: sudo ./servidorhttp
 * Este sencillo servidor es capaz de servir páginas en formato HTML.
 *
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 80 //Definimos el número de puerto
#define BACKLOG 5 //Número de conexiones entrantes
#define MAX_DATA 1024 //Número de tamaño máximo de buffer
#define MAX_LENGTH_PATH 256 //Tamaño máximo del path para el archivo pedido.
#define BEGIN_STRING_FILE 5 // Índice a partir del que se empezará a leer la petición.


/**
 * En caso de que no se encuentre, se enviará un mensaje 404.
 * int socket(int domain, int type, int protocol);
 * Analicemos los argumentos:
 *
 * --domain-- Se podrá establecer como AF_INET (para usar 
 * los protocolos ARPA de Internet), o como AF_UNIX (si 
 * se desea crear sockets para la comunicación interna 
 * del sistema). Éstas son las más usadas, pero no las 
 * únicas. Existen muchas más, aunque no se nombrarán aquí.
 *
 * --type-- Aquí se debe especificar la clase de socket que
 * queremos usar (de Flujos o de Datagramas). Las variables
 * que deben aparecer son SOCK_STREAM o SOCK_DGRAM según 
 * querramos usar sockets de Flujo o de Datagramas, respectivamente.
 * 
 * --protocol-- Aquí, simplemente se puede establecer el protocolo a 0.
 * La función socket() nos devuelve un descriptor de socket, el 
 * cual podremos usar luego para llamadas al sistema. Si nos devuelve
 * -1, se ha producido un error (obsérvese que esto puede resultar útil
 *  para rutinas de verificación de errores).
 */

int main(){
	int fd_servidor, fd_cliente; /*Los descriptores de fichero*/
	struct sockaddr_in cliente; /*Para la información de la dirección del cliente*/
	struct sockaddr_in servidor; /*Para la información de la dirección del servidor*/
	int sin_size;
	char bufenc[MAX_DATA]; //Buffer de lectura 
	char bufenv[MAX_DATA]; //Buffer de escritura
	char path[MAX_LENGTH_PATH]; //Buffer para path del archivo
	FILE *file_to_send; //Archivo que vamos a enviar
	int character; //Caracter que nos permitirá leer desde el archivo
	// Mensaje de error
	char* error =   "HTTP/1.0 404 Not Found\n"
			"Content-type: text/html\n"
  			"\n"
  			"<html>\n"
  			" <body>\n"
  			"  <h1>No se encontr&oacute; la p&aacute;gina</h1>\n"
  			"  <p>La p&aacute;gina solicitada no fue encontrada en el servidor.</p>\n"
  			" </body>\n"
  			"</html>\n";
 
	int sum, len,aux;
	int count = 0;

	if((fd_servidor = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Error al invocar la función socket()\n");
		exit(-1);
	}

	/* Indicamos que el tipo de conexión será
	 * utilizando los protocolos DARPA de internet */
	servidor.sin_family = AF_INET;

	/* Convertimos el valor de puerto a bits de variable corta de red*/
	servidor.sin_port = htons(PORT); 
	/* Indicamos que debe de tomar la dirección IP 
	 * de la máquina en que se está ejecutando*/
	servidor.sin_addr.s_addr = INADDR_ANY;
	/* Ponemos en cero el resto de la estructura*/
	memset(&(servidor.sin_zero),'\0',8);

	/* Unimos los datos definidos en la variable servidor 
	 * con el socket creado anteriormente*/
	if( bind( fd_servidor,(struct sockaddr *)&servidor, 
		sizeof(struct sockaddr)) == -1){
		printf("Error al invocar la función bind()\n" );
		exit(-1);
	}

	/* Hacemos la llamada a listen, donde indicamos el número de conexiones que soportará */
	if( listen(fd_servidor, BACKLOG) == -1){
		printf("Error al invocar la función listen()\n" );
		exit(-1);
	}

	while(1){
		sin_size = sizeof(struct sockaddr_in);
		if((fd_cliente = accept(fd_servidor, (struct sockaddr *) &cliente,
			&sin_size)) == -1){
			printf("Error al invocar la función accept()\n");
			exit(-1);
		}
		/* Se muestra la IP del cliente */
		printf("Se obtuvo una conexión desde %s\n", inet_ntoa(cliente.sin_addr));
		/* Leemos lo recibido por el cliente*/
		len = recv(fd_cliente,bufenc,1024,0);
		bufenc[len] = '\0';
		printf("Recibido: %s\n", bufenc);
		aux = 0;
		/* Procesamos los header recibidos */
		while(bufenc[BEGIN_STRING_FILE + aux] != ' '){
			path[aux] = bufenc[BEGIN_STRING_FILE + aux];
			aux++;
		}
		if(aux < 1){
			strcpy(path,"index.html");
		}else{
			path[aux] = '\0';
		}
		printf("Nombre del archivo a enviar: %s\n", path );
		/* Se envia un mensaje de bienvenida al cliente*/
		//send(fd_cliente,error,22,0);
		if(fork() == 0){
			close(fd_servidor); //El proceso hijo no lo necesita
			if((file_to_send = fopen(path,"r")) != (FILE*)0){
				count = 0;
				sum = 0;
				while((character = getc(file_to_send)) != EOF){
					bufenv[count++] = character;
					sum++;
					if(count >= 1024){
						send(fd_cliente,bufenv,count,0);
						count = 0;
					}
				}
				send(fd_cliente,bufenv,count,0);
				printf("Total de bytes enviados del archivo (%s): %d\n", path, sum);	
			} else{
				send(fd_cliente,error,strlen(error),0);
				sum = strlen(error);
				printf("Total de bytes enviados del archivo (%s): %d\n", "error", sum);	
			}
			fclose(file_to_send);
			close(fd_cliente);
			printf("Total de bytes enviados: %d\n", sum);
			exit(0);
		}else{
			close(fd_cliente);
		}
	}
	return 0;
}

