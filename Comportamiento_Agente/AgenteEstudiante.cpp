#include "AgenteEstudiante.hpp"
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

AgenteEstudiante::AgenteEstudiante(int id, int profundidadMax, double tiempoMax, int numHeuristica, ModoJuego modo) 
    : id(id), profundidadMax(profundidadMax), tiempoMaxSegundos(tiempoMax), numHeuristica(numHeuristica), modo(modo), abortarBanda(false) {
    nodosVisitados = 0;
}

bool AgenteEstudiante::tieneLimiteDeTiempo() const {
    return modo != ModoJuego::STATUS;
}

std::pair<int, int> AgenteEstudiante::think(const Tablero& tablero) {
    std::pair<int, int> mejor;
    nodosVisitados = 0;
    abortarBanda = false;
    inicioBusqueda = std::chrono::steady_clock::now();

    switch (modo)
    {
    case ModoJuego::ALEATORIO:
        return JuegaAleatorio(tablero);
        break;
    
    case ModoJuego::STATUS:
        Status(tablero, mejor);
        return mejor;
        break;    

    case ModoJuego::MINIMAX:
        minimax(tablero, 0, profundidadMax, mejor);
        return mejor;
        break; 

    case ModoJuego::INTELIGENTE:
        return JuegaInteligente(tablero);   
        break;
    }
        
    return {-1, -1};
}


/**
 * @brief Compara dos tableros para identificar cuál ha sido el movimiento realizado.
 * @param padre Estado inicial del tablero.
 * @param hijo Estado resultante tras un movimiento.
 * @return Un par (fila, columna) con la posición de la nueva pieza.
 */
std::pair<int, int> SacarMovimiento(const Tablero& padre, const Tablero &hijo){
    for(int f=0; f<padre.getFilas(); ++f)
        for(int c=0; c<padre.getColumnas(); ++c)
            if (padre.getCelda(f,c) == 0 && hijo.getCelda(f,c) != 0) 
                return {f, c};
    return {-1, -1};
}

/**
 * @brief Implementa un agente que juega de forma totalmente aleatoria.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida al azar.
 */
std::pair<int, int> AgenteEstudiante::JuegaAleatorio(const Tablero& tablero) {

    // Calculo los tableros descendientes de tablero
    auto sucesores = tablero.getSucesores();

    // Si no tiene descendientes, paso el turno
    if (sucesores.empty()) return {-1, -1};

    // Elijo aleatoriamente uno de los descendientes
    int elegido = rand() % sucesores.size();

    // Saco el movimiento realizado comparando el tablero original con el elegido.
    std::pair<int,int> Mov = SacarMovimiento(tablero, sucesores[elegido]);

    return Mov;
}


/**
 * @brief Algoritmo de resolución completa para estados de final de juego.
 * Determina si una posición está matemáticamente ganada, perdida o empatada.
 * @param tablero Estado a evaluar.
 * @param Mov [Salida] La jugada óptima encontrada.
 * @return Resultado del análisis (VICTORIA, DERROTA o EMPATE).
 */
AgenteEstudiante::Resultado AgenteEstudiante::Status(const Tablero &tablero, std::pair<int,int> &Mov) {
   /* ============== Este trozo de código se tiene que quedar aquí  =============== */
   nodosVisitados++;

   /* ============== Empieza a partir de aquí tu implementación  =============== */
   Tablero t = tablero; 
   
   int ganador = t.comprobarGanador(); 
   
   if (ganador == id) return Resultado::VICTORIA;
   
   if (ganador != 0 && ganador != -1) return Resultado::DERROTA; 
   
   auto sucesores = t.getSucesores();
   if (ganador == -1 || !t.tieneMovimientosValidos() || sucesores.empty()) return Resultado::EMPATE;

   int jugadorActual = t.getJugadorTurno();
   bool soyMax = (jugadorActual == id);

   //RECURSIVIDAD
   if (soyMax) {
      Resultado mejorResultado = Resultado::DERROTA;

      for (const auto& suc : sucesores) {
         std::pair<int, int> dummyMov;
         Resultado resHijo = Status(suc, dummyMov);

         if (resHijo == Resultado::VICTORIA) {
            Mov = SacarMovimiento(tablero, suc);
            return Resultado::VICTORIA; 
         }

         if (resHijo == Resultado::EMPATE && mejorResultado == Resultado::DERROTA) {
            mejorResultado = Resultado::EMPATE; 
            Mov = SacarMovimiento(tablero, suc);
         }
      }
      
      if (mejorResultado == Resultado::DERROTA && !sucesores.empty())
         Mov = SacarMovimiento(tablero, sucesores[0]);

      return mejorResultado;

   } else {
      Resultado peorResultado = Resultado::VICTORIA;

      for (const auto& suc : sucesores) {
         std::pair<int, int> dummyMov;
         Resultado resHijo = Status(suc, dummyMov);

         if (resHijo == Resultado::DERROTA) {
            Mov = SacarMovimiento(tablero, suc);
            return Resultado::DERROTA; 
         }
         
         if (resHijo == Resultado::EMPATE && peorResultado == Resultado::VICTORIA) {
            peorResultado = Resultado::EMPATE; 
            Mov = SacarMovimiento(tablero, suc);
         }
      }
      
      if (peorResultado == Resultado::VICTORIA && !sucesores.empty())
         Mov = SacarMovimiento(tablero, sucesores[0]);

      return peorResultado;
   }
}

/**
 * @brief Implementación del algoritmo Minimax clásico.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado.
 */
double AgenteEstudiante::minimax(const Tablero &tablero, int profundidad, int prof_Max, std::pair<int,int> &Mov) {
   /* ============== Este trozo de código se tiene que quedar aquí  =============== */
   nodosVisitados++;

   if (tiempoMaxSegundos > 0 && std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
      abortarBanda = true;
   }
   if (abortarBanda) {
      return heuristica(tablero); 
   }
   /* ============== Empieza a partir de aquí tu implementación  =============== */

   Mov = {-1, -1}; 
   Tablero t = tablero; 
   int ganador = t.comprobarGanador(); 
   
   if (ganador == id) return GANAR - profundidad; 
   if (ganador != 0 && ganador != -1) return PERDER + profundidad; 
   
   if (ganador == -1) {
       int ganadorTie = t.getGanadorDesempate();
       if (ganadorTie == id) return (GANAR / 2.0) - profundidad; 
       if (ganadorTie != 0 && ganadorTie != -1) return (PERDER / 2.0) + profundidad; 
       return 0.0; 
   }

   if (profundidad == prof_Max) return heuristica(t);

   auto sucesores = t.getSucesoresConMovimientos();

   if (sucesores.empty()) {
       if (!t.tieneMovimientosValidos()) {
           int ganadorTie = t.getGanadorDesempate();
           if (ganadorTie == id) return (GANAR / 2.0) - profundidad;
           if (ganadorTie != 0 && ganadorTie != -1) return (PERDER / 2.0) + profundidad;
           return 0.0;
       }
       
       Tablero t_pass = t;
       t_pass.pasarTurno();
       std::pair<int, int> dummyMov;
       return minimax(t_pass, profundidad + 1, prof_Max, dummyMov);
   }

   int jugadorActual = t.getJugadorTurno();
   bool soyMax = (jugadorActual == id);
   
   // SALVAVIDAS INTEGRADO
   Mov = sucesores[0].second;

   if (soyMax) {
      double mejorValor = MenosInfinito;
      for (auto it = sucesores.begin(); it != sucesores.end() && !abortarBanda; ++it) {
         std::pair<int, int> dummyMov;
         double valorHijo = minimax(it->first, profundidad + 1, prof_Max, dummyMov);
         if (valorHijo > mejorValor) {
            mejorValor = valorHijo;
            Mov = it->second; 
         }
      }
      if (mejorValor == MenosInfinito && abortarBanda) return heuristica(t);
      return mejorValor;

   } else {
      double peorValor = MasInfinito;
      for (auto it = sucesores.begin(); it != sucesores.end() && !abortarBanda; ++it) {
         std::pair<int, int> dummyMov;
         double valorHijo = minimax(it->first, profundidad + 1, prof_Max, dummyMov);
         if (valorHijo < peorValor) {
            peorValor = valorHijo;
            Mov = it->second;
         }
      }
      if (peorValor == MasInfinito && abortarBanda) return heuristica(t);
      return peorValor;
   }
}


/**
 * @brief Punto de entrada para el juego inteligente.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida por el algoritmo de búsqueda.
 */
std::pair<int, int> AgenteEstudiante::JuegaInteligente(const Tablero& tablero) {
    std::pair<int,int> Mov;

    double valor = alfaBeta(tablero, 0, profundidadMax, MenosInfinito, MasInfinito, Mov);
    std::cout << "Valor Minimax: " << valor << "\tJugada: (" << Mov.first << ", " << Mov.second << ")\n";
    return Mov;
}




/**
 * @brief Implementación del algoritmo Minimax con Poda Alfa-Beta.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param alfa Valor mínimo garantizado para el jugador MAX.
 * @param beta Valor máximo garantizado para el jugador MIN.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado tras la poda.
 */
double AgenteEstudiante::alfaBeta(const Tablero &tablero, int profundidad, int prof_Max, double alfa, double beta, std::pair<int,int> &Mov) {
   /* ============== Este trozo de código se tiene que quedar aquí  =============== */
   nodosVisitados++;

   // FIX CRÍTICO: Si nos quedamos sin tiempo, NO devolvemos 0.
   // Devolvemos la heurística para salvar la valoración de la rama.
   if (tiempoMaxSegundos > 0 && std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
      abortarBanda = true;
   }
   if (abortarBanda) {
      return heuristica(tablero); 
   }
   /* ============== Empieza a partir de aquí tu implementación  =============== */

   Mov = {-1, -1}; 
   Tablero t = tablero; 
   int ganador = t.comprobarGanador(); 
   
   if (ganador == id) return GANAR - profundidad; 
   if (ganador != 0 && ganador != -1) return PERDER + profundidad; 
   
   // Gestión del Tie-Break
   if (ganador == -1) {
      int ganadorTie = t.getGanadorDesempate();
      if (ganadorTie == id) return (GANAR / 2.0) - profundidad; 
      if (ganadorTie != 0 && ganadorTie != -1) return (PERDER / 2.0) + profundidad; 
      return 0.0; 
   }

   if (profundidad == prof_Max) return heuristica(t);

   auto sucesores = t.getSucesoresConMovimientos();

   if (sucesores.empty()) {
      if (!t.tieneMovimientosValidos()) {
         int ganadorTie = t.getGanadorDesempate();
         if (ganadorTie == id) return (GANAR / 2.0) - profundidad;
         if (ganadorTie != 0 && ganadorTie != -1) return (PERDER / 2.0) + profundidad;
         return 0.0;
      }
      Tablero t_pass = t;
      t_pass.pasarTurno();
      std::pair<int, int> dummyMov;
      return alfaBeta(t_pass, profundidad + 1, prof_Max, alfa, beta, dummyMov);
   }

   int jugadorActual = t.getJugadorTurno();
   bool soyMax = (jugadorActual == id);
   
   // SALVAVIDAS INTEGRADO: Asignamos el primer movimiento por si abortamos en el bucle
   Mov = sucesores[0].second;

   if (soyMax) {
      std::sort(sucesores.begin(), sucesores.end(), [this](const auto& a, const auto& b) {
         return this->heuristica(a.first) > this->heuristica(b.first);
      });

      double mejorValor = MenosInfinito;
      bool podar = false;

      for (auto it = sucesores.begin(); it != sucesores.end() && !podar && !abortarBanda; ++it) {
         std::pair<int, int> dummyMov;
         double valorHijo = alfaBeta(it->first, profundidad + 1, prof_Max, alfa, beta, dummyMov);
         if (valorHijo > mejorValor) {
            mejorValor = valorHijo;
            Mov = it->second;
         }
         if (mejorValor > alfa) alfa = mejorValor;
         if (alfa >= beta) podar = true; 
      }
      // Evitar devolver MenosInfinito si se abortó de golpe
      if (mejorValor == MenosInfinito && abortarBanda) return heuristica(t);
      return mejorValor;

   } else {
      std::sort(sucesores.begin(), sucesores.end(), [this](const auto& a, const auto& b) {
          return this->heuristica(a.first) < this->heuristica(b.first);
      });

      double peorValor = MasInfinito;
      bool podar = false;

      for (auto it = sucesores.begin(); it != sucesores.end() && !podar && !abortarBanda; ++it) {
         std::pair<int, int> dummyMov;
         double valorHijo = alfaBeta(it->first, profundidad + 1, prof_Max, alfa, beta, dummyMov);
         if (valorHijo < peorValor) {
            peorValor = valorHijo;
            Mov = it->second;
         }
         if (peorValor < beta) beta = peorValor;
         if (alfa >= beta) podar = true; 
      }

      if (peorValor == MasInfinito && abortarBanda) return heuristica(t);

      return peorValor;
   }
}

/**
 * @brief Función heurística para evaluar la calidad de un tablero.
 * @param tablero Estado a evaluar.
 * @return Puntuación numérica (positiva para ventaja de J1, negativa para J2).
 */
double AgenteEstudiante::heuristica(const Tablero& tablero) {
    switch(numHeuristica) {
        case 0: return heuristicaPrueba(tablero);
                break;
        case 1: return heuristica1(tablero);
                break;
        case 2: return heuristica2(tablero);
                break;
        default: return heuristica1(tablero);
    }
}

double AgenteEstudiante::heuristicaPrueba(const Tablero& tablero) {
    // n es el número de fichas en línea para ganar.
    int n = tablero.getNParaGanar();
    int oponente = (id == 1) ? 2 : 1;
    double score_positivo = 0;

    double score_negativo = 0;

    for (int f=0; f< tablero.getFilas(); f++ ){
        for (int c = 0; c< tablero.getColumnas(); c++){
            if (tablero.getCelda(f,c) != 0 ){
                int valor = tablero.getFilas()-abs(f-(tablero.getFilas()/2)) + tablero.getColumnas()-abs(c-(tablero.getColumnas()/2)); 
                if (tablero.getCelda(f,c) == id){
                  score_positivo += valor;
                 }
                else {
                  score_negativo += valor;
                }
            }
        }
    }

   
    return score_positivo - score_negativo;
}


double AgenteEstudiante::heuristica1(const Tablero& tablero) {
    int oponente = (id == 1) ? 2 : 1;
    double score = 0.0;
    int filas = tablero.getFilas();
    int cols = tablero.getColumnas();

    // 1. Pesos Tácticos Calibrados
    // Se castiga drásticamente una línea cerrada y se premia la amenaza de líneas abiertas.
    const double PESO_4_ABIERTO = 500000.0; // Amenaza letal inmediata
    const double PESO_4_CERRADO = 15000.0;  // Útil, pero bloqueado
    const double PESO_3_ABIERTO = 80000.0;  // Muy peligroso, se convertirá en 4 abierto
    const double PESO_3_CERRADO = 2000.0;
    const double PESO_2 = 150.0;
    const double PESO_CENTRO = 30.0;        // Fomenta jugar en el medio del tablero

    // Vectores de dirección: Horizontal, Vertical, Diagonal Principal, Diagonal Secundaria
    int df[] = {0, 1, 1, 1};
    int dc[] = {1, 0, 1, -1};

    // 2. Escáner de Tablero (Topología y Líneas)
    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < cols; c++) {
            int jugador_celda = tablero.getCelda(f, c);
            
            // Si la casilla está vacía, la ignoramos
            if (jugador_celda == 0) continue;

            // --- A. Control del Centro ---
            // Las fichas centrales tienen más vectores de conexión
            int distCentroF = abs(f - filas / 2);
            int distCentroC = abs(c - cols / 2);
            double valorCentro = (filas - distCentroF + cols - distCentroC) * PESO_CENTRO;
            
            if (jugador_celda == id) score += valorCentro;
            else score -= valorCentro;

            // --- B. Escáner Direccional de Líneas ---
            for (int d = 0; d < 4; d++) {
                int prev_f = f - df[d];
                int prev_c = c - dc[d];
                
                // Optimizador: Solo evaluamos la línea si estamos en su origen
                // Evita sumar la misma línea múltiples veces.
                if (prev_f >= 0 && prev_f < filas && prev_c >= 0 && prev_c < cols) {
                    if (tablero.getCelda(prev_f, prev_c) == jugador_celda) continue;
                }

                int longitud = 1;
                int cur_f = f + df[d];
                int cur_c = c + dc[d];
                
                // Avanzamos en la dirección mientras las fichas sean del mismo color
                while (cur_f >= 0 && cur_f < filas && cur_c >= 0 && cur_c < cols && 
                       tablero.getCelda(cur_f, cur_c) == jugador_celda) {
                    longitud++;
                    cur_f += df[d];
                    cur_c += dc[d];
                }

                // Analizamos los extremos de la línea encontrada
                int extremos_abiertos = 0;
                
                // Extremo 1 (Origen - 1)
                if (prev_f >= 0 && prev_f < filas && prev_c >= 0 && prev_c < cols && 
                    tablero.getCelda(prev_f, prev_c) == 0) {
                    extremos_abiertos++;
                }
                
                // Extremo 2 (Fin + 1)
                if (cur_f >= 0 && cur_f < filas && cur_c >= 0 && cur_c < cols && 
                    tablero.getCelda(cur_f, cur_c) == 0) {
                    extremos_abiertos++;
                }

                // --- C. Valoración de la Línea ---
                double valor_linea = 0.0;
                
                if (longitud >= tablero.getNParaGanar()) {
                    valor_linea = 10000000.0; // Condición de victoria
                } else if (longitud == 4) {
                    if (extremos_abiertos == 2) valor_linea = PESO_4_ABIERTO;
                    else if (extremos_abiertos == 1) valor_linea = PESO_4_CERRADO;
                } else if (longitud == 3) {
                    if (extremos_abiertos == 2) valor_linea = PESO_3_ABIERTO;
                    else if (extremos_abiertos == 1) valor_linea = PESO_3_CERRADO;
                } else if (longitud == 2 && extremos_abiertos > 0) {
                    valor_linea = PESO_2 * extremos_abiertos;
                }

                // Asignamos la puntuación al jugador correspondiente
                if (jugador_celda == id) score += valor_linea;
                else score -= valor_linea;
            }
        }
    }

    // 3. Modificador de Iniciativa (Turno)
    // El jugador que tiene el turno posee una ventaja enorme para cerrar tácticas (ej: un 3 abierto).
    int turnoActual = tablero.getJugadorTurno();
    if (turnoActual == id) {
        score *= 1.25; 
    } else {
        score *= 0.75; 
    }

    return score;
}

double AgenteEstudiante::heuristica2(const Tablero& tablero) {
   //A implementar por el estudiante
   return 0;
}

