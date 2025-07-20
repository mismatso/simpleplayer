#!/bin/bash

# Mensaje de bienvenida
echo "=== Generador de biblioteca de canciones en formato JSON ==="
echo "Este script generará un archivo JSON con la información de"
echo "las canciones en el directorio especificado."
echo

# Si no se paso ningún argumento la variable $directory es igual a “./mp3”
directory=${1:-"./mp3"}
file_name="./bin/canciones.json"

# Verificar si el directorio existe
if [ ! -d "$directory" ]; then
  echo "El directorio $directory no existe." && echo
  exit 1
fi 

# Verificar si ffprobe está instalado
if ! command -v ffprobe &> /dev/null; then
  echo "ffprobe no está instalado. Por favor, instálalo para continuar."
  exit 1
fi

# Crear o vaciar el archivo $file_name
echo -n "" > $file_name

echo "Generando el archivo $file_name"
echo "indexando los MP3 del directorio: [$directory]"

# Iniciar el archivo JSON
echo "[" > $file_name

while IFS= read -r line; do

  # Ignorar líneas vacías
  [[ -z "$line" ]] && continue
  
  directorio="$directory"
  archivo="$line.mp3"

  # Aquí utilizamos el patrón «suffix removal» (%) para extraer el nombre del artista
  # del nombre del archivo, asumiendo que el formato es "artista - título".
  artista="${line%% - *}"

  # Aquí utilizamos el patrón «prefix removal» (#) para extraer el título
  # del nombre del archivo, asumiendo que el formato es "artista - título".
  titulo="${line#* - }"

  # Obtener duración en segundos
  duracion=$(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 "$directorio/$line.mp3")

  # Si no se pudo obtener la duración, continuar
  [[ -z "$duracion" ]] && continue

  # Convertir duración a minutos con dos decimales
  duracion_minutos=$(awk "BEGIN { printf \"%.2f\", $duracion / 60 }")

  # Generar entrada JSON
  echo "  {" >> $file_name
  echo "    \"artista\": \"${artista}\"," >> $file_name
  echo "    \"titulo\": \"${titulo}\"," >> $file_name
  echo "    \"duracion_minutos\": ${duracion_minutos}," >> $file_name
  echo "    \"directorio\": \"${directorio}\"," >> $file_name
  echo "    \"archivo\": \"${archivo}\"" >> $file_name
  echo "  }," >> $file_name

done < <(find "$directory" -type f -name "*.mp3" -exec basename {} \; | sed 's/\.mp3$//')

# Eliminar la última coma y cerrar JSON
sed -i '$ s/},/}/' $file_name
echo "]" >> $file_name

echo "Archivo $file_name generado exitosamente." && echo
