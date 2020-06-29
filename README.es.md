[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) ![Platforms: linux-64 | win-64](https://img.shields.io/badge/platform-linux--64%20|%20win--64-success.svg)

![English Documentation](https://github.com/d3adlym1nd/unnamed_rat/blob/master/README.md)
# unnamed_rat
Herramienta de Administracion Remota multiplataforma desde linea de comandos (RAT)

### Dependencias
- libssl
- libnotify (Notificaciones de escritorio) - Opcional

### Funciones incluidas en el cliente
- Conexion atravez de TLS
- Transferencia de archivos
- Shell inversa interactiva
- Recopilacion de informacion
- Descarga de archivos de servidores HTTP(s)

### Languaje
Edita el fichero `headers.hpp` y descomenta el lenguaje que quieres usar en el programa.
```cpp
//Español
#define ES

//Ingles
#define EN
```

### Miscelaneo
Para habilitar la salida de texto con colores, o las notificaciones de escritorio(Linux), edita el fichero `headers.hpp` y comenta/descomenta las siguientes lineas para activar/desactivar estas opciones
```cpp
#define _COLOR 

//Solo linux
#define _NOTIFY
```
Para usar la opcion de `notify` se debe instalar `libnotify` en el sistema, y compilar el programa con el archivo `makenotify`. Asi: `make -f makenotify`

### Compilando (Cliente)
Edita el archivo `headers.hpp` y comenta/descomenta esta linea:
```cpp
#define _DEBUG
```
para habilitar/deshabilitar la salida de texto a la consola. Edita el archivo `main.cpp` y cambia la siguiente linea:
```cpp
Cli->Connect("YOUR HOST", "PORT")
```
con el host y puerto al cual el cliente se conectara. Si la opcion `#define _DEBUG` se dejara descomentada, entonces el host y el puerto se deben pasar como argumentos al cliente. Ejemplo `./Client 127.0.0.1 31337`. Compilar con `make`.

### Compilando (Servidor)
Generar el certificado y llave privada en el servidor con `openssl req -x509 -newkey rsa:4096 -out cacer.pem -outform PEM -days 1825 -nodes`, copia ambos archivos `cacer.pem` y `prikvey.pem` en el directorio del servidor.
Edita el archivo `headers.hpp` y modifica la linea
```cpp
#define Max_Clients 10
```
si quieres manejar mas de 10 clientes. Compilar con `make`.


### Servidor (Termux)
Para usarlo en termux primero se instala el software requerido:
```sh
pkg install openssl openssl-tool make git nano
```
Luego editar el archivo `headers.hpp` con `nano` y descomentar lo siguiente:
```cpp
#define _TERMUX
```
 generar el certificado y la llave privada como se explica arriba, finalmente compilar con `make`.

### Comandos del servidor
Escribe `ayuda`, `?`, o `aiuda` a cualquier momento(a excepcion de la shell inversa) para mostrar los comandos disponibles.

#### Shell Principal
Comando | Descripcion
------- | ------
cli | Ejecuta acciones en los clientes
ayuda, ?, aiuda | Muestra comandos disponibles
salir | Finaliza el servidor

#### Shell Cliente (Modo interactivo)
Commando | Descripcion
-------- | ------
subir | Sube un archivo local al cliente
descargar | Descarga archivos del cliente
shell | Invoca un shell interactiva inversa
httpd | Fuerza al cliente a descargar un archivo desde una url
info | Obtiene informacion basica del cliente
ayuda, ?, aiuda | Muestra los comandos disponibles
salir | Cierra sesion interactiva (No la conexion)
- Mas en camino...

## Probado en
- Debian
- Arch
- Windows 7
- Windows 10
- Android 9 (Termux)

## Contribuciones
Cualquier tipo de contribucion es bienvenida!!!

## Capturas de pantalla
![](https://i.imgur.com/r6FewoQ.jpg)
![](https://i.imgur.com/fUgwlZx.jpg)
![](https://i.imgur.com/AZqPXmg.jpg)

![](https://i.imgur.com/p04wBN1.jpg)

![](https://i.imgur.com/NF7cQUC.jpg)

![](https://i.imgur.com/7Q4yjxh.jpg)

![](https://i.imgur.com/TNRV7kh.jpg)

# Descargo de responsabilidad (DISCLAIMER)
Este software esta hecho con propositos de pruebas solamente, diseñado para ejecutarse en un ambiente controlado, no debe ejecutar en un ambiente de trabajo real. El desarollador no es responsable de cualquier daño o repercusiones legales. Usalo bajo tu propio riesgo.
