#!/bin/bash

echo "Iniciando batería de pruebas..."
echo "(Esto puede tardar unos minutos, la pantalla se irá actualizando)"
echo "--------------------------------------------------------"

# Variable para llevar la cuenta de la partida actual
CONTADOR=1

# Función para jugar una partida, ocultar el texto basura y evaluar el ganador
jugar_partida() {
    local nivel_ninja=$1
    local somos_j1=$2 # 1 si empezamos nosotros, 2 si empieza el ninja
    
    if [ "$somos_j1" -eq 1 ]; then
        comando="./n_en_raya -p1 inteligente -id1 1 -p2 $nivel_ninja -nogui"
        mensaje_pantalla="Partida $CONTADOR vs $nivel_ninja (como primer jugador) en proceso..."
    else
        comando="./n_en_raya -p1 $nivel_ninja -p2 inteligente -id2 1 -nogui"
        mensaje_pantalla="Partida $CONTADOR vs $nivel_ninja (como segundo jugador) en proceso..."
    fi

    # Mostramos por pantalla que la partida ha empezado
    echo -n "$mensaje_pantalla "

    # Ejecutamos el juego, guardamos la salida y filtramos la línea final
    salida=$(eval $comando)
    linea_final=$(echo "$salida" | grep "PARTIDA FINALIZADA")

    # Evaluamos quién ha ganado basándonos en la línea final
    resultado=""
    if [[ "$linea_final" == *"Gana el Jugador 1"* ]]; then
        if [ "$somos_j1" -eq 1 ]; then
            resultado="VICTORIA"
        else
            resultado="DERROTA"
        fi
    elif [[ "$linea_final" == *"Gana el Jugador 2"* ]]; then
        if [ "$somos_j1" -eq 2 ]; then
            resultado="VICTORIA"
        else
            resultado="DERROTA"
        fi
    else
        resultado="EMPATE"
    fi

    # Mostramos por pantalla que ha terminado y el resultado
    echo "[¡Completada! -> $resultado]"

    # Aumentamos el contador para la siguiente partida
    CONTADOR=$((CONTADOR + 1))
}

# 1. Partidas empezando nosotros (Jugador 1)
jugar_partida "ninja1" 1
jugar_partida "ninja2" 1
jugar_partida "ninja3" 1
jugar_partida "ninja4" 1

# 2. Partidas empezando los ninjas (Jugador 2)
jugar_partida "ninja1" 2
jugar_partida "ninja2" 2
jugar_partida "ninja3" 2
jugar_partida "ninja4" 2

echo "--------------------------------------------------------"
echo "¡Batería finalizada con éxito!"
