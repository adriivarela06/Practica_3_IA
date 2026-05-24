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
      return alfaBeta(t_pass, profundidad + 1, prof_Max, alfa, beta, dummyMov);
   }

   int jugadorActual = t.getJugadorTurno();
   bool soyMax = (jugadorActual == id);
   
   Mov = sucesores[0].second;

   if (soyMax) {
      double mejorValor = MenosInfinito;

      for (auto it = sucesores.begin(); it != sucesores.end() && !abortarBanda; ++it) {
         std::pair<int, int> dummyMov;
         double valorHijo = alfaBeta(it->first, profundidad + 1, prof_Max, alfa, beta, dummyMov);
         if (valorHijo > mejorValor) {
            mejorValor = valorHijo;
            Mov = it->second;
         }
         if (mejorValor > alfa) alfa = mejorValor;
         if (alfa >= beta) break; 
      }
      if (mejorValor == MenosInfinito && abortarBanda) return heuristica(t);
      return mejorValor;

   } else {
      double peorValor = MasInfinito;

      for (auto it = sucesores.begin(); it != sucesores.end() && !abortarBanda; ++it) {
         std::pair<int, int> dummyMov;
         double valorHijo = alfaBeta(it->first, profundidad + 1, prof_Max, alfa, beta, dummyMov);
         if (valorHijo < peorValor) {
            peorValor = valorHijo;
            Mov = it->second;
         }
         if (peorValor < beta) beta = peorValor;
         if (alfa >= beta) break; 
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
    double score = 0.0;
    int filas   = tablero.getFilas();
    int cols    = tablero.getColumnas();
    int n_ganar = tablero.getNParaGanar();
    
    int mi_id = id;
    int op_id = (id == 1) ? 2 : 1;

    // Pesos para ventanas deslizantes
    const double PESO_4 = 100000.0; 
    const double PESO_3 = 8000.0;
    const double PESO_2 = 300.0;
    const double PESO_1 = 10.0;

    int df[] = {0, 1, 1, -1};
    int dc[] = {1, 0, 1, 1};

    // ── 1. Contar fichas totales de cada jugador ──────────────────────────
    int mis_fichas_total = 0;
    int op_fichas_total  = 0;
    for (int f = 0; f < filas; f++)
        for (int c = 0; c < cols; c++) {
            int cel = tablero.getCelda(f, c);
            if (cel == mi_id) mis_fichas_total++;
            else if (cel == op_id) op_fichas_total++;
        }

    // Si el rival tiene más fichas que yo, penalizamos fuerte
    // (puede indicar que activamos una bomba o casilla roja)
    score += (mis_fichas_total - op_fichas_total) * 500.0;

    // ── 2. Escáner de ventanas deslizantes ───────────────────────────────
    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < cols; c++) {

            // Control del centro
            int celda = tablero.getCelda(f, c);
            if (celda != 0) {
                double val_centro = (filas - abs(f - filas/2) + cols - abs(c - cols/2)) * 3.0;
                if (celda == mi_id) score += val_centro;
                else                score -= val_centro;
            }

            for (int d = 0; d < 4; d++) {
                int end_f = f + df[d] * (n_ganar - 1);
                int end_c = c + dc[d] * (n_ganar - 1);
                
                if (end_f < 0 || end_f >= filas || end_c < 0 || end_c >= cols) continue;

                int mis_v  = 0, op_v = 0;
                bool tiene_roja = false, tiene_amarilla = false;

                for (int i = 0; i < n_ganar; i++) {
                    int ff = f + df[d] * i;
                    int cc = c + dc[d] * i;
                    int jugador = tablero.getCelda(ff, cc);
                    Tablero::TipoCelda tipo = tablero.getTipoCelda(ff, cc);

                    if (jugador == mi_id)      mis_v++;
                    else if (jugador == op_id) op_v++;

                    if (tipo == Tablero::TipoCelda::ROJO)     tiene_roja     = true;
                    if (tipo == Tablero::TipoCelda::AMARILLO)  tiene_amarilla = true;
                }

                // Ventana limpia mía
                if (mis_v > 0 && op_v == 0) {
                    double bonus = 0;
                    if      (mis_v == 4) bonus = PESO_4;
                    else if (mis_v == 3) bonus = PESO_3;
                    else if (mis_v == 2) bonus = PESO_2;
                    else                 bonus = PESO_1;

                    // Penalizar ventanas que pasan por casillas peligrosas vacías
                    if (tiene_amarilla) bonus *= 0.05; // bomba destruye la fila/col
                    if (tiene_roja)     bonus *= 0.1;  // roja convierte ficha en rival
                    score += bonus;
                }
                // Ventana limpia del rival
                else if (op_v > 0 && mis_v == 0) {
                    double malus = 0;
                    if      (op_v == 4) malus = PESO_4 * 2.0;  // urgente bloquear
                    else if (op_v == 3) malus = PESO_3 * 1.5;
                    else if (op_v == 2) malus = PESO_2 * 1.2;
                    else                malus = PESO_1;
                    score -= malus;
                }
            }
        }
    }

    // ── 3. Penalizar casillas vacías peligrosas accesibles ────────────────
    // Recorremos casillas vacías especiales y penalizamos su proximidad
    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < cols; c++) {
            if (tablero.getCelda(f, c) != 0) continue;

            Tablero::TipoCelda tipo = tablero.getTipoCelda(f, c);
            if (tipo == Tablero::TipoCelda::NORMAL || tipo == Tablero::TipoCelda::VERDE) continue;

            // Comprobamos si hay fichas mías en fila/col (para amarilla)
            // o fichas mías adyacentes (para roja)
            if (tipo == Tablero::TipoCelda::AMARILLO) {
                int mis_en_fila = 0, mis_en_col = 0;
                int op_en_fila  = 0, op_en_col  = 0;
                for (int cc = 0; cc < cols; cc++) {
                    if (tablero.getCelda(f, cc) == mi_id) mis_en_fila++;
                    if (tablero.getCelda(f, cc) == op_id) op_en_fila++;
                }
                for (int ff = 0; ff < filas; ff++) {
                    if (tablero.getCelda(ff, c) == mi_id) mis_en_col++;
                    if (tablero.getCelda(ff, c) == op_id) op_en_col++;
                }
                // Si activarla nos destruye más fichas de las que destruye al rival, penalizar
                int danio_propio = mis_en_fila + mis_en_col;
                int danio_rival  = op_en_fila  + op_en_col;
                score -= danio_propio * 2000.0;
                score += danio_rival  * 800.0;  // puede ser tácticamente buena si destruye al rival
            }

            if (tipo == Tablero::TipoCelda::ROJO) {
                // Si tenemos fichas adyacentes somos propensos a ir aquí — penalizar
                bool vecino_mio = false;
                int ddf[] = {-1,-1,-1,0,0,1,1,1};
                int ddc[] = {-1, 0, 1,-1,1,-1,0,1};
                for (int k = 0; k < 8; k++) {
                    int vf = f + ddf[k], vc = c + ddc[k];
                    if (vf >= 0 && vf < filas && vc >= 0 && vc < cols && tablero.getCelda(vf, vc) == mi_id)
                        vecino_mio = true;
                }
                if (vecino_mio) score -= 1500.0;
            }
        }
    }

    return score;
}

double AgenteEstudiante::heuristica2(const Tablero& tablero) {
   //A implementar por el estudiante
   return 0;
}

