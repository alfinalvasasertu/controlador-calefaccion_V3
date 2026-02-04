/*****************************************************************************************
 * CONTROLADOR CALDERA ESP32 - VERSION 559
 * BUILD: 2026-02-04 14:30 CET - REESCRITURA COMPLETA SEGÚN TABLA VERDAD
 * 
 * CAMBIOS V559 (REESCRITURA TOTAL):
 * ════════════════════════════════════════════════════════════════════════════════════════
 * ANÁLISIS EXHAUSTIVO TABLA DE LA VERDAD - 14 ESCENARIOS:
 * 
 * REGLAS FUNDAMENTALES:
 * 1. POST-CIRCULACIÓN se inicia cuando GT se APAGA (flanco de apagado)
 * 2. Durante POST: Bombas funcionan en ALTERNANCIA (ignoran todo)
 * 3. Después de POST: Bombas QUEDAN PARADAS hasta nueva orden
 * 4. Bombas solo funcionan si: SIST=0V Y JEF=0V Y sin alarmas
 * 5. Si durante POST la condición se resuelve → ABORTAR POST
 * 
 * ESCENARIOS IMPLEMENTADOS:
 * 
 * Línea 3: Normal → Bombas ALTERNANCIA, GT funciona si condiciones
 * Línea 7: JEF 0V→3.3V → GT apaga, POST inicia, Bombas alternancia
 * Línea 8: JEF=3.3V estable → Bombas PARADAS, POST apagada, GT apagado
 * Línea 9: SIST 0V→3.3V → GT apaga, POST inicia, Bombas alternancia  
 * Línea 10: SIST=3.3V estable → Bombas PARADAS, POST apagada, GT apagado
 * Línea 11-13: EMERG activa → TODO PARADO
 * Línea 14: AL_GT=3.3V estable → Bombas PARADAS, GT bloqueado
 * Línea 15: AL_GT 0V→3.3V → POST inicia, Bombas alternancia
 * 
 * CORRECCIONES CRÍTICAS:
 * - Jefatura OFF NO apaga bombas directamente, solo apaga GT
 * - Sistema OFF NO apaga bombas directamente, solo apaga GT
 * - Alarma GT NO apaga bombas, solo bloquea GT
 * - Después de POST → Bombas permanecen PARADAS
 * 
 * CAMBIOS V557:
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 1. CORRECCIÓN PIN2 SEGÚN TABLA DE LA VERDAD - OPCIÓN A (CRÍTICO)
 *    - PIN2 ahora indica si el sistema PUEDE FUNCIONAR (no solo estado de jefatura)
 *    - Requiere PIN15=0V (jefatura ON) Y PIN4=0V (sistema ON) para PIN2=0V
 *    - Si cualquiera está OFF → PIN2=3.3V
 *    
 * 2. LÓGICA PIN2 ACTUALIZADA
 *    - PIN15=0V Y PIN4=0V → PIN2=0V (MARCHA JEFAT/AZUL) → Registro 40232=1
 *    - PIN15=3.3V O PIN4=3.3V → PIN2=3.3V (OFF/GRIS) → Registro 40232=0
 *    - Flanco PIN2 (0V→3.3V) sigue detectándose para apagar GT y post-circulación
 *    
 * 3. ESCENARIOS VERIFICADOS SEGÚN TABLA DE LA VERDAD
 *    - Escenario 1: PIN4: 0V→3.3V (sistema se apaga)
 *      • PIN2: 0V→3.3V (aunque PIN15=0V, sistema OFF bloquea)
 *      • GT se apaga (PIN14→3.3V)
 *      • Inicia post-circulación (PIN47→0V)
 *      • Bombas mantienen alternancia durante post-circ
 *      • Al finalizar: Bombas apagadas (PIN12=3.3V, PIN13=3.3V)
 *    
 *    - Escenario 2: PIN4=3.3V permanente (sistema OFF)
 *      • PIN2=3.3V (OFF)
 *      • Bombas apagadas
 *      • GT apagado
 *      • POST apagada
 * 
 * CAMBIOS V556:
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 1. CORRECCIÓN BOMBAS TRAS POST-CIRCULACIÓN POR ALARMA GT (CRÍTICO)
 *    - PROBLEMA: Al finalizar post-circulación por alarma GT, bombas se reactivaban
 *    - CAUSA: Lógica normal no verificaba si alarma GT seguía activa
 *    - SOLUCIÓN: Nueva verificación en PRIORIDAD 6 antes de activar bombas
 *    - Si alarmaGT=true (PIN11=3.3V) → Bombas permanecen apagadas
 *    - Las bombas solo funcionan si GT puede estar operativo (sin alarma)
 * 
 * 2. FLUJO CORRECTO POST-CIRCULACIÓN POR ALARMA GT
 *    - Alarma GT activa (PIN11: 0V→3.3V)
 *    - GT se apaga (PIN14→3.3V)
 *    - Inicia post-circulación (PIN47→0V)
 *    - Bomba funciona durante tiempo configurado
 *    - Finaliza post-circulación (PIN47: 0V→3.3V)
 *    - Bombas se apagan y permanecen apagadas (PIN12=3.3V, PIN13=3.3V)
 *    - NUEVO: Mientras alarma GT persista, bombas NO se reactivan
 * 
 * 3. DOCUMENTACIÓN MEJORADA
 *    - Comentarios actualizados para aclarar comportamiento con alarma GT
 *    - Explicación de por qué bombas no deben funcionar sin GT disponible
 * 
 * CAMBIOS V555:
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 1. CORRECCIÓN LÓGICA PIN2 (JEFATURA) - CRÍTICO
 *    - PIN2 ahora refleja DIRECTAMENTE PIN15 sin condiciones adicionales
 *    - PIN15=0V (jefatura ON) → PIN2=0V (MARCHA JEFAT/AZUL) → Registro 40232=1
 *    - PIN15=3.3V (jefatura OFF) → PIN2=3.3V (OFF/GRIS) → Registro 40232=0
 *    - Eliminada condición bloqueo_postcirc_hasta_demanda en actualización de PIN2
 * 
 * 2. DETECCIÓN FLANCO PIN2 PARA CONTROL GT (CRÍTICO)
 *    - Nueva variable pin2_fisico para leer estado de PIN2 en cada ciclo
 *    - Nueva variable pin2_anterior para detectar flancos de PIN2
 *    - Flanco PIN2 (0V→3.3V) detectado en ejecutarLogicaControl()
 *    - Flanco PIN2→OFF apaga GT (PIN14: 0V→3.3V) e inicia post-circulación
 * 
 * 3. ACTUALIZACIÓN FUNCIONES DE LECTURA
 *    - leerEstadosFisicos() ahora lee también PIN2 (indicador salida)
 *    - actualizarPreviosEstados() actualiza pin2_anterior
 *    - setup() inicializa pin2_anterior correctamente
 * 
 * 4. DOCUMENTACIÓN MEJORADA
 *    - Comentarios actualizados para aclarar relación PIN15→PIN2→Registro
 *    - Explicación detallada de detección de flanco PIN2 y su efecto en GT
 * 
 * CAMBIOS V554:
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 1. CORRECCIÓN COMPLETA VISUALIZACIÓN WEB (CRÍTICO)
 *    - PIN2: Nombre corregido "PARO POR JEFATURA" → "JEFATURA"
 *    - PIN2: Estados ahora coinciden con tabla CSV (0V=MARCHA JEFAT/AZUL, 3.3V=OFF/GRIS)
 *    - PIN40: Texto corregido "AVERIA_GENERAL" → "AVERIA GT", color ROJO → VERDE
 *    - PIN45: Color reposo corregido VERDE → GRIS (3.3V=EN RANGO/GRIS)
 * 
 * 2. CORRECCIÓN REGISTRO MODBUS PIN2 (40232)
 *    - Ahora refleja estado físico del PIN, no concepto lógico invertido
 *    - PIN2=0V (MARCHA JEFAT) → Registro=1
 *    - PIN2=3.3V (OFF) → Registro=0
 *    - Se lee directamente de digitalRead() en vez de calcular inverso lógico
 * 
 * 3. DOCUMENTACIÓN MEJORADA
 *    - Comentarios actualizados para reflejar tabla CSV correctamente
 *    - Aclarado que PIN2 es indicador de estado, no de paro
 *    - Eliminada lógica innecesaria de `prog_en_vigor` que no se usaba
 * 
 * CAMBIOS V553 (anteriores):
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 1. CORRECCIÓN CRÍTICA REGISTRO 40002
 *    - Renombrado cfg_postCirculacionSeg → cfg_postCirculacionMin
 *    - Confirmado: Registro 40002 está en MINUTOS (ya multiplicaba x60000 correctamente)
 *    - Corregidos todos los mensajes de log para reflejar "minutos"
 * 
 * 2. CORRECCIÓN POST-CIRCULACIÓN EN SISTEMA OFF (CRÍTICO)
 *    - PROBLEMA: PIN4>3.3V apagaba bombas directamente sin post-circulación
 *    - SOLUCIÓN: Verificar si GT estaba ON antes de apagar
 *    - Si GT activo → Iniciar post-circulación ANTES de apagar bombas
 *    - Flujo: PIN4>3.3V → PIN14>3.3V → Inicia post-circ → PIN47>0V
 * 
 * 3. CORRECCIÓN LÓGICA ALARMA GT Y POST-CIRCULACIÓN
 *    - Aclarado flujo: PIN11>3.3V → Fuerza PIN14>3.3V (apaga GT)
 *    - Flanco PIN14: 0V→3.3V (GT apagado) → Inicia post-circulación
 *    - La lógica ya estaba correctamente implementada (sin_alarma_gt)
 * 
 * 4. ACTUALIZACIÓN .github/copilot-instructions.md
 *    - Documentadas especificaciones de pines según tabla CSV
 *    - Añadida interpretación de estados (columnas G/H/J y N/L/M)
 *    - Documentada tabla de la verdad con 14 escenarios
 * 
 * CAMBIOS V546 (previos):
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 1. CAMBIO AUTOMÁTICO BOMBA EN AVERÍA (CRÍTICO)
 *    - Detecta avería RT en bomba activa en tiempo real
 *    - Cambia automáticamente a la otra bomba si está disponible
 *    - Si ambas no disponibles, para todo el sistema
 *    - Reinicia contador de alternancia al cambiar por avería
 * 
 * 2. ACTUALIZACIÓN VALORES CONFIGURACIÓN EN WEB UI
 *    - Columna "Valor Actual" ahora se actualiza cada 2 segundos
 *    - Sincronización correcta con cambios vía Modbus
 *    - Estado NTP también se actualiza en tiempo real
 * 
 * CARACTERÍSTICAS V541 (WiFi robusta):
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 1. AP WiFi SIEMPRE ACTIVO
 *    - Monitoreo continuo de estado AP (cada 3s)
 *    - Reactivación automática si falla
 *    - IP fija: 192.168.4.1 (SSID: Caldera_ESP32S3)
 * 
 * 2. STA WiFi CON RECONEXIÓN INTELIGENTE
 *    - Backoff exponencial (5s → 120s máx)
 *    - Reintentos sin bloqueo del sistema
 *    - Timeout de 20s por intento
 *    - Modo WIFI_AP_STA garantizado siempre
 * 
 * 3. ESCANEO WIFI ASÍNCRONO
 *    - Scan no bloqueante (async)
 *    - Protección contra escaneos simultáneos
 *    - Resultados mediante polling
 * 
 * 4. ENDPOINTS WIFI MEJORADOS
 *    - Validación de parámetros
 *    - Manejo de errores robusto
 *    - Respuestas HTTP correctas
 * 
 * CARACTERÍSTICAS BASE (desde V527):
 * ════════════════════════════════════════════════════════════════════════════════════════
 * 1. Sincronización NTP con servidor pool.ntp.org
 * 2. Configuración automática cada 24 horas (opcional)
 * 3. Ajuste automático horario verano/invierno DST (opcional)
 * 4. Sincronización manual desde Web UI
 * 5. Nuevos registros Modbus: 40105 (auto 24h), 40106 (auto DST), 40107 (trigger manual)
 * 6. Persistencia NVS de configuración NTP
 * 7. Sección dedicada en pestaña Configuración con estado visual
 * 8. Endpoints web: /syncntp, /saventpconfig
 * 
 * BASE V526: PERSISTENCIA MODBUS COMPLETA, LÓGICA INTACTA
 *****************************************************************************************/

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <WiFi.h>           // Gestión de la red WiFi
#include <ModbusIP_ESP8266.h> // La librería Modbus TCP (compatible con S3)
#include <time.h>  // Para NTP

/* =========================================================================================
   PINOUT DEFINITIVO - Tabla_PIN_Variables_Registros_BASICA.xlsx
   ========================================================================================= */

// ENTRADAS DIGITALES - Lógica inversa: 0V=ON/CERRADO, 3.3V=OFF/ABIERTO
#define PIN_SYS_ONOFF         4   // PIN_SISTEMA
#define PIN_SELECTOR_PROG     5   // PIN_PROG
#define PIN_SW_B1             6   // PIN_BOMBA1_INT
#define PIN_SW_B2             7   // PIN_BOMBA2_INT
#define PIN_JEFATURA          15  // PIN_JEFATURA
#define PIN_TERM_BC           16  // PIN_TERM_BC
#define PIN_SW_GT             17  // PIN_GT_SWITCH
#define PIN_SW_BC             18  // PIN_BC_SWITCH

// ENTRADAS DE SEGURIDAD - Contactos NC: 0V=OK, 3.3V=ALARMA
#define PIN_EMERGENCIA        8   // PIN_EMERGENCIA
#define PIN_RT1               9   // PIN_RT_B1
#define PIN_RT2               10  // PIN_RT_B2
#define PIN_AL_GT             11  // PIN_ALARMA_GT

// ENTRADA ANALÓGICA
#define PIN_NTC_IMP           1  // PIN_NTC (Sonda NTC Impulsión)

// SALIDAS RELES - 0V=ACTIVADO, 3.3V=DESACTIVADO
#define PIN_OUT_B1            12   // RELE_B1 (Contactor Bomba 1)
#define PIN_OUT_B2            13   // RELE_B2 (Contactor Bomba 2)
#define PIN_OUT_GT            14   // RELE_GT (Relé Grupo Térmico)
#define PIN_OUT_BC            21   // RELE_BC (Relé Bomba Condensación)
#define PIN_OUT_POST          47   // POSTCIRC_INDIC (Indicación Post-circulación)

// SALIDAS INDICADORES Y DE SEÑALIZACIÓN - 0V=OFF, 3.3V=ON
#define PIN_SOBRECALENTAMIENTO 38   // SOBRECALENTAMIENTO (temp > max)
#define PIN_AVERIA_B1         39   // AVERIA_B1 (Indicador Avería Bomba 1)
#define PIN_AVERIA_GT        40   // AVERIA_GT (Indicador Avería Grupo Térmico)
#define PIN_AVERIA_B2         41   // AVERIA_B2 (Indicador Avería Bomba 2)
#define PIN_PROG_ACTIVA       42   // PROG_ACTIVA (Indicador Programación Activa)
#define PIN_PARO_JEFATURA     2    // JEFATURA (Indicador estado Jefatura: 0V=MARCHA JEFAT, 3.3V=OFF)
#define PIN_TEMP_FUERA_RANGO  45   // TEMPERATURA FUERA DE RANGO
#define PIN_INDIC_EMERGENCIA  43   // EMERGENCIA (Indicador salida: 0V=EMERGENCIA, 3.3V=NO_EMERGENCIA)
#define PIN_INDIC_SISTEMA     48   // SISTEMA (Indicador salida: 0V=SISTEMA_ON, 3.3V=SISTEMA_OFF)

/* =========================================================================================
   HELPERS ACTUALIZADOS - DETECCIÓN DE FLANCOS FÍSICOS
   ========================================================================================= */

inline bool isON(uint8_t pin) {
  return (digitalRead(pin) == LOW);  // 0V = ON
} 

inline bool isALARMA(uint8_t pin) {
  return (digitalRead(pin) == HIGH); // 3.3V = ALARMA
}

inline void setOutput(uint8_t pin, bool state) {
  digitalWrite(pin, state ? HIGH : LOW);
}

// Función para leer voltaje aproximado de un PIN digital
float readPinVoltage(uint8_t pin) {
  return digitalRead(pin) == HIGH ? 3.3 : 0.0;
}

// Función para leer temperatura NTC 10kΩ en PIN36
float leerTemperaturaNTC() {
  int adcValue = analogRead(PIN_NTC_IMP);
  float voltaje = (adcValue / 4095.0) * 3.3;
  
  float R1 = 10000.0;
  float Vcc = 3.3;
  
  if (voltaje >= Vcc - 0.01) voltaje = Vcc - 0.01;
  
  float Rntc = R1 * voltaje / (Vcc - voltaje);
  
  // NTC 10kΩ @ 25°C, B=3950
  float R0 = 10000.0;
  float T0 = 298.15;
  float B = 3950.0;
  
  float tempK = 1.0 / (1.0/T0 + (1.0/B) * log(Rntc/R0));
  float tempC = tempK - 273.15;
  
  if (tempC < -20.0) tempC = -20.0;
  if (tempC > 150.0) tempC = 150.0;
  
  return tempC;
}

/* =========================================================================================
   VARIABLES NTP - DEBEN DECLARARSE ANTES DE sincronizarNTP()
   ========================================================================================= */
// NTP y reloj - configuración servidor
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
bool ntpSyncOK = false;

// Configuración NTP - PERSISTENTES
uint16_t cfg_ntpAuto24h = 0;                 // 40105 (0=no, 1=sí)
uint16_t cfg_ntpAutoDST = 0;                 // 40106 (0=no, 1=sí)
unsigned long ultimaSincronizacionNTP_ms = 0;
bool ntpSincronizando = false;

bool sincronizarNTP() {
  if (ntpSincronizando || WiFi.status() != WL_CONNECTED) {
    return false;
  }
  
  Serial.println("⏰ Iniciando sincronización NTP...");
  ntpSincronizando = true;
  
  // Configurar zona horaria según DST activado
  long offsetSec = gmtOffset_sec;
  int dstSec = cfg_ntpAutoDST ? daylightOffset_sec : 0;
  
  configTime(offsetSec, dstSec, ntpServer);
  
  // Intentar obtener hora con timeout corto
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 5000)) {  // timeout de 5 segundos
    ntpSyncOK = true;
    ultimaSincronizacionNTP_ms = millis();
    Serial.println("✓ Sincronización NTP exitosa");
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.printf("  Hora actual: %s\n", buffer);
    ntpSincronizando = false;
    return true;
  } else {
    Serial.println("✗ Fallo sincronización NTP (timeout)");
    ntpSincronizando = false;
    return false;
  }
}

/* =========================================================================================
   CONFIGURACIÓN PERSISTENTE (NVS) - COMPLETAMENTE REVISADA
   ========================================================================================= */
Preferences prefs;

// ===== GESTIÓN AP / STA DINÁMICA =====
bool ap_forced_off = false;
unsigned long sta_connected_since_ms = 0;
unsigned long sta_lost_since_ms = 0;
const unsigned long STA_STABLE_TIME_MS = 5000;   // 5 s con STA OK para apagar AP
const unsigned long STA_LOST_TIME_MS   = 10000;  // 10 s de caída STA para reactivar AP

// ===== STA ROBUSTA (DOS) =====
unsigned long wifi_last_attempt_ms = 0;
const unsigned long WIFI_RETRY_INTERVAL_MS = 15000; // 15 s
bool sta_connecting = false;


// ===== AP ROBUSTO (UNO) =====
bool ap_started = false;
IPAddress apIP(192, 168, 4, 1);


// Parámetros configurables (Modbus Holding Registers) - AHORA PERSISTENTES COMPLETOS
uint16_t cfg_alternanciaHoras = 120;         // 40001 (MINUTOS) - Valor por defecto
uint16_t cfg_postCirculacionMin = 10;        // 40002 (MINUTOS) - Valor por defecto
int16_t  cfg_tempMinGT_x10 = 550;            // 40003 (55.0°C) - Valor por defecto
int16_t  cfg_tempMaxGT_x10 = 700;            // 40004 (70.0°C) - Valor por defecto
uint16_t cfg_sensorMode = 1;                 // 40005 (0=fijo, 1=NTC) - Valor por defecto
int16_t  cfg_tempFijaGT_x10 = 600;           // 40006 (60.0°C) - Valor por defecto
uint16_t cfg_schedEnable = 0;                // 40007 - Valor por defecto

// Horarios programación - AHORA PERSISTENTES
uint16_t cfg_schedMananaON = 480;            // 40100 (08:00) - Valor por defecto
uint16_t cfg_schedMananaOFF = 840;           // 40101 (14:00) - Valor por defecto
uint16_t cfg_schedTardeON = 960;             // 40102 (16:00) - Valor por defecto
uint16_t cfg_schedTardeOFF = 1320;           // 40103 (22:00) - Valor por defecto
uint16_t cfg_schedDiasMask = 62;             // 40104 (Lun-Vie) - Valor por defecto

/* =========================================================================================
   VARIABLES DE ESTADO - ACTUALIZADAS PARA DETECCIÓN DE FLANCOS
   ========================================================================================= */

// Estados salidas principales
bool bomba1_ON = false;
bool bomba2_ON = false;
bool grupoTermico_ON = false;
bool bombaCondensacion_ON = false;
bool postCirculacion_ON = false;

// Estado anterior de GT (para detectar flanco LÓGICO, no físico)
bool grupoTermico_ON_anterior = false;

// Estados físicos actuales (leídos de pines)
bool pin32_fisico = false;  // Sistema ON/OFF
bool pin27_fisico = false;  // Jefatura (PIN15 entrada)
bool pin2_fisico = false;   // Jefatura indicador (PIN2 salida)
bool pin21_fisico = false;  // GT (salida)
bool pin16_fisico = false;  // RT1
bool pin17_fisico = false;  // RT2

// Estados anteriores (para detección de flancos)
bool pin32_anterior = false;
bool pin27_anterior = false;
bool pin2_anterior = false;   // Para detectar flanco PIN2 (0V→3.3V)
bool pin21_anterior = false;
bool pin16_anterior = false;
bool pin17_anterior = false;

// Estados alarmas
bool alarmaRT1 = false;
bool alarmaRT2 = false;
bool alarmaEmergencia = false;
bool alarmaGT = false;
bool alarmaGT_anterior = false;  // Para detectar flanco alarma GT

// Control alternancia
bool alternancia_suspendida = false;
bool turno_bomba1 = true;
unsigned long alternancia_inicio_ms = 0;


bool postcirc_motivo_jefatura = false;


// Contadores tiempo funcionamiento bombas
unsigned long tiempoB1_ms = 0;
unsigned long tiempoB2_ms = 0;
unsigned long ultimoUpdateContadores = 0;

// Contadores TOTALES de vida de las bombas
unsigned long tiempoB1_total_ms = 0;
unsigned long tiempoB2_total_ms = 0;




// --- TIEMPO DE ALTERNANCIA PARA UI ---

// ===== Pausa de alternancia (para congelar el cronómetro durante post-circ/bloqueos) =====
unsigned long alt_pause_start_ms     = 0;  // 0 = no pausado
unsigned long alt_pause_acumulado_ms = 0;  // suma de pausas cerradas

// ===== Bloqueo tras post-circulación por Jefatura OFF =====
bool bloqueo_postcirc_hasta_demanda = false;

// ===== Motivo de la post-circulación (para decidir si aplicar bloqueo) =====
enum MotivoPostCirc { PC_NONE=0, PC_GT_OFF=1, PC_SISTEMA_OFF=2, PC_JEFATURA_OFF=3, PC_DOBLE_AVERIA=4 };
MotivoPostCirc motivo_postcirc = PC_NONE;


// hacia delante (sube 00:00 -> periodo)
unsigned long alternancia_transcurrida_seg = 0;
// hacia atrás (baja periodo -> 00:00, por si lo quieres mostrar)
unsigned long alternancia_restante_seg = 0;


// Control post-circulación
bool post_circulacion_activa = false;
unsigned long post_circulacion_inicio_ms = 0;
uint8_t bomba_post_circulacion = 0;  // 0=ninguna, 1=B1, 2=B2, 3=BC
unsigned long tiempoRestantePostCirc_seg = 0;
bool bloqueo_reinicio_postcirc_alarmas = false;  // Bloquea reinicio automático tras finalizar post-circ por alarmas

// Temperatura actual
float temperaturaActual = 25.0;

/* =========================================================================================
   FUNCIONES AUXILIARES NUEVAS - SOLO PARA LÓGICA
   ========================================================================================= */

// Leer estados físicos de pines importantes
void leerEstadosFisicos() {
  pin32_fisico = isON(PIN_SYS_ONOFF);      // 0V = ON
  pin27_fisico = isON(PIN_JEFATURA);       // 0V = ON (PIN15 entrada)
  pin2_fisico  = isON(PIN_PARO_JEFATURA);  // 0V = MARCHA JEFAT (PIN2 salida indicador)
  pin21_fisico = (digitalRead(PIN_OUT_GT) == LOW);  // 0V = ON (salida física)
  pin16_fisico = isALARMA(PIN_RT1);        // 3.3V = AVERÍA
  pin17_fisico = isALARMA(PIN_RT2);        // 3.3V = AVERÍA
}

// Detectar flanco ON→OFF (0V→3.3V)
bool detectarFlancoOFF(bool estado_anterior, bool estado_actual) {
  return (estado_anterior && !estado_actual);
}

// Verificar disponibilidad de bombas (interruptor ON y sin avería)
bool bomba1_disponible() {
  return isON(PIN_SW_B1) && !isALARMA(PIN_RT1);
}

bool bomba2_disponible() {
  return isON(PIN_SW_B2) && !isALARMA(PIN_RT2);
}

// Determinar qué bomba debe funcionar según alternancia
uint8_t determinarBombaActiva() {
  // Si alternancia suspendida (por avería o manual)
  if (alternancia_suspendida) {
    if (bomba1_disponible() && !bomba2_disponible()) return 1;
    if (!bomba1_disponible() && bomba2_disponible()) return 2;
    return 0; // Ninguna disponible
  }
  
  // Alternancia normal por tiempo
  unsigned long ahora = millis();
  unsigned long tiempo_alternancia_ms = cfg_alternanciaHoras * 60000UL;
  
  if (turno_bomba1) {
    if (bomba1_disponible()) {
      if (ahora - alternancia_inicio_ms >= tiempo_alternancia_ms) {
        // Cambio de turno
        turno_bomba1 = false;
        alternancia_inicio_ms = ahora;
        return 2;
      }
      return 1;
    } else if (bomba2_disponible()) {
      alternancia_suspendida = true;
      return 2;
    }
  } else {
    if (bomba2_disponible()) {
      if (ahora - alternancia_inicio_ms >= tiempo_alternancia_ms) {
        turno_bomba1 = true;
        alternancia_inicio_ms = ahora;
        return 1;
      }
      return 2;
    } else if (bomba1_disponible()) {
      alternancia_suspendida = true;
      return 1;
    }
  }
  
  return 0; // Ninguna disponible
}



// Iniciar post-circulación

void iniciarPostCirculacion() {
  post_circulacion_activa = true;
  post_circulacion_inicio_ms = millis();
  postCirculacion_ON = true;

   // === INICIO DE PAUSA DE ALTERNANCIA (si no estaba ya) ===
  if (alt_pause_start_ms == 0) alt_pause_start_ms = millis();

  // PRESERVAR estado actual de bombas (la que estaba funcionando debe continuar)
  // NO usar determinarBombaActiva() porque queremos mantener la que YA estaba ON
  if (bomba1_ON) {
    bomba_post_circulacion = 1;
    // Mantener bomba1_ON = true (ya estaba activada)
    bomba2_ON = false;
    bombaCondensacion_ON = false;
    Serial.println("POST-CIRC: Bomba 1 continúa (sin interrupción)");
  } else if (bomba2_ON) {
    bomba_post_circulacion = 2;
    bomba1_ON = false;
    // Mantener bomba2_ON = true (ya estaba activada)
    bombaCondensacion_ON = false;
    Serial.println("POST-CIRC: Bomba 2 continúa (sin interrupción)");
  } else {
    // Ninguna bomba activa, determinar cuál usar
    uint8_t bomba_activa = determinarBombaActiva();
    if (bomba_activa == 1) {
      bomba_post_circulacion = 1;
      bomba1_ON = bomba1_disponible();
      bomba2_ON = false;
      bombaCondensacion_ON = false;
      Serial.println("POST-CIRC: Bomba 1 arranca");
    } else if (bomba_activa == 2) {
      bomba_post_circulacion = 2;
      bomba1_ON = false;
      bomba2_ON = bomba2_disponible();
      bombaCondensacion_ON = false;
      Serial.println("POST-CIRC: Bomba 2 arranca");
    } else {
      bomba_post_circulacion = 3;
      bomba1_ON = false;
      bomba2_ON = false;
      bombaCondensacion_ON = true;
      Serial.println("POST-CIRC: BC emergencia");
    }
  }
  
  // GT se apaga inmediatamente al iniciar post-circulación
  grupoTermico_ON = false;
  
  // ACTUALIZAR SALIDAS FÍSICAS INMEDIATAMENTE para evitar cualquier delay
  setOutput(PIN_OUT_B1, !bomba1_ON);
  setOutput(PIN_OUT_B2, !bomba2_ON);
  setOutput(PIN_OUT_GT, !grupoTermico_ON);
  setOutput(PIN_OUT_BC, !bombaCondensacion_ON);
  setOutput(PIN_OUT_POST, !postCirculacion_ON);
}




// Cancelar post-circulación
void cancelarPostCirculacion() {
  post_circulacion_activa = false;
  bomba_post_circulacion = 0;
  postCirculacion_ON = false;
  Serial.println("POST-CIRC: Cancelada");
}


/* =========================================================================================
   MODBUS TCP - COMPLETAMENTE REVISADO PARA PERSISTENCIA TOTAL
   ========================================================================================= */
ModbusIP mb;

// Definición registros Holding (base 40001, offset 0)
#define MB_REG_ALT_HOURS           0    // 40001
#define MB_REG_PUMP_STOP_DELAY     1    // 40002
#define MB_REG_GT_TMIN             2    // 40003
#define MB_REG_GT_TMAX             3    // 40004
#define MB_REG_GT_SENSOR_MODE      4    // 40005
#define MB_REG_GT_TFIXED           5    // 40006
#define MB_REG_SCHED_ENABLE        6    // 40007
#define MB_REG_SCHED_M_ON          100  // 40100
#define MB_REG_SCHED_M_OFF         101  // 40101
#define MB_REG_SCHED_T_ON          102  // 40102
#define MB_REG_SCHED_T_OFF         103  // 40103
#define MB_REG_SCHED_DOW_MASK      104  // 40104
#define MB_REG_NTP_AUTO_24H        105  // 40105 - Sincronización automática cada 24h
#define MB_REG_NTP_AUTO_DST        106  // 40106 - Ajuste automático horario verano/invierno
#define MB_REG_NTP_SYNC_NOW        107  // 40107 - Trigger sincronización manual (write 1)

// Registros estados (solo lectura, offset 200+)
#define MB_REG_BOMBA1_STATE        200  // 40201
#define MB_REG_BOMBA2_STATE        201  // 40202
#define MB_REG_GT_STATE            202  // 40203
#define MB_REG_ALARM_RT1           203  // 40204
#define MB_REG_ALARM_RT2           204  // 40205
#define MB_REG_ALARM_EMERG         205  // 40206
#define MB_REG_ALARM_GT            206  // 40207
#define MB_REG_TEMP_X10            207  // 40208
// Registros adicionales de PINs (INTACTOS)
#define MB_REG_PIN_SYS_ONOFF       210  // 40211
#define MB_REG_PIN_PROG_SEL        211  // 40212
#define MB_REG_PIN_SW_B1           212  // 40213
#define MB_REG_PIN_SW_B2           213  // 40214
#define MB_REG_PIN_JEFATURA        214  // 40215
#define MB_REG_PIN_TERM_BC         215  // 40216
#define MB_REG_PIN_SW_GT           216  // 40217
#define MB_REG_PIN_SW_BC           217  // 40218
#define MB_REG_PIN_EMERGENCIA      218  // 40219
#define MB_REG_PIN_RT1             219  // 40220
#define MB_REG_PIN_RT2             220  // 40221
#define MB_REG_PIN_AL_GT           221  // 40222
#define MB_REG_PIN_OUT_BC          222  // 40223
#define MB_REG_PIN_OUT_POST        223  // 40224
#define MB_REG_PIN_LED_B1_RUN      224  // 40225
#define MB_REG_PIN_LED_B2_RUN      225  // 40226
#define MB_REG_PIN_LED_GT_RUN      226  // 40227
#define MB_REG_PIN_PARO_JEF        231  // 40232
#define MB_REG_PIN_TEMP_FUERA      232  // 40233
#define MB_REG_PIN_INDIC_EMERG     233  // 40234
#define MB_REG_PIN_INDIC_SIST      234  // 40235

// Registros reset contadores (40228-40231)
#define MB_REG_RESET_B1_PARCIAL    227  // 40228
#define MB_REG_RESET_B1_TOTAL      228  // 40229
#define MB_REG_RESET_B2_PARCIAL    229  // 40230
#define MB_REG_RESET_B2_TOTAL      230  // 40231

// Registros contadores de tiempo (solo lectura, en segundos)
#define MB_REG_B1_TIEMPO_PARCIAL_L 240  // 40241 - Bomba 1 parcial LOW word
#define MB_REG_B1_TIEMPO_PARCIAL_H 241  // 40242 - Bomba 1 parcial HIGH word
#define MB_REG_B2_TIEMPO_PARCIAL_L 242  // 40243 - Bomba 2 parcial LOW word
#define MB_REG_B2_TIEMPO_PARCIAL_H 243  // 40244 - Bomba 2 parcial HIGH word
#define MB_REG_B1_TIEMPO_TOTAL_L   244  // 40245 - Bomba 1 total LOW word
#define MB_REG_B1_TIEMPO_TOTAL_H   245  // 40246 - Bomba 1 total HIGH word
#define MB_REG_B2_TIEMPO_TOTAL_L   246  // 40247 - Bomba 2 total LOW word
#define MB_REG_B2_TIEMPO_TOTAL_H   247  // 40248 - Bomba 2 total HIGH word
#define MB_REG_ALT_TRANSCURRIDA    248  // 40249 - Alternancia transcurrida (seg)
#define MB_REG_ALT_RESTANTE        249  // 40250 - Alternancia restante (seg)
#define MB_REG_POST_ACTIVA         250  // 40251 - Post-circulación activa (0/1)
#define MB_REG_POST_RESTANTE       251  // 40252 - Post-circulación restante (seg)


// ===== NUEVA FUNCIÓN: Actualizar variable interna cuando Modbus escribe =====
void updateInternalVariableFromModbus(uint16_t regNum, uint16_t val) {
  switch(regNum) {
    case MB_REG_ALT_HOURS:
      cfg_alternanciaHoras = val;
      Serial.printf("NVS: Alternancia actualizada a %d minutos\n", val);
      break;
    case MB_REG_PUMP_STOP_DELAY:
      cfg_postCirculacionMin = val;
      Serial.printf("NVS: Post-circulación actualizada a %d minutos\n", val);
      break;
    case MB_REG_GT_TMIN:
      cfg_tempMinGT_x10 = val;
      Serial.printf("NVS: Temp mín GT actualizada a %.1f°C\n", val/10.0);
      break;
    case MB_REG_GT_TMAX:
      cfg_tempMaxGT_x10 = val;
      Serial.printf("NVS: Temp máx GT actualizada a %.1f°C\n", val/10.0);
      break;
    case MB_REG_GT_SENSOR_MODE:
      cfg_sensorMode = val;
      Serial.printf("NVS: Modo sensor GT actualizado a %d\n", val);
      break;
    case MB_REG_GT_TFIXED:
      cfg_tempFijaGT_x10 = val;
      Serial.printf("NVS: Temp fija GT actualizada a %.1f°C\n", val/10.0);
      break;
    case MB_REG_SCHED_ENABLE:
      cfg_schedEnable = val;
      Serial.printf("NVS: Programación habilitada: %d\n", val);
      break;
    case MB_REG_SCHED_M_ON:
      cfg_schedMananaON = val;
      Serial.printf("NVS: Sched mañana ON actualizado a %d minutos\n", val);
      break;
    case MB_REG_SCHED_M_OFF:
      cfg_schedMananaOFF = val;
      Serial.printf("NVS: Sched mañana OFF actualizado a %d minutos\n", val);
      break;
    case MB_REG_SCHED_T_ON:
      cfg_schedTardeON = val;
      Serial.printf("NVS: Sched tarde ON actualizado a %d minutos\n", val);
      break;
    case MB_REG_SCHED_T_OFF:
      cfg_schedTardeOFF = val;
      Serial.printf("NVS: Sched tarde OFF actualizado a %d minutos\n", val);
      break;
    case MB_REG_SCHED_DOW_MASK:
      cfg_schedDiasMask = val;
      Serial.printf("NVS: Días programación actualizados a 0x%02X\n", val);
      break;
    case MB_REG_NTP_AUTO_24H:
      cfg_ntpAuto24h = val;
      Serial.printf("NVS: NTP Auto 24h actualizado a %d\n", val);
      break;
    case MB_REG_NTP_AUTO_DST:
      cfg_ntpAutoDST = val;
      Serial.printf("NVS: NTP Auto DST actualizado a %d\n", val);
      // Reconfigurar timezone inmediatamente
      configTime(gmtOffset_sec, cfg_ntpAutoDST ? daylightOffset_sec : 0, ntpServer);
      break;
    case MB_REG_NTP_SYNC_NOW:
      if (val == 1) {
        Serial.println("NVS: Trigger sincronización NTP");
        sincronizarNTP();
        mb.Hreg(MB_REG_NTP_SYNC_NOW, 0);  // Reset automático
      }
      break;
  }
}

// ===== CALLBACK MODBUS REVISADO =====
uint16_t modbusCallback(TRegister* reg, uint16_t val) {
  TAddress addr = reg->address;
  uint16_t regNum = addr.address;

  // Guardar cualquier registro escribible en NVS
  prefs.begin("caldera", false);
  prefs.putUShort(String("reg_" + String(regNum)).c_str(), val);
  prefs.end();

  // Actualizar variable interna correspondiente
  updateInternalVariableFromModbus(regNum, val);

  Serial.printf("MODBUS: Registro %d guardado en NVS = %d\n", regNum, val);
  return val;
}

// ===== NUEVA FUNCIÓN: Configurar TODOS los callbacks Modbus =====
void setupModbusCallbacks() {
  // Registros de configuración principal (40001-40007)
  mb.onSetHreg(MB_REG_ALT_HOURS, modbusCallback);
  mb.onSetHreg(MB_REG_PUMP_STOP_DELAY, modbusCallback);
  mb.onSetHreg(MB_REG_GT_TMIN, modbusCallback);
  mb.onSetHreg(MB_REG_GT_TMAX, modbusCallback);
  mb.onSetHreg(MB_REG_GT_SENSOR_MODE, modbusCallback);
  mb.onSetHreg(MB_REG_GT_TFIXED, modbusCallback);
  mb.onSetHreg(MB_REG_SCHED_ENABLE, modbusCallback);
  
  // Registros de programación horaria (40100-40104)
  mb.onSetHreg(MB_REG_SCHED_M_ON, modbusCallback);
  mb.onSetHreg(MB_REG_SCHED_M_OFF, modbusCallback);
  mb.onSetHreg(MB_REG_SCHED_T_ON, modbusCallback);
  mb.onSetHreg(MB_REG_SCHED_T_OFF, modbusCallback);
  mb.onSetHreg(MB_REG_SCHED_DOW_MASK, modbusCallback);
  
  // Registros de configuración NTP (40105-40107)
  mb.onSetHreg(MB_REG_NTP_AUTO_24H, modbusCallback);
  mb.onSetHreg(MB_REG_NTP_AUTO_DST, modbusCallback);
  mb.onSetHreg(MB_REG_NTP_SYNC_NOW, modbusCallback);
  
  Serial.println("✓ Callbacks Modbus configurados para TODOS los registros escribibles");
}

// ===== NUEVA FUNCIÓN: Cargar TODA la configuración desde NVS =====
void loadAllSettingsFromNVS() {
  prefs.begin("caldera", false);
  
  // Cargar y actualizar TODOS los registros Modbus desde NVS
  // Si no existe en NVS, usa el valor por defecto actual
  
  // Registros principales (40001-40007)
  cfg_alternanciaHoras = prefs.getUShort("reg_0", cfg_alternanciaHoras);
  mb.Hreg(MB_REG_ALT_HOURS, cfg_alternanciaHoras);
  
  cfg_postCirculacionMin = prefs.getUShort("reg_1", cfg_postCirculacionMin);
  mb.Hreg(MB_REG_PUMP_STOP_DELAY, cfg_postCirculacionMin);
  
  cfg_tempMinGT_x10 = prefs.getUShort("reg_2", cfg_tempMinGT_x10);
  mb.Hreg(MB_REG_GT_TMIN, cfg_tempMinGT_x10);
  
  cfg_tempMaxGT_x10 = prefs.getUShort("reg_3", cfg_tempMaxGT_x10);
  mb.Hreg(MB_REG_GT_TMAX, cfg_tempMaxGT_x10);
  
  cfg_sensorMode = prefs.getUShort("reg_4", cfg_sensorMode);
  mb.Hreg(MB_REG_GT_SENSOR_MODE, cfg_sensorMode);
  
  cfg_tempFijaGT_x10 = prefs.getUShort("reg_5", cfg_tempFijaGT_x10);
  mb.Hreg(MB_REG_GT_TFIXED, cfg_tempFijaGT_x10);
  
  cfg_schedEnable = prefs.getUShort("reg_6", cfg_schedEnable);
  mb.Hreg(MB_REG_SCHED_ENABLE, cfg_schedEnable);
  
  // Programación horaria (40100-40104)
  cfg_schedMananaON = prefs.getUShort("reg_100", cfg_schedMananaON);
  mb.Hreg(MB_REG_SCHED_M_ON, cfg_schedMananaON);
  
  cfg_schedMananaOFF = prefs.getUShort("reg_101", cfg_schedMananaOFF);
  mb.Hreg(MB_REG_SCHED_M_OFF, cfg_schedMananaOFF);
  
  cfg_schedTardeON = prefs.getUShort("reg_102", cfg_schedTardeON);
  mb.Hreg(MB_REG_SCHED_T_ON, cfg_schedTardeON);
  
  cfg_schedTardeOFF = prefs.getUShort("reg_103", cfg_schedTardeOFF);
  mb.Hreg(MB_REG_SCHED_T_OFF, cfg_schedTardeOFF);
  
  cfg_schedDiasMask = prefs.getUShort("reg_104", cfg_schedDiasMask);
  mb.Hreg(MB_REG_SCHED_DOW_MASK, cfg_schedDiasMask);
  
  // Configuración NTP (40105-40106)
  cfg_ntpAuto24h = prefs.getUShort("reg_105", cfg_ntpAuto24h);
  mb.Hreg(MB_REG_NTP_AUTO_24H, cfg_ntpAuto24h);
  
  cfg_ntpAutoDST = prefs.getUShort("reg_106", cfg_ntpAutoDST);
  mb.Hreg(MB_REG_NTP_AUTO_DST, cfg_ntpAutoDST);
  
  // Cargar horas de vida totales de bombas
  tiempoB1_total_ms = prefs.getULong("b1_total_ms", 0);
  tiempoB2_total_ms = prefs.getULong("b2_total_ms", 0);
  
  prefs.end();
  
  Serial.println("✓ Configuración COMPLETA cargada desde NVS a Modbus");
}

// ===== FUNCIÓN PARA GESTIÓN DE WIFI =====
void gestionWiFi() {
    static unsigned long ultimoCheck = 0;
    static unsigned long ultimoIntento = 0;
    static unsigned long backoffTime = 5000;  // Inicia en 5s, aumenta exponencialmente
    static bool wifiConectadoAnterior = false;
    
    // Verificar estado WiFi cada 3 segundos
    if (millis() - ultimoCheck >= 3000) {
        ultimoCheck = millis();
        
        // ===== GARANTIZAR QUE AP SIEMPRE ESTÁ ACTIVO =====
        if (!ap_started || WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {
            Serial.println("⚠️  AP WiFi caído, reactivando...");
            WiFi.mode(WIFI_AP_STA);
            IPAddress apIP(192, 168, 4, 1);
            bool apOk = WiFi.softAP("Caldera_ESP32S3", "caldera2026", 1, 0, 4);
            if (apOk) {
                WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
                ap_started = true;
                Serial.println("✓ AP WiFi reactivado");
            }
        }
        
        bool wifiConectadoAhora = (WiFi.status() == WL_CONNECTED);
        
        // ===== EVENTO: Desconexión STA =====
        if (wifiConectadoAnterior && !wifiConectadoAhora) {
            Serial.println("⚠️  WiFi STA DESCONECTADO");
            sta_lost_since_ms = millis();
            backoffTime = 5000;  // Reset backoff
        }
        
        // ===== EVENTO: Reconexión STA =====
        if (!wifiConectadoAnterior && wifiConectadoAhora) {
            Serial.println("✓ WiFi STA CONECTADO");
            Serial.print("   IP: ");
            Serial.println(WiFi.localIP());
            Serial.print("   RSSI: ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
            sta_connecting = false;
            sta_connected_since_ms = millis();
            sta_lost_since_ms = 0;
            backoffTime = 5000;  // Reset backoff
            
            // Reconfigurar NTP cuando se reconecta
            if (!ntpSincronizando) {
                configTime(gmtOffset_sec, cfg_ntpAutoDST ? daylightOffset_sec : 0, ntpServer);
            }
        }
        
        // ===== REINTENTOS DE CONEXIÓN STA (sin bloquear) =====
        if (!wifiConectadoAhora && !sta_connecting) {
            // Solo intentar si pasó el tiempo de backoff
            if (millis() - ultimoIntento >= backoffTime) {
                prefs.begin("caldera", true);
                String ssid_saved = prefs.getString("wifi_ssid", "");
                String pass_saved = prefs.getString("wifi_pass", "");
                prefs.end();
                
                if (ssid_saved.length() > 0) {
                    Serial.printf("🔄 Reintentando WiFi (espera: %.1fs)\n", backoffTime/1000.0);
                    WiFi.mode(WIFI_AP_STA);  // Garantizar modo dual
                    WiFi.begin(ssid_saved.c_str(), pass_saved.c_str());
                    sta_connecting = true;
                    ultimoIntento = millis();
                    
                    // Incrementar backoff (máximo 120 segundos)
                    backoffTime = (unsigned long)min((double)backoffTime * 1.5, 120000.0);
                }
            }
        }
        
        // ===== TIMEOUT DE CONEXIÓN (máximo 20 segundos por intento) =====
        if (sta_connecting && (millis() - ultimoIntento > 20000)) {
            Serial.println("✗ Timeout WiFi STA (20s)");
            WiFi.disconnect();
            sta_connecting = false;
        }
        
        wifiConectadoAnterior = wifiConectadoAhora;
    }
}


/* =========================================================================================
   WEB SERVER - 100% INTACTO (MISMO HTML QUE V423_2)
   ========================================================================================= */
WebServer server(80);

// HTML COMPLETAMENTE INTACTO - MISMO QUE EN V423_2
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Controlador Caldera V527</title>
<style>
* { margin: 0; padding: 0; box-sizing: border-box; }
body { font-family: Arial, sans-serif; background: #f5f5f5; padding: 20px; }
.container { max-width: 1400px; margin: 0 auto; background: white; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
.header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 20px; border-radius: 8px 8px 0 0; }
.header h1 { font-size: 28px; margin-bottom: 5px; }
.tabs { display: flex; background: #f8f9fa; border-bottom: 2px solid #dee2e6; }
.tab { padding: 15px 30px; cursor: pointer; border: none; background: transparent; font-size: 16px; color: #495057; transition: all 0.3s; }
.tab:hover { background: #e9ecef; }
.tab.active { background: white; color: #667eea; border-bottom: 3px solid #667eea; font-weight: bold; }
.content { display: none; padding: 30px; }
.content.active { display: block; }
.info-box { background: #e7f3ff; border-left: 4px solid #2196F3; padding: 15px; margin: 20px 0; border-radius: 4px; display: grid; grid-template-columns: repeat(2, 1fr); gap: 8px 15px; }
.info-box p { margin: 0; }
@media (max-width: 768px) { .info-box { grid-template-columns: 1fr; } }
table { width: 100%; border-collapse: collapse; margin: 20px 0; }
th, td { padding: 12px; text-align: left; border-bottom: 1px solid #dee2e6; }
th { background: #667eea; color: white; font-weight: bold; }
tr:hover { background: #f8f9fa; }
.led { display: inline-block; width: 14px; height: 14px; border-radius: 50%; margin-right: 8px; box-shadow: 0 0 5px rgba(0,0,0,0.3); }
.led-green { background: #4CAF50; box-shadow: 0 0 10px #4CAF50; }
.led-red { background: #f44336; box-shadow: 0 0 10px #f44336; }
.led-blue { background: #2196F3; box-shadow: 0 0 10px #2196F3; }
.led-gray { background: #9e9e9e; }
input[type=number], input[type=text], input[type=password], input[type=time] { padding: 8px 12px; border: 1px solid #ced4da; border-radius: 4px; font-size: 14px; width: 200px; }
input[type=number]:focus, input[type=text]:focus, input[type=password]:focus, input[type=time]:focus { outline: none; border-color: #667eea; box-shadow: 0 0 0 3px rgba(102,126,234,0.1); }
button { padding: 10px 20px; background: #667eea; color: white; border: none; border-radius: 4px; cursor: pointer; font-size: 14px; margin: 5px; transition: all 0.3s; }
button:hover { background: #5568d3; transform: translateY(-1px); box-shadow: 0 4px 8px rgba(0,0,0,0.2); }
.config-row { padding: 15px; border-bottom: 1px solid #eee; display: flex; align-items: center; justify-content: space-between; }
.config-label { font-weight: bold; flex: 1; }
.config-value { flex: 1; text-align: right; }
.wifi-item { padding: 12px; border: 1px solid #dee2e6; margin: 8px 0; cursor: pointer; border-radius: 4px; transition: all 0.3s; }
.wifi-item:hover { background: #f8f9fa; border-color: #667eea; }
h2 { color: #333; margin-bottom: 20px; padding-bottom: 10px; border-bottom: 2px solid #667eea; }
h3 { color: #555; margin: 25px 0 15px 0; }
</style>
</head>
<body>
<div class="container">
<div class="header">
<h1>🔥 Controlador Caldera ESP32 V527</h1>
<p>Sistema de Control Industrial - Tiempo Real con Sincronización NTP</p>
</div>

<div class="tabs">
<button class="tab active" onclick="showTab(0)">📊 Principal</button>
<button class="tab" onclick="showTab(1)">⚙️ Configuración</button>
<button class="tab" onclick="showTab(2)">📡 WiFi</button>
<button class="tab" onclick="showTab(3)">🚨 Alarmas</button>
<button class="tab" onclick="showTab(4)">⏰ Programación</button>
</div>

<div id="tab0" class="content active">
<h2>Estado del Sistema</h2>
<div class="info-box">
<p><strong>🕐 Hora:</strong> <span id="hora">--:--:--</span></p>
<p><strong>📅 Fecha:</strong> <span id="fecha">--/--/----</span></p>
<p><strong>🌡️ Temperatura:</strong> <span id="temp">--.-</span> °C</p>
<p><strong>📊 Rango:</strong> <span id="temp-rango" style="font-weight:bold;">--</span></p>
<p><strong>⏱️ Bomba 1 (Parcial):</strong> <span id="b1">00:00:00</span></p>
<p><strong>⏱️ Bomba 2 (Parcial):</strong> <span id="b2">00:00:00</span></p>
<p><strong>⏱️ Bomba 1 (Total):</strong> <span id="b1_total">00:00:00</span></p>
<p><strong>⏱️ Bomba 2 (Total):</strong> <span id="b2_total">00:00:00</span></p>
<p><strong>🔄 Alternancia:</strong> <span id="alt">00:00</span></p>
<p><strong>🔥 Post‑Circulación:</strong> <span id="post">00:00:00</span></p>
</div>
<h3>Entradas Digitales</h3>
<table id="tbl-entradas-dig">
<tr><th>PIN</th><th>Nombre</th><th>Estado</th><th>Nº Registro</th><th>Valor</th></tr>
</table>
<h3>Entradas Analógicas</h3>
<table id="tbl-entradas-ana">
<tr><th>PIN</th><th>Nombre</th><th>Nº Registro</th><th>Valor (Unidades)</th></tr>
</table>
<h3>Salidas Digitales</h3>
<table id="tbl-salidas-dig">
<tr><th>PIN</th><th>Nombre</th><th>Estado</th><th>Nº Registro</th><th>Valor</th></tr>
</table>
</div>

<div id="tab1" class="content">
<h2>Configuración del Sistema</h2>
<div class="info-box">Los cambios se guardan automáticamente en memoria no volátil (NVS)</div>
<h3>Parámetros Generales</h3>
<table>
<tr><th>Nº Registro</th><th>Parámetro</th><th>Valor Actual</th><th>Nuevo Valor</th><th>Unidad / Escalado</th><th>Acción</th></tr>
<tr><td>40001 (0)</td><td>Alternancia Bombas</td><td><span id="val0" style="font-weight:bold;color:#667eea">--</span></td><td><input type="number" id="cfg0"></td><td>minutos (x1)</td><td><button onclick="setCfg(0)">✓ Enviar</button></td></tr>
<tr><td>40002 (1)</td><td>Post-circulación</td><td><span id="val1" style="font-weight:bold;color:#667eea">--</span></td><td><input type="number" id="cfg1"></td><td>minutos (x1)</td><td><button onclick="setCfg(1)">✓ Enviar</button></td></tr>
<tr><td>40003 (2)</td><td>Temp Mínima GT</td><td><span id="val2" style="font-weight:bold;color:#667eea">--</span></td><td><input type="number" step="0.1" id="cfg2"></td><td>°C </td><td><button onclick="setCfg(2)">✓ Enviar</button></td></tr>
<tr><td>40004 (3)</td><td>Temp Máxima GT</td><td><span id="val3" style="font-weight:bold;color:#667eea">--</span></td><td><input type="number" step="0.1" id="cfg3"></td><td>°C </td><td><button onclick="setCfg(3)">✓ Enviar</button></td></tr>
<tr><td>40005 (4)</td><td>Modo Sensor GT</td><td><span id="val4" style="font-weight:bold;color:#667eea">--</span></td><td><input type="number" id="cfg4"></td><td>0=fijo, 1=NTC</td><td><button onclick="setCfg(4)">✓ Enviar</button></td></tr>
<tr><td>40006 (5)</td><td>Temp Fija GT</td><td><span id="val5" style="font-weight:bold;color:#667eea">--</span></td><td><input type="number" step="0.1" id="cfg5"></td><td>°C </td><td><button onclick="setCfg(5)">✓ Enviar</button></td></tr>
<tr><td>40007 (6)</td><td>Horario Habilitado</td><td><span id="val6" style="font-weight:bold;color:#667eea">--</span></td><td><input type="number" id="cfg6"></td><td>0=no, 1=sí</td><td><button onclick="setCfg(6)">✓ Enviar</button></td></tr>
</table>
<h3 style="margin-top:30px">Reset Contadores Bombas</h3>
<table>
<tr><th>Nº Registro</th><th>Acción</th><th>Descripción</th></tr>
<tr><td>40228 (227)</td><td><button onclick="resetContador(227)">🔄 Reset Bomba 1</button></td><td>Resetea contador Bomba 1</td></tr>
<tr><td>40229 (228)</td><td><button onclick="resetContador(228)">🔄 Reset Bomba 2</button></td><td>Resetea contador Bomba 2</td></tr>
</table>
<h3 style="margin-top:30px">⏰ Sincronización Hora NTP</h3>
<div class="info-box">
<p><strong>Servidor NTP:</strong> pool.ntp.org</p>
<p><strong>Zona Horaria:</strong> CET (GMT+1)</p>
<p id="ntp-status"><strong>Estado:</strong> <span id="ntp-sync-status">Desconocido</span></p>
<p id="ultima-sync"><strong>Última Sincronización:</strong> <span id="ntp-last-sync">Nunca</span></p>
</div>
<table>
<tr>
<td><button onclick="sincronizarNTP()">🌐 Establecer Hora NTP</button></td>
<td><label><input type="checkbox" id="ntp-auto-24h" onchange="saveNTPConfig()"> Actualizar automáticamente cada 24 horas</label></td>
<td><label><input type="checkbox" id="ntp-auto-dst" onchange="saveNTPConfig()"> Ajuste automático horario Invierno/Verano</label></td>
</tr>
</table>
<h3 style="margin-top:30px">⏰ Configuración Fecha y Hora Manual</h3>
<div class="info-box">Configura manualmente si NTP no está disponible</div>
<table>
<tr>
<td><input type="number" id="year" placeholder="Año" min="2024" max="2099" style="width:80px"></td>
<td><input type="number" id="month" placeholder="Mes" min="1" max="12" style="width:60px"></td>
<td><input type="number" id="day" placeholder="Día" min="1" max="31" style="width:60px"></td>
<td><input type="number" id="hour" placeholder="Hora" min="0" max="23" style="width:60px"></td>
<td><input type="number" id="minute" placeholder="Min" min="0" max="59" style="width:60px"></td>
<td><input type="number" id="second" placeholder="Seg" min="0" max="59" style="width:60px"></td>
<td><button onclick="setDateTime()">✓ Establecer Hora</button></td>
</tr>
</table>
</div>

<div id="tab2" class="content">
<h2>Configuración WiFi</h2>
<div class="info-box">
<p><strong>IP Punto de Acceso (AP):</strong> <span id="ip-ap">192.168.4.1</span></p>
<p><strong>IP Estación (STA):</strong> <span id="ip-sta">No conectado</span></p>
<p><strong>SSID AP:</strong> Caldera_ESP32</p>
<p><strong>Password:</strong> caldera2026</p>
</div>
<button onclick="scanWiFi()">🔍 Escanear Redes WiFi</button>
<div id="wifi-list"></div>
<h3>Conectar a Red WiFi</h3>
<p><input type="text" id="wifi-ssid" placeholder="SSID de la red"></p>
<p><input type="password" id="wifi-pass" placeholder="Contraseña"></p>
<button onclick="connectWiFi()">🔌 Conectar</button>
</div>

<div id="tab3" class="content">
<h2>Estado de Alarmas</h2>
<table id="tbl-alarmas">
<tr><th>PIN</th><th>Nombre</th><th>Estado</th><th>Valor Actual</th><th>Valor de Referencia</th></tr>
</table>
</div>

<div id="tab4" class="content">
<h2>Programación Horaria</h2>
<div class="info-box">
<label><input type="checkbox" id="sched-enable"> Activar programación horaria (requiere selector PROG=ON en PIN33)</label>
<p style="margin-top:10px"><strong>NOTA:</strong> Función de guardar pendiente de implementar en backend</p>
</div>
<h3>Horarios</h3>
<table>
<tr><th>Tramo</th><th>Hora ON</th><th>Hora OFF</th></tr>
<tr><td>Mañana</td><td><input type="time" id="sched-m-on"></td><td><input type="time" id="sched-m-off"></td></tr>
<tr><td>Tarde</td><td><input type="time" id="sched-t-on"></td><td><input type="time" id="sched-t-off"></td></tr>
</table>
<h3 style="margin-top:30px">Días de la Semana</h3>
<table style="width:auto; text-align:center">
<tr>
<th></th>
<th>LUNES</th>
<th>MARTES</th>
<th>MIÉRCOLES</th>
<th>JUEVES</th>
<th>VIERNES</th>
<th>SÁBADO</th>
<th>DOMINGO</th>
</tr>
<tr>
<td><strong>MAÑANA</strong></td>
<td><input type="checkbox" id="day-mon-m"></td>
<td><input type="checkbox" id="day-tue-m"></td>
<td><input type="checkbox" id="day-wed-m"></td>
<td><input type="checkbox" id="day-thu-m"></td>
<td><input type="checkbox" id="day-fri-m"></td>
<td><input type="checkbox" id="day-sat-m"></td>
<td><input type="checkbox" id="day-sun-m"></td>
</tr>
<tr>
<td><strong>TARDE</strong></td>
<td><input type="checkbox" id="day-mon-t"></td>
<td><input type="checkbox" id="day-tue-t"></td>
<td><input type="checkbox" id="day-wed-t"></td>
<td><input type="checkbox" id="day-thu-t"></td>
<td><input type="checkbox" id="day-fri-t"></td>
<td><input type="checkbox" id="day-sat-t"></td>
<td><input type="checkbox" id="day-sun-t"></td>
</tr>
</table>
<button onclick="saveSchedule()" style="margin-top:20px">💾 Guardar Programación</button>
</div>

</div>

<script>
let data = {};

function showTab(n) {
  document.querySelectorAll('.tab').forEach((t, i) => {
    t.classList.toggle('active', i === n);
  });
  document.querySelectorAll('.content').forEach((c, i) => {
    c.classList.toggle('active', i === n);
  });
}

function updateData() {
  fetch('/data').then(r => r.json()).then(d => {
    data = d;
    updateUI();
  }).catch(e => console.error('Error:', e));
}

function updateUI() {
  // Actualizar hora y fecha
  document.getElementById('hora').innerHTML = data.hora;
  document.getElementById('fecha').innerHTML = data.fecha;
  document.getElementById('temp').innerHTML = data.temp || '--.-';
  
  // Estado del rango de temperatura
  let rangoEl = document.getElementById('temp-rango');
  if (data.temp_rango === true) {
    rangoEl.innerHTML = '✅ En rango';
    rangoEl.style.color = '#4CAF50';
  } else if (data.temp_rango === false) {
    rangoEl.innerHTML = '⚠️ Fuera de rango';
    rangoEl.style.color = '#ff9800';
  } else {
    rangoEl.innerHTML = '--';
    rangoEl.style.color = '#999';
  }
  
  // Actualizar contadores tiempo (formato HH:MM)
  document.getElementById('b1').innerHTML = data.b1_tiempo;
  document.getElementById('b2').innerHTML = data.b2_tiempo;
  
  // Totales de vida
  document.getElementById('b1_total').innerHTML = data.b1_total || '00:00:00';
  document.getElementById('b2_total').innerHTML = data.b2_total || '00:00:00';

  // Alternancia y post-circulación
  document.getElementById('alt').innerHTML  = data.alt_tiempo || '00:00';
  document.getElementById('post').innerHTML = data.post_tiempo || '00:00:00';

  
  // Entradas digitales - CON VOLTAJE REAL Y MODBUS
  let html = '<tr><th>PIN</th><th>Nombre</th><th>Estado</th><th>Voltaje Real</th><th>Valor Modbus</th><th>Nº Registro</th></tr>';
  html += `<tr><td>4</td><td>PIN_SISTEMA</td><td><span class="led led-${data.pin4_state?'green':'gray'}"></span>${data.pin4_state?'MARCHA':'OFF'}</td><td>${data.pin4_volt}V</td><td>${data.pin4_mb}</td><td>40211 (210)</td></tr>`;
  html += `<tr><td>5</td><td>PIN_PROG</td><td><span class="led led-${data.pin5_state?'green':'gray'}"></span>${data.pin5_state?'PROG. ON':'PROG. OFF'}</td><td>${data.pin5_volt}V</td><td>${data.pin5_mb}</td><td>40212 (211)</td></tr>`;
  html += `<tr><td>15</td><td>PIN_JEFATURA</td><td><span class="led led-${data.pin15_state?'green':'gray'}"></span>${data.pin15_state?'MARCHA':'OFF'}</td><td>${data.pin15_volt}V</td><td>${data.pin15_mb}</td><td>40215 (214)</td></tr>`;
  html += `<tr><td>6</td><td>PIN_BOMBA1_INT</td><td><span class="led led-${data.pin6_state?'green':'gray'}"></span>${data.pin6_state?'ON':'OFF'}</td><td>${data.pin6_volt}V</td><td>${data.pin6_mb}</td><td>40213 (212)</td></tr>`;
  html += `<tr><td>7</td><td>PIN_BOMBA2_INT</td><td><span class="led led-${data.pin7_state?'green':'gray'}"></span>${data.pin7_state?'ON':'OFF'}</td><td>${data.pin7_volt}V</td><td>${data.pin7_mb}</td><td>40214 (213)</td></tr>`;
  html += `<tr><td>17</td><td>PIN_GT_SWITCH</td><td><span class="led led-${data.pin17_state?'green':'gray'}"></span>${data.pin17_state?'ON':'OFF'}</td><td>${data.pin17_volt}V</td><td>${data.pin17_mb}</td><td>40217 (216)</td></tr>`;
  html += `<tr><td>18</td><td>PIN_BC_SWITCH</td><td><span class="led led-${data.pin18_state?'green':'gray'}"></span>${data.pin18_state?'ON':'OFF'}</td><td>${data.pin18_volt}V</td><td>${data.pin18_mb}</td><td>40218 (217)</td></tr>`;
  html += `<tr><td>16</td><td>PIN_TERM_BC</td><td><span class="led led-${data.pin16_state?'green':'gray'}"></span>${data.pin16_state?'ON':'OFF'}</td><td>${data.pin16_volt}V</td><td>${data.pin16_mb}</td><td>40216 (215)</td></tr>`;
  html += `<tr><td>9</td><td>PIN_RT_B1</td><td><span class="led led-${data.pin9_state?'red':'gray'}"></span>${data.pin9_state?'AVERIA RT1':'NO AVERIA'}</td><td>${data.pin9_volt}V</td><td>${data.pin9_mb}</td><td>40220 (219)</td></tr>`;
  html += `<tr><td>10</td><td>PIN_RT_B2</td><td><span class="led led-${data.pin10_state?'red':'gray'}"></span>${data.pin10_state?'AVERIA RT2':'NO AVERIA'}</td><td>${data.pin10_volt}V</td><td>${data.pin10_mb}</td><td>40221 (220)</td></tr>`;
  html += `<tr><td>8</td><td>PIN_EMERGENCIA</td><td><span class="led led-${data.pin8_state?'red':'gray'}"></span>${data.pin8_state?'EMERGENCIA':'NO EMERGENCIA'}</td><td>${data.pin8_volt}V</td><td>${data.pin8_mb}</td><td>40219 (218)</td></tr>`;
  html += `<tr><td>11</td><td>PIN_ALARMA_GT</td><td><span class="led led-${data.pin11_state?'red':'gray'}"></span>${data.pin11_state?'AVERIA GT':'NO ALARMA'}</td><td>${data.pin11_volt}V</td><td>${data.pin11_mb}</td><td>40222 (221)</td></tr>`;
  document.getElementById('tbl-entradas-dig').innerHTML = html;
  
  // Entradas analógicas
  html = '<tr><th>PIN</th><th>Nombre</th><th>Nº Registro</th><th>Valor (Unidades)</th></tr>';
  html += `<tr><td>1</td><td>PIN_NTC</td><td>40208 (207)</td><td>${data.temp} °C</td></tr>`;
  document.getElementById('tbl-entradas-ana').innerHTML = html;
  
  // Salidas digitales - LED VERDE cuando voltaje=3.3V (HIGH = activo)
  html = '<tr><th>PIN</th><th>Nombre</th><th>Estado</th><th>Voltaje Real</th><th>Valor Modbus</th><th>Nº Registro</th></tr>';
  html += `<tr><td>12</td><td>RELE_B1</td><td><span class="led led-${data.pin12_volt==0?'green':'gray'}"></span>${data.pin12_volt==0?'MARCHA B1':'PARO B1'}</td><td>${data.pin12_volt}V</td><td>${data.pin12_mb}</td><td>40201 (200)</td></tr>`;
  html += `<tr><td>13</td><td>RELE_B2</td><td><span class="led led-${data.pin13_volt==0?'green':'gray'}"></span>${data.pin13_volt==0?'MARCHA B2':'PARO B2'}</td><td>${data.pin13_volt}V</td><td>${data.pin13_mb}</td><td>40202 (201)</td></tr>`;
  html += `<tr><td>14</td><td>RELE_GT</td><td><span class="led led-${data.pin14_volt==0?'green':'gray'}"></span>${data.pin14_volt==0?'MARCHA GT':'PARO GT'}</td><td>${data.pin14_volt}V</td><td>${data.pin14_mb}</td><td>40203 (202)</td></tr>`;
  html += `<tr><td>21</td><td>RELE_BC</td><td><span class="led led-${data.pin21_volt==0?'green':'gray'}"></span>${data.pin21_volt==0?'MARCHA BC':'PARO BC'}</td><td>${data.pin21_volt}V</td><td>${data.pin21_mb}</td><td>40223 (222)</td></tr>`;
  html += `<tr><td>47</td><td>POSTCIRC_INDIC</td><td><span class="led led-${data.pin47_volt==0?'blue':'gray'}"></span>${data.pin47_volt==0?'POST_CIRCULACION':'OFF'}</td><td>${data.pin47_volt}V</td><td>${data.pin47_mb}</td><td>40224 (223)</td></tr>`;
  html += `<tr><td>38</td><td>SOBRECALENTAMIENTO</td><td><span class="led led-${data.pin38_volt==0?'red':'green'}"></span>${data.pin38_volt==0?'SOBRECALENTAMIENTO':'EN RANGO'}</td><td>${data.pin38_volt}V</td><td>${data.pin38_mb}</td><td>40225 (224)</td></tr>`;
  html += `<tr><td>40</td><td>AVERIA_GT</td><td><span class="led led-${data.pin40_volt==0?'green':'gray'}"></span>${data.pin40_volt==0?'AVERIA GT':'NO_AVERIA_GT'}</td><td>${data.pin40_volt}V</td><td>${data.pin40_mb}</td><td>40226 (225)</td></tr>`;
  html += `<tr><td>42</td><td>PROG_ACTIVA</td><td><span class="led led-${data.pin42_volt==0?'green':'gray'}"></span>${data.pin42_volt==0?'PROG_ACTIVA':'NO_PROG'}</td><td>${data.pin42_volt}V</td><td>${data.pin42_mb}</td><td>40227 (226)</td></tr>`;
  html += `<tr><td>2</td><td>JEFATURA</td><td><span class="led led-${data.pin2_volt==0?'blue':'gray'}"></span>${data.pin2_volt==0?'MARCHA JEFAT':'OFF'}</td><td>${data.pin2_volt}V</td><td>${data.pin2_mb}</td><td>40232 (231)</td></tr>`;
  html += `<tr><td>45</td><td>TEMPERATURA FUERA DE RANGO</td><td><span class="led led-${data.pin45_volt==0?'red':'gray'}"></span>${data.pin45_volt==0?'FUERA RANGO':'EN RANGO'}</td><td>${data.pin45_volt}V</td><td>${data.pin45_mb}</td><td>40233 (232)</td></tr>`;
  html += `<tr><td>43</td><td>EMERGENCIA</td><td><span class="led led-${data.pin43_volt==0?'red':'gray'}"></span>${data.pin43_volt==0?'EMERGENCIA':'NO_EMERGENCIA'}</td><td>${data.pin43_volt}V</td><td>${data.pin43_mb}</td><td>40234 (233)</td></tr>`;
  html += `<tr><td>48</td><td>SISTEMA</td><td><span class="led led-${data.pin48_volt==0?'green':'gray'}"></span>${data.pin48_volt==0?'SISTEMA_ON':'SISTEMA_OFF'}</td><td>${data.pin48_volt}V</td><td>${data.pin48_mb}</td><td>40235 (234)</td></tr>`;
  html += `<tr><td>39</td><td>AVERIA_B1</td><td><span class="led led-${data.pin39_volt==0?'red':'gray'}"></span>${data.pin39_volt==0?'AVERIA B1':'NO_AVERIA_B1'}</td><td>${data.pin39_volt}V</td><td>${data.pin39_mb}</td><td>40204 (203)</td></tr>`;
  html += `<tr><td>41</td><td>AVERIA_B2</td><td><span class="led led-${data.pin41_volt==0?'red':'gray'}"></span>${data.pin41_volt==0?'AVERIA B2':'NO_AVERIA_B2'}</td><td>${data.pin41_volt}V</td><td>${data.pin41_mb}</td><td>40205 (204)</td></tr>`;
  document.getElementById('tbl-salidas-dig').innerHTML = html;
  
  // Alarmas - CON VOLTAJE REAL Y MODBUS
  html = '<tr><th>PIN</th><th>Nombre</th><th>Estado</th><th>Voltaje Real</th><th>Valor Modbus</th></tr>';
  html += `<tr><td>9</td><td>Alarma RT Bomba 1</td><td><span class="led led-${data.pin9_state?'red':'gray'}"></span>${data.pin9_state?'ALARMA':'OFF'}</td><td>${data.pin9_volt}V</td><td>${data.pin9_mb}</td></tr>`;
  html += `<tr><td>10</td><td>Alarma RT Bomba 2</td><td><span class="led led-${data.pin10_state?'red':'gray'}"></span>${data.pin10_state?'ALARMA':'OFF'}</td><td>${data.pin10_volt}V</td><td>${data.pin10_mb}</td></tr>`;
  html += `<tr><td>8</td><td>Emergencia</td><td><span class="led led-${data.pin8_state?'red':'gray'}"></span>${data.pin8_state?'ALARMA':'OFF'}</td><td>${data.pin8_volt}V</td><td>${data.pin8_mb}</td></tr>`;
  html += `<tr><td>11</td><td>Alarma Grupo Térmico</td><td><span class="led led-${data.pin11_state?'red':'gray'}"></span>${data.pin11_state?'ALARMA':'OFF'}</td><td>${data.pin11_volt}V</td><td>${data.pin11_mb}</td></tr>`;
  document.getElementById('tbl-alarmas').innerHTML = html;
  
  // Actualizar IP STA
  document.getElementById('ip-sta').innerHTML = data.ip_sta || 'No conectado';
  
  // Actualizar valores actuales en tabla de configuración
  if (document.getElementById('val0')) {
    document.getElementById('val0').textContent = data.cfg_alt || '--';
    document.getElementById('val1').textContent = data.cfg_post || '--';
    document.getElementById('val2').textContent = data.cfg_tmin ? (data.cfg_tmin/10).toFixed(1) : '--';
    document.getElementById('val3').textContent = data.cfg_tmax ? (data.cfg_tmax/10).toFixed(1) : '--';
    document.getElementById('val4').textContent = data.cfg_smode !== undefined ? data.cfg_smode : '--';
    document.getElementById('val5').textContent = data.cfg_tfix ? (data.cfg_tfix/10).toFixed(1) : '--';
    document.getElementById('val6').textContent = data.cfg_schen !== undefined ? data.cfg_schen : '--';
    
    // Actualizar estado NTP
    document.getElementById('ntp-sync-status').textContent = data.ntp_sync_ok ? 'Sincronizado ✅' : 'No sincronizado ⚠️';
    document.getElementById('ntp-last-sync').textContent = data.ntp_last_sync || 'Nunca';
  }
}

function setCfg(reg) {
  let val = document.getElementById('cfg' + reg).value;
  // Aplicar escalado x10 para temperatura (registros 2,3,5: Tmin, Tmax, Tfija GT)
  if (reg == 2 || reg == 3 || reg == 5) {
    val = Math.round(val * 10);  // Escalar °C a décimas de °C
  } else {
    val = Math.round(val);
  }
  fetch(`/setcfg?r=${reg}&v=${val}`).then(() => {
    alert('✓ Valor enviado correctamente');
    setTimeout(updateData, 500);
  });
}

function resetContador(reg) {
  fetch(`/reset?r=${reg}`).then(() => {
    alert('✓ Contador reseteado');
    setTimeout(updateData, 500);
  });
}

function setDateTime() {
  let y = document.getElementById('year').value;
  let m = document.getElementById('month').value;
  let d = document.getElementById('day').value;
  let h = document.getElementById('hour').value;
  let min = document.getElementById('minute').value;
  let s = document.getElementById('second').value;
  
  if (!y || !m || !d || !h || !min || !s) {
    alert('⚠️ Completa todos los campos');
    return;
  }
  
  fetch(`/settime?y=${y}&m=${m}&d=${d}&h=${h}&min=${min}&s=${s}`).then(() => {
    alert('✓ Hora configurada');
    setTimeout(updateData, 500);
  });
}

function scanWiFi() {
  document.getElementById('wifi-list').innerHTML = '<p>🔍 Escaneando redes WiFi...</p>';
  
  // Iniciar escaneo asíncrono
  fetch('/scanwifi').then(() => {
    // Esperar resultados con polling cada 500ms
    let checkResults = setInterval(() => {
      fetch('/scanresults').then(r => r.json()).then(d => {
        if (d.status === 'complete') {
          clearInterval(checkResults);
          let html = '<h3>Redes Disponibles:</h3>';
          d.networks.forEach(n => {
            html += `<div class="wifi-item" onclick="document.getElementById('wifi-ssid').value='${n}'">${n}</div>`;
          });
          document.getElementById('wifi-list').innerHTML = html || '<p>No se encontraron redes</p>';
        } else if (d.status === 'failed') {
          clearInterval(checkResults);
          document.getElementById('wifi-list').innerHTML = '<p>⚠️ Error en escaneo WiFi</p>';
        }
        // Si status === 'scanning', continuar esperando
      });
    }, 500);
  });
}

function connectWiFi() {
  let ssid = document.getElementById('wifi-ssid').value;
  let pass = document.getElementById('wifi-pass').value;
  if (!ssid) { alert('Introduce un SSID'); return; }
  fetch(`/connectwifi?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}`).then(() => {
    alert('Conectando a ' + ssid + '...');
  });
}

function saveSchedule() {
  // Leer horarios
  let m_on = document.getElementById('sched-m-on').value.split(':');
  let m_off = document.getElementById('sched-m-off').value.split(':');
  let t_on = document.getElementById('sched-t-on').value.split(':');
  let t_off = document.getElementById('sched-t-off').value.split(':');
  
  // Convertir a minutos desde medianoche
  let mañana_on = parseInt(m_on[0])*60 + parseInt(m_on[1]);
  let mañana_off = parseInt(m_off[0])*60 + parseInt(m_off[1]);
  let tarde_on = parseInt(t_on[0])*60 + parseInt(t_on[1]);
  let tarde_off = parseInt(t_off[0])*60 + parseInt(t_off[1]);
  
  // Leer días (7 checkboxes)
  let dias = 0;
  if (document.getElementById('day-mon-m').checked || document.getElementById('day-mon-t').checked) dias |= (1<<0);
  if (document.getElementById('day-tue-m').checked || document.getElementById('day-tue-t').checked) dias |= (1<<1);
  if (document.getElementById('day-wed-m').checked || document.getElementById('day-wed-t').checked) dias |= (1<<2);
  if (document.getElementById('day-thu-m').checked || document.getElementById('day-thu-t').checked) dias |= (1<<3);
  if (document.getElementById('day-fri-m').checked || document.getElementById('day-fri-t').checked) dias |= (1<<4);
  if (document.getElementById('day-sat-m').checked || document.getElementById('day-sat-t').checked) dias |= (1<<5);
  if (document.getElementById('day-sun-m').checked || document.getElementById('day-sun-t').checked) dias |= (1<<6);
  
  // Enviar al servidor
  let url = `/savesched?m_on=${mañana_on}&m_off=${mañana_off}&t_on=${tarde_on}&t_off=${tarde_off}&dias=${dias}`;
  fetch(url).then(r => r.text()).then(txt => {
    alert('✓ Programación guardada correctamente');
  });
}

function sincronizarNTP() {
  // Mostrar mensaje de sincronización
  document.getElementById('ntp-sync-status').textContent = 'Sincronizando...';
  
  fetch('/syncntp').then(r => {
    if (r.ok) {
      alert('✓ Sincronización NTP exitosa');
      setTimeout(updateData, 500);
    } else {
      alert('⚠️ Error en sincronización NTP');
      document.getElementById('ntp-sync-status').textContent = 'Error';
    }
  }).catch(() => {
    alert('⚠️ Error de conexión');
    document.getElementById('ntp-sync-status').textContent = 'Error de conexión';
  });
}

function saveNTPConfig() {
  let auto24h = document.getElementById('ntp-auto-24h').checked ? 1 : 0;
  let autodst = document.getElementById('ntp-auto-dst').checked ? 1 : 0;
  
  fetch(`/saventpconfig?auto24h=${auto24h}&autodst=${autodst}`).then(r => {
    if (r.ok) {
      console.log('✓ Configuración NTP guardada');
    }
  });
}

function loadConfig() {
  fetch('/data').then(r => r.json()).then(d => {
    document.getElementById('cfg0').value = d.cfg_alt;
    document.getElementById('cfg1').value = d.cfg_post;
    document.getElementById('cfg2').value = (d.cfg_tmin/10);
    document.getElementById('cfg3').value = (d.cfg_tmax/10);
    document.getElementById('cfg4').value = d.cfg_smode;
    document.getElementById('cfg5').value = (d.cfg_tfix/10);
    document.getElementById('cfg6').value = d.cfg_schen;
    
    // Valores actuales (solo visualización)
    document.getElementById('val0').textContent = d.cfg_alt;
    document.getElementById('val1').textContent = d.cfg_post;
    document.getElementById('val2').textContent = (d.cfg_tmin/10).toFixed(1);
    document.getElementById('val3').textContent = (d.cfg_tmax/10).toFixed(1);
    document.getElementById('val4').textContent = d.cfg_smode;
    document.getElementById('val5').textContent = (d.cfg_tfix/10).toFixed(1);
    document.getElementById('val6').textContent = d.cfg_schen;
    
    // Configuración NTP
    document.getElementById('ntp-auto-24h').checked = (d.cfg_ntp_auto24h == 1);
    document.getElementById('ntp-auto-dst').checked = (d.cfg_ntp_autodst == 1);
    document.getElementById('ntp-sync-status').textContent = d.ntp_sync_ok ? 'Sincronizado' : 'No sincronizado';
    document.getElementById('ntp-last-sync').textContent = d.ntp_last_sync;
  });
}

// Actualización automática cada 2 segundos
setInterval(updateData, 2000);
updateData();
loadConfig();  // Cargar configuración inicial
</script>
</body>
</html>
)rawliteral";

// Handlers web - INTACTOS PERO REVISADOS
void handleRoot() { server.send_P(200, "text/html", HTML_PAGE); }

void handleData() {
  // LEER ESTADO FÍSICO REAL DE TODOS LOS PINES + VOLTAJES

  String json = "{";

  // === Numeración real de pines (ESP32-S3) para la UI ===
  json += "\"n_sys\":" + String(PIN_SYS_ONOFF) + ",";
  json += "\"n_prog\":" + String(PIN_SELECTOR_PROG) + ",";
  json += "\"n_sw_b1\":" + String(PIN_SW_B1) + ",";
  json += "\"n_sw_b2\":" + String(PIN_SW_B2) + ",";
  json += "\"n_jef\":" + String(PIN_JEFATURA) + ",";
  json += "\"n_term_bc\":" + String(PIN_TERM_BC) + ",";
  json += "\"n_sw_gt\":" + String(PIN_SW_GT) + ",";
  json += "\"n_sw_bc\":" + String(PIN_SW_BC) + ",";
  json += "\"n_emerg\":" + String(PIN_EMERGENCIA) + ",";
  json += "\"n_rt1\":" + String(PIN_RT1) + ",";
  json += "\"n_rt2\":" + String(PIN_RT2) + ",";
  json += "\"n_al_gt\":" + String(PIN_AL_GT) + ",";
  json += "\"n_out_b1\":" + String(PIN_OUT_B1) + ",";
  json += "\"n_out_b2\":" + String(PIN_OUT_B2) + ",";
  json += "\"n_out_gt\":" + String(PIN_OUT_GT) + ",";
  json += "\"n_out_bc\":" + String(PIN_OUT_BC) + ",";
  json += "\"n_out_post\":" + String(PIN_OUT_POST) + ",";
  json += "\"n_sobrecalent\":" + String(PIN_SOBRECALENTAMIENTO) + ",";
  json += "\"n_averia_b1\":" + String(PIN_AVERIA_B1) + ",";
  json += "\"n_averia_gen\":" + String(PIN_AVERIA_GT) + ",";
  json += "\"n_averia_b2\":" + String(PIN_AVERIA_B2) + ",";
  json += "\"n_prog_activa\":" + String(PIN_PROG_ACTIVA) + ",";
  json += "\"n_paro_jef\":" + String(PIN_PARO_JEFATURA) + ",";
  json += "\"n_ntc\":" + String(PIN_NTC_IMP) + ",";

  // ENTRADAS DIGITALES - Lectura física + voltaje
  json += "\"pin4_state\":" + String(digitalRead(PIN_SYS_ONOFF) == LOW ? "true" : "false") + ",";
  json += "\"pin4_volt\":" + String(readPinVoltage(PIN_SYS_ONOFF), 2) + ",";
  json += "\"pin4_mb\":" + String(mb.Hreg(MB_REG_PIN_SYS_ONOFF)) + ",";

  // PROG ⇒ LOW = ON (corregido)
  json += "\"pin5_state\":" + String(digitalRead(PIN_SELECTOR_PROG) == LOW ? "true" : "false") + ",";
  json += "\"pin5_volt\":" + String(readPinVoltage(PIN_SELECTOR_PROG), 2) + ",";
  json += "\"pin5_mb\":" + String(mb.Hreg(MB_REG_PIN_PROG_SEL)) + ",";

  json += "\"pin15_state\":" + String(digitalRead(PIN_JEFATURA) == LOW ? "true" : "false") + ",";
  json += "\"pin15_volt\":" + String(readPinVoltage(PIN_JEFATURA), 2) + ",";
  json += "\"pin15_mb\":" + String(mb.Hreg(MB_REG_PIN_JEFATURA)) + ",";

  json += "\"pin6_state\":" + String(digitalRead(PIN_SW_B1) == LOW ? "true" : "false") + ",";
  json += "\"pin6_volt\":" + String(readPinVoltage(PIN_SW_B1), 2) + ",";
  json += "\"pin6_mb\":" + String(mb.Hreg(MB_REG_PIN_SW_B1)) + ",";

  json += "\"pin7_state\":" + String(digitalRead(PIN_SW_B2) == LOW ? "true" : "false") + ",";
  json += "\"pin7_volt\":" + String(readPinVoltage(PIN_SW_B2), 2) + ",";
  json += "\"pin7_mb\":" + String(mb.Hreg(MB_REG_PIN_SW_B2)) + ",";

  json += "\"pin17_state\":" + String(digitalRead(PIN_SW_GT) == LOW ? "true" : "false") + ",";
  json += "\"pin17_volt\":" + String(readPinVoltage(PIN_SW_GT), 2) + ",";
  json += "\"pin17_mb\":" + String(mb.Hreg(MB_REG_PIN_SW_GT)) + ",";

  json += "\"pin18_state\":" + String(digitalRead(PIN_SW_BC) == LOW ? "true" : "false") + ",";
  json += "\"pin18_volt\":" + String(readPinVoltage(PIN_SW_BC), 2) + ",";
  json += "\"pin18_mb\":" + String(mb.Hreg(MB_REG_PIN_SW_BC)) + ",";

  json += "\"pin16_state\":" + String(digitalRead(PIN_TERM_BC) == LOW ? "true" : "false") + ",";
  json += "\"pin16_volt\":" + String(readPinVoltage(PIN_TERM_BC), 2) + ",";
  json += "\"pin16_mb\":" + String(mb.Hreg(MB_REG_PIN_TERM_BC)) + ",";

  json += "\"pin9_state\":" + String(digitalRead(PIN_RT1) == HIGH ? "true" : "false") + ",";
  json += "\"pin9_volt\":" + String(readPinVoltage(PIN_RT1), 2) + ",";
  json += "\"pin9_mb\":" + String(mb.Hreg(MB_REG_PIN_RT1)) + ",";

  json += "\"pin10_state\":" + String(digitalRead(PIN_RT2) == HIGH ? "true" : "false") + ",";
  json += "\"pin10_volt\":" + String(readPinVoltage(PIN_RT2), 2) + ",";
  json += "\"pin10_mb\":" + String(mb.Hreg(MB_REG_PIN_RT2)) + ",";

  json += "\"pin8_state\":" + String(digitalRead(PIN_EMERGENCIA) == HIGH ? "true" : "false") + ",";
  json += "\"pin8_volt\":" + String(readPinVoltage(PIN_EMERGENCIA), 2) + ",";
  json += "\"pin8_mb\":" + String(mb.Hreg(MB_REG_PIN_EMERGENCIA)) + ",";

  json += "\"pin11_state\":" + String(digitalRead(PIN_AL_GT) == HIGH ? "true" : "false") + ",";
  json += "\"pin11_volt\":" + String(readPinVoltage(PIN_AL_GT), 2) + ",";
  json += "\"pin11_mb\":" + String(mb.Hreg(MB_REG_PIN_AL_GT)) + ",";

  // SALIDAS DIGITALES
  json += "\"pin12_state\":" + String(digitalRead(PIN_OUT_B1) == HIGH ? "true" : "false") + ",";
  json += "\"pin12_volt\":" + String(readPinVoltage(PIN_OUT_B1), 2) + ",";
  json += "\"pin12_mb\":" + String(mb.Hreg(MB_REG_BOMBA1_STATE)) + ",";

  json += "\"pin13_state\":" + String(digitalRead(PIN_OUT_B2) == HIGH ? "true" : "false") + ",";
  json += "\"pin13_volt\":" + String(readPinVoltage(PIN_OUT_B2), 2) + ",";
  json += "\"pin13_mb\":" + String(mb.Hreg(MB_REG_BOMBA2_STATE)) + ",";

  json += "\"pin14_state\":" + String(digitalRead(PIN_OUT_GT) == HIGH ? "true" : "false") + ",";
  json += "\"pin14_volt\":" + String(readPinVoltage(PIN_OUT_GT), 2) + ",";
  json += "\"pin14_mb\":" + String(mb.Hreg(MB_REG_GT_STATE)) + ",";

  json += "\"pin21_state\":" + String(digitalRead(PIN_OUT_BC) == HIGH ? "true" : "false") + ",";
  json += "\"pin21_volt\":" + String(readPinVoltage(PIN_OUT_BC), 2) + ",";
  json += "\"pin21_mb\":" + String(mb.Hreg(MB_REG_PIN_OUT_BC)) + ",";

  json += "\"pin47_state\":" + String(digitalRead(PIN_OUT_POST) == HIGH ? "true" : "false") + ",";
  json += "\"pin47_volt\":" + String(readPinVoltage(PIN_OUT_POST), 2) + ",";
  json += "\"pin47_mb\":" + String(mb.Hreg(MB_REG_PIN_OUT_POST)) + ",";

  json += "\"pin38_state\":" + String(digitalRead(PIN_SOBRECALENTAMIENTO) == HIGH ? "true" : "false") + ",";
  json += "\"pin38_volt\":" + String(readPinVoltage(PIN_SOBRECALENTAMIENTO), 2) + ",";
  json += "\"pin38_mb\":" + String(mb.Hreg(MB_REG_PIN_LED_B1_RUN)) + ",";

  json += "\"pin40_state\":" + String(digitalRead(PIN_AVERIA_GT) == HIGH ? "true" : "false") + ",";
  json += "\"pin40_volt\":" + String(readPinVoltage(PIN_AVERIA_GT), 2) + ",";
  json += "\"pin40_mb\":" + String(mb.Hreg(MB_REG_PIN_LED_B2_RUN)) + ",";

  json += "\"pin42_state\":" + String(digitalRead(PIN_PROG_ACTIVA) == HIGH ? "true" : "false") + ",";
  json += "\"pin42_volt\":" + String(readPinVoltage(PIN_PROG_ACTIVA), 2) + ",";
  json += "\"pin42_mb\":" + String(mb.Hreg(MB_REG_PIN_LED_GT_RUN)) + ",";

  json += "\"pin2_state\":" + String(digitalRead(PIN_PARO_JEFATURA) == HIGH ? "true" : "false") + ",";
  json += "\"pin2_volt\":" + String(readPinVoltage(PIN_PARO_JEFATURA), 2) + ",";
  json += "\"pin2_mb\":" + String(mb.Hreg(MB_REG_PIN_PARO_JEF)) + ",";

  json += "\"pin45_state\":" + String(digitalRead(PIN_TEMP_FUERA_RANGO) == HIGH ? "true" : "false") + ",";
  json += "\"pin45_volt\":" + String(readPinVoltage(PIN_TEMP_FUERA_RANGO), 2) + ",";
  json += "\"pin45_mb\":" + String(mb.Hreg(MB_REG_PIN_TEMP_FUERA)) + ",";

  json += "\"pin43_state\":" + String(digitalRead(PIN_INDIC_EMERGENCIA) == HIGH ? "true" : "false") + ",";
  json += "\"pin43_volt\":" + String(readPinVoltage(PIN_INDIC_EMERGENCIA), 2) + ",";
  json += "\"pin43_mb\":" + String(mb.Hreg(MB_REG_PIN_INDIC_EMERG)) + ",";

  json += "\"pin48_state\":" + String(digitalRead(PIN_INDIC_SISTEMA) == HIGH ? "true" : "false") + ",";
  json += "\"pin48_volt\":" + String(readPinVoltage(PIN_INDIC_SISTEMA), 2) + ",";
  json += "\"pin48_mb\":" + String(mb.Hreg(MB_REG_PIN_INDIC_SIST)) + ",";

  json += "\"pin39_state\":" + String(digitalRead(PIN_AVERIA_B1) == HIGH ? "true" : "false") + ",";
  json += "\"pin39_volt\":" + String(readPinVoltage(PIN_AVERIA_B1), 2) + ",";
  json += "\"pin39_mb\":" + String(mb.Hreg(MB_REG_ALARM_RT1)) + ",";

  json += "\"pin41_state\":" + String(digitalRead(PIN_AVERIA_B2) == HIGH ? "true" : "false") + ",";
  json += "\"pin41_volt\":" + String(readPinVoltage(PIN_AVERIA_B2), 2) + ",";
  json += "\"pin41_mb\":" + String(mb.Hreg(MB_REG_ALARM_RT2)) + ",";

  
  // TEMPERATURA
  json += "\"temp\":" + String(temperaturaActual, 1) + ",";
  
  // Estado del rango de temperatura
  float temp_min = cfg_tempMinGT_x10 / 10.0f;
  float temp_max = cfg_tempMaxGT_x10 / 10.0f;
  bool temp_en_rango = (temperaturaActual >= temp_min && temperaturaActual <= temp_max);
  json += "\"temp_rango\":" + String(temp_en_rango ? "true" : "false") + ",";
  
  // TIEMPOS DE FUNCIONAMIENTO BOMBAS
  unsigned long b1_segundos_total = tiempoB1_ms / 1000;
  unsigned long b1_horas = b1_segundos_total / 3600;
  unsigned long b1_mins = (b1_segundos_total % 3600) / 60;
  unsigned long b1_secs = b1_segundos_total % 60;
  
  unsigned long b2_segundos_total = tiempoB2_ms / 1000;
  unsigned long b2_horas = b2_segundos_total / 3600;
  unsigned long b2_mins = (b2_segundos_total % 3600) / 60;
  unsigned long b2_secs = b2_segundos_total % 60;
  
  char b1_tiempo[12], b2_tiempo[12];
  sprintf(b1_tiempo, "%02lu:%02lu:%02lu", b1_horas, b1_mins, b1_secs);
  sprintf(b2_tiempo, "%02lu:%02lu:%02lu", b2_horas, b2_mins, b2_secs);
  
  json += "\"b1_tiempo\":\"" + String(b1_tiempo) + "\",";
  json += "\"b2_tiempo\":\"" + String(b2_tiempo) + "\",";

  
  // --- TOTALES VIDA (HH:MM:SS) ---
  unsigned long b1t_seg = tiempoB1_total_ms / 1000;
  unsigned long b1t_h = b1t_seg / 3600;
  unsigned long b1t_m = (b1t_seg % 3600) / 60;
  unsigned long b1t_s = b1t_seg % 60;

  unsigned long b2t_seg = tiempoB2_total_ms / 1000;
  unsigned long b2t_h = b2t_seg / 3600;
  unsigned long b2t_m = (b2t_seg % 3600) / 60;
  unsigned long b2t_s = b2t_seg % 60;

  char b1_total[12], b2_total[12];
  sprintf(b1_total, "%02lu:%02lu:%02lu", b1t_h, b1t_m, b1t_s);
  sprintf(b2_total, "%02lu:%02lu:%02lu", b2t_h, b2t_m, b2t_s);

  json += "\"b1_total\":\"" + String(b1_total) + "\",";
  json += "\"b2_total\":\"" + String(b2_total) + "\",";

  
  // TIEMPO POST-CIRCULACIÓN RESTANTE
  unsigned long post_horas = tiempoRestantePostCirc_seg / 3600;
  unsigned long post_mins = (tiempoRestantePostCirc_seg % 3600) / 60;
  unsigned long post_secs = tiempoRestantePostCirc_seg % 60;
  
  char post_tiempo[12];
  sprintf(post_tiempo, "%02lu:%02lu:%02lu", post_horas, post_mins, post_secs);
  json += "\"post_tiempo\":\"" + String(post_tiempo) + "\",";
  json += "\"post_activo\":" + String(postCirculacion_ON ? "true" : "false") + ",";
  
  // TIEMPO ALTERNANCIA TRANSCURRIDO (MM:SS) - para UI (cuenta hacia delante)
  unsigned long alt_mins = alternancia_transcurrida_seg / 60;
  unsigned long alt_secs = alternancia_transcurrida_seg % 60;
  char alt_tiempo[6];
  sprintf(alt_tiempo, "%02lu:%02lu", alt_mins, alt_secs);
  json += "\"alt_tiempo\":\"" + String(alt_tiempo) + "\",";

  // Hora actual
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    json += "\"hora\":\"" + String(timeStr) + "\",";
    
    char dateStr[20];
    strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
    json += "\"fecha\":\"" + String(dateStr) + "\",";
  } else {
    json += "\"hora\":\"--:--:--\",";
    json += "\"fecha\":\"--/--/----\",";
  }
  
  // IP STA
  json += "\"ip_sta\":\"" + WiFi.localIP().toString() + "\",";
  
  // CONFIGURACIÓN
  json += "\"cfg_alt\":" + String(cfg_alternanciaHoras) + ",";
  json += "\"cfg_post\":" + String(cfg_postCirculacionMin) + ",";
  json += "\"cfg_tmin\":" + String(cfg_tempMinGT_x10) + ",";
  json += "\"cfg_tmax\":" + String(cfg_tempMaxGT_x10) + ",";
  json += "\"cfg_smode\":" + String(cfg_sensorMode) + ",";
  json += "\"cfg_tfix\":" + String(cfg_tempFijaGT_x10) + ",";
  json += "\"cfg_schen\":" + String(cfg_schedEnable) + ",";
  
  // CONFIGURACIÓN NTP
  json += "\"cfg_ntp_auto24h\":" + String(cfg_ntpAuto24h) + ",";
  json += "\"cfg_ntp_autodst\":" + String(cfg_ntpAutoDST) + ",";
  json += "\"ntp_sync_ok\":" + String(ntpSyncOK ? "true" : "false") + ",";
  
  // Tiempo desde última sincronización NTP
  if (ultimaSincronizacionNTP_ms > 0) {
    unsigned long tiempoDesdeSync_seg = (millis() - ultimaSincronizacionNTP_ms) / 1000;
    unsigned long horas = tiempoDesdeSync_seg / 3600;
    unsigned long mins = (tiempoDesdeSync_seg % 3600) / 60;
    char tiempoSync[32];
    sprintf(tiempoSync, "Hace %lu:%02lu h", horas, mins);
    json += "\"ntp_last_sync\":\"" + String(tiempoSync) + "\"";
  } else {
    json += "\"ntp_last_sync\":\"Nunca\"";
  }
  
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleSetCfg() {
  if (server.hasArg("r") && server.hasArg("v")) {
    int reg = server.arg("r").toInt();
    int val = server.arg("v").toInt();
    mb.Hreg(reg, val);
  }
  server.send(200, "text/plain", "OK");
}


void handleReset() {
  if (server.hasArg("r")) {
    int reg = server.arg("r").toInt();

    // SOLO resetear contadores específicos, NO toda la configuración
    if (reg == MB_REG_RESET_B1_PARCIAL) {
      tiempoB1_ms = 0;
      Serial.println("✓ Reset parcial Bomba 1");
    }
    else if (reg == MB_REG_RESET_B1_TOTAL) {
      tiempoB1_total_ms = 0;
      prefs.begin("caldera", false);
      prefs.putULong("b1_total_ms", 0);
      prefs.end();
      Serial.println("✓ Reset total Bomba 1");
    }
    else if (reg == MB_REG_RESET_B2_PARCIAL) {
      tiempoB2_ms = 0;
      Serial.println("✓ Reset parcial Bomba 2");
    }
    else if (reg == MB_REG_RESET_B2_TOTAL) {
      tiempoB2_total_ms = 0;
      prefs.begin("caldera", false);
      prefs.putULong("b2_total_ms", 0);
      prefs.end();
      Serial.println("✓ Reset total Bomba 2");
    }

    // Resetear el registro Modbus (pone a 0)
    mb.Hreg(reg, 0);
  }
  server.send(200, "text/plain", "OK");
}


void handleSetTime() {
  if (server.hasArg("y") && server.hasArg("m") && server.hasArg("d") &&
      server.hasArg("h") && server.hasArg("min") && server.hasArg("s")) {
    
    struct tm timeinfo;
    timeinfo.tm_year = server.arg("y").toInt() - 1900;
    timeinfo.tm_mon = server.arg("m").toInt() - 1;
    timeinfo.tm_mday = server.arg("d").toInt();
    timeinfo.tm_hour = server.arg("h").toInt();
    timeinfo.tm_min = server.arg("min").toInt();
    timeinfo.tm_sec = server.arg("s").toInt();
    
    time_t t = mktime(&timeinfo);
    struct timeval tv = { .tv_sec = t };
    settimeofday(&tv, NULL);
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

void handleSyncNTP() {
  bool success = sincronizarNTP();
  if (success) {
    server.send(200, "text/plain", "OK");
  } else {
    server.send(500, "text/plain", "Error en sincronización NTP");
  }
}

void handleSaveNTPConfig() {
  if (server.hasArg("auto24h") && server.hasArg("autodst")) {
    cfg_ntpAuto24h = server.arg("auto24h").toInt();
    cfg_ntpAutoDST = server.arg("autodst").toInt();
    
    // Guardar en NVS
    prefs.begin("caldera", false);
    prefs.putUShort("reg_105", cfg_ntpAuto24h);
    prefs.putUShort("reg_106", cfg_ntpAutoDST);
    prefs.end();
    
    // Actualizar Modbus
    mb.Hreg(MB_REG_NTP_AUTO_24H, cfg_ntpAuto24h);
    mb.Hreg(MB_REG_NTP_AUTO_DST, cfg_ntpAutoDST);
    
    // Reconfigurar timezone
    configTime(gmtOffset_sec, cfg_ntpAutoDST ? daylightOffset_sec : 0, ntpServer);
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

void handleScanWiFi() {
  // Verificar si hay un escaneo en curso
  int status = WiFi.scanComplete();
  if (status == WIFI_SCAN_RUNNING) {
    server.send(409, "text/plain", "Escaneo ya en progreso");
    return;
  }
  
  // Limpiar resultados previos si existen
  if (status >= 0) {
    WiFi.scanDelete();
  }
  
  // Iniciar escaneo asíncrono
  int result = WiFi.scanNetworks(true); // true = async, no bloquea el loop
  if (result == WIFI_SCAN_FAILED) {
    server.send(500, "text/plain", "Error al iniciar escaneo");
    return;
  }
  
  server.send(202, "text/plain", "Scan iniciado");
}

void handleScanResults() {
  // Obtener resultados del escaneo asíncrono
  int n = WiFi.scanComplete();
  
  if (n == WIFI_SCAN_RUNNING) {
    server.send(202, "application/json", "{\"status\":\"scanning\"}");
    return;
  }
  
  if (n == WIFI_SCAN_FAILED) {
    server.send(500, "application/json", "{\"status\":\"failed\"}");
    WiFi.scanDelete(); // Limpiar
    return;
  }
  
  // Escaneo completo, devolver resultados
  String json = "{\"status\":\"complete\",\"networks\":[";
  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    json += "\"" + WiFi.SSID(i) + "\"";
  }
  json += "]}";
  
  WiFi.scanDelete(); // Limpiar resultados
  server.send(200, "application/json", json);
}

void handleConnectWiFi() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    
    // Validación básica
    if (ssid.length() == 0) {
        server.send(400, "text/plain", "SSID vacío");
        return;
    }
    
    // Guardar credenciales en NVS
    prefs.begin("caldera", false);
    prefs.putString("wifi_ssid", ssid);
    prefs.putString("wifi_pass", pass);
    prefs.end();
    
    // Desconectar STA si ya estaba conectado
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
    }
    
    // Asegurar modo dual AP+STA
    WiFi.mode(WIFI_AP_STA);
    
    // Iniciar conexión en background (NO bloquea)
    WiFi.begin(ssid.c_str(), pass.c_str());
    sta_connecting = true;
    
    Serial.printf("🔌 Conectando a WiFi: %s\n", ssid.c_str());
    server.send(200, "text/plain", "Conectando a: " + ssid);
  } else {
    server.send(400, "text/plain", "Parámetros incompletos");
  }
}

void handleSaveSchedule() {
  if (server.hasArg("m_on")) cfg_schedMananaON = server.arg("m_on").toInt();
  if (server.hasArg("m_off")) cfg_schedMananaOFF = server.arg("m_off").toInt();
  if (server.hasArg("t_on")) cfg_schedTardeON = server.arg("t_on").toInt();
  if (server.hasArg("t_off")) cfg_schedTardeOFF = server.arg("t_off").toInt();
  if (server.hasArg("dias")) cfg_schedDiasMask = server.arg("dias").toInt();
  
  prefs.begin("caldera", false);
  prefs.putUShort("schedMon", cfg_schedMananaON);
  prefs.putUShort("schedMoff", cfg_schedMananaOFF);
  prefs.putUShort("schedTon", cfg_schedTardeON);
  prefs.putUShort("schedToff", cfg_schedTardeOFF);
  prefs.putUShort("schedDow", cfg_schedDiasMask);
  prefs.end();
  
  // Actualizar registros Modbus
  mb.Hreg(MB_REG_SCHED_M_ON, cfg_schedMananaON);
  mb.Hreg(MB_REG_SCHED_M_OFF, cfg_schedMananaOFF);
  mb.Hreg(MB_REG_SCHED_T_ON, cfg_schedTardeON);
  mb.Hreg(MB_REG_SCHED_T_OFF, cfg_schedTardeOFF);
  mb.Hreg(MB_REG_SCHED_DOW_MASK, cfg_schedDiasMask);
  
  server.send(200, "text/plain", "OK");
}

/* =========================================================================================
   LÓGICA DE CONTROL - INTACTA (MISMA QUE V525)
   ========================================================================================= */

void leerEntradas() {
  // Leer alarmas
  alarmaRT1 = isALARMA(PIN_RT1);
  alarmaRT2 = isALARMA(PIN_RT2);
  alarmaEmergencia = isALARMA(PIN_EMERGENCIA);
  alarmaGT = isALARMA(PIN_AL_GT);
  
  // Leer temperatura NTC real
  temperaturaActual = leerTemperaturaNTC();
}


// Actualiza los estados "anteriores" con los valores leídos en este ciclo
inline void actualizarPreviosEstados() {
  pin21_anterior = pin21_fisico;
  pin27_anterior = pin27_fisico;
  pin2_anterior  = pin2_fisico;
  pin32_anterior = pin32_fisico;
  pin16_anterior = pin16_fisico;
  pin17_anterior = pin17_fisico;
  alarmaGT_anterior = alarmaGT;
}




void ejecutarLogicaControl() {
  
// ======= GUARDAR ESTADO ANTERIOR DE GT (antes de modificarlo) =======
grupoTermico_ON_anterior = grupoTermico_ON;

// =========================================================================
// LÓGICA DE CONTROL REESCRITA SEGÚN TABLA DE LA VERDAD (100%)
// =========================================================================
// REGLAS FUNDAMENTALES:
// 1. POST-CIRCULACIÓN = INDEPENDIENTE (bombas funcionan sin interrupciones)
// 2. JEFATURA controla GT, NO las bombas
// 3. ALARMA GT bloquea GT, NO las bombas
// 4. Al finalizar POST-CIRC: Bombas QUEDAN PARADAS hasta nueva orden
// 5. Bombas solo funcionan si: SISTEMA=0V Y JEFATURA=0V Y sin alarmas RT
// =========================================================================

leerEstadosFisicos();

// Detección de flancos físicos
bool flanco_sistema_on   = (!pin32_anterior && pin32_fisico);   // 3.3V→0V (Sistema ON)
bool flanco_sistema_off  = (pin32_anterior && !pin32_fisico);   // 0V→3.3V (Sistema OFF)
bool flanco_jefatura_on  = (!pin27_anterior && pin27_fisico);   // 3.3V→0V (Jefatura ON)
bool flanco_jefatura_off = (pin27_anterior && !pin27_fisico);   // 0V→3.3V (Jefatura OFF)

// =========================================================================
// PRIORIDAD 1: EMERGENCIA (Línea 11-13 tabla)
// =========================================================================
if (alarmaEmergencia) {
    bomba1_ON = false;
    bomba2_ON = false;
    grupoTermico_ON = false;
    bombaCondensacion_ON = false;
    postCirculacion_ON = false;
    if (post_circulacion_activa) cancelarPostCirculacion();
    actualizarPreviosEstados();
    return;
}

// =========================================================================
// PRIORIDAD 2: POST-CIRCULACIÓN ACTIVA (Líneas 5,7,9,15 tabla)
// =========================================================================
// Durante POST: Bombas en ALTERNANCIA, ignoran todo lo demás
// Al FINALIZAR POST: Bombas QUEDAN PARADAS (3.3V)
// =========================================================================
if (post_circulacion_activa) {
    unsigned long tiempo_transcurrido = millis() - post_circulacion_inicio_ms;
    unsigned long tiempo_total_ms = cfg_postCirculacionMin * 60000UL;

    if (tiempo_transcurrido >= tiempo_total_ms) {
        // POST-CIRCULACIÓN FINALIZADA
        Serial.println("✅ POST-CIRCULACIÓN FINALIZADA → Bombas QUEDAN PARADAS");
        bomba1_ON = false;
        bomba2_ON = false;
        bombaCondensacion_ON = false;
        grupoTermico_ON = false;
        postCirculacion_ON = false;
        cancelarPostCirculacion();
        
        // Las bombas QUEDAN PARADAS hasta que se cumplan condiciones normales
        actualizarPreviosEstados();
        return;
    } else {
        // POST-CIRCULACIÓN ACTIVA - Mantener bombas en ALTERNANCIA
        tiempoRestantePostCirc_seg = (tiempo_total_ms - tiempo_transcurrido) / 1000;
        
        // FORZAR bombas ON sin verificar interruptores
        if (bomba_post_circulacion == 1) {
            bomba1_ON = true;
            bomba2_ON = false;
        } else if (bomba_post_circulacion == 2) {
            bomba1_ON = false;
            bomba2_ON = true;
        } else {
            bomba1_ON = false;
            bomba2_ON = false;
            bombaCondensacion_ON = true;
        }
        
        grupoTermico_ON = false;  // GT siempre OFF durante POST
        postCirculacion_ON = true;
        actualizarPreviosEstados();
        return;
    }
}

// =========================================================================
// PRIORIDAD 3: SISTEMA OFF (Líneas 9-10 tabla)
// =========================================================================
// Línea 9: SIST 0V→3.3V (flanco OFF) → GT OFF, POST inicia, Bombas ALTERNANCIA
// Línea 10: SIST=3.3V estable → GT OFF, POST finalizado, Bombas=3.3V (PARADAS)
// =========================================================================
if (!pin32_fisico) {  // Sistema OFF (PIN4=3.3V lógico invertido: HIGH físico)
    // Si GT estaba ON, iniciar POST-CIRCULACIÓN (Línea 9)
    if (flanco_sistema_off && grupoTermico_ON) {
        Serial.println("⚠️ SISTEMA OFF (flanco) → Iniciando POST-CIRC");
        grupoTermico_ON = false;
        iniciarPostCirculacion();
        actualizarPreviosEstados();
        return;  // Bombas continúan en POST
    }
    
    // SIST OFF estable (Línea 10): Todo parado
    bomba1_ON = false;
    bomba2_ON = false;
    grupoTermico_ON = false;
    bombaCondensacion_ON = false;
    postCirculacion_ON = false;
    actualizarPreviosEstados();
    return;
}

// =========================================================================
// PRIORIDAD 4: JEFATURA OFF (Líneas 7-8 tabla)
// =========================================================================
// Línea 7: JEF 0V→3.3V (flanco OFF) → GT OFF, POST inicia, Bombas ALTERNANCIA  
// Línea 8: JEF=3.3V estable → GT OFF, POST finalizado, Bombas=3.3V (PARADAS)
// IMPORTANTE: JEFATURA controla GT, NO las bombas
// =========================================================================
if (!pin27_fisico) {  // Jefatura OFF (PIN15=3.3V lógico invertido: HIGH físico)
    // Si GT estaba ON, iniciar POST-CIRCULACIÓN (Línea 7)
    if (flanco_jefatura_off && grupoTermico_ON) {
        Serial.println("⚠️ JEFATURA OFF (flanco) → Iniciando POST-CIRC");
        grupoTermico_ON = false;
        iniciarPostCirculacion();
        actualizarPreviosEstados();
        return;  // Bombas continúan en POST
    }
    
    // JEF OFF estable (Línea 8): GT OFF, bombas paradas
    grupoTermico_ON = false;
    bomba1_ON = false;
    bomba2_ON = false;
    bombaCondensacion_ON = false;
    postCirculacion_ON = false;
    actualizarPreviosEstados();
    return;
}

// =========================================================================
// PRIORIDAD 5: ALARMA GT (Líneas 14-15 tabla)
// =========================================================================
// Línea 14: AL_GT=3.3V estable → GT OFF, Bombas=3.3V (PARADAS)
// Línea 15: AL_GT 0V→3.3V (flanco) → GT OFF, POST inicia, Bombas ALTERNANCIA
// IMPORTANTE: ALARMA GT solo bloquea GT, NO controla bombas directamente
// =========================================================================
if (alarmaGT) {
    // Detectar flanco alarma GT (0V→3.3V)
    bool flanco_alarma_gt = (!alarmaGT_anterior && alarmaGT);
    
    // Flanco alarma GT: Iniciar POST si GT estaba ON (Línea 15)
    if (flanco_alarma_gt && grupoTermico_ON) {
        Serial.println("⚠️ ALARMA GT (flanco) → Iniciando POST-CIRC");
        grupoTermico_ON = false;
        iniciarPostCirculacion();
        actualizarPreviosEstados();
        return;  // Bombas continúan en POST
    }
    
    // Alarma GT estable (Línea 14): GT bloqueado, bombas paradas
    grupoTermico_ON = false;
    bomba1_ON = false;
    bomba2_ON = false;
    postCirculacion_ON = false;
    actualizarPreviosEstados();
    return;
}

// =========================================================================
// PRIORIDAD 6: LÓGICA NORMAL (Líneas 1-6 tabla)
// =========================================================================
// Condiciones: SIST=0V, JEF=0V, sin alarmas
// Bombas: ALTERNANCIA según disponibilidad
// GT: ON si bombas funcionan + interruptor + temperatura OK + programación
// =========================================================================

// 6.1: Verificar disponibilidad bombas y gestionar averías RT
if (alarmaRT1 && alarmaRT2) {
    // Ambas bombas averiadas: Usar bomba condensación si hay demanda
    bomba1_ON = false;
    bomba2_ON = false;
    grupoTermico_ON = false;
} else if (alarmaRT1 && !alarmaRT2) {
    // Solo bomba 1 averiada: Usar bomba 2
    if (bomba1_ON) {  // Si bomba 1 estaba activa, cambiar a bomba 2
        Serial.println("⚠️ AVERÍA RT1 → Cambiando a Bomba 2");
        bomba1_ON = false;
        bomba2_ON = true;
        turno_bomba1 = false;
        alternancia_inicio_ms = millis();
    } else {
        bomba1_ON = false;
        bomba2_ON = true;
    }
} else if (!alarmaRT1 && alarmaRT2) {
    // Solo bomba 2 averiada: Usar bomba 1
    if (bomba2_ON) {  // Si bomba 2 estaba activa, cambiar a bomba 1
        Serial.println("⚠️ AVERÍA RT2 → Cambiando a Bomba 1");
        bomba1_ON = true;
        bomba2_ON = false;
        turno_bomba1 = true;
        alternancia_inicio_ms = millis();
    } else {
        bomba1_ON = true;
        bomba2_ON = false;
    }
} else {
    // Ambas bombas disponibles: ALTERNANCIA normal
    alternancia_suspendida = false;
    uint8_t bomba_activa = determinarBombaActiva();
    if (bomba_activa == 1) {
        bomba1_ON = true;
        bomba2_ON = false;
    } else if (bomba_activa == 2) {
        bomba1_ON = false;
        bomba2_ON = true;
    } else {
        bomba1_ON = false;
        bomba2_ON = false;
    }
}

// 6.2: Control GT (depende de bombas + temperatura + programación)
bool alguna_bomba_ON = (bomba1_ON || bomba2_ON);
bool interruptor_gt_ON = isON(PIN_SW_GT);
float temp_min = cfg_tempMinGT_x10 / 10.0f;
float temp_max = cfg_tempMaxGT_x10 / 10.0f;
bool temp_en_rango = (temperaturaActual >= temp_min && temperaturaActual <= temp_max);

// Programación horaria
bool permiso_horario = true;
if (cfg_schedEnable == 1 && isON(PIN_SELECTOR_PROG)) {
    permiso_horario = false;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        int minutos_actuales = timeinfo.tm_hour * 60 + timeinfo.tm_min;
        int dia_semana = timeinfo.tm_wday;
        bool dia_habilitado = (cfg_schedDiasMask & (1 << dia_semana)) != 0;
        
        if (dia_habilitado) {
            bool en_franja_manana = (minutos_actuales >= cfg_schedMananaON && minutos_actuales < cfg_schedMananaOFF);
            bool en_franja_tarde = (minutos_actuales >= cfg_schedTardeON && minutos_actuales < cfg_schedTardeOFF);
            permiso_horario = (en_franja_manana || en_franja_tarde);
        }
    }
}

if (alguna_bomba_ON && interruptor_gt_ON && temp_en_rango && permiso_horario) {
    grupoTermico_ON = true;
} else {
    grupoTermico_ON = false;
}

// 6.3: Bomba condensación (independiente)
bool interruptor_bc_ON = isON(PIN_SW_BC);
bool termostato_bc_ON = isON(PIN_TERM_BC);
bombaCondensacion_ON = (interruptor_bc_ON && termostato_bc_ON);

// 6.4: Actualizar contadores alternancia
{
    unsigned long ahora = millis();
    unsigned long periodo_ms = cfg_alternanciaHoras * 60000UL;

    if (periodo_ms > 0 && ahora >= alternancia_inicio_ms) {
        unsigned long transcurrido = ahora - alternancia_inicio_ms;
        alternancia_transcurrida_seg = (transcurrido < periodo_ms) ? (transcurrido / 1000) : (periodo_ms / 1000);
        alternancia_restante_seg = (transcurrido < periodo_ms) ? ((periodo_ms - transcurrido) / 1000) : 0;
    } else {
        alternancia_transcurrida_seg = 0;
        alternancia_restante_seg = 0;
    }
}

postCirculacion_ON = false;
actualizarPreviosEstados();

}


void actualizarSalidas() {
  // LÓGICA INVERTIDA para placa LOW LEVEL TRIGGER:
  // LOW (0V) = relé ON, HIGH (3.3V) = relé OFF
  
  // Salidas principales (invertidas)
  setOutput(PIN_OUT_B1, !bomba1_ON);
  setOutput(PIN_OUT_B2, !bomba2_ON);
  setOutput(PIN_OUT_GT, !grupoTermico_ON);
  setOutput(PIN_OUT_BC, !bombaCondensacion_ON);
  setOutput(PIN_OUT_POST, !postCirculacion_ON);
  
  // ===== NUEVOS INDICADORES (V526 Hardware 8 canales) =====
  // NOTA: setOutput(pin, true) = 3.3V, setOutput(pin, false) = 0V
  // Para activar indicador (0V) cuando condición=true, usar setOutput(pin, !condicion)
  
  // PIN 38 - SOBRECALENTAMIENTO: REPOSO=3.3V (EN RANGO), ACTIVADO=0V (SOBRECALENTAMIENTO)
  bool sobrecalentamiento = (temperaturaActual > (cfg_tempMaxGT_x10 / 10.0f));
  setOutput(PIN_SOBRECALENTAMIENTO, !sobrecalentamiento);  // 0V cuando hay sobrecalentamiento
  
  // PIN 40 - AVERIA_GT: REPOSO=3.3V (NO AVERIA), ACTIVADO=0V (AVERIA GT)
  setOutput(PIN_AVERIA_GT, !alarmaEmergencia);  // 0V cuando hay emergencia
  
  // PIN 42 - PROG_ACTIVA: REPOSO=3.3V (NO_PROG), ACTIVADO=0V (PROG_ACTIVA)
  bool prog_activa = isON(PIN_SELECTOR_PROG);  // true cuando selector PROG está en ON
  setOutput(PIN_PROG_ACTIVA, !prog_activa);  // 0V cuando prog activa
  
  // PIN 2 - JEFATURA (salida indicadora): SEGÚN TABLA CSV - OPCIÓN A
  // Reposo: PIN2=3.3V → "OFF" (GRIS), registro 40232=0
  // Activo: PIN2=0V → "MARCHA JEFAT" (AZUL), registro 40232=1
  // Lógica: PIN2 indica si el sistema PUEDE FUNCIONAR (requiere JEFATURA Y SISTEMA ON)
  //   - PIN15=0V Y PIN4=0V → PIN2=0V (MARCHA JEFAT/AZUL) → Registro=1
  //   - PIN15=3.3V O PIN4=3.3V → PIN2=3.3V (OFF/GRIS) → Registro=0
  // IMPORTANTE: Flanco PIN2 (0V→3.3V) se detecta en logicaCaldera() para apagar GT
  bool jefatura_on = isON(PIN_JEFATURA);  // true cuando PIN15=0V
  bool sistema_on = isON(PIN_SYS_ONOFF);  // true cuando PIN4=0V
  bool sistema_puede_funcionar = jefatura_on && sistema_on;  // Ambos ON
  setOutput(PIN_PARO_JEFATURA, !sistema_puede_funcionar);  // 0V cuando ambos ON, 3.3V cuando alguno OFF
  
  // PIN 45 - TEMP_FUERA_RANGO: REPOSO=3.3V (EN RANGO), ACTIVADO=0V (FUERA RANGO)
  float temp_min = cfg_tempMinGT_x10 / 10.0f;
  float temp_max = cfg_tempMaxGT_x10 / 10.0f;
  bool temp_fuera = (temperaturaActual < temp_min) || (temperaturaActual > temp_max);
  setOutput(PIN_TEMP_FUERA_RANGO, !temp_fuera);  // 0V cuando fuera de rango
  
  // PIN 43 - EMERGENCIA (indicador salida): REPOSO=3.3V (NO_EMERGENCIA), ACTIVADO=0V (EMERGENCIA)
  setOutput(PIN_INDIC_EMERGENCIA, !alarmaEmergencia);  // 0V cuando hay emergencia
  
  // PIN 48 - SISTEMA (indicador salida): REPOSO=3.3V (SISTEMA_OFF), ACTIVADO=0V (SISTEMA_ON)
  bool sistema_on_actual = isON(PIN_SYS_ONOFF);  // true cuando PIN4=0V
  setOutput(PIN_INDIC_SISTEMA, !sistema_on_actual);  // 0V cuando sistema ON
  
  // PIN 39 - AVERIA_B1: REPOSO=3.3V (NO_AVERIA_B1), ACTIVADO=0V (AVERIA B1)
  setOutput(PIN_AVERIA_B1, !alarmaRT1);  // 0V cuando hay avería
  
  // PIN 41 - AVERIA_B2: REPOSO=3.3V (NO_AVERIA_B2), ACTIVADO=0V (AVERIA B2)
  setOutput(PIN_AVERIA_B2, !alarmaRT2);  // 0V cuando hay avería

  // ====== Reset de parciales en primer arranque real (flanco ON) ======
  static bool b1_prev_on = false;
  static bool b2_prev_on = false;
  static bool parcial_reset_pendiente = false;
  static uint8_t bomba_objetivo_alternancia = 0;

  // Nota: marcamos el objetivo cuando alterna la lógica (hazlo en determinarBombaActiva si quieres afinar).
  // Si no quieres tocar más, dejamos que el reset se ejecute cuando detecte arranque de la bomba que esté ON.

  bool flanco_on_b1 = (!b1_prev_on && bomba1_ON);
  bool flanco_on_b2 = (!b2_prev_on && bomba2_ON);

  // Reset diferido: si alternancia decidió cambiar, resetea el parcial de la que entra en su primer ON.
  // Si no tienes aún el marcado de 'parcial_reset_pendiente', puedes resetear cuando detectes que cambió de bomba:
  // (Versión mínima robusta sin tocar más funciones)
  static uint8_t ultima_bomba_on = 0;
  uint8_t bomba_actual_on = bomba1_ON ? 1 : (bomba2_ON ? 2 : 0);

  // Detectar cambio de bomba (de 1->2 o 2->1) y armar reset pendiente
  if ((ultima_bomba_on != 0) && (bomba_actual_on != 0) && (bomba_actual_on != ultima_bomba_on)) {
    parcial_reset_pendiente = true;
    bomba_objetivo_alternancia = bomba_actual_on; // la que entra
  }

  // Ejecutar reset cuando realmente arranca
  if (parcial_reset_pendiente) {
    if (flanco_on_b1 && bomba_objetivo_alternancia == 1) {
      tiempoB1_ms = 0;           // Reset PARCIAL B1
      parcial_reset_pendiente = false;
    }
    if (flanco_on_b2 && bomba_objetivo_alternancia == 2) {
      tiempoB2_ms = 0;           // Reset PARCIAL B2
      parcial_reset_pendiente = false;
    }
  }

  b1_prev_on = bomba1_ON;
  b2_prev_on = bomba2_ON;
  ultima_bomba_on = bomba_actual_on;

  // --- Contadores basados en ESTADO LÓGICO (no leer el pin físico) ---
  unsigned long ahora = millis();
  unsigned long deltaTiempo = ahora - ultimoUpdateContadores;

  if (bomba1_ON) {
    tiempoB1_ms       += deltaTiempo;    // PARCIAL B1
    tiempoB1_total_ms += deltaTiempo;    // TOTAL B1
  }

  if (bomba2_ON) {
    tiempoB2_ms       += deltaTiempo;    // PARCIAL B2
    tiempoB2_total_ms += deltaTiempo;    // TOTAL B2
  }

  ultimoUpdateContadores = ahora;

  // --- Persistir los TOTALES cada ~60 s (para no machacar la flash) ---
  static unsigned long lastPersist = 0;
  if (ahora - lastPersist > 60000UL) {
    prefs.begin("caldera", false);
    prefs.putULong("b1_total_ms", tiempoB1_total_ms);
    prefs.putULong("b2_total_ms", tiempoB2_total_ms);
    prefs.end();
    lastPersist = ahora;
  }
}


void actualizarModbus() {
  // Actualizar registros Modbus con configuración actual
  mb.Hreg(MB_REG_ALT_HOURS, cfg_alternanciaHoras);
  mb.Hreg(MB_REG_PUMP_STOP_DELAY, cfg_postCirculacionMin);
  mb.Hreg(MB_REG_GT_TMIN, cfg_tempMinGT_x10);
  mb.Hreg(MB_REG_GT_TMAX, cfg_tempMaxGT_x10);
  mb.Hreg(MB_REG_GT_SENSOR_MODE, cfg_sensorMode);
  mb.Hreg(MB_REG_GT_TFIXED, cfg_tempFijaGT_x10);
  mb.Hreg(MB_REG_SCHED_ENABLE, cfg_schedEnable);
  mb.Hreg(MB_REG_SCHED_M_ON, cfg_schedMananaON);
  mb.Hreg(MB_REG_SCHED_M_OFF, cfg_schedMananaOFF);
  mb.Hreg(MB_REG_SCHED_T_ON, cfg_schedTardeON);
  mb.Hreg(MB_REG_SCHED_T_OFF, cfg_schedTardeOFF);
  mb.Hreg(MB_REG_SCHED_DOW_MASK, cfg_schedDiasMask);
  
  // Estados (registros de solo lectura)
  mb.Hreg(MB_REG_BOMBA1_STATE, bomba1_ON ? 1 : 0);
  mb.Hreg(MB_REG_BOMBA2_STATE, bomba2_ON ? 1 : 0);
  mb.Hreg(MB_REG_GT_STATE, grupoTermico_ON ? 1 : 0);
  mb.Hreg(MB_REG_ALARM_RT1, alarmaRT1 ? 1 : 0);
  mb.Hreg(MB_REG_ALARM_RT2, alarmaRT2 ? 1 : 0);
  mb.Hreg(MB_REG_ALARM_EMERG, alarmaEmergencia ? 1 : 0);
  mb.Hreg(MB_REG_ALARM_GT, alarmaGT ? 1 : 0);
  mb.Hreg(MB_REG_TEMP_X10, (int16_t)(temperaturaActual * 10));
  
  // Estados de todos los PINs
  mb.Hreg(MB_REG_PIN_SYS_ONOFF, isON(PIN_SYS_ONOFF) ? 1 : 0);
  mb.Hreg(MB_REG_PIN_PROG_SEL, isON(PIN_SELECTOR_PROG) ? 1 : 0);
  mb.Hreg(MB_REG_PIN_SW_B1, isON(PIN_SW_B1) ? 1 : 0);
  mb.Hreg(MB_REG_PIN_SW_B2, isON(PIN_SW_B2) ? 1 : 0);
  mb.Hreg(MB_REG_PIN_JEFATURA, isON(PIN_JEFATURA) ? 1 : 0);
  mb.Hreg(MB_REG_PIN_TERM_BC, isON(PIN_TERM_BC) ? 1 : 0);
  mb.Hreg(MB_REG_PIN_SW_GT, isON(PIN_SW_GT) ? 1 : 0);
  mb.Hreg(MB_REG_PIN_SW_BC, isON(PIN_SW_BC) ? 1 : 0);
  mb.Hreg(MB_REG_PIN_EMERGENCIA, alarmaEmergencia ? 1 : 0);
  mb.Hreg(MB_REG_PIN_RT1, alarmaRT1 ? 1 : 0);
  mb.Hreg(MB_REG_PIN_RT2, alarmaRT2 ? 1 : 0);
  mb.Hreg(MB_REG_PIN_AL_GT, alarmaGT ? 1 : 0);
  mb.Hreg(MB_REG_PIN_OUT_BC, bombaCondensacion_ON ? 1 : 0);
  mb.Hreg(MB_REG_PIN_OUT_POST, postCirculacion_ON ? 1 : 0);
  
  // Contadores de tiempo - Bomba 1 parcial (32 bits)
  unsigned long b1_parcial_seg = tiempoB1_ms / 1000;
  mb.Hreg(MB_REG_B1_TIEMPO_PARCIAL_L, (uint16_t)(b1_parcial_seg & 0xFFFF));
  mb.Hreg(MB_REG_B1_TIEMPO_PARCIAL_H, (uint16_t)(b1_parcial_seg >> 16));
  
  // Contadores de tiempo - Bomba 2 parcial (32 bits)
  unsigned long b2_parcial_seg = tiempoB2_ms / 1000;
  mb.Hreg(MB_REG_B2_TIEMPO_PARCIAL_L, (uint16_t)(b2_parcial_seg & 0xFFFF));
  mb.Hreg(MB_REG_B2_TIEMPO_PARCIAL_H, (uint16_t)(b2_parcial_seg >> 16));
  
  // Contadores de tiempo - Bomba 1 total (32 bits)
  unsigned long b1_total_seg = tiempoB1_total_ms / 1000;
  mb.Hreg(MB_REG_B1_TIEMPO_TOTAL_L, (uint16_t)(b1_total_seg & 0xFFFF));
  mb.Hreg(MB_REG_B1_TIEMPO_TOTAL_H, (uint16_t)(b1_total_seg >> 16));
  
  // Contadores de tiempo - Bomba 2 total (32 bits)
  unsigned long b2_total_seg = tiempoB2_total_ms / 1000;
  mb.Hreg(MB_REG_B2_TIEMPO_TOTAL_L, (uint16_t)(b2_total_seg & 0xFFFF));
  mb.Hreg(MB_REG_B2_TIEMPO_TOTAL_H, (uint16_t)(b2_total_seg >> 16));
  
  // Alternancia
  mb.Hreg(MB_REG_ALT_TRANSCURRIDA, (uint16_t)alternancia_transcurrida_seg);
  mb.Hreg(MB_REG_ALT_RESTANTE, (uint16_t)alternancia_restante_seg);
  
  // Post-circulación
  mb.Hreg(MB_REG_POST_ACTIVA, post_circulacion_activa ? 1 : 0);
  mb.Hreg(MB_REG_POST_RESTANTE, (uint16_t)tiempoRestantePostCirc_seg);
  
  // Actualizar registros Modbus de indicadores según ESTADO FÍSICO DE LOS PINES
  // IMPORTANTE: Los registros reflejan si el PIN está ACTIVO (0V), no el estado lógico
  // Por ejemplo: PIN2=0V (MARCHA JEFAT) → Registro=1, PIN2=3.3V (OFF) → Registro=0
  
  bool sobrecalentamiento = (temperaturaActual > (cfg_tempMaxGT_x10 / 10.0f));
  bool prog_activa = isON(PIN_SELECTOR_PROG);
  bool temp_fuera = (temperaturaActual < (cfg_tempMinGT_x10 / 10.0f)) || (temperaturaActual > (cfg_tempMaxGT_x10 / 10.0f));
  bool sistema_on_actual = isON(PIN_SYS_ONOFF);  // Leer estado de PIN4
  
  // PIN2: Leer estado físico del pin para el registro Modbus
  // PIN2=0V (MARCHA JEFAT/AZUL) → digitalRead=LOW → Registro=1
  // PIN2=3.3V (OFF/GRIS) → digitalRead=HIGH → Registro=0
  // PIN2 refleja directamente PIN15: PIN15=0V → PIN2=0V → Reg=1
  bool pin2_activo = (digitalRead(PIN_PARO_JEFATURA) == LOW);
  
  mb.Hreg(MB_REG_PIN_LED_B1_RUN, sobrecalentamiento ? 1 : 0);  // PIN38 - Sobrecalentamiento
  mb.Hreg(MB_REG_PIN_LED_B2_RUN, alarmaEmergencia ? 1 : 0);    // PIN40 - Avería GT (NOTA: tabla dice que usa alarmaEmergencia)
  mb.Hreg(MB_REG_PIN_LED_GT_RUN, prog_activa ? 1 : 0);         // PIN42 - Programación activa
  mb.Hreg(MB_REG_PIN_PARO_JEF, pin2_activo ? 1 : 0);           // PIN2 - JEFATURA (1=MARCHA JEFAT/0V, 0=OFF/3.3V)
  mb.Hreg(MB_REG_PIN_TEMP_FUERA, temp_fuera ? 1 : 0);          // PIN45 - Temp fuera rango
  mb.Hreg(MB_REG_PIN_INDIC_EMERG, alarmaEmergencia ? 1 : 0);   // PIN43 - Indicador Emergencia
  mb.Hreg(MB_REG_PIN_INDIC_SIST, sistema_on_actual ? 1 : 0);   // PIN48 - Indicador Sistema
}


// ===== ARRANQUE AP ROBUSTO =====

void startAP() {
  if (ap_started || ap_forced_off) return;

  bool ok = WiFi.softAP("Caldera_ESP32", "caldera2026", 6, 0, 4);
  if (ok) {
    WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
    ap_started = true;
    Serial.print("✓ WiFi AP activo - IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("✗ ERROR iniciando WiFi AP");
  }
}

/* =========================================================================================
   DEBUG PERSISTENCIA MODBUS
   ========================================================================================= */

void debugModbusPersistency() {
  Serial.println("\n=== DEBUG PERSISTENCIA MODBUS ===");
  
  prefs.begin("caldera", true);
  
  Serial.printf("40001 (Alternancia): Modbus=%d, Variable=%d, NVS=%d\n",
    mb.Hreg(MB_REG_ALT_HOURS),
    cfg_alternanciaHoras,
    prefs.getUShort("reg_0", 9999));
  
  Serial.printf("40002 (Post-circ): Modbus=%d, Variable=%d, NVS=%d\n",
    mb.Hreg(MB_REG_PUMP_STOP_DELAY),
    cfg_postCirculacionMin,
    prefs.getUShort("reg_1", 9999));
  
  Serial.printf("40100 (Sched Mañana ON): Modbus=%d, Variable=%d, NVS=%d\n",
    mb.Hreg(MB_REG_SCHED_M_ON),
    cfg_schedMananaON,
    prefs.getUShort("reg_100", 9999));
  
  Serial.printf("40104 (Días programación): Modbus=%d, Variable=%d, NVS=%d\n",
    mb.Hreg(MB_REG_SCHED_DOW_MASK),
    cfg_schedDiasMask,
    prefs.getUShort("reg_104", 9999));
  
  prefs.end();
  
  Serial.println("=================================\n");
}

/* =========================================================================================
   SETUP - COMPLETAMENTE REVISADO PARA PERSISTENCIA TOTAL
   ========================================================================================= */

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n===========================================");
  Serial.println("CONTROLADOR CALDERA ESP32 V546");
  Serial.println("BUILD: 2026-02-02 - CAMBIO AUTO BOMBA AVERIA");
  Serial.println("===========================================\n");
  
  // ==========================================================
  // SOLUCIÓN PROBLEMA RELÉS AL INICIO
  // ==========================================================
  
  // 1. CONFIGURAR TODOS LOS PINES DE SALIDA PRIMERO
  pinMode(PIN_OUT_B1, OUTPUT);
  pinMode(PIN_OUT_B2, OUTPUT);
  pinMode(PIN_OUT_GT, OUTPUT);
  pinMode(PIN_OUT_BC, OUTPUT);
  pinMode(PIN_OUT_POST, OUTPUT);
  pinMode(PIN_SOBRECALENTAMIENTO, OUTPUT);
  pinMode(PIN_AVERIA_B1, OUTPUT);
  pinMode(PIN_AVERIA_GT, OUTPUT);
  pinMode(PIN_AVERIA_B2, OUTPUT);
  pinMode(PIN_PROG_ACTIVA, OUTPUT);
  pinMode(PIN_PARO_JEFATURA, OUTPUT);
  pinMode(PIN_TEMP_FUERA_RANGO, OUTPUT);
  pinMode(PIN_INDIC_EMERGENCIA, OUTPUT);
  pinMode(PIN_INDIC_SISTEMA, OUTPUT);
  
  // 2. ESCRIBIR HIGH (3.3V = OFF) INMEDIATAMENTE
  digitalWrite(PIN_OUT_B1, HIGH);
  digitalWrite(PIN_OUT_B2, HIGH);
  digitalWrite(PIN_OUT_GT, HIGH);
  digitalWrite(PIN_OUT_BC, HIGH);
  digitalWrite(PIN_OUT_POST, HIGH);
  digitalWrite(PIN_SOBRECALENTAMIENTO, HIGH);
  digitalWrite(PIN_AVERIA_B1, HIGH);
  digitalWrite(PIN_AVERIA_GT, HIGH);
  digitalWrite(PIN_AVERIA_B2, HIGH);
  digitalWrite(PIN_PROG_ACTIVA, HIGH);
  digitalWrite(PIN_PARO_JEFATURA, HIGH);
  digitalWrite(PIN_TEMP_FUERA_RANGO, HIGH);
  digitalWrite(PIN_INDIC_EMERGENCIA, HIGH);  // 3.3V = NO_EMERGENCIA
  digitalWrite(PIN_INDIC_SISTEMA, HIGH);     // 3.3V = SISTEMA_OFF
  
  delay(100); // Pequeña pausa para estabilizar
  
  // 3. AHORA CONFIGURAR ENTRADAS
  pinMode(PIN_SYS_ONOFF, INPUT_PULLUP);
  pinMode(PIN_SELECTOR_PROG, INPUT_PULLUP);
  pinMode(PIN_SW_B1, INPUT_PULLUP);
  pinMode(PIN_SW_B2, INPUT_PULLUP);
  pinMode(PIN_JEFATURA, INPUT_PULLUP);
  pinMode(PIN_TERM_BC, INPUT_PULLUP);
  pinMode(PIN_SW_GT, INPUT_PULLUP);
  pinMode(PIN_SW_BC, INPUT_PULLUP);
  pinMode(PIN_EMERGENCIA, INPUT_PULLUP);
  pinMode(PIN_RT1, INPUT_PULLUP);
  pinMode(PIN_RT2, INPUT_PULLUP);
  pinMode(PIN_AL_GT, INPUT_PULLUP);
  pinMode(PIN_NTC_IMP, INPUT);
  
  Serial.println("✓ Pines inicializados (sin glitch en relés)");

  
  // ==========================================================
  // CONFIGURACIÓN ADC ESP32‑S3 (NTC en GPIO1)
  // ==========================================================
  analogReadResolution(12);                       // 0..4095
  analogSetPinAttenuation(PIN_NTC_IMP, ADC_11db); // ~0–3.3V

  
  // ==========================================================
  // CARGAR TODA LA CONFIGURACIÓN DESDE NVS (NUEVO MÉTODO)
  // ==========================================================
  Serial.println("\nCargando configuración desde NVS...");
  loadAllSettingsFromNVS();
  
// ==========================================================
// WIFI AP+STA - CONFIGURACIÓN ROBUSTA Y DEFINITIVA
// ==========================================================
Serial.println("\n=== CONFIGURACIÓN WIFI ===");

// 1. Configurar ESP32 en modo DUAL (AP + STA)
WiFi.mode(WIFI_AP_STA);
WiFi.setAutoReconnect(false);  // Manejamos reconexión manualmente para más control
WiFi.persistent(false);         // No guardar en flash automáticamente

// 2. Configurar Punto de Acceso (SIEMPRE ACTIVO)
IPAddress apIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

Serial.println("  Iniciando AP WiFi...");
bool apStarted = WiFi.softAP("Caldera_ESP32S3", "caldera2026", 1, 0, 4);

if (apStarted) {
    WiFi.softAPConfig(apIP, gateway, subnet);
    ap_started = true;
    Serial.println("  ✓ AP WiFi ACTIVO");
    Serial.println("    SSID: Caldera_ESP32S3");
    Serial.println("    Contraseña: caldera2026");
    Serial.print("    IP: ");
    Serial.println(WiFi.softAPIP());
} else {
    ap_started = false;
    Serial.println("  ✗ FALLO AL INICIAR AP");
}

// 3. Intentar conexión a STA si hay credenciales guardadas
prefs.begin("caldera", true);
String ssid_saved = prefs.getString("wifi_ssid", "");
String pass_saved = prefs.getString("wifi_pass", "");
prefs.end();

if (ssid_saved.length() > 0) {
    Serial.println("\n  Conectando a STA WiFi...");
    Serial.print("    SSID: ");
    Serial.println(ssid_saved);
    
    WiFi.begin(ssid_saved.c_str(), pass_saved.c_str());
    
    // Esperar hasta 15 segundos a conexión (SIN BLOQUEAR COMPLETAMENTE)
    unsigned long startTime = millis();
    int intentos = 0;
    
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 15000)) {
        delay(500);
        Serial.print(".");
        intentos++;
        
        // Si lleva muchos intentos sin conectar, abandonar antes
        if (intentos > 20) break;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n  ✓ STA WiFi CONECTADO");
        Serial.print("    IP STA: ");
        Serial.println(WiFi.localIP());
        Serial.print("    RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        sta_connected_since_ms = millis();
        sta_connecting = false;
    } else {
        Serial.println("\n  ⚠️  STA WiFi no conectado (usando solo AP)");
        Serial.println("     Reintentos automáticos activos");
        WiFi.disconnect();
        sta_connecting = false;
    }
} else {
    Serial.println("  ℹ️  Sin credenciales STA guardadas");
    Serial.println("     Solo AP disponible");
}


// 4. Configurar NTP si WiFi STA está conectado
if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConfigurando NTP...");
    configTime(gmtOffset_sec, cfg_ntpAutoDST ? daylightOffset_sec : 0, ntpServer);
    sincronizarNTP();
}

  // ==========================================================
  // MODBUS TCP - CON PERSISTENCIA COMPLETA
  // ==========================================================
  mb.server();
  
  // Añadir registros Modbus con valores cargados desde NVS
  mb.addHreg(MB_REG_ALT_HOURS, cfg_alternanciaHoras);
  mb.addHreg(MB_REG_PUMP_STOP_DELAY, cfg_postCirculacionMin);
  mb.addHreg(MB_REG_GT_TMIN, cfg_tempMinGT_x10);
  mb.addHreg(MB_REG_GT_TMAX, cfg_tempMaxGT_x10);
  mb.addHreg(MB_REG_GT_SENSOR_MODE, cfg_sensorMode);
  mb.addHreg(MB_REG_GT_TFIXED, cfg_tempFijaGT_x10);
  mb.addHreg(MB_REG_SCHED_ENABLE, cfg_schedEnable);
  mb.addHreg(MB_REG_SCHED_M_ON, cfg_schedMananaON);
  mb.addHreg(MB_REG_SCHED_M_OFF, cfg_schedMananaOFF);
  mb.addHreg(MB_REG_SCHED_T_ON, cfg_schedTardeON);
  mb.addHreg(MB_REG_SCHED_T_OFF, cfg_schedTardeOFF);
  mb.addHreg(MB_REG_SCHED_DOW_MASK, cfg_schedDiasMask);
  
  // Registros NTP (40105-40107)
  mb.addHreg(MB_REG_NTP_AUTO_24H, cfg_ntpAuto24h);
  mb.addHreg(MB_REG_NTP_AUTO_DST, cfg_ntpAutoDST);
  mb.addHreg(MB_REG_NTP_SYNC_NOW, 0);
  
  // Registros de solo lectura
  for (int i = 200; i <= 230; i++) {
    mb.addHreg(i, 0);
  }
  
  // Configurar TODOS los callbacks para persistencia
  setupModbusCallbacks();
  
  Serial.println("✓ Servidor Modbus TCP iniciado (puerto 502)");
  Serial.println("  Persistencia completa activada para todos los registros");
  Serial.println();
  
  // ==========================================================
  // WEB SERVER
  // ==========================================================
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/setcfg", handleSetCfg);
  server.on("/reset", handleReset);
  server.on("/settime", handleSetTime);
  server.on("/syncntp", handleSyncNTP);
  server.on("/saventpconfig", handleSaveNTPConfig);
  server.on("/scanwifi", handleScanWiFi);
  server.on("/scanresults", handleScanResults);  // Obtener resultados de escaneo asíncrono
  server.on("/connectwifi", handleConnectWiFi);
  server.on("/savesched", handleSaveSchedule);
  
  server.begin();
  Serial.println("✓ Servidor Web iniciado");
  Serial.println("  URL: http://" + WiFi.softAPIP().toString());
  Serial.println();
  
  // ==========================================================
  // INICIALIZAR DETECCIÓN DE FLANCOS Y ALTERNANCIA
  // ==========================================================
  leerEstadosFisicos();
  pin21_anterior = pin21_fisico;
  pin27_anterior = pin27_fisico;
  pin2_anterior  = pin2_fisico;
  pin32_anterior = pin32_fisico;
  pin16_anterior = pin16_fisico;
  pin17_anterior = pin17_fisico;
  
  alternancia_inicio_ms = millis();
  turno_bomba1 = true;
  
  ultimoUpdateContadores = millis();
  
  // ==========================================================
  // MOSTRAR DEBUG INICIAL
  // ==========================================================
  debugModbusPersistency();
  
  Serial.println("✅ SISTEMA INICIALIZADO CORRECTAMENTE");
  Serial.println("   PERSISTENCIA MODBUS COMPLETA ACTIVADA");
  Serial.println("===========================================\n");
}


/* =========================================================================================
   LOOP PRINCIPAL - INTACTO
   ========================================================================================= */

void loop() {
  mb.task();
  server.handleClient();
  
    // GESTIÓN DE WIFI
    gestionWiFi();

  static unsigned long ultimoCiclo = 0;
  if (millis() - ultimoCiclo >= 1000) {  // 1 segundo (suficiente para control de calefacción)
    ultimoCiclo = millis();
    
    leerEntradas();
    ejecutarLogicaControl();
    actualizarSalidas();
    actualizarModbus();
  }
  
  static unsigned long ultimoDebug = 0;
  if (millis() - ultimoDebug >= 30000) {  // Debug cada 30 segundos
    ultimoDebug = millis();
    Serial.printf("[%lu] B1=%d B2=%d GT=%d BC=%d POST=%d | T=%.1f°C | Alarmas: E=%d RT1=%d RT2=%d GT=%d\n",
      millis()/1000, bomba1_ON, bomba2_ON, grupoTermico_ON, bombaCondensacion_ON, postCirculacion_ON,
      temperaturaActual, alarmaEmergencia, alarmaRT1, alarmaRT2, alarmaGT);
  }
  
  // Sincronización automática NTP cada 24 horas
  static unsigned long ultimaSincronizacion24h = 0;
  if (cfg_ntpAuto24h && WiFi.status() == WL_CONNECTED) {
    unsigned long tiempoTranscurrido = millis() - ultimaSincronizacion24h;
    // 24 horas = 86400000 ms
    if (tiempoTranscurrido >= 86400000UL || ultimaSincronizacion24h == 0) {
      if (sincronizarNTP()) {
        ultimaSincronizacion24h = millis();
        Serial.println("✓ Sincronización NTP automática completada");
      }
    }
  }
  
 
}

/* =========================================================================================
   FIN DEL CÓDIGO V536
   ========================================================================================= */