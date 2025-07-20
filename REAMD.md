# SIMPLE PLAYER

## Instalar dependencias en GNU/Linux Debian

Simple Player requiere la librería JSON de Niels Lohmann (nlohmann).

```bash
mkdir vendor
wget https://github.com/nlohmann/json/releases/latest/download/json.hpp -O vendor/json.hpp
```

Se requiere el programa `mpg123`, que es un reproductor de archivos de audio en formato MP3 que funciona desde la línea de comandos. Es una herramienta ligera, rápida y eficiente, comúnmente utilizada en sistemas GNU/Linux y Unix-like, aunque también está disponible para otras plataformas.

```bash
sudo apt-get install mpg123
```

También se requiere `ffprobe`, que forma parte de la suite de herramientas que se instalan `ffmpeg`.

```bash
sudo apt install ffmpeg
```

## Genera el archivo de canciones.json

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

