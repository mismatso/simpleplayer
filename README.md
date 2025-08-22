# SimplePlayer

![Debian](https://img.shields.io/badge/debian-12+-green)
![Ubuntu](https://img.shields.io/badge/ubuntu-22.04-green)
![License](https://img.shields.io/github/license/mismatso/simpleplayer)

Una interfaz de línea de comandos que implementa un reproductor de música en modo de texto, que permite cargar, reproducir, pausar y gestionar música mp3 desde una lista de reproducción personalizable. Escrito en C++.

## Requisitos

- g++ (soporte C++17 o superior).
- nlohmann/json (se descarga como vendor/json.hpp).
- ffmpeg (para ffplay y ffprobe).

## Clonar el repositorio

Abra una terminal y clone el repositorio.

```bash
git clone https://github.com/mismatso/simpleplayer.git ~/.simpleplayer
```

## Instalar dependencias en GNU/Linux Debian

Simple Player requiere la librería JSON de Niels Lohmann (nlohmann).

```bash
cd ~/.simpleplayer

mkdir vendor

wget https://github.com/nlohmann/json/releases/latest/download/json.hpp -O vendor/json.hpp
```

SimplePlayer también requiere `ffplay` y `ffprobe`, que forman parte de la suite de herramientas de `ffmpeg`.

```bash
sudo apt install ffmpeg
```

## Generar el archivo de canciones.json

Simple Player requiere que se exista una archivo `canciones.json`, el cual deberá conteder el índice de las canciones disponibles en la biblioteca del usuario. Es a partir de esta lista general de canciones que el usuario podrá crear su lista de reproducción.

```bash
./generar_biblioteca.sh ~/Music/mp3
```

Lo anterior generá un archivo JSON, con esta estructura.

```json
[
  {
    "artista": "Soda Stereo",
    "titulo": "De música ligera",
    "duracion_minutos": 3.50,
    "directorio": "/home/jdoe/Music/mp3",
    "archivo": "Soda Stereo - De música ligera.mp3"
  },
  {
    "artista": "La Unión",
    "titulo": "Lobo Hombre en París",
    "duracion_minutos": 3.85,
    "directorio": "/home/jdoe/Music/mp3",
    "archivo": "La Unión - Lobo Hombre en París.mp3"
  }
]
```

## Compilar SimplePlayer

Para compilar el código fuente de SimplePlayer, asegúrese de tener instalado un compilador de C++ como `g++`. Luego, ejecute el siguiente comando en la terminal:

```bash
g++ -std=c++17 -I vendor -o bin/simpleplayer simpleplayer.cpp
```

Ahora puedes crear un enlace simbólico para facilitar la ejecución de SimplePlayer:

```bash
sudo ln -s ~/.simpleplayer/bin/simpleplayer /usr/local/bin/simpleplayer
```

Si no tiene permisos de sudo, puede agregar ~/.simpleplayer/bin a su PATH en .bashrc o .zshrc:

```bash
export PATH="$HOME/.simpleplayer/bin:$PATH"
```

## Ejecutar SimplePlayer

Si creaste el enlace simbólico o modificaste tu PATH, ahora puedes ejecutar SimplePlayer desde cualquier lugar:

```bash
simpleplayer
```
En caso contrario, puedes ejecutar SimplePlayer desde el directorio `bin`:

```bash
~/.simpleplayer/bin/simpleplayer
```

## Contribuir

¡Las contribuciones son bienvenidas!, para colaborar:

1. Haga un fork del repositorio.
2. Cree una rama (git checkout -b feature/nueva-funcionalidad).
3. Haga un commit con los cambios (git commit -m "Agrego nueva funcionalidad").
4. Haga un push (git push origin feature/nueva-funcionalidad).
5. Abra un Pull Request en GitHub.

Si encuentra errores o desea proponer mejoras, use la sección Issues.

## Autor

Este proyecto ha sido desarrollado por [Misael Matamoros](https://t.me/mismatso).

- YouTube: [MizaqScreencasts](https://www.youtube.com/MizaqScreencasts)
- Twitter: [@mismatso](https://twitter.com/mismatso)
- Telegram: [@mismatso](https://t.me/mismatso)

## Licencia

[SimplePlayer](https://github.com/mismatso/simpleplayer) © 2024 por [Misael Matamoros](https://t.me/mismatso) está licenciado bajo los términos de la **GNU General Public License, version 3 (GPLv3)**. Para más detalles, consulte el archivo [LICENSE](/LICENSE).

[![GPLv3](https://www.gnu.org/graphics/gplv3-with-text-136x68.png)](https://www.gnu.org/licenses/gpl-3.0.html)