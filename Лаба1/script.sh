#!/bin/sh
set -e  # Выходить при любой ошибке

exceptions(){
    echo "Ошибка: $1" >&2
    exit "$2"
}

file="$1"
outputName=""

if [ $# -eq 0 ]; then exceptions "укажите файл" 1; fi

out_dir=$(pwd)
case "$file" in
    /*) abs_file="$file" ;;
    *) abs_file="$out_dir/$file" ;;
esac

if [ ! -r "$abs_file" ]; then exceptions "файл не читается" 2; fi

outputName=$(grep -m 1 "Output:" "$abs_file" | sed 's/.*Output:[[:space:]]*//' || true)

if [ -z "$outputName" ]; then exceptions "не найден комментарий Output:" 3; fi

tmpDir=$(mktemp -d)

exit_handler() {
    rc=$?
    trap - EXIT
    rm -rf "$tmpDir"
    exit $rc
}
trap exit_handler EXIT HUP INT QUIT PIPE TERM

cd "$tmpDir"

ext="${abs_file##*.}"

if [ "$ext" = "c" ]; then
    if ! cc "$abs_file" -o "$outputName"; then exceptions "компиляции C" 4; fi
elif [ "$ext" = "cpp" ]; then
    if ! c++ "$abs_file" -o "$outputName"; then exceptions "компиляции C++" 4; fi
elif [ "$ext" = "tex" ]; then
    baseName=$(basename "$abs_file" .tex)

    if ! pdflatex -interaction=nonstopmode "$abs_file" > /dev/null; then 
        exceptions "компиляции LaTeX" 4
    fi
    
    mv "$baseName.pdf" "$outputName.pdf"
    outputName="$outputName.pdf"
else
    exceptions "неизвестный формат файла" 5
fi 

mv "$outputName" "$out_dir/$outputName"

echo "Сборка завершена успешно"
exit 0
