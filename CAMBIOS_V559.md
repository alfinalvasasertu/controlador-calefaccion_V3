# CAMBIOS V559 - REESCRITURA COMPLETA DE LÓGICA DE CONTROL

**Fecha**: 2025-02-04
**Build**: V559
**Autor**: Reescritura completa según Tabla de la Verdad

---

## RESUMEN EJECUTIVO

V559 implementa una **reescritura COMPLETA** de la función `ejecutarLogicaControl()` para cumplir 100% con la Tabla de la Verdad (`3_TABLA_VERDAD_ARTEMIO.csv`).

### PROBLEMAS CRÍTICOS CORREGIDOS

1. ❌ **V558**: Jefatura OFF apagaba las bombas directamente
   - ✅ **V559**: Jefatura solo controla GT, NO las bombas

2. ❌ **V558**: Alarma GT apagaba las bombas directamente
   - ✅ **V559**: Alarma GT solo bloquea GT, NO las bombas

3. ❌ **V558**: Bombas seguían funcionando después de finalizar post-circulación
   - ✅ **V559**: Bombas QUEDAN PARADAS al finalizar post-circ

4. ❌ **V558**: Lógica confusa con múltiples returns y bloqueos
   - ✅ **V559**: Estructura clara con prioridades bien definidas

---

## REGLAS FUNDAMENTALES IMPLEMENTADAS

```cpp
// 1. POST-CIRCULACIÓN = INDEPENDIENTE
//    Durante POST: Bombas en alternancia, ignoran todo
//    Al FINALIZAR: Bombas QUEDAN PARADAS hasta nueva orden

// 2. JEFATURA controla GT, NO las bombas
//    JEF OFF → GT OFF (+ POST si estaba ON)
//    Bombas siguen su propia lógica

// 3. ALARMA GT bloquea GT, NO las bombas
//    AL_GT activa → GT bloqueado
//    Bombas siguen su propia lógica

// 4. Al finalizar POST-CIRC: Bombas QUEDAN PARADAS
//    Solo vuelven a funcionar si se cumplen condiciones normales:
//    - SISTEMA = 0V
//    - JEFATURA = 0V
//    - Sin alarmas RT1/RT2

// 5. Flancos (0V→3.3V) inician POST-CIRCULACIÓN
//    - Sistema OFF (flanco)
//    - Jefatura OFF (flanco)
//    - Alarma GT (flanco)
```

---

## ESTRUCTURA DE PRIORIDADES

```
PRIORIDAD 1: EMERGENCIA
├─ Todo apagado
└─ Cancelar POST si activa

PRIORIDAD 2: POST-CIRCULACIÓN ACTIVA
├─ Mantener bombas en ALTERNANCIA
├─ Ignorar todos los demás estados
└─ Al finalizar: Bombas QUEDAN PARADAS (3.3V)

PRIORIDAD 3: SISTEMA OFF
├─ Línea 9 tabla: Flanco OFF → Iniciar POST
└─ Línea 10 tabla: Estable OFF → Todo parado

PRIORIDAD 4: JEFATURA OFF
├─ Línea 7 tabla: Flanco OFF → GT OFF, Iniciar POST
├─ Línea 8 tabla: Estable OFF → GT OFF, Bombas paradas
└─ IMPORTANTE: Jefatura NO controla bombas directamente

PRIORIDAD 5: ALARMA GT
├─ Línea 15 tabla: Flanco alarma → GT OFF, Iniciar POST
├─ Línea 14 tabla: Estable alarma → GT OFF, Bombas paradas
└─ IMPORTANTE: Alarma GT NO controla bombas directamente

PRIORIDAD 6: LÓGICA NORMAL
├─ Gestión averías RT1/RT2 con alternancia
├─ Control GT según temperatura + programación
└─ Bomba condensación independiente
```

---

## CORRESPONDENCIA CON TABLA DE LA VERDAD

| Línea | Condición | GT | POST | Bombas | Implementado |
|-------|-----------|-----|------|--------|--------------|
| 1-4 | Normal | Variable | 3.3V | ALTERNANCIA | ✅ PRIORIDAD 6 |
| 5 | JEF flanco OFF | 3.3V | 0V | ALTERNANCIA | ✅ PRIORIDAD 4 |
| 6 | JEF estable OFF | 3.3V | 3.3V | 3.3V (OFF) | ✅ PRIORIDAD 4 |
| 7 | SIST flanco OFF | 3.3V | 0V | ALTERNANCIA | ✅ PRIORIDAD 3 |
| 8 | SIST estable OFF | 3.3V | 3.3V | 3.3V (OFF) | ✅ PRIORIDAD 3 |
| 9-12 | EMERGENCIA | 3.3V | 3.3V | 3.3V (OFF) | ✅ PRIORIDAD 1 |
| 13 | AL_GT estable | 3.3V | 3.3V | 3.3V (OFF) | ✅ PRIORIDAD 5 |
| 14 | AL_GT flanco | 3.3V | 0V | ALTERNANCIA | ✅ PRIORIDAD 5 |

---

## VARIABLES NUEVAS

```cpp
bool alarmaGT_anterior = false;  // Detectar flanco alarma GT (0V→3.3V)
```

---

## FUNCIONES MODIFICADAS

### `ejecutarLogicaControl()`
- **Antes**: 400 líneas, lógica confusa, múltiples returns
- **Ahora**: 220 líneas, estructura clara con 6 prioridades

### `actualizarPreviosEstados()`
- **Añadido**: `alarmaGT_anterior = alarmaGT;`

---

## COMPORTAMIENTO POST-CIRCULACIÓN

### Durante POST (0V en PIN47)
```cpp
if (post_circulacion_activa) {
    // FORZAR bombas ON, ignorar TODO lo demás
    if (bomba_post_circulacion == 1) {
        bomba1_ON = true;
        bomba2_ON = false;
    } else if (bomba_post_circulacion == 2) {
        bomba1_ON = false;
        bomba2_ON = true;
    }
    grupoTermico_ON = false;  // GT siempre OFF
    return;  // IGNORAR todo lo demás
}
```

### Al Finalizar POST (3.3V en PIN47)
```cpp
if (tiempo_transcurrido >= tiempo_total_ms) {
    Serial.println("✅ POST-CIRCULACIÓN FINALIZADA → Bombas QUEDAN PARADAS");
    bomba1_ON = false;
    bomba2_ON = false;
    grupoTermico_ON = false;
    cancelarPostCirculacion();
    return;  // Bombas PARADAS hasta nueva orden
}
```

---

## EJEMPLOS DE FUNCIONAMIENTO

### Caso 1: Jefatura OFF (Líneas 7-8)
```
Estado inicial: Sistema ON, Jefatura ON, GT funcionando, Bomba 1 activa

1. Usuario apaga Jefatura (PIN15: 0V→3.3V)
   → Flanco detectado

2. V559 ejecuta:
   - GT OFF (3.3V)
   - Iniciar POST-CIRCULACIÓN
   - Bomba 1 CONTINÚA funcionando (sin interrupción)

3. Durante 5 minutos (POST activo):
   - Bomba 1 sigue funcionando
   - GT permanece OFF
   - Ignora todos los demás estados

4. POST finaliza:
   - Bomba 1 se APAGA (3.3V)
   - Bomba 1 QUEDA PARADA
   
5. Usuario enciende Jefatura (PIN15: 3.3V→0V):
   - Si Sistema ON + sin alarmas → Bombas VUELVEN a funcionar
```

### Caso 2: Alarma GT activa (Líneas 14-15)
```
Estado inicial: Sistema ON, Jefatura ON, GT funcionando, Bomba 2 activa

1. Alarma GT se activa (PIN11: 0V→3.3V)
   → Flanco detectado

2. V559 ejecuta:
   - GT OFF (3.3V)
   - Iniciar POST-CIRCULACIÓN
   - Bomba 2 CONTINÚA funcionando

3. Durante POST (5 min):
   - Bomba 2 funcionando
   - GT bloqueado

4. POST finaliza:
   - Bomba 2 se APAGA
   - Bomba 2 QUEDA PARADA

5. Alarma GT se resuelve (PIN11: 3.3V→0V):
   - Si Sistema ON + Jefatura ON → Bombas VUELVEN a funcionar
   - GT puede volver a encenderse si se cumplen condiciones
```

---

## ELIMINACIONES DE V558

### Código ELIMINADO (causaba errores):

```cpp
// ❌ ELIMINADO: Jefatura apagando bombas directamente
if (!pin27_fisico && !post_circulacion_activa) {
    bomba1_ON = false;  // ← MAL: Jefatura no controla bombas
    bomba2_ON = false;
}

// ❌ ELIMINADO: Alarma GT apagando bombas directamente  
if (alarmaGT) {
    bomba1_ON = false;  // ← MAL: Alarma GT no controla bombas
    bomba2_ON = false;
    return;
}

// ❌ ELIMINADO: Bloqueos complejos tras post-circulación
if (bloqueo_postcirc_hasta_demanda) {
    // Lógica innecesaria, reemplazada por return directo
}
```

---

## VERIFICACIÓN DE COMPILACIÓN

```bash
✅ Sin errores de compilación
✅ Sin warnings
✅ Código listo para carga en ESP32-S3
```

---

## TESTING RECOMENDADO

### Test 1: Jefatura OFF
1. Encender sistema normalmente
2. Apagar Jefatura → Verificar POST inicia
3. Durante POST → Bomba sigue funcionando
4. POST finaliza → Bomba se apaga
5. Encender Jefatura → Bomba vuelve a funcionar

### Test 2: Sistema OFF
1. Encender sistema normalmente
2. Apagar Sistema → Verificar POST inicia
3. Durante POST → Bomba sigue funcionando
4. POST finaliza → Bomba se apaga
5. Encender Sistema → Bomba vuelve a funcionar

### Test 3: Alarma GT
1. Encender sistema normalmente con GT activo
2. Activar alarma GT → Verificar POST inicia
3. Durante POST → Bomba sigue, GT bloqueado
4. POST finaliza → Bomba se apaga
5. Resolver alarma → Bomba vuelve, GT puede encender

### Test 4: Emergencia durante POST
1. Iniciar POST (cualquier método)
2. Activar emergencia → Todo se apaga inmediatamente
3. POST cancelada

---

## NOTAS IMPORTANTES

⚠️ **CRÍTICO**: Esta versión implementa la lógica EXACTA de la Tabla de la Verdad.

⚠️ **IMPORTANTE**: No modificar la estructura de prioridades sin revisar tabla.

⚠️ **RECORDATORIO**: Las bombas SOLO funcionan si:
   - SISTEMA = 0V (ON)
   - JEFATURA = 0V (ON)
   - Sin alarmas RT1/RT2 (o alternancia automática si solo 1 averiada)

---

## PRÓXIMOS PASOS

1. ✅ Compilar V559 (COMPLETADO - Sin errores)
2. ⏳ Cargar en ESP32-S3
3. ⏳ Verificar funcionamiento según tests recomendados
4. ⏳ Validar con usuario en hardware real

---

**CONCLUSIÓN**: V559 es una reescritura COMPLETA y LIMPIA que sigue 100% la Tabla de la Verdad. Corrige todos los errores fundamentales de V558.
