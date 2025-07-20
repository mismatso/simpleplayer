#!/bin/bash

# Mensaje de bienvenida
echo "=== Generador de biblioteca de canciones en formato JSON ==="
echo "Este script generará un archivo JSON con la información de"
echo "las canciones en el directorio especificado."
echo

# Si no se paso ningún argumento la variable $directory es igual a “./mp3”
directory=${1:-"./mp3"}

# Verificar si el directorio existe
# imprimo un enter antes del mensaje de error

if [ ! -d "$directory" ]; then
  echo "El directorio $directory no existe." && echo
  exit 1
fi 

# Verificar si ffprobe está instalado
if ! command -v ffprobe &> /dev/null; then
  echo "ffprobe no está instalado. Por favor, instálalo para continuar."
  exit 1
fi

# Crear o vaciar el archivo canciones.json
echo -n "" > canciones.json

echo "Generando el archivo canciones.json"
echo "en el directorio $directory..."

# Iniciar el archivo JSON
echo "[" > canciones.json

while IFS= read -r line; do

  # Ignorar líneas vacías
  [[ -z "$line" ]] && continue
  
  directorio="$directory"
  archivo="$line.mp3"
  artista="${line%% - *}"
  titulo="${line#* - }"

  # Obtener duración en segundos
  duracion=$(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 "$directorio/$line.mp3")

  # Si no se pudo obtener la duración, continuar
  [[ -z "$duracion" ]] && continue

  # Convertir duración a minutos con dos decimales
  duracion_minutos=$(awk "BEGIN { printf \"%.2f\", $duracion / 60 }")

  # Generar entrada JSON
  echo "  {" >> canciones.json
  echo "    \"artista\": \"${artista}\"," >> canciones.json
  echo "    \"titulo\": \"${titulo}\"," >> canciones.json
  echo "    \"duracion_minutos\": ${duracion_minutos}," >> canciones.json
  echo "    \"directorio\": \"${directorio}\"," >> canciones.json
  echo "    \"archivo\": \"${archivo}\"" >> canciones.json
  echo "  }," >> canciones.json

done < <(find "$directory" -type f -name "*.mp3" -exec basename {} \; | sed 's/\.mp3$//')

# Eliminar la última coma y cerrar JSON
sed -i '$ s/},/}/' canciones.json
echo "]" >> canciones.json

echo "Archivo canciones.json generado exitosamente." && echo
