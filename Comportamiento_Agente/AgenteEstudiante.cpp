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

   //RECURSIVIDAD
   if (jugadorActual == id) {
      Resultado mejorResultado = Resultado::DERROTA;

      for (const auto& suc : sucesores) {
         std::pair<int, int> auxMov;
         Resultado resHijo = Status(suc, auxMov);

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
         std::pair<int, int> auxMov;
         Resultado resHijo = Status(suc, auxMov);

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
      std::pair<int, int> auxMov;

      return minimax(t_pass, profundidad + 1, prof_Max, auxMov);
   }

   int jugadorActual = t.getJugadorTurno();
   Mov = sucesores[0].second;

   if (jugadorActual == id) {
      double mejorValor = MenosInfinito;
      for (auto it = sucesores.begin(); it != sucesores.end() && !abortarBanda; ++it) {
         std::pair<int, int> auxMov;
         double valorHijo = minimax(it->first, profundidad + 1, prof_Max, auxMov);

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
         std::pair<int, int> auxMov;
         double valorHijo = minimax(it->first, profundidad + 1, prof_Max, auxMov);

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
      std::pair<int, int> auxMov;

      return alfaBeta(t_pass, profundidad + 1, prof_Max, alfa, beta, auxMov);
   }

   int jugadorActual = t.getJugadorTurno();
   
   Mov = sucesores[0].second;

   if (jugadorActual == id) {
      double mejorValor = MenosInfinito;
      bool salida = false;

      for (auto it = sucesores.begin(); it != sucesores.end() && !abortarBanda && !salida; ++it) {
         std::pair<int, int> auxMov;
         double valorHijo = alfaBeta(it->first, profundidad + 1, prof_Max, alfa, beta, auxMov);

         if (valorHijo > mejorValor) {
            mejorValor = valorHijo;
            Mov = it->second;
         }
         if (mejorValor > alfa) alfa = mejorValor;
         if (alfa >= beta) salida = true; 
      }

      if (mejorValor == MenosInfinito && abortarBanda) return heuristica(t);

      return mejorValor;

   } else {
      double peorValor = MasInfinito;
      bool salida = false;

      for (auto it = sucesores.begin(); it != sucesores.end() && !abortarBanda && !salida; ++it) {
         std::pair<int, int> auxMov;
         double valorHijo = alfaBeta(it->first, profundidad + 1, prof_Max, alfa, beta, auxMov);

         if (valorHijo < peorValor) {
            peorValor = valorHijo;
            Mov = it->second;
         }
         if (peorValor < beta) beta = peorValor;
         if (alfa >= beta) salida = true ;
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

   int filas = tablero.getFilas(), 
       cols = tablero.getColumnas(), 
       n = tablero.getNParaGanar(),
       mi_id = id, 
       op_id = (id == 1) ? 2 : 1,
       df[] = {0, 1, 1, -1}, 
       dc[] = {1, 0, 1, 1},
       ddf[] = {-1, -1, -1, 0, 0, 1, 1, 1}, 
       ddc[] = {-1, 0, 1, -1, 1, -1, 0, 1};
   
   const double p_propios[] = {0, 10.0, 300.0, 8000.0, 100000.0},
                p_rivales[] = {0, 10.0, 360.0, 12000.0, 200000.0};

   for (int f = 0; f < filas; f++) {
      for (int c = 0; c < cols; c++) {
         int celda = tablero.getCelda(f, c);
         Tablero::TipoCelda tipo = tablero.getTipoCelda(f, c);

         if (celda != 0) {
            double val_centro = (filas - abs(f - filas/2) + cols - abs(c - cols/2)) * 3.0;
            score += (celda == mi_id) ? (500.0 + val_centro) : -(500.0 + val_centro);
         } 
         else if (tipo == Tablero::TipoCelda::AMARILLO) { 
            for (int i = 0; i < std::max(filas, cols); i++) {
               if (i < cols) 
                  score += tablero.getCelda(f, i) == op_id ? 800 : (tablero.getCelda(f, i) == mi_id ? -2000 : 0);
               if (i < filas) 
                  score += tablero.getCelda(i, c) == op_id ? 800 : (tablero.getCelda(i, c) == mi_id ? -2000 : 0);
            }
         } else if (tipo == Tablero::TipoCelda::ROJO) { 
            bool salida = false;
            for (int k = 0; k < 8 && !salida; k++) {
               int vf = f + ddf[k], 
                   vc = c + ddc[k];
               if (vf >= 0 && vf < filas && vc >= 0 && vc < cols && tablero.getCelda(vf, vc) == mi_id){
                  score -= 1500.0;
                  salida = true;
               }
            }
         }

         for (int d = 0; d < 4; d++) {
            if (f + df[d]*(n-1) >= 0 && f + df[d]*(n-1) < filas && c + dc[d]*(n-1) >= 0 && c + dc[d]*(n-1) < cols) {
               int mis_v = 0, 
                   op_v = 0;
               bool t_ama = false, 
                    t_roja = false;

               for (int i = 0; i < n; i++) {
                  int celda_linea = tablero.getCelda(f + df[d]*i, c + dc[d]*i);
                  Tablero::TipoCelda tipo_linea = tablero.getTipoCelda(f + df[d]*i, c + dc[d]*i);
                  
                  if (celda_linea == mi_id) mis_v++; 
                  else if (celda_linea == op_id) op_v++;

                  if (tipo_linea == Tablero::TipoCelda::AMARILLO) t_ama = true;
                  if (tipo_linea == Tablero::TipoCelda::ROJO) t_roja = true;
               }

               if (mis_v > 0 && op_v == 0)
                  score += p_propios[mis_v] * (t_ama ? 0.05 : 1.0) * (t_roja ? 0.1 : 1.0);
               else if (op_v > 0 && mis_v == 0)
                  score -= p_rivales[op_v];
            }
         }
      }
   }
   return score;
}

double AgenteEstudiante::heuristica2(const Tablero& tablero) {
   int mi_id = id,
       rival_id = (mi_id == 1) ? 2 : 1;

   Tablero tab_copia = tablero;
   int ganador = tab_copia.comprobarGanador();

   if (ganador == mi_id)
      return 10000.0;
   else if (ganador == rival_id)
      return -10000.0;
   else if (ganador == -1) 
      return 0.0;
   
   int mis_4 = tablero.contarCombinaciones(4, mi_id),
       rival_4 = tablero.contarCombinaciones(4, rival_id);

   return (mis_4 * 10.0) - (rival_4 * 50.0);
}
