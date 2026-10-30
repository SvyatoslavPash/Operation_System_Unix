#!/bin/sh
VOLUME_DIR="/data"
LOCK_FILE="$VOLUME_DIR/.lock"
LOCK_FD=200

exceptions(){
    echo "Ошибка: $1" >&2
    exit "${2:-1}"
}

[ -d "$VOLUME_DIR" ] || exceptions "каталог $VOLUME_DIR не существует"
[ -w "$VOLUME_DIR" ] || exceptions "нет прав на запись в $VOLUME_DIR"

CONTAINER_ID=$(cat /proc/sys/kernel/random/uuid)

FILE_COUNTER=0
CURRENT_FILE=""

exit_handler() {
    if [ -n "$CURRENT_FILE" ]; then
        rm -f "$CURRENT_FILE"
        echo "Trap: Удалён файл $CURRENT_FILE при выходе."
    fi
    exit 0
}
trap exit_handler EXIT HUP INT QUIT PIPE TERM

echo "Контейнер $CONTAINER_ID запущен."

eval "exec ${LOCK_FD}>>\"\$LOCK_FILE\"" || exceptions "не удалось открыть lock_file $LOCK_FILE"

while true; do
    FILE_COUNTER=$((FILE_COUNTER + 1))

    flock -x "$LOCK_FD" || exceptions "не удалось захватить блокировку"

    i=1
    while true; do
        num=$(printf '%03d' "$i")
        filename="$VOLUME_DIR/$num"

        if [ ! -e "$filename" ]; then
            echo "$CONTAINER_ID $FILE_COUNTER" > "$filename"
            echo "Создан: $filename"
            CURRENT_FILE="$filename"
            break
        fi
        i=$((i + 1))
    done

    flock -u "$LOCK_FD"

    sleep 1

    rm -f "$CURRENT_FILE"
    echo "Удалён: $CURRENT_FILE"
    CURRENT_FILE=""

    sleep 1
done
