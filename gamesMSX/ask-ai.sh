#!/bin/bash

rm ask-ai-out.txt

# Llista de carpetes a ignorar
# ignore_list=".vscode|InteractMUI|SwiftMUI|SwiftUI-Official|xcode|__MACOSX|.build|.swiftpm|Tests"
ignore_list="00-OpenGLHelloWorld|01-VulkanHelloWorld|.vscode|__MACOSX|.build|.swiftpm|Tests"

# Funció per convertir la llista de carpetes en un format usable per 'find'
convert_for_find() {
  echo $(echo $1 | sed 's/|/ -o -path .\//g; s/^/-path .\//')
}

echo "This is the folders and files structure:" > ask-ai-out.txt

# Guarda l'estructura del projecte (ignorant les carpetes especificades) en un fitxer temporal
tree . -I $ignore_list >> ask-ai-out.txt

# Missatge inicial per als arxius .swift
echo "These are the files:" >> ask-ai-out.txt

# Busca tots els arxius .swift ignorat les carpetes especificades i guarda el nom seguit del seu contingut
find . \( $(convert_for_find $ignore_list) \) -prune -o -type f -name "*.swift" -exec bash -c 'echo "----- {}" >> ask-ai-out.txt; cat {} >> ask-ai-out.txt; echo "" >> ask-ai-out.txt' \;

echo "Process completed. Output saved in ask-ai-out.txt."
